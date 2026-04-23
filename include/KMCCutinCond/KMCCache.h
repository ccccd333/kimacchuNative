#pragma once

namespace KMCCT {

    enum class KMCCacheType { armo, none };

    class KMCCacheContainer {
    public:
        KMCCacheContainer() {}
        // Add
        // worn armor
        int Add(RE::FormID armo) { 
            int idx = c_index;
            cache_worn_armor[c_index] = armo;
            is_cacheable = true;
            c_index++;
            return idx;
        }

        
        void PreCache();
        void CacheReset();
        void RetryCache();
        void SetFailedCache(std::string name, std::function<bool()> func) {
            std::lock_guard<std::mutex> lock(faild_cache_mtx);
            if (!invalid_cache.contains(name)) {
                invalid_cache[name] = func;
            }
        }

        // Update Func

        // worn armor
        void OnEquipEvent(const RE::TESEquipEvent* event);

        // Get Func

        // worn armor
        bool PreInitWornArmo() {
            std::lock_guard<std::mutex> lock(worn_armo_mtx);
            return pre_init_worn_armo;
        }

        bool ResetEquipCacheOnFailure() { 
            return CacheEquipment();
        }

        std::unordered_map<int, bool>& GetWornArmorResult() { 
            std::lock_guard<std::mutex> lock(worn_armo_mtx);
            return cache_worn_armor_result;
        }

    public:
        bool is_cacheable = false;
        bool end_cache = false;

    private:
        // worn armor
        void PreCacheEquipment();

        // worn armor
        bool CacheEquipment();

    private:

        // faild cache
        std::mutex faild_cache_mtx;
        std::map<std::string, std::function<bool()>> invalid_cache;

        // worn armor
        const std::string CHACHE_FAILD_WORN_ARMO = "worn_armo";
        std::mutex worn_armo_mtx;
        std::unordered_map<int, RE::FormID> cache_worn_armor;
        std::unordered_map<int, bool> cache_worn_armor_result;
        bool pre_init_worn_armo = false;
        bool chec_pre_init_worn_armo = false;
        // cache index
        int c_index = 0;
    };
}