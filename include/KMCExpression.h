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

        ~KMCExpression();
        void Setup();
        void Init();
        void Reset() { 
            papyrus_end_exp = false;
            exp_loop_now = false;
            f_papyrus_end_exp = false;
            f_exp_loop_now = false;
        }

        void PushExpFunc(uint64_t rand, uint64_t frand, bool force, float ex_exp_time);
        void TryKMCExp(STMFGPair *mfg);
        void TryKMCFLExp(STMFGPair *mfg);

        void SetEndPapurusExp();
        void SetFLEndPapurusExp();
    private:
        void PPushExpFunc(uint64_t rand, bool force, float ex_exp_time);
        void FPushExpFunc(uint64_t rand, uint64_t frand, bool force, float ex_exp_time);

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
    };
}