#include "KMCDisplayWordAndTexture.h"
#include "KMCConfig.h"
#include "KMCPrismaUIBridge.h"
#include <nlohmann/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;

SINGLETONBODY(KMCCT::KMCDisplayWordAndTexture)
namespace KMCCT {
    using json = nlohmann::json;

	void KMCDisplayWordAndTexture::Init() {
 
        try {
            if (!Parse(COMMON_PATH + PLAYER_WORD_PATH + "/" + DISPLAY_WORD_AND_TEXTURE_PATH, KMCDisplayType::PLAYER)) {
                loaded = false;
            }
        } catch (std::runtime_error ex) {
            loaded = false;
            ERROR("ERROR LOADING {}", ex.what());
        }
	}

    bool KMCDisplayWordAndTexture::Parse(std::string path, KMCDisplayType disp_type, int follower_index) {

        std::ifstream stream(path);

        if (!stream.is_open()) {
            throw std::runtime_error("Failed open file.");
        }

        if (!json::accept(stream)) {
            throw std::runtime_error("Incorrect json format.");
        }

        stream.seekg(0, std::ios::beg);

        json j = json::parse(stream);

        std::string base_path = j.value("base_path", "");

        std::string full_base_path = PRISMA_UI_HTML_PATH + base_path;
        if (!fs::exists(full_base_path) || !fs::is_directory(full_base_path)) {
            ERROR("DisplayWordAndTexture.json, Base path not found: {}", full_base_path);
            return false;
        }

        if (!j.contains("entries") || !j["entries"].is_object()) {
            ERROR("DisplayWordAndTexture.json, define base_path and entries in the root field.");
            return false;
        }

        int type = static_cast<int>(disp_type);

        for (auto& [key, entry] : j["entries"].items()) {
            int id = std::stoi(key);

            std::string category = entry.value("category", "");

            // player(0) -> { 1:"Idle", 2:"Idle" }
            category_map[type][id] = category;

            // player(0) -> {"Idle":3}
            category_range_map[type][category]++;

            // player(0) -> {"Idle":[1,2,21]}
            category_index_map[type][category].push_back(id);

            auto range = entry["texture_range"];
            auto start = range.value("start", 1);
            auto end = range.value("end", 1);

            for (int i = start; i <= end; i++) {
                std::string file_path = full_base_path + key + "/" + std::to_string(i) + ".png";

                if (!fs::exists(file_path)) {
                    ERROR("Missing file: {}", file_path);
                    return false;  // 1‚Â‚Å‚à–³‚¯‚ê‚ÎNGAJS‘¤‚ÅƒGƒ‰[‚É‚È‚é
                }
            }
        }

        j["display_type"] = static_cast<int>(disp_type);
        j["follower_index"] = follower_index;
        SKSE::log::info("DisplayWordAndTexture.json Loaded ==> {}", path);
        KMCPrismaUIBridge::GetSingleton()->AddPath(j);

        return true;
    }
}