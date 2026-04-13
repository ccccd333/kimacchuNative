#pragma once
namespace KMCCT {

    // 表情データ
    struct ExpressionData {
        std::vector<std::string> modifier;
        std::vector<std::string> phoneme;
        std::string expression;
        int time = 0;
    };

    // 音声・SE・OAR等のアドオン情報
    struct AddonData {
        ExpressionData expression;
        std::string voice_ref;
        std::map<std::string, std::string> sound_effects;
        std::string oar_ref;
    };

    struct ActorAddonSet {
        std::vector<std::pair<int, AddonData>> cutin_entries;
    };

    class KMCDisplayAddon {
        SINGLETONHEADER(KMCDisplayAddon)
    public:
        ~KMCDisplayAddon() {}

        const std::string DISPLAY_ADD_ON_PATH = "DisplayAddons.json";

        void Setup();

        const ActorAddonSet* GetActorAddons(int actor_id) const;


    private:
        bool Parse(const std::string& path, int disp_type);

        std::map<int, ActorAddonSet> actor_addons;

        bool loaded = false;
    };
}