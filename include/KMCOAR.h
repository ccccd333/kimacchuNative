#pragma once
#include "KMCUtility.h"
#include "KMCCutinCondition.h"

namespace KMCCT {
    struct FOARCompDetail {
    public:
        FOARCompDetail() {}
        FOARCompDetail(std::vector<std::pair<uint64_t, OARCompDetail>> oar) { 
            oar_components = oar;
        }
    public:
        std::vector<std::pair<uint64_t, OARCompDetail>> oar_components;
    };

    struct FPlayNowOARCompDetail {
    public:
        FPlayNowOARCompDetail() { index = -1; }
        FPlayNowOARCompDetail(OARCompDetail oar, int ind) { 
            oar_components = oar; 
            index = ind;
        }

    public:
        OARCompDetail oar_components;
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
        std::vector<std::pair<uint64_t, OARCompDetail>> GetOARComponents() { return oar_components; }

    private:
        void PPushOARFunc(uint64_t rand, bool force, float ex_oar_time);
        void FPushOARFunc(uint64_t rand, uint64_t frand, bool force, float ex_oar_time);

    private:
        OARCompDetail now;
        std::vector<FPlayNowOARCompDetail> fplays;

        std::vector<std::pair<uint64_t, OARCompDetail>> oar_components;
        std::vector<std::pair<uint64_t, FOARCompDetail>> f_oar_components;
    };
}