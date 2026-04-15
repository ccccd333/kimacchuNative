#include "KMCDisplayWordAndTexture.h"
#include "KMCConfig.h"
#include "KMCPrismaUIBridge.h"
#include "KMCCutin.h"
#include <nlohmann/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;

SINGLETONBODY(KMCCT::KMCDisplayWordAndTexture)
namespace KMCCT {
    using json = nlohmann::json;

	void KMCDisplayWordAndTexture::Init() {
 
        try {
            if (!Parse(COMMON_PATH + PLAYER_WORD_PATH + "/" + DISPLAY_WORD_AND_TEXTURE_PATH, 
                (int)KMCDisplayType::PLAYER,
                KMCConfig::GetSingleton()->GetPlayer()->GetName())) {
                loaded = false;
            }

            std::vector<KMCFollower> *followers = KMCConfig::GetSingleton()->GetFollowers();
            if (followers && !followers->empty()) {
                for (const auto &follower : *followers) {
                    if (!follower.follower) continue;
                    int follower_index = follower.index + 1;
                    if (!Parse(COMMON_PATH + FOLLOWER_WORD_PATH + std::to_string(follower_index) + "/" + DISPLAY_WORD_AND_TEXTURE_PATH,
                               follower_index, follower.follower->GetName())) {
                        loaded = false;
                    }
                }
            }
        } catch (std::runtime_error ex) {
            loaded = false;
            ERROR("ERROR LOADING {}", ex.what());
        }
	}

    bool KMCDisplayWordAndTexture::Parse(std::string path, int disp_type, std::string actor_name) {

        std::ifstream stream(path);

        if (!stream.is_open()) {
            throw std::runtime_error("Failed open file. Path ==> " + path);
        }

        if (!json::accept(stream)) {
            throw std::runtime_error("Incorrect json format. Path ==> " + path);
        }

        stream.seekg(0, std::ios::beg);

        json j = json::parse(stream);

        std::string base_path = j.value("base_path", "");

        std::string full_base_path = PRISMA_UI_HTML_PATH + base_path;
        if (!fs::exists(full_base_path) || !fs::is_directory(full_base_path)) {
            ERROR("DisplayWordAndTexture.json, Base path not found: {}", full_base_path);
            return false;
        }

        std::string bg_path = j.value("bg_path", "");
        std::string full_bg_path = PRISMA_UI_HTML_PATH + base_path;
        if (!fs::exists(full_bg_path)) {
            ERROR("DisplayWordAndTexture.json, Background path not found: {}", full_bg_path);
            return false;
        }

        if (!j.contains("entries") || !j["entries"].is_object()) {
            ERROR("DisplayWordAndTexture.json, define base_path and entries in the root field.");
            return false;
        }
        bool is_missing_file = false;
        int type = disp_type;

        for (auto& [key, entry] : j["entries"].items()) {
            int id = std::stoi(key);

            std::string category = entry.value("category", "");

            // player(0) -> { 1:"Idle", 2:"Idle" }
            category_map[type][id] = category;

            // player(0) -> {"Idle":3}
            category_range_map[type][category]++;

            // player(0) -> {"Idle":[1,2,21]}
            category_index_map[type][category].push_back(id);

            CutinEntry data;
            data.category = category;
            data.word = entry.value("word", "");
            data.is_full_screen = entry.value("is_full_screen", false);
            data.display_time = entry.value("display_time", 5.0f);
            
            if (entry.contains("texture_range") && entry["texture_range"].is_object()) {
                data.range_start = entry["texture_range"].value("start", 1);
                data.range_end = entry["texture_range"].value("end", 1);
            } else {
                data.range_start = 1;
                data.range_end = 1;
            }

            entries_data_map[type][id] = data;

            for (int i = data.range_start; i <= data.range_end; i++) {
                std::string file_path = full_base_path + key + "/" + std::to_string(i) + ".png";
                if (!fs::exists(file_path)) {
                    ERROR("Missing file: {}", file_path);
                    is_missing_file = true;
                }
            }
        }

        if (is_missing_file) {
            // 1つでもpngが無ければNG、JS側でエラーになる
            ERROR("[Error]Some image files could not be loaded. Therefore, the cut-in function will be disabled.");
            return false;
        }

        if (type == (int)KMCDisplayType::PLAYER) {
            KMCCutin::GetSingleton()->CategoryRandomizer();
            auto &first_values = KMCCutin::GetSingleton()->GetCategoryFirstValues();
            if (first_values.empty()) {
                ERROR("[Error]In DisplayWordAndTexture.json, there are no category definitions on the player side. Therefore, the cut-in function will be disabled.");
                return false;
            }
            j["first_values"] = first_values;
        } else {
            // player 基準のためフォロワー側はカテゴリにあるものでフィルタ
            const auto &first_values = KMCCutin::GetSingleton()->GetCategoryFirstValues();
            const auto &category_temp_map = category_map[type];
            std::vector<int> filtered;
            if (!category_temp_map.empty()) {
                for (auto first_input : first_values) {
                    if (category_temp_map.contains(first_input)) {
                        filtered.push_back(first_input);
                    }
                }
                j["first_values"] = filtered;
            } else {
                // カテゴリが空、設定できないようにする
                ERROR(
                    "[Error]Follower DisplayWordAndTexture.json, there are no category definitions on the player side. "
                    "Therefore, the cut-in function will be disabled.");
                return false;
            }

        }

        j["display_type"] = type;
        j["actor_name"] = actor_name;
        SKSE::log::info("DisplayWordAndTexture.json Loaded ==> {}", path);
        KMCPrismaUIBridge::GetSingleton()->KMCDefineCutin(j);

        return true;
    }
}