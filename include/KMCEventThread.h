#pragma once
#include "KMCConfig.h"
#include "KMCUtility.h"

namespace KMCCT {

    void KMCTimer(long long limit);
    
    
    void wrap_UpdateModifiedContainer(std::vector<std::string> &mod_container, int &StrageUtilEndIndex, int &ModStartIndex,
                                      int &ModEndIndex, KMCProfil &profile);
    void UpdateModifiedContainer(std::vector<std::string> *mod_container, int *StrageUtilEndIndex, int *ModStartIndex,
                                 int *ModEndIndex, KMCProfil *profile);
    void KMCTimerWithWaitTask(long long limit);
    void CutInPeriodicCall();
    void CutInConditionPeriodicCall();
    //void CutInPeriodicCall_Cycle();
    //void CutInPeriodicCall_Evaluation();
    void ProfilePeriodicCall();
    void PapyrusPeriodicCall();
#pragma region init function
   // void InitMain(std::vector<float> *floatArray);
#pragma endregion

#pragma region interrupt event
    void InterruptEventAddItem();
    void InterruptEventOnHit();
#pragma endregion

#pragma region oar
    void TryKMCOAR(OARCompDetail *ocd);
#pragma endregion

#pragma region expression
    void TryKMCExp(STMFGPair *mfg_pair);
    void TryKMCFLExp(STMFGPair *mfg_pair);
#pragma endregion

    void wrap_InterruptCutInEventManager(std::function<void(void)> fn);
    void InterruptProfileEventManager();

    void KMCSetInitFlag();
    bool KMCGetInitFlag();
    void LaunchOAR(OARCompDetail &ocd);
    void LaunchExp(STMFGPair &mfg_pair);
    void LaunchFLExp(STMFGPair &mfg_pair);

    typedef void (*WaitWidgetFunction)(int wid);

    const std::string STATE_MANAGER_CONFIG_KEY = "event_cool_time";
    const std::string PROFILE_DELAY_TIME_CONFIG_KEY = "profile_delay_time";
    const std::string PROFILE_POLLING_COUNT_CONFIG_KEY = "profile_polling_count";
    const std::string PROFILE_SETTING_KEY_NAME_OTHER = "other";
    const std::string TEXT_FADE_IN_OUT_TIME_SETTING = "text_fade_in_out_time";
    const std::string WIDGET_FADE_IN_OUT_TIME_SETTING = "widget_fade_in_out_time";

    const std::string ENABLE_CUT_IN_SETTING = "enable_cut_in";
    const std::string ENABLE_PROFILE_SETTING = "enable_profile";

    const long long INIT_WAIT_TIME = 5;
    const int INTERRUPT_EVENT_COOL_TIME = 5;
    const float INTERRUPT_SHOW_PROFILE_DELAY_TIME = 1.0f;
    const int PROFILE_POLLING_COUNT = 3;
    class KMCEventThread {
        SINGLETONHEADER(KMCEventThread)
    public:
        ~KMCEventThread();
        bool IsAlreadyInited();
        void InitWordsAndWidgets(RE::BSFixedString skyroot, std::vector<float> floatArray);
        void Init();
        void CutInCreate(std::vector<std::string> variableArray);
        void TryShowProfile();
        void MCMSettingChange(std::vector<float> floatArray);

        bool IsShuttingDown() const { return is_shutting_down.load(std::memory_order_relaxed); }

        int GetForceEndAnim() const { return force_end_anim.load(std::memory_order_relaxed); }
        void SetForceEndAnim(bool flag) { force_end_anim.store(flag, std::memory_order_relaxed); }
        
        void Reset();
        bool GetProfileInitEnd();
        bool GetInitFirstFlag();
        bool GetEnableProfileFlag();
    public:
        std::atomic<bool> force_end_anim;
        
        
    private:
        std::atomic<bool> is_shutting_down{false};
        std::vector<float> papyrus_floatArray;
        std::vector<float> papyrus_mcm;
    };

    
}