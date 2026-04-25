#include "KMCDisplayAddon.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "KMCConfig.h"
#include "KMCDisplayWordAndTexture.h"
#include "KMCUtility.h"

SINGLETONBODY(KMCCT::KMCDisplayAddon)

namespace KMCCT {

    using json = nlohmann::json;

    void KMCDisplayAddon::Setup() {
        try {
            if (!Parse(COMMON_PATH + PLAYER_WORD_PATH + "/" + DISPLAY_ADD_ON_PATH, (int)KMCDisplayType::PLAYER)) {
                loaded = false;
            }

            std::vector<KMCFollower>* followers = KMCConfig::GetSingleton()->GetFollowers();
            if (followers && !followers->empty()) {
                for (const auto& follower : *followers) {
                    auto actorPtr = follower.followerHandle.get();
                    if (!actorPtr) continue;
                    int follower_index = follower.index + 1;
                    if (!Parse(COMMON_PATH + FOLLOWER_WORD_PATH + std::to_string(follower_index) + "/" +
                                   DISPLAY_ADD_ON_PATH,
                               follower_index)) {
                        loaded = false;
                    }
                }
            }
        } catch (std::runtime_error ex) {
            KMC_ERROR("ERROR LOADING {}", ex.what());
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

        ActorAddonSet new_set;

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

                if (value.contains("sound_effect") && value["sound_effect"].is_array()) {
                    for (auto& se_item : value["sound_effect"]) {
                        SoundEffectData se_data;
                        se_data.name = se_item.value("name", "");
                        se_data.ref = se_item.value("ref", "");
                        se_data.timing = se_item.value("timing", 0.0f);
                        se_data.emit_from = se_item.value("emit_from", 0);

                        data.sound_effects.push_back(se_data);
                    }
                }

                if (value.contains("oar")) {
                    auto& oar = value["oar"];
                    data.oar_ref = oar.value("ref", "");
                    data.anim_duration = oar.value("anim_duration", 0.0f);
                }

                new_set.cutin_entries.push_back({cutin_no, data});

            } catch (...) {
                KMC_ERROR("ERROR [KMCDisplayAddon::Parse] Entry no {}", cutin_no_str);
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