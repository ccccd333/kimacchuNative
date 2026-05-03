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

        bool GetWaitFlag() { return is_speaking.load(std::memory_order_relaxed); }
        void SetWaitFlag(bool set) { is_speaking.store(set, std::memory_order_relaxed);}

        int GetIsinSceneState() const { return isinscene_state.load(std::memory_order_relaxed); }
        void SetIsinSceneState(int next_state) { isinscene_state.store(next_state, std::memory_order_relaxed); }

        void Reset();
    private:
        void DispStopIcon(bool suspension_request);
        bool KMCCheckWaitTask();

    private:
        std::mutex aaaakmc_waittask_mtx_;
        std::atomic<int> isinscene_state{0};
        std::atomic<bool> is_speaking{false};
        int aaaaWaitTextWidget = -1;
    };
}