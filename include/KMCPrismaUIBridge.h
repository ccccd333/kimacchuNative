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

        void AddPath(const json &j);

        void KMCPlayPlayerCutin(int group);


    private:
    };

}