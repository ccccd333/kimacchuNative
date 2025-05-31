#pragma once
#include "KMCUtility.h"

namespace KMCCT {

    const std::string IN_SCENE_CONFIG_KEY = "scene_wait_time";
    const std::string ADD_ITEM_CUTIN_VALUE = "add_item_cutin_value";
    const std::string ISINSCENE_NPC_DETECT_RANGE_EXTERIOR = "isinscene_npc_detect_range_exterior";
    const std::string ISINSCENE_NPC_DETECT_RANGE_INTERIOR = "isinscene_npc_detect_range_interior";
    const std::string CUTIN_CHANCE_FHU = "cutin_chance_fhu";
    const std::string CUTIN_CHANCE_FASTTRAVEL = "cutin_chance_fasttravel";
    const std::string CUTIN_CHANCE_INJURY = "cutin_chance_injury";
    const std::string CUTIN_CHANCE_COMBAT = "cutin_chance_combat";
    const std::string CUTIN_CHANCE_SNEAK = "cutin_chance_sneak";
    const std::string CUTIN_CHANCE_MOUNT = "cutin_chance_mount";
    const std::string CUTIN_CHANCE_IDLE = "cutin_chance_idle";
    const std::string CUTIN_CHANCE_ISINDUNGEON = "cutin_chance_isindungeon";
    const std::string CUTIN_PRIORITY_FHU = "cutin_priority_fhu";
    const std::string CUTIN_PRIORITY_FASTTRAVEL = "cutin_priority_fasttravel";
    const std::string CUTIN_PRIORITY_INJURY = "cutin_priority_injury";
    const std::string CUTIN_PRIORITY_COMBAT = "cutin_priority_combat";
    const std::string CUTIN_PRIORITY_SNEAK = "cutin_priority_sneak";
    const std::string CUTIN_PRIORITY_MOUNT = "cutin_priority_mount";
    const std::string CUTIN_PRIORITY_IDLE = "cutin_priority_idle";
    const std::string CUTIN_PRIORITY_ISINDUNGEON = "cutin_priority_isindungeon";
    const std::string INTERRUPT_EVENT_ADDITEM = "interrupt_event_add_item";
    const std::string INTERRUPT_EVENT_ONHIT = "interrupt_event_on_hit";

    const std::string PROFILE_INVISI_COMBAT = "combat";
    const std::string PROFILE_INVISI_IDLE = "idle";
    const std::string PROFILE_INVISI_SCENE = "scene";
    const std::string PROFILE_INVISI_MOVE = "move";

    const std::string PAPYRUS_TYPE_MOD_NAME = "SP";

    struct State {
    public:
        State(RE::Actor* a, std::string cname, int th = 0, RE::BGSKeyword* k = nullptr, RE::TESFaction* f = nullptr,
              RE::TESGlobal* g = nullptr) { 
            keyword = k;
            faction = f;
            global = g;
            actor = a;
            cutinName = cname;
            threshold = th;
        }

        State(RE::Actor* a, std::string cname, RE::BGSKeyword* k, int th = 0) {
            keyword = k;
            faction = nullptr;
            global = nullptr;
            actor = a;
            cutinName = cname;
            threshold = th;
        }

        State(RE::Actor* a, std::string cname, RE::TESFaction* f, int th = 0) {
            keyword = nullptr;
            faction = f;
            global = nullptr;
            actor = a;
            cutinName = cname;
            threshold = th;
        }

        State(RE::Actor* a, std::string cname, RE::TESGlobal* g, int th = 0) {
            keyword = nullptr;
            faction = nullptr;
            global = g;
            actor = a;
            cutinName = cname;
            threshold = th;
        }

        State() {}
    public:
        RE::BGSKeyword* keyword;
        RE::TESFaction* faction;
        RE::TESGlobal* global;
        RE::Actor* actor;
        std::string cutinName = "";
        int threshold = 0;
        KMCTESGlobalParam gparam;
        KMCStrageUtilParam stuparam;
    };
    typedef std::string (*StateFunc)(int any, State st);
    struct StateControll{
    public:
        StateControll() {}

        StateControll(StateFunc f, State st) { 
            func = f;
            state = st;
        }

    public:
        StateFunc func;
        State state;

    };

    struct StrageUtilAccessMap {
    public:
        std::vector<std::string> access_keys;
        std::vector<KMCValueType> results_value_type;
        std::vector<MultiTypeValue> results;
    };

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
        ~KMCStateManager() {}

        void Register();
        std::string GetPlayerState();
        int IsInScene();
        bool GetProfileInvisibleState(bool suspend);
        bool GetStoppingState();
        bool GetWhetherThereNoState();
        
         
        void Init(); 
        void Reset();
        //void SetFHUStatus(float vag, float ana, float ora);
        std::vector<std::string> GetStrageUtilAccessKeys();
        void SetResultStrageUtil(std::vector<std::string> result);
        std::vector<MultiTypeValue> *GetStrageUtilResult() { return &strage_util_access_map.results; }
        std::vector<KMCValueType>* GetStrageUtilValueType() { return &strage_util_access_map.results_value_type; }
        //void AllowDialogue(RE::Actor* a_speaker, RE::TESTopic* a_topic);

        // event 
        void AddItemEvent(const RE::TESContainerChangedEvent* event);
        void OnHitEvent(const RE::TESHitEvent* event);
        void FastTravelEndEvent(const RE::TESFastTravelEndEvent* event);
        void MenuOpenCloseEvent(const RE::MenuOpenCloseEvent* event);
    private:
        std::string GetHighestPriorityPlayer();
        void InitDetectionItems(std::vector<std::pair<std::string, std::string>>* dconfig, RE::Actor* actor,
                                StateFunc f, KMCDetectionType type, std::vector<std::string> messages);
        void InitDetectionStrageUtil(std::vector<std::pair<std::string, std::string>>* dconfig, RE::Actor* actor);
    private:
        
        std::vector<RE::BGSKeyword*> keywordsExcludeDuringScene;
        RE::BGSKeyword* actorNPC;
        StrageUtilAccessMap strage_util_access_map;
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
