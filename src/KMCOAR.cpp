#include "KMCOAR.h"
#include "KMCConfig.h"
#include "KMCEventThread.h"
#include "KMCDisplayAddon.h"
#include "KMCDisplayWordAndTexture.h"

SINGLETONBODY(KMCCT::KMCOAR)

namespace KMCCT {
    void KMCOAR::Init() { 
        const auto player_addon_set = KMCDisplayAddon::GetSingleton()->GetActorAddons((int)KMCDisplayType::PLAYER);
        if (player_addon_set) {
            for (const auto& [key, value] : player_addon_set->cutin_entries) {
                try {
                    auto spvalue = KMCSplit(value.oar_ref, ',');

                    if (spvalue.size() == 2) {
                        RE::TESGlobal* global = nullptr;
                        global = (RE::TESGlobal*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));

                        if (global != nullptr) {
                            global->value = 0.0f;
                            oar_components.push_back(std::make_pair(key, OARCompDetail(global, value.anim_duration)));

                        } else {
                            KMC_ERROR("[DisplayAddons.json] [OAR] [Player] GlobalForm not found. Key: {}", key);
                        }
                    } else {
                        KMC_ERROR(
                            "[DisplayAddons.json] [OAR] [Player] Invalid oar_ref format (expected FormID,Plugin). Key: "
                            "{}, Value: {}",
                            key, value.oar_ref);
                    }
                } catch (...) {
                    KMC_ERROR("[DisplayAddons.json] [OAR] [Player] Unknown fatal error.  Key: {}", key);
                }
            }
        }

        auto foar = KMCConfig::GetSingleton()->GetFollowers();
        for (const auto& fins : *foar) {
            try {
                int f_index = fins.index + 1;
                const auto follower_addon_set = KMCDisplayAddon::GetSingleton()->GetActorAddons(f_index);
                if (!follower_addon_set) continue;

                std::vector<std::pair<uint64_t, OARCompDetail>> oarc;

                for (const auto &[key, value] : follower_addon_set->cutin_entries) {
                    auto spvalue = KMCSplit(value.oar_ref, ',');

                    if (spvalue.size() == 2) {
                        RE::TESGlobal* global = nullptr;
                        global = (RE::TESGlobal*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));

                        if (global != nullptr) {
                            global->value = 0.0f;
                            oarc.push_back(std::make_pair(key, OARCompDetail(global, value.anim_duration)));

                        } else {
                            KMC_ERROR("[DisplayAddons.json] [OAR] [Follower{}] GlobalForm not found. Key: {}", f_index,
                                  key);
                        }
                    } else {
                        KMC_ERROR("[DisplayAddons.json] [OAR] [Follower{}] Invalid oar_ref format. Key: {}, Value: {}", fins.index, key,
                              value.oar_ref);
                    }
                }

                f_oar_components.push_back(std::make_pair(fins.index, FOARCompDetail(oarc)));
                fplays.push_back(FPlayNowOARCompDetail(OARCompDetail(), fins.index));
            } catch (...) {
                KMC_ERROR("[DisplayAddons.json] [OAR] [Follower{}] Unknown fatal error.", fins.index);
            }
        }

        Reset();
    }

    void KMCOAR::Reset() { 
        now.global = nullptr;
        now.time = 0.0f;
        for (auto& [key, value] : oar_components) {
            value.global->value = 0.0f;
        }

        for (auto& value : fplays) {
            value.oar_components.global = nullptr;
        }

        for (auto& [key, value] : f_oar_components) {
            for (auto& [ikey, ivalue] : value.oar_components) {
                ivalue.global->value = 0.0f;
            }
        }
    }

    void KMCOAR::PushOARFunc(uint64_t rand, uint64_t frand, bool force, float ex_oar_time) {
        if (frand == -1) {
            PPushOARFunc(rand, force, ex_oar_time);
        } else {
            FPushOARFunc(rand, frand, force, ex_oar_time);
        }
    }

    void KMCOAR::TryKMCOAR(OARCompDetail* ocd) {
        ocd->global->value = 1.0f;

        KMCCT::KMCTimer(ocd->time * KMCCT::WHILE_WAIT_TIME);

        auto* thread = KMCCT::KMCEventThread::GetSingleton();
        if (thread->GetForceEndAnim() || thread->IsShuttingDown()) {
            return;
        }

        ocd->global->value = 0.0f;
    }

    void KMCOAR::PPushOARFunc(uint64_t rand, bool force, float ex_oar_time) {
        auto comps =
            std::find_if(oar_components.begin(), oar_components.end(), [rand](const auto& p) { return p.first == rand; });
        if (comps != oar_components.end()) {
            if (comps->second.global == nullptr) {
                return;
            }

            if (now.global != comps->second.global) {
                if (now.global != nullptr) {
                    now.global->value = 0.0f;
                }
                now = comps->second;

                if (force) now.time = ex_oar_time;
                LaunchOAR(now);
            } else if (now.global == comps->second.global && now.global->value <= 0.5f) {
                now = comps->second;
                if (force) now.time = ex_oar_time;
                LaunchOAR(now);
            }
        }
    }

    void KMCOAR::FPushOARFunc(uint64_t rand, uint64_t frand, bool force, float ex_oar_time) {
        auto it = std::find_if(f_oar_components.begin(), f_oar_components.end(),
                               [frand](const auto& p) { return p.first == frand; });
        if (it != f_oar_components.end()) {

            auto foar = it->second.oar_components;
            auto it2 = std::find_if(foar.begin(), foar.end(), [rand](const auto& p) { return p.first == rand; });
            if (it2 != foar.end()) {
                auto comp = it2->second;
                if (comp.global == nullptr) {
                    return;
                }

                for (int i = 0; i < fplays.size(); i++) {
                    auto *fOldOarComp = &(fplays[i]);
                    if (fOldOarComp->index == frand) {
                        if (fOldOarComp->oar_components.global != comp.global) {
                            if (fOldOarComp->oar_components.global != nullptr) {
                                fOldOarComp->oar_components.global->value = 0.0f;
                            }

                            fOldOarComp->oar_components = comp;
                            if (force) {
                                fOldOarComp->oar_components.time = ex_oar_time;
                            }
                            LaunchOAR(fOldOarComp->oar_components);
                        } else if (fOldOarComp->oar_components.global == comp.global &&
                                   fOldOarComp->oar_components.global->value <= 0.5f) {
                            fOldOarComp->oar_components = comp;
                            if (force) {
                                fOldOarComp->oar_components.time = ex_oar_time;
                            }
                            LaunchOAR(fOldOarComp->oar_components);
                        }
                    }
                }
            }
        }
    }
}