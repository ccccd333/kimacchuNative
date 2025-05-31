#include "KMCCutinCond/KMCCache.h"
#include "KMCGameEventListener.h"
#include "KMCConfig.h"

namespace KMCCT {
    void KMCCacheContainer::PreCache() { 
        PreCacheEquipment();
    }

    void KMCCacheContainer::CacheReset() { 
        CacheEquipment();
    }

    void KMCCacheContainer::OnEquipEvent(const RE::TESEquipEvent *event) {
        if (event && event->actor.get()) {
            auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
            if (player && event->actor.get()->GetFormID() == player->GetFormID()) {
                for (auto &[index, armo_formid] : cache_worn_armor) {
                    if (armo_formid == event->baseObject) {
                        std::lock_guard<std::mutex> lock(worn_armo_mtx);
                        cache_worn_armor_result[index] = event->equipped;
                    }                    
                }
            }
        }
    }

    void KMCCacheContainer::PreCacheEquipment() {
        for (auto &[index, armo_formid] : cache_worn_armor) {
            cache_worn_armor_result[index] = false;
        }

        KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
            [this](const RE::TESEquipEvent *event) { this->OnEquipEvent(event); });
    }

    void KMCCacheContainer::CacheEquipment() {
        {
            std::lock_guard<std::mutex> lock(worn_armo_mtx);
            for (auto &[index, armo_formid] : cache_worn_armor) {
                cache_worn_armor_result[index] = false;
            }
        }

        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();

        if (cache_worn_armor.size() > 0) {
            auto inv = player->GetInventory();

            for (auto &[item, data] : inv) {
                const auto &[count, entry] = data;
                if (count > 0 && entry->IsWorn()) {
                    for (auto &[index, armo_formid] : cache_worn_armor) {
                        const auto armor = entry->GetObject()->As<RE::TESObjectARMO>();
                        if (armor->GetFormID() == armo_formid) {
                            std::lock_guard<std::mutex> lock(worn_armo_mtx);
                            cache_worn_armor_result[index] = true;
                        }
                    }
                }
            }
        }
    }
}