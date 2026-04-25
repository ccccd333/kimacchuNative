#pragma once
#include "KMCUtility.h"

namespace KMCCT {
    const std::string EXPRESSION_FILE_NAME = "Expression.json";

    enum class MFG_TYPE { modifier, phoneme, expression
    };

    struct STMFG{
    public:
        STMFG() {}

        std::map<uint64_t, STMFGPair> mfg;
    };

    class KMCExpression {
        SINGLETONHEADER(KMCExpression)
    public:
        const int EXP_NOT_END_LOOP = 100;
        const int EXP_WAIT_LOOP = 250;
        const int EXP_TIMER_LOOP = 500;

        ~KMCExpression();
        void Setup();
        void Init();
        void Reset() { 
            papyrus_end_exp = false;
            exp_loop_now = false;
            f_papyrus_end_exp = false;
            f_exp_loop_now = false;

            force_exp_loop = false;
            force_cool_time = 0.0f;
            force_exp_time = 0.0f;

            is_wait = false;
            is_cutin = false;
        }

        int OnStandby();
        void OnFinished();
        void PushExpFunc(int rand, int frand, bool force, float ex_exp_time);
        void TryKMCExp(STMFGPair *mfg);
        void TryKMCFLExp(STMFGPair *mfg);

        void SetEndPapurusExp();
        void SetFLEndPapurusExp();

        int ForceExp(std::string exp_id, float cool_time, float exp_time);
        int GetCutInID(std::string aaaakmctype);
    private:
        void PPushExpFunc(int rand, bool force, float ex_exp_time);
        void FPushExpFunc(int rand, int frand, bool force, float ex_exp_time);
        bool GetWaitFlag();
        int KMCEXPTimer(long long limit);

        void CategoryRandomizer();
    private:

        STMFGPair p_now;
        STMFG player_mfg;
        bool papyrus_end_exp = false;
        bool exp_loop_now = false;

        std::map<uint64_t, STMFG> follower_mfg;
        STMFGPair f_now;
        bool f_papyrus_end_exp = false;
        bool f_exp_loop_now = false;
        //std::map<uint64_t, FExpCompDetail> FOARComponents;

        RE::TESForm *form;
        std::mutex mtx;
        std::mutex f_mtx;
        std::mutex wit_mtx;

        // force exp
        float force_cool_time = 0.0f;
        float force_exp_time = 0.0f;
        
        bool force_exp_loop = false;

        bool is_wait = false;
        bool is_cutin = false;

        std::unordered_map<std::string, KMCRandomData> kmc_category_rand_map;
        std::vector<int> kmc_category_first_values;
    };
}