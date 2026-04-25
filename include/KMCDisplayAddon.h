#pragma once
namespace KMCCT {

    struct ExpressionData {
        std::vector<std::string> modifier;
        std::vector<std::string> phoneme;
        std::string expression;
        int time = 0;
    };

    struct SoundEffectData {
        std::string name;
        std::string ref;
        float timing = 0.0f;
        int emit_from = 0;
    };

    struct AddonData {
        ExpressionData expression;
        std::string voice_ref;
        std::vector<SoundEffectData> sound_effects;
        std::string oar_ref;
        float anim_duration = 0.0f;
    };

    struct ActorAddonSet {
        std::vector<std::pair<int, AddonData>> cutin_entries;
    };

    class KMCDisplayAddon {
        SINGLETONHEADER(KMCDisplayAddon)
    public:
        const std::string DISPLAY_ADD_ON_PATH = "DisplayAddons.json";

        void Setup();

        const ActorAddonSet* GetActorAddons(int actor_id) const;


    private:
        bool Parse(const std::string& path, int disp_type);

        std::map<int, ActorAddonSet> actor_addons;

        bool loaded = false;
    };
}