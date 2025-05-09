#include "KMCOAR.h"
#include "KMCConfig.h"
#include "KMCEventThread.h"

SINGLETONBODY(KMCCT::KMCOAR)

namespace KMCCT {
    void KMCOAR::Init() { 
        auto poar = KMCConfig::GetSingleton()->getIConnectOAR();
       
        for (auto [key, value] : *poar) {
            try {
                auto spvalue = KMCSplit(value, ',');

                RE::TESGlobal* global = nullptr;
                global = (RE::TESGlobal*)RE::TESDataHandler::GetSingleton()->LookupForm(
                    std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));

                if (global != nullptr) {
                    global->value = 0.0f;
                    OARComponents.push_back(
                        std::make_pair(std::stoi(key), OARCompDetail(global, std::stof(spvalue.at(2)))));

                } else {
                    ERROR("OARComps.json not found global value key = {}", key);
                }
            } catch (...) {
                ERROR("OARComps.json fatal key: {} value : {}", key, value);
            }
        }

        auto foar = KMCConfig::GetSingleton()->getFollowers();
        for (auto fins : *foar) {
            try {
                std::vector<std::pair<uint64_t, OARCompDetail>> oarc;

                for (auto [key, value] : fins.IConnectOAR) {
                    auto spvalue = KMCSplit(value, ',');

                    RE::TESGlobal* global = nullptr;
                    global = (RE::TESGlobal*)RE::TESDataHandler::GetSingleton()->LookupForm(
                        std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));

                    if (global != nullptr) {
                        global->value = 0.0f;
                        oarc.push_back(
                            std::make_pair(std::stoi(key), OARCompDetail(global, std::stof(spvalue.at(2)))));

                    } else {
                        ERROR("OARComps.json not found global value key = {}", key);
                    }
                }

                FOARComponents.push_back(std::make_pair(fins.index, FOARCompDetail(oarc)));
                fplays.push_back(FPlayNowOARCompDetail(OARCompDetail(), fins.index));
            } catch (...) {
                ERROR("follower {} OARComps.json fatal", fins.index);
            }
        }

        Reset();
    }

    void KMCOAR::Reset() { 
        now.global = nullptr;
        now.time = 0.0f;
        for (auto& [key, value] : OARComponents) {
            value.global->value = 0.0f;
        }

        for (auto& value : fplays) {
            value.OARComponents.global = nullptr;
        }

        for (auto& [key, value] : FOARComponents) {
            for (auto& [ikey, ivalue] : value.OARComponents) {
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

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        ocd->global->value = 0.0f;
    }

    void KMCOAR::PPushOARFunc(uint64_t rand, bool force, float ex_oar_time) {
        auto comps =
            std::find_if(OARComponents.begin(), OARComponents.end(), [rand](const auto& p) { return p.first == rand; });
        if (comps != OARComponents.end()) {
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
        auto it = std::find_if(FOARComponents.begin(), FOARComponents.end(),
                               [frand](const auto& p) { return p.first == frand; });
        if (it != FOARComponents.end()) {

            auto foar = it->second.OARComponents;
            auto it2 = std::find_if(foar.begin(), foar.end(), [rand](const auto& p) { return p.first == rand; });
            if (it2 != foar.end()) {
                auto comp = it2->second;
                if (comp.global == nullptr) {
                    return;
                }

                for (int i = 0; i < fplays.size(); i++) {
                    auto *fOldOarComp = &(fplays[i]);
                    if (fOldOarComp->index == frand) {
                        if (fOldOarComp->OARComponents.global != comp.global) {
                            if (fOldOarComp->OARComponents.global != nullptr) {
                                fOldOarComp->OARComponents.global->value = 0.0f;
                            }

                            fOldOarComp->OARComponents = comp;
                            if (force) {
                                fOldOarComp->OARComponents.time = ex_oar_time;
                            }
                            LaunchOAR(fOldOarComp->OARComponents);
                        } else if (fOldOarComp->OARComponents.global == comp.global &&
                                   fOldOarComp->OARComponents.global->value <= 0.5f) {
                            fOldOarComp->OARComponents = comp;
                            if (force) {
                                fOldOarComp->OARComponents.time = ex_oar_time;
                            }
                            LaunchOAR(fOldOarComp->OARComponents);
                        }
                    }
                }
            }
        }
    }
}