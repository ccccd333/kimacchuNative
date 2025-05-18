#pragma once
namespace KMCCT {
    class KMCCCMainCategory {
    public:
        static constexpr const char* NOTHING = "nothing";
        static constexpr const char* PLAYER = "player";
        static constexpr const char* LOCATION = "location";
        static constexpr const char* TEMP_KEYWORD = "temp_keyword";
        static constexpr const char* CROSS_HAIR = "cross_hair";
        static constexpr const char* FORMULA = "formula";
        
    };

    class KMCCCSubCategory {
    public:
        static constexpr const char* MOVE = "move";
        static constexpr const char* COMBAT = "combat";
        static constexpr const char* RUNNING = "running";

        static constexpr const char* SNEAKING = "sneaking";
        static constexpr const char* IDLE = "idle";

        static constexpr const char* KEYWORD = "keyword";
        static constexpr const char* MAGIC_EFFECT_KEYWORD = "magic_effect_keyword";

        static constexpr const char* KILL = "kill";

        static constexpr const char* BODY_SLOT = "body_slot";
    };
}