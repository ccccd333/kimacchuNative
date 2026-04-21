#pragma once
#include "KMCUtility.h"

namespace KMCCT {
    const long long READY_TIMEOUT = 30000;

    const long long SE_PROGRESS_ADDTION_MS = 100;
    const long long SE_TIMER_INTERVAL_MS = SE_PROGRESS_ADDTION_MS;

    struct KMCCutinValues {
    public:
        KMCCutinValues() {}
        KMCCutinValues(std::string type, float time, float animtime, float vol, float oar_time, bool overri_oar_flag) {
            aaaakmctype = type;
            aaaakmctime = time;
            aaaakmcAnimtime = animtime;
            aaaakmcvolum = vol;
            aaaakmcoar = oar_time;
            overri_oar_time = overri_oar_flag;
        }
        KMCCutinValues(std::string type, float time, float animtime, float vol) {
            aaaakmctype = type;
            aaaakmctime = time;
            aaaakmcAnimtime = animtime;
            aaaakmcvolum = vol;
            overri_oar_time = false;
        }

    public:
        std::string aaaakmctype = "";
        std::string aaaakmcExptype = "";
        float aaaakmctime = 1.0f;
        float aaaakmcAnimtime = 1.0f;
        float aaaakmcvolum = 1.0f;
        float aaaakmcoar = 1.0f;
        float aaaakmcexp = 1.0f;
        bool overri_oar_time = false;
        bool overri_exp_time = false;
        bool overri_fc_exp = false;
    };

    void KMCPlayAnim(KMCAnimST *st, int &playerorfollower);
    void KMCPlay(KMCAnimST *st, int &playerorfollower);
    void KMCOnCutinStartReady(KMCAnimST *st, int &playerorfollower);
    void KMCOnCutinEnd(KMCAnimST *st, int &playerorfollower);
    void KMCOARFuncStart(KMCAnimST *st, int &playerorfollower);
    void KMCExpFuncStart(KMCAnimST *st, int &playerorfollower);
    void KMCBatchPreloadGroups(KMCAnimST *st, int &playerorfollower);

    class KMCCutin {
        SINGLETONHEADER(KMCCutin)
    public:
        ~KMCCutin();
        void InitCutin(std::string skyroot, std::vector<float> *floatArray);

        void InterruptCutInEventManager(KMCInterruptPushCutInData data);

        int CondCutIn(KMCCutinValues val);
        int IterCutIn(KMCCutinValues val);

        bool GetAnimNow();
        void SetAnimNow(bool set);

        bool GetCutinStartReady();
        void SetCutinStartReady(bool set);

        bool GetCutinFinished();
        void SetCutinFinished(bool set);

        void AnimationLoopSimple(const KMCAnimData &anim_data);
        void AnimationLoopWithSE(const KMCAnimData &anim_data);
        void PlaySE(long long time, int trackid, int frand, std::string record, float volume);
        void MCMSettingChange(std::vector<float> *floatArray);

        bool ExistCategory(std::string t) { return kmc_category_rand_map.contains(t); }
        const std::vector<int> &GetCategoryFirstValues() const { return kmc_category_first_values; }

        void CategoryRandomizer();

        void Reset() {
            PlayerNamePlate.LoadedText = 0;
            PlayerNamePlate.LoadedWidget = 0;
            cutin_start_ready.store(false);
            cutin_finished.store(false);
        }



        KMCNPLoadedWidget *GetPlayerNamePlate() { return &PlayerNamePlate; }


    private:
        int CutIn(KMCCutinValues val);
        int GetCutInID(std::string aaaakmctype);
        int PeekNextCutInID(std::string aaaakmctype);
        void RefreshRandValues(size_t size, int min, int max, const std::vector<int> *random_indices,
                                         std::vector<int> &out_values);
        
        void PlayDuoCutin(KMCAnimST st);
        void PlaySoloCutin(KMCAnimST st);

    private:
        bool animnow = false;
        std::mutex animnow_mtx;

        std::atomic<bool> cutin_start_ready{false};
        std::atomic<bool> cutin_finished{false};

        time_point<Clock> event_start = Clock::now();
        float followerDetectRange = 1000.0;
        std::unordered_map<std::string, KMCRandomData> kmc_category_rand_map;
        std::vector<int> kmc_category_first_values;
        // player
        KMCNPLoadedWidget PlayerNamePlate;

        // follower
    };
}