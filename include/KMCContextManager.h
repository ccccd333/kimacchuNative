#pragma once
#include "KMCUtility.h"

namespace KMCCT {

    const std::string INTERRUPT_EVENT_ONHIT = "interrupt_event_on_hit";

    namespace StateNames {
        inline constexpr std::string_view FTEndFar = "FTEndFar";
        inline constexpr std::string_view Injury = "Injury";
        inline constexpr std::string_view Combat = "Combat";
        inline constexpr std::string_view Sneak = "Sneak";
        inline constexpr std::string_view Mount = "Mount";
        inline constexpr std::string_view Dungeon = "InDungeon";
        inline constexpr std::string_view Idle = "Idle";
        inline constexpr std::string_view FHU = "FHU";
    }

    namespace ExtendedContextNames {
        inline constexpr std::string_view Global = "global";
        inline constexpr std::string_view DetectionMagicEffectKeyword = "detection_magic_effect_keyword";
        inline constexpr std::string_view Faction = "faction";
        inline constexpr std::string_view Keyword = "keyword";
    }

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
    struct StateControll {
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

    struct ActionStateEntry {
        std::string name;
        int priority;
        int chance;
    };

    class KMCContextManager {
        SINGLETONHEADER(KMCContextManager)
    public:
        const std::string CUTIN_CONTEXT_PATH = "CutinContext.json";

        void Setup();
        void Init();

        std::string GetPlayerState();

        // event
        void OnHitEvent(const RE::TESHitEvent* event);
        void FastTravelEndEvent(const RE::TESFastTravelEndEvent* event);

        bool IsReady() const { return loaded; }

    private:
        std::string GetHighestPriorityPlayer();
        void InitDetectionItems(std::vector<std::pair<int, std::string>>* dconfig, RE::Actor* actor,
                                StateFunc f, KMCDetectionType type, std::vector<std::string> messages);

        bool Parse(std::string path);

        std::map<int, StateControll> states;
        bool loaded = true;

        std::vector<ActionStateEntry> action_states;
        std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> extended_context_data;
    };
}