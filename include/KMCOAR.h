#pragma once
#include "KMCUtility.h"
#include "KMCCutinCondition.h"

namespace KMCCT {
    struct FOARCompDetail {
    public:
        FOARCompDetail() {}
        FOARCompDetail(std::vector<std::pair<uint64_t, OARCompDetail>> oar) { 
            OARComponents = oar;
        }
    public:
        std::vector<std::pair<uint64_t, OARCompDetail>> OARComponents;
    };

    struct FPlayNowOARCompDetail {
    public:
        FPlayNowOARCompDetail() { index = -1; }
        FPlayNowOARCompDetail(OARCompDetail oar, int ind) { 
            OARComponents = oar; 
            index = ind;
        }

    public:
        OARCompDetail OARComponents;
        int index;
    };

    class KMCOAR {
        SINGLETONHEADER(KMCOAR)
    public:
        ~KMCOAR();
        void Init();
        void Reset();
        void PushOARFunc(uint64_t rand, uint64_t frand, bool force, float ex_oar_time);
        void TryKMCOAR(OARCompDetail *ocd);
        std::vector<std::pair<uint64_t, OARCompDetail>> GetOARComponents() { return OARComponents; }

    private:
        void PPushOARFunc(uint64_t rand, bool force, float ex_oar_time);
        void FPushOARFunc(uint64_t rand, uint64_t frand, bool force, float ex_oar_time);

    private:
        OARCompDetail now;
        std::vector<FPlayNowOARCompDetail> fplays;

        std::vector<std::pair<uint64_t, OARCompDetail>> OARComponents;
        std::vector<std::pair<uint64_t, FOARCompDetail>> FOARComponents;
    };
}