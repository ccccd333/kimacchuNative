#pragma once
#include "KMCUtility.h"

namespace KMCCT {
    void WaitWidgetVisible(int tid);
    void WaitWidgetInVisible(int tid);

    class KMCWaitTask {
        SINGLETONHEADER(KMCWaitTask)
    public:
        bool KMCCheckWait();
        void KMCPushWaitTask(KMCWaitType id, KMCWaitConfigs config);
        void KMCPushWaitTaskReset();
        void KMCPushWaitTaskClear();
        bool KMCCheckCutInStopped(KMCWaitType id);

        bool GetWaitFlag() { return aaaaWaitWidgetDisped; }
        void SetWaitFlag(bool set) { aaaaWaitWidgetDisped = set; }

        int GetIsinSceneState() const { return isinscene_state.load(std::memory_order_relaxed); }
        void SetIsinSceneState(int next_state) { isinscene_state.store(next_state, std::memory_order_relaxed); }

        void Reset();
    private:
        void DispWWidget(bool suspensionRequest);
        bool KMCCheckWaitTask();

    private:
        std::mutex aaaakmc_waittask_mtx_;
        std::atomic<int> isinscene_state{0};
        bool aaaaWaitWidgetDisped = false;
        int aaaaWaitTextWidget = -1;
    };
}