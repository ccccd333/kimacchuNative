#include "KMCDisplayAddon.h"
#include "KMCUtility.h"
#include "KMCConfig.h"
#include "KMCDisplayWordAndTexture.h"

#include <fstream>
#include <nlohmann/json.hpp>

SINGLETONBODY(KMCCT::KMCDisplayAddon)

namespace KMCCT {

    using json = nlohmann::json;

	void KMCDisplayAddon::Setup() {
        try {
            if (!Parse(COMMON_PATH + PLAYER_WORD_PATH + "/" + DISPLAY_ADD_ON_PATH,
                       (int)KMCDisplayType::PLAYER)) {
                loaded = false;
            }

            std::vector<KMCFollower>* followers = KMCConfig::GetSingleton()->GetFollowers();
            if (followers && !followers->empty()) {
                for (const auto& follower : *followers) {
                    if (!follower.follower) continue;
                    int follower_index = follower.index + 1;
                    if (!Parse(COMMON_PATH + FOLLOWER_WORD_PATH + std::to_string(follower_index) + "/" +
                                   DISPLAY_ADD_ON_PATH, follower_index)) {
                        loaded = false;
                    }
                }
            }
        } catch (std::runtime_error ex) {
            ERROR("ERROR LOADING {}", ex.what());
            loaded = false;
        }
    }

    bool KMCDisplayAddon::Parse(const std::string& path, int disp_type) {
        std::ifstream stream(path);

        if (!stream.is_open()) {
            throw std::runtime_error("Failed open file. Path ==> " + path);
        }

        if (!json::accept(stream)) {
            throw std::runtime_error("Incorrect json format. Path ==> " + path);
        }

        stream.seekg(0, std::ios::beg);

        json j = json::parse(stream);

        actor_addons.clear();

        // 指定アクターの既存データをクリアして新規作成
        ActorAddonSet new_set;

        // JSONのトップレベルにある "1", "2"... などのカットイン番号をループ（記述順）
        for (auto& [cutin_no_str, value] : j.items()) {
            try {
                int cutin_no = std::stoi(cutin_no_str);
                AddonData data;

                if (value.contains("expression")) {
                    auto& exp = value["expression"];
                    data.expression.modifier = exp.value("modifier", std::vector<std::string>{});
                    data.expression.phoneme = exp.value("phoneme", std::vector<std::string>{});
                    data.expression.expression = exp.value("expression", "");
                    data.expression.time = exp.value("time", 0);
                }

                if (value.contains("voice_sound")) {
                    data.voice_ref = value["voice_sound"].value("ref", "");
                }

                if (value.contains("sound_effect")) {
                    for (auto& [se_name, se_val] : value["sound_effect"].items()) {
                        data.sound_effects[se_name] = se_val.get<std::string>();
                    }
                }

                if (value.contains("oar")) {
                    data.oar_ref = value["oar"].value("ref", "");
                }

                new_set.cutin_entries.push_back({cutin_no, data});

            } catch (...) {
                continue;
            }
        }

        actor_addons[disp_type] = std::move(new_set);
        return true;
    }

    const ActorAddonSet* KMCDisplayAddon::GetActorAddons(int actor_id) const {
        auto it = actor_addons.find(actor_id);
        if (it != actor_addons.end()) {
            return &(it->second);
        }
        return nullptr;
    }
}