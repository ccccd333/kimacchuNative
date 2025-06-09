#include "KMCCutinCond/KMCCache.h"
#include "KMCGameEventListener.h"
#include "KMCConfig.h"
#include "IWWFunctions.h"

namespace KMCCT {
    void KMCCacheContainer::PreCache() { 
        PreCacheEquipment();
    }

    void KMCCacheContainer::CacheReset() { 
        end_cache = false;

        // worn armo
        pre_init_worn_armo = false;
        chec_pre_init_worn_armo = false;
        CacheEquipment();
    }

    void KMCCacheContainer::RetryCache() {
        if (end_cache) return;

        std::map<std::string, std::function<bool()>> copy_c;

        {
            std::lock_guard<std::mutex> lock(faild_cache_mtx);
            if (invalid_cache.empty()) {
                end_cache = true;
                return;
            }

            copy_c = invalid_cache;
        }

        std::vector<std::string> erase_n;

        for (auto &[cache_key, cache_value] : copy_c) {
            bool result = cache_value();
            if (result) {
                erase_n.push_back(cache_key);
            }
        }

        { 
            std::lock_guard<std::mutex> lock(faild_cache_mtx); 
            for (auto &cache_key : erase_n) {
                if (invalid_cache.contains(cache_key)) {
                    invalid_cache.erase(cache_key);
                }
            }
        }
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

    bool KMCCacheContainer::CacheEquipment() {
        bool end_status = false;

        {
            std::lock_guard<std::mutex> lock(worn_armo_mtx);
            end_status = pre_init_worn_armo;
            if (chec_pre_init_worn_armo) return end_status;
            chec_pre_init_worn_armo = true;

            
            for (auto &[index, armo_formid] : cache_worn_armor) {
                cache_worn_armor_result[index] = false;
            }
        }

        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            if (cache_worn_armor.size() > 0) {
                auto inv = player->GetInventory();
                for (auto &[item, data] : inv) {
                    const auto &[count, entry] = data;
                    if (count > 0 && entry->IsWorn()) {
                        for (auto &[index, armo_formid] : cache_worn_armor) {
                            const auto armor = entry->GetObject()->As<RE::TESObjectARMO>();
                            if (armor && armor->GetFormID() == armo_formid) {
                                std::lock_guard<std::mutex> lock(worn_armo_mtx);
                                cache_worn_armor_result[index] = true;
                            }
                        }
                    }
                }
            }

            {
                std::lock_guard<std::mutex> lock(worn_armo_mtx);
                pre_init_worn_armo = true;
            }

            return true;
        } else {
            {
                std::lock_guard<std::mutex> lock(worn_armo_mtx);
                chec_pre_init_worn_armo = false;
            }
            SetFailedCache(CHACHE_FAILD_WORN_ARMO, [this]() { return this->ResetEquipCacheOnFailure(); });
        }

        return false;
    }
}