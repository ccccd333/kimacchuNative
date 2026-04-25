#include "KMCConfig.h"
#include "KMCEventThread.h"
#include "KMCWaitTask.h"
#include "KMCStateManager.h"
#include "KMCPrismaUIBridge.h"

SINGLETONBODY(KMCCT::KMCWaitTask)

namespace KMCCT {
	// カットインの停止などを管理
	// ついでに停止中ウェジットも管理
    std::map<int, KMCCT::WaitWidgetFunction> AnimWaitWidget{
        {0, KMCCT::WaitWidgetVisible}};
    std::string wt_aaaakmcroot = "";
    std::vector<KMCWaitConfigs> aaaaKmcPushTasks((size_t)KMCWaitType::max);

#pragma region function pointer

    void KMCCT::WaitWidgetVisible(int tid) { 
        KMCPrismaUIBridge::GetSingleton()->KMCShowStopIcon();
    }

    void KMCCT::WaitWidgetInVisible(int tid) { KMCPrismaUIBridge::GetSingleton()->KMCHideStopIcon();
    }

#pragma endregion

    bool KMCWaitTask::KMCCheckWait() {
        int state = KMCCT::KMCStateManager::GetSingleton()->IsInScene();
        SetIsinSceneState(state);
        //out_state = state;
        if (state == -3) {
            // If the cell is not attached, such as during the main menu
            aaaaWaitWidgetDisped = true;
            KMCCT::KMCTimer(KMCCT::WAIT_CYCLE_MS);
        } else {
            if (KMCCheckWaitTask()) {
                DispWWidget(true);
                aaaaWaitWidgetDisped = true;
                KMCCT::KMCTimer(KMCCT::WAIT_CYCLE_MS);
            } else if (aaaaWaitWidgetDisped) {
                DispWWidget(false);
                aaaaWaitWidgetDisped = false;
            } else {
                aaaaWaitWidgetDisped = false;
            }
        }

        return aaaaWaitWidgetDisped;
    }

    void KMCWaitTask::KMCPushWaitTask(KMCWaitType id, KMCWaitConfigs config) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_waittask_mtx_);
            KMC_LOG("KMCPushWaitTask : id {}", (int)id);
            aaaaKmcPushTasks[(size_t)id] = config;
        }
    }

    void KMCWaitTask::KMCPushWaitTaskReset() {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_waittask_mtx_);
            for (int i = 0; i < aaaaKmcPushTasks.size(); i++) {
                KMCWaitConfigs config = aaaaKmcPushTasks[i];
                if (config.enable) {
                    aaaaKmcPushTasks[i] = KMCWaitConfigs(config.waittime_ms, Clock::now(), config.type, true);
                }
            }
        }
    }

    void KMCWaitTask::KMCPushWaitTaskClear() {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_waittask_mtx_);
            for (int i = 0; i < aaaaKmcPushTasks.size(); i++) {
                KMCWaitConfigs config = aaaaKmcPushTasks[i];
                aaaaKmcPushTasks[i] = KMCWaitConfigs(config.waittime_ms, Clock::now(), config.type, false);
            }
        }
    }

    bool KMCWaitTask::KMCCheckCutInStopped(KMCWaitType id) {
        bool stopped = false;
        {
            std::lock_guard<std::mutex> lock(aaaakmc_waittask_mtx_);
            stopped = aaaaKmcPushTasks[(int)id].enable;
        }
        return stopped;
    }

    void KMCWaitTask::Reset() { 
        isinscene_state = 0;
        aaaaWaitWidgetDisped = false;
    }

    void KMCWaitTask::DispWWidget(bool suspensionRequest) {
        if (suspensionRequest) {
            if (!aaaaWaitWidgetDisped) {
                KMCCT::WaitWidgetVisible(aaaaWaitTextWidget);
            }
        } else {
            if (aaaaWaitWidgetDisped) {
                KMCCT::WaitWidgetInVisible(aaaaWaitTextWidget);
            }
        }
    }

    bool KMCWaitTask::KMCCheckWaitTask() {

        {
            std::lock_guard<std::mutex> lock(aaaakmc_waittask_mtx_);

            bool suspensionRequest = false;
            time_point<Clock> end = Clock::now();
            for (int i = 0; i < aaaaKmcPushTasks.size(); i++) {
                KMCWaitConfigs config = aaaaKmcPushTasks[i];
                if (config.enable) {
                    milliseconds diff = duration_cast<milliseconds>(end - config.entrytime);
                    long long dur = diff.count();
                    if (dur >= config.waittime_ms) {
                        aaaaKmcPushTasks[i] = KMCWaitConfigs(config.waittime_ms, end, config.type, false);
                    } else {
                        suspensionRequest = true;
                    }
                } else if (config.isCBTask) {
                    for (auto [callbackfunc, param] : config.tasks) {
                        callbackfunc(param);
                    }
                    config.ClearCallBackTask();
                    aaaaKmcPushTasks[i] = config;
                }
            }

            KMC_LOG("KMCCheckWaitTask : suspensionRequest {}", suspensionRequest);

            return suspensionRequest;
        }
    }
}