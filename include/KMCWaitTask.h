#pragma once
#include "KMCUtility.h"

namespace KMCCT {
    void WaitWidgetVisible(int tid);
    void WaitWidgetInVisible(int tid);
    void WaitWidgetFadeIn(int tid);
    void WaitWidgetFadeOut(int tid);

    class KMCWaitTask {
        SINGLETONHEADER(KMCWaitTask)
    public:
        ~KMCWaitTask();
        bool KMCCheckWait();
        void KMCPushWaitTask(KMCWaitType id, KMCWaitConfigs config);
        void KMCPushWaitTaskReset();
        void KMCPushWaitTaskClear();
        bool KMCCheckCutInStopped(KMCWaitType id);

        bool GetWaitFlag() { return aaaaWaitWidgetDisped; }
        void SetWaitFlag(bool set) { aaaaWaitWidgetDisped = set; }

        int GetIsinSceneState() { return isinscene_state; }

        void Reset();
    private:
        void DispWWidget(bool suspensionRequest);
        bool KMCCheckWaitTask();

    private:
        std::mutex aaaakmc_waittask_mtx_;
        int isinscene_state = 0;
        bool aaaaWaitWidgetDisped = false;
        int aaaaWaitTextWidget = -1;
    };
}