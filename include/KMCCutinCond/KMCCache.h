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
            isCacheable = true;
            c_index++;
            return idx;
        }

        
        void PreCache();
        void CacheReset();

        // Update Func

        // worn armor
        void OnEquipEvent(const RE::TESEquipEvent* event);

        // Get Func

        // worn armor
        std::unordered_map<int, bool>& GetWornArmorResult() { 
            std::lock_guard<std::mutex> lock(worn_armo_mtx);
            return cache_worn_armor_result;
        }

    public:
        bool isCacheable = false;

    private:
        // worn armor
        void PreCacheEquipment();

        // worn armor
        void CacheEquipment();

    private:

        // worn armor
        std::mutex worn_armo_mtx;
        std::unordered_map<int, RE::FormID> cache_worn_armor;
        std::unordered_map<int, bool> cache_worn_armor_result;

        // cache index
        int c_index = 0;
    };
}