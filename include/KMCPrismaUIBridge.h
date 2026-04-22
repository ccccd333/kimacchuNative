#pragma once
#include "PrismaUI_API.h"
#include <nlohmann/json.hpp>

namespace KMCCT {
    using json = nlohmann::json;


	class KMCPrismaUIBridge {
        SINGLETONHEADER(KMCPrismaUIBridge)
    public:
        void Init();

        void Focus();

        void UnFocus();

        // cutin
        void KMCDefineCutin(const json &j);

        void KMCBatchPreloadGroups(int id, int next_group, int f_id, int f_next_group);

        void KMCPlayPlayerCutin(int group, int next_group);

        void KMCPlayFollowerCutin(int id, int group, int next_group);

        // stop icon
        void KMCShowStopIcon();

        void KMCHideStopIcon();

        void KMCStopAndHideCutinAndIcon();

        // profile
        void KMCSetupProfile(const json &j);

        void KMCShowProfile();

        void KMCHideProfile();

        void KMCUpdateProfileText(const json &j);

    private:
    };

}