#include "KMCConfig.h"
#include "KMCEventThread.h"
#include "KMCWaitTask.h"
#include "KMCStateManager.h"

SINGLETONBODY(KMCCT::KMCWaitTask)

namespace KMCCT {
	// カットインの停止などを管理
	// ついでに停止中ウェジットも管理
    std::map<int, KMCCT::WaitWidgetFunction> AnimWaitWidget{
        {0, KMCCT::WaitWidgetVisible}, {1, KMCCT::WaitWidgetFadeOut}, {2, KMCCT::WaitWidgetFadeIn}};
    std::string wt_aaaakmcroot = "";
    std::vector<KMCWaitConfigs> aaaaKmcPushTasks((size_t)KMCWaitType::max);

#pragma region function pointer

    void KMCCT::WaitWidgetVisible(int tid) {
        if (tid != -1) {
            IWW::MainFunctions::GetSingleton()->SetVisible(wt_aaaakmcroot, tid, true);
        }
    }

    void KMCCT::WaitWidgetInVisible(int tid) {
        if (tid != -1) {
            IWW::MainFunctions::GetSingleton()->SetVisible(wt_aaaakmcroot, tid, false);
        }
    }

    void KMCCT::WaitWidgetFadeIn(int tid) {
        // auto npafade = KMCCT::KMCConfig::GetSingleton()->getINamePlateAnimation((int)end_fadeout);

        const static int strength = 1;       // KMCFindVector(&(npafade->settings), "fadeoutspeed", 1);
        const static int speedupvalue = 1;   // KMCFindVector(&(npafade->settings), "speedupvalue", 1);
        const static int startalpha = 0;     // KMCFindVector(&(npafade->settings), "startalpha", 255);
        const static int stAddTiming = 100;  // KMCFindVector(&(npafade->settings), "switchgearsMS", 50);
        const static int t =
            2 *
            KMCCT::TIME_SCALE_MS;  // KMCFindVector(&(npafade->settings), "animationtime", 1) * KMCCT::TIME_SCALE_MS;

        long long freq = 2;
        int add = speedupvalue;
        int wid = tid;

        int alpha = startalpha;
        int stock = 0;

        if (wid != -1) {
            IWW::MainFunctions::GetSingleton()->SetVisible(wt_aaaakmcroot, wid, true);
            time_point<Clock> start = Clock::now();

            while (true) {
                ++stock;
                if (stock >= stAddTiming) {
                    stock -= stAddTiming;
                    add += speedupvalue;
                }
                alpha += (strength * add);

                if (alpha > 0) {
                    IWW::MainFunctions::GetSingleton()->SetTransparency(wt_aaaakmcroot, wid, alpha);
                } else {
                    break;
                }

                time_point<Clock> end;
                long long dur = 0;

                end = Clock::now();
                milliseconds diff = duration_cast<milliseconds>(end - start);
                dur = diff.count();
                if (dur >= t) {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(freq));
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    break;
                }
            }
        }
    }

    void KMCCT::WaitWidgetFadeOut(int tid) {
        // auto npafade = KMCCT::KMCConfig::GetSingleton()->getINamePlateAnimation((int)end_fadeout);

        const static int strength = 1;       // KMCFindVector(&(npafade->settings), "fadeoutspeed", 1);
        const static int speedupvalue = 1;   // KMCFindVector(&(npafade->settings), "speedupvalue", 1);
        const static int startalpha = 255;   // KMCFindVector(&(npafade->settings), "startalpha", 255);
        const static int stAddTiming = 100;  // KMCFindVector(&(npafade->settings), "switchgearsMS", 50);
        const static int t =
            2 *
            KMCCT::TIME_SCALE_MS;  // KMCFindVector(&(npafade->settings), "animationtime", 1) * KMCCT::TIME_SCALE_MS;

        long long freq = 2;
        int add = speedupvalue;
        int wid = tid;

        int alpha = startalpha;
        int stock = 0;

        if (wid != -1) {
            IWW::MainFunctions::GetSingleton()->SetVisible(wt_aaaakmcroot, wid, true);
            time_point<Clock> start = Clock::now();

            while (true) {
                ++stock;
                if (stock >= stAddTiming) {
                    stock -= stAddTiming;
                    add += speedupvalue;
                }
                alpha -= (strength * add);

                if (alpha > 0) {
                    IWW::MainFunctions::GetSingleton()->SetTransparency(wt_aaaakmcroot, wid, alpha);
                } else {
                    break;
                }

                time_point<Clock> end;
                long long dur = 0;

                end = Clock::now();
                milliseconds diff = duration_cast<milliseconds>(end - start);
                dur = diff.count();
                if (dur >= t) {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(freq));
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    break;
                }
            }
        }
    }
#pragma endregion

    void KMCWaitTask::InitWaitTask(std::string skyroot, std::vector<float>* floatArray) { 
        wt_aaaakmcroot = skyroot;
        std::string filename = KMCCT::PICT_PATH1 + "/" + KMCCT::WAIT_WIDGET_PICT_NAME + KMCCT::PICT_TYPE;
        int rwid = -1;
        if (IsFileExist(KMCCT::PICT_ROOT + filename)) {
            int wid = IWW::MainFunctions::GetSingleton()->LoadWidget(wt_aaaakmcroot, filename, 10000, 10000, false);
            rwid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoadNamePlate(wid);
        } else {
            WARN("File path not found. If not intended, no problem. {}", filename);
        }

        aaaaWaitTextWidget = rwid;

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
            KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
            return;
        }
        auto namep = KMCCT::KMCConfig::GetSingleton()->getINamePlate();
        if (namep->size() == 0) {
            WARN("NamePlate.json undefine. disable wait widget.");
            return;
        }

        KMCNamePlate pnpsetting = (*namep)[0].second;

        if (rwid > 0) {
            IWW::MainFunctions::GetSingleton()->SetPosX(wt_aaaakmcroot, rwid, pnpsetting.font_x);
            IWW::MainFunctions::GetSingleton()->SetPosY(wt_aaaakmcroot, rwid, pnpsetting.font_y);
            IWW::MainFunctions::GetSingleton()->SetSizeH(wt_aaaakmcroot, rwid, KMCCT::WAIT_WIDGET_SIZE);
            IWW::MainFunctions::GetSingleton()->SetSizeW(wt_aaaakmcroot, rwid, KMCCT::WAIT_WIDGET_SIZE);
        }
    }

    bool KMCWaitTask::KMCCheckWait() {
        isinscene_state = KMCCT::KMCStateManager::GetSingleton()->IsInScene();
        //out_state = state;
        if (isinscene_state == -3) {
            // If the cell is not attached, such as during the main menu
            aaaaWaitWidgetDisped = true;
            KMCCT::KMCTimer(KMCCT::WAIT_CYCLE_MS);
        } else {
            if (KMCCheckWaitTask()) {
                aaaaWaitWidgetDisped = true;
                DispWWidget(true);
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
            LOG("KMCPushWaitTask : id {}", (int)id);
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
            for (auto [key, value] : AnimWaitWidget) {
                value(aaaaWaitTextWidget);
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    break;
                }
            }
        } else {
            KMCCT::WaitWidgetInVisible(aaaaWaitTextWidget);
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

            LOG("KMCCheckWaitTask : suspensionRequest {}", suspensionRequest);

            return suspensionRequest;
        }
    }
}