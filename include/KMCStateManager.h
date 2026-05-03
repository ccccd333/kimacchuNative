#pragma once
#include "KMCUtility.h"

namespace KMCCT {

    const std::string IN_SCENE_CONFIG_KEY = "scene_wait_time";
    const std::string ISINSCENE_NPC_DETECT_RANGE_EXTERIOR = "isinscene_npc_detect_range_exterior";
    const std::string ISINSCENE_NPC_DETECT_RANGE_INTERIOR = "isinscene_npc_detect_range_interior";
    

    const std::string PROFILE_INVISI_COMBAT = "combat";
    const std::string PROFILE_INVISI_IDLE = "idle";
    const std::string PROFILE_INVISI_SCENE = "scene";
    const std::string PROFILE_INVISI_MOVE = "move";

    const std::string PAPYRUS_TYPE_MOD_NAME = "SP";



    struct ProfileInvisibleStat {
    public:
        ProfileInvisibleStat() {}
        ProfileInvisibleStat(bool com, bool sc, bool mo) { 
            combat = com;
            scene = sc;
            move = mo;
        }
    public:
        bool combat = false;
        bool scene = false;
        bool move = false;
    };

    class KMCStateManager {
        SINGLETONHEADER(KMCStateManager)
    public:
        void Register();
        
        int IsInScene();
        bool GetProfileInvisibleState(bool suspend);
        bool GetStoppingState();
        bool GetWhetherThereNoState();

        void AddSceneExclusionKeyword(RE::BGSKeyword* a_keyword) {
            if (a_keyword) {
                scene_exclusion_keywords.push_back(a_keyword);
            }
        }
        
         
        void Init(); 
        void Reset();

        // event 
        void MenuOpenCloseEvent(const RE::MenuOpenCloseEvent* event);

    private:
        
        std::vector<RE::BGSKeyword*> scene_exclusion_keywords;
        RE::BGSKeyword* actorNPC;
        ProfileInvisibleStat profile_invisible_status;
        std::set<std::string> stopping_state = {
            "BarterMenu",
            "Book Menu",
            "Book Menu",
            "Console",
            "Crafting Menu",
            "Dialogue Menu",
            "GiftMenu",
            "InventoryMenu",
            "Journal Menu",
            "LevelUp Menu",
            "Lockpicking Menu"
            "MagicMenu",
            "MapMenu",
            "RaceSex Menu",
            "Sleep/Wait Menu",
            "StatsMenu",
            "TweenMenu",
        };
        std::set<std::string> current_state;
    };
}
