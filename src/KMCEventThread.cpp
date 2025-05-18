#include "KMCEventThread.h"

#include <IWWPapyrus.h>

#include <chrono>

#include "KMCConfig.h"
#include "KMCCutin.h"
#include "KMCCutinCondition.h"
#include "KMCOAR.h"
#include "KMCProfile.h"
#include "KMCSound.h"
#include "KMCStateManager.h"
#include "KMCUtility.h"
#include "KMCWaitTask.h"
#include "KMCTempKeywordManager.h"
#include "KMCExpression.h"
#include "thread_pool.h"

SINGLETONBODY(KMCCT::KMCEventThread)

// thread poolの管理場所、他のモジュールはスレッドを起動する際にwrapメソッドでアクセスする

std::string aaaakmcroot = "";
float aaaakmcvolum = 0.5f;
long long aaaakmcAnimtime = 4;
long long aaaakmctime = 5;
long long aaaakmcCycle = 10;
long long aaaakmcUpdatePapyrusCycle = 10;

bool init_first = false;
bool is_already_inited = false;
bool enable_cutin = true;
bool enable_profile = true;

// std::mutex load_mtx_;

std::mutex aaaakmc_wait_init_mtx_;
std::mutex aaaakmc_is_already_init_mtx_;

ThreadPoolExecutor executor;

bool isInitEnd = false;
bool isProfileInitEnd = false;
float followerDetectRange = 1000.0;
std::vector<std::string> cutinArg;
RE::TESForm *form;

std::vector<int> LoadedWidgetsFactory;

using Clock = std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::time_point;
using namespace std::literals::chrono_literals;

#pragma region Show Profile
void KMCCT::InterruptProfileEventManager() { KMCCT::KMCProfile::GetSingleton()->InterruptProfileEventManager(); }

void KMCCT::wrap_UpdateModifiedContainer(std::vector<std::string> &mod_container, int &StrageUtilEndIndex,
                                         int &ModStartIndex, int &ModEndIndex, KMCProfil &profile) {
    executor
        .submit(UpdateModifiedContainer, &mod_container, &StrageUtilEndIndex, &ModStartIndex, &ModEndIndex, &profile)
        .wait_for(std::chrono::seconds(0));
}

void KMCCT::UpdateModifiedContainer(std::vector<std::string> *mod_container, int *StrageUtilEndIndex,
                                    int *ModStartIndex, int *ModEndIndex, KMCProfil *profile) {
    KMCCT::KMCProfile::GetSingleton()->UpdateModifiedContainer(mod_container, StrageUtilEndIndex, ModStartIndex,
                                                               ModEndIndex, profile);
}
#pragma endregion

#pragma region Timer ex Function
void KMCCT::KMCTimer(long long limit) {
    time_point<Clock> start = Clock::now();
    time_point<Clock> end;
    long long dur = 0;
    // sleep
    while (true) {
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
        end = Clock::now();
        milliseconds diff = duration_cast<milliseconds>(end - start);
        dur = diff.count();
        if (dur >= limit) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::WHILE_WAIT_TIME));
    }
}

void KMCCT::KMCTimerWithWaitTask(long long limit) {
    time_point<Clock> start = Clock::now();
    time_point<Clock> end;
    long long dur = 0;
    // sleep
    while (true) {
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
        end = Clock::now();
        milliseconds diff = duration_cast<milliseconds>(end - start);
        dur = diff.count();
        if (dur >= limit) {
            break;
        }

        if (KMCCT::KMCWaitTask::GetSingleton()->GetWaitFlag()) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::WHILE_WAIT_TIME));
    }
}
#pragma endregion

#pragma region main loop
void KMCCT::CutInPeriodicCall() {
    
    while (true) {
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
        std::string cutin_name = "";
        STCutinSetting cutin_setting;
        if (isInitEnd && KMCCT::KMCWaitTask::GetSingleton()->KMCCheckWait()) {
            // std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::WHILE_WAIT_TIME));
            continue;
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
        // auto func_arg = RE::MakeFunctionArguments();
        //RE::BSScript::Internal::VirtualMachine::GetSingleton()->SendEventAll("event name", func_arg);
        //PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuExpression");
        cutin_setting.time = aaaakmctime;
        cutin_setting.anim_time = aaaakmcAnimtime;
        cutin_setting.volume = aaaakmcvolum;

        if (isInitEnd && !KMCCT::KMCCutin::GetSingleton()->GetAnimNow()) {
            KMCCutinValues cutin_values =
                KMCCutinValues(cutin_name, cutin_setting.time, cutin_setting.anim_time, cutin_setting.volume);

            KMCCT::KMCCutin::GetSingleton()->CondCutIn(cutin_values);
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::WHILE_WAIT_TIME / 2));

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
    }
}

void KMCCT::CutInConditionPeriodicCall() {
    while (true) {
        LOG("[CutInConditionPeriodicCall]")
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
        int result = 0;
        if (isInitEnd && KMCCT::KMCWaitTask::GetSingleton()->GetIsinSceneState() == -3) {
            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CUT_IN_COND_WHILE_WAIT_TIME));
            continue;
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }

        result = KMCCT::KMCCutinCondition::GetSingleton()->ToMove(KMCCCStartArg(Clock::now()));

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CUT_IN_COND_WHILE_WAIT_TIME));

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
    }
}
//namespace KMCCT {
//    void JStrageMapN(std::string tag, int count, int root, int i) {
//        
//        for (int jk = 0; jk < count; jk++) {
//            std::string kn = "";
//            kn = JMap::nextKey(root, kn).c_str();
//            int njmc = JMap::count(root);
//            int njac = JArray::count(root);
//
//            LOG("{} nkey = {}, jmc = {}, jac = {}, nindex = {}, index = {}", tag, kn, njmc, njac, jk, i);
//            if (njmc > 0) {
//                int obj = JMap::getObj(root, kn);
//                if (obj > 0) {
//                    JStrageMapN(tag + "." + kn, njmc, obj, jk + i);
//                } else {
//                    int allk = root;
//                    int allv = root;
//                    std::string mkey = "";
//                    for (int mki = 0; mki < njmc; mki++) {
//                        allk = JMap::allKeys(allk);
//                        mkey = JMap::nextKey(allk, mkey).c_str();
//                        allv = JMap::allKeys(allv);
//
//                        int value_type = JMap::valueType(allk, mkey);
//                        float f = JMap::getFlt(allk, mkey, -99);
//                        int int_i = JMap::getInt(allk, mkey, -99);
//                        std::string s = JMap::getStr(allk, mkey, "").c_str();
//                        LOG("(MAP) {} nkey = {}, {} (MAP::Key)Type = {} {} {} {}", tag, mkey, allk, value_type, f, int_i,
//                            s);
//
//                        value_type = JMap::valueType(allv, mkey);
//                        f = JMap::getFlt(allv, mkey, -99);
//                        int_i = JMap::getInt(allv, mkey, -99);
//                        s = JMap::getStr(allv, mkey, "").c_str();
//                        LOG("(MAP) {} nkey = {}, {} (MAP::Value)Type = {} {} {} {}", tag, mkey, allv, value_type, f,
//                            int_i, s);
//                    }
//                }
//            }
//            if (njac > 0) {
//                for (int ai = 0; ai < njac; ai++) {
//                    int value_type = JArray::valueType(root, ai);
//                    float f = JArray::getFlt(root, ai, -99);
//                    int int_i = JArray::getInt(root, ai, -99);
//                    std::string s = JArray::getStr(root, ai, "").c_str();
//                    LOG("(ARRAY) {} nkey = {}, (ARRAY)index = {}, (ARRAY)Type = {} {} {} {}", tag, kn, ai,
//                        value_type, f, int_i, s);
//                }
//            }
//            
//        }
//    }
//
//    void JStrageMap() {
//
//        for (int i = 0; i < 1000000; i++) {
//            std::string k = "";
//            k = JMap::nextKey(i, k).c_str();
//            int jmc = JMap::count(i);
//            int jac = JArray::count(i);
//            if (jmc > 0 || jac > 0) {
//                LOG("root.{} nkey = {}, jmc = {}, jac = {}, index = {}", k, k, jmc, jac, i);
//                if (jmc > 0){
//                    
//                    int allk = i;
//                    int allv = i;
//                    std::string mkey = k;
//                    for (int mki = 0; mki < jmc; mki++) {
//                        
//                        allk = JMap::allKeys(i);
//                        
//                        allv = JMap::allValues(i);
//
//                        int value_type = JMap::valueType(i, mkey);
//                        float f = JMap::getFlt(i, mkey, -99);
//                        int int_i = JMap::getInt(i, mkey, -99);
//                        std::string s = JMap::getStr(i, mkey, "").c_str();
//                        LOG("(MAP) {} nkey = {}, {} (MAP::Key)Type = {} {} {} {}", k, mkey, allk, value_type, f, int_i, s);
//
//                        value_type = JMap::valueType(i, mkey);
//                        f = JMap::getFlt(i, mkey, -99);
//                        int_i = JMap::getInt(i, mkey, -99);
//                        s = JMap::getStr(i, mkey, "").c_str();
//                        LOG("(MAP) {} nkey = {}, {} (MAP::Value)Type = {} {} {} {}", k, mkey, allv, value_type, f, int_i, s);
//
//                        mkey = JMap::nextKey(i, mkey).c_str();
//                    }
//                }
//
//                if (jac > 0) {
//                    for (int ai = 0; ai < jac; ai++) {
//                        int  value_type = JArray::valueType(i, ai);
//                        float f = JArray::getFlt(i, ai, -99);
//                        int int_i = JArray::getInt(i, ai, -99);
//                        std::string s = JArray::getStr(i, ai, "").c_str();
//                        LOG("(ARRAY) root.{} nkey = {}, (ARRAY)index = {}, (ARRAY)Type = {}, {} {} {}", k, k, ai,
//                            value_type, f, int_i, s);
//                    }
//                }
//            }
//        }
//
//        RE::DebugNotification("JStrageMap Find Finish");
//    }
//}

void KMCCT::ProfilePeriodicCall() {
    //JStrageMap();
    while (true) {
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
        
        if (isProfileInitEnd) {
            if (!KMCCT::KMCProfile::GetSingleton()->Get_update_prifile() &&
                KMCCT::KMCProfile::GetSingleton()->Get_show_prifile() &&
                !KMCCT::KMCProfile::GetSingleton()->Get_showing_profile() &&
                KMCCT::KMCStateManager::GetSingleton()->GetProfileInvisibleState(
                    KMCCT::KMCWaitTask::GetSingleton()->GetWaitFlag())) {
                // 戦闘中などは消すようにする。json内容次第。
                KMCCT::KMCProfile::GetSingleton()->ShowProfile(false);
                KMCCT::KMCProfile::GetSingleton()->Set_switch_disp_profile_flag(false);
            }
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::INSPECTION_LOOP_MS));

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }
    }
}

void KMCCT::PapyrusPeriodicCall() {

    time_point<Clock> start = Clock::now();

    while (true) {
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            break;
        }

        if (isProfileInitEnd || isInitEnd) {
            time_point<Clock> end;
            long long dur = 0;
            end = Clock::now();
            milliseconds diff = duration_cast<milliseconds>(end - start);
            dur = diff.count();
            if (dur >= aaaakmcUpdatePapyrusCycle * KMCCT::PAPYRUS_UPDATE_WHILE_WAIT_TIME) {
                auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();

                if (player) {
                    RE::Actor *ac = player;
                    PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "UpdateProf", form, ac);
                    start = Clock::now();
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::PAPYRUS_UPDATE_WHILE_WAIT_TIME));
    }
}
#pragma endregion

#pragma region cutin init function
int KMCCT::WaitLoad(int *wid, uint64_t *rand, std::string *root,
                    std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> **loadedWedget) {
    return KMCCT::KMCCutin::GetSingleton()->WaitLoad(wid, rand, root, loadedWedget);
}

int KMCCT::WaitMultLoad(int *wid, uint64_t *rand, std::string *root, int *index,
                        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> **loadedWedget) {
    return KMCCT::KMCCutin::GetSingleton()->WaitMultLoad(wid, rand, root, index, loadedWedget);
}

int KMCCT::WaitLoadText(int *wid, uint64_t *rand, std::string *root,
                        std::vector<std::pair<uint64_t, int>> **loadedText) {
    return KMCCT::KMCCutin::GetSingleton()->WaitLoadText(wid, rand, root, loadedText);
}

int KMCCT::WaitLoadNamePlate(int *wid) { return KMCCT::KMCCutin::GetSingleton()->WaitLoadNamePlate(wid); }

void KMCCT::InitMain(std::vector<float> *floatArray) {
    isInitEnd = false;
    isProfileInitEnd = false;
    auto setting = KMCCT::KMCConfig::GetSingleton()->getISetting();
    long long init_wait_time =
        KMCFindVector(setting, KMCCT::OTHER_INITIALIZATION_WAITING_TIME, KMCCT::INIT_WAIT_TIME) * KMCCT::TIME_SCALE_MS;
    // Allow 5 seconds for traffic jam.
    KMCCT::KMCTimer(init_wait_time);
    if (enable_profile) {
        KMCCT::KMCProfile::GetSingleton()->InitProfile(aaaakmcroot, floatArray);
        isProfileInitEnd = true;
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }
    }

    if (enable_cutin) {
        KMCCT::KMCWaitTask::GetSingleton()->InitWaitTask(aaaakmcroot, floatArray);
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        KMCCT::KMCCutin::GetSingleton()->InitCutin(aaaakmcroot, floatArray);

        isInitEnd = true;
    }

    RE::DebugNotification("KMC Init End");
}
#pragma endregion

#pragma region oar
void KMCCT::TryKMCOAR(OARCompDetail *ocd) { KMCCT::KMCOAR::GetSingleton()->TryKMCOAR(ocd); }
#pragma endregion

#pragma region expression
void KMCCT::TryKMCExp(STMFGPair *mfg_pair) { KMCCT::KMCExpression::GetSingleton()->TryKMCExp(mfg_pair); }
void KMCCT::TryKMCFLExp(STMFGPair *mfg_pair) { KMCCT::KMCExpression::GetSingleton()->TryKMCFLExp(mfg_pair); }
#pragma endregion

#pragma region wait task
void KMCCT::wrap_InterruptCutInEventManager(std::function<void(void)> fn) {
    executor.submit(fn).wait_for(std::chrono::seconds(0));
}
#pragma endregion

#pragma region use: register
void KMCCT::KMCSetInitFlag() {
    init_first = false;
    is_already_inited = false;
    KMCCT::KMCProfile::GetSingleton()->Set_first_profile_update(false);
}

bool KMCCT::KMCGetInitFlag() { return init_first; }

void KMCCT::KMCLoadedWidget() {
    // for (int i = 0; i < LoadedWidgetsFactory.size(); i++) {
    //     LOG("LoadedWidget root {} ID {}", aaaakmcroot,LoadedWidgetsFactory[i]);
    // IWW::MainFunctions::GetSingleton()->Destroy(aaaakmcroot, LoadedWidgetsFactory.at(i));
    // std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_DESTROY_MS));
    //}
    // LoadedWidgetsFactory.clear();
}
#pragma endregion

#pragma region use: oar
void KMCCT::LaunchOAR(OARCompDetail &ocd) { executor.submit(TryKMCOAR, &ocd).wait_for(std::chrono::seconds(0)); }

#pragma endregion

#pragma region use: exp
void KMCCT::LaunchExp(STMFGPair &mfg_pair) { executor.submit(TryKMCExp, &mfg_pair).wait_for(std::chrono::seconds(0)); }
void KMCCT::LaunchFLExp(STMFGPair &mfg_pair) { executor.submit(TryKMCFLExp, &mfg_pair).wait_for(std::chrono::seconds(0)); }
#pragma endregion

namespace KMCCT {

    KMCEventThread::~KMCEventThread() { 
        forceendanim = true;
    }

    bool KMCEventThread::IsAlreadyInited() {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_is_already_init_mtx_);
            if (is_already_inited == false) {
                is_already_inited = true;
                return false;
            } else {
                return true;
            }
        }
    }

    // 関数名が微妙だけどpapyrus側でskyuiが準備完了時にここに入ってくる
    void KMCEventThread::InitWordsAndWidgets(RE::BSFixedString skyroot, std::vector<float> floatArray) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_wait_init_mtx_);
            if (init_first) return;
            init_first = true;
            // if (aaaakmcroot == skyroot.c_str()) return;
            aaaakmcroot = skyroot.c_str();
            aaaakmcvolum = floatArray.at(0);
            aaaakmcAnimtime = floatArray.at(1);
            aaaakmctime = floatArray.at(2);
            aaaakmcCycle = floatArray.at(3);
            aaaakmcUpdatePapyrusCycle = floatArray.at(20);

            // LOG("Player POS {} {} {} {} {} {} {} {}", aaaaPlayerTextX, aaaaPlayerTextY, aaaaPlayerWidgetX,
            //     aaaaPlayerWidgetY, aaaaPlayerNTextX, aaaaPlayerNTextY, aaaaPlayerNWidgetX, aaaaPlayerNWidgetY);

            // LOG("Follower POS {} {} {} {} {} {} {} {}", aaaaFollowerTextX, aaaaFollowerTextY, aaaaFollowerWidgetX,
            //     aaaaFollowerWidgetY, aaaaFollowerNTextX, aaaaFollowerNTextY, aaaaFollowerNWidgetX,
            //     aaaaFollowerNWidgetY);

            isInitEnd = false;
            isProfileInitEnd = false;
            forceendanim = false;

            KMCCT::KMCWaitTask::GetSingleton()->SetWaitFlag(false);

            KMCCT::KMCCutin::GetSingleton()->SetAnimNow(false);

            // LOG("KMCEventThread::Init root = {} ms {} me {} up {}", aaaakmcroot, ModStartIndex, ModEndIndex,
            // update_prifile);

            Reset();
            KMCCT::KMCWaitTask::GetSingleton()->KMCPushWaitTaskClear();

            // CategoryRandomizer();

            // KMCCT::KMCStateManager::GetSingleton()->SetFHUStatus(0.0f, 0.0f, 0.0f);

            if (aaaakmcroot != "") {
                papyrus_floatArray = std::move(floatArray);
                executor.submit(InitMain, &papyrus_floatArray).wait_for(std::chrono::seconds(0));

                if (enable_cutin) {
                    executor.submit(CutInPeriodicCall).wait_for(std::chrono::seconds(0));
                    executor.submit(CutInConditionPeriodicCall).wait_for(std::chrono::seconds(0));
                }

                if (enable_profile) {
                    executor.submit(ProfilePeriodicCall).wait_for(std::chrono::seconds(0));
                }

                executor.submit(PapyrusPeriodicCall).wait_for(std::chrono::seconds(0));
                
            } else {
                ERROR("Error KMCEventThread::Init skyroot not found.");
            }
        }
    }

    // これはゲームロード時に入ってくる
    void KMCEventThread::Init() {
        auto setting = KMCCT::KMCConfig::GetSingleton()->getISetting();
        enable_cutin = KMCFindVector(setting, ENABLE_CUT_IN_SETTING, true);
        enable_profile = KMCFindVector(setting, ENABLE_PROFILE_SETTING, true);
        form = RE::TESDataHandler::GetSingleton()->LookupForm(0x806, "KimachuuCutIn.esp"); 
    }

    void KMCEventThread::MCMSettingChange(std::vector<float> floatArray) {
        papyrus_mcm = std::move(floatArray);

        aaaakmcUpdatePapyrusCycle = papyrus_mcm.at(20);
        // 現在はカットイン用しかMCMがないので一旦
        KMCCT::KMCCutin::GetSingleton()->MCMSettingChange(&papyrus_mcm);
    }

    void KMCEventThread::CutInCreate(std::vector<std::string> variableArray) {
        if (isInitEnd && !KMCCT::KMCCutin::GetSingleton()->GetAnimNow() &&
            !KMCCT::KMCWaitTask::GetSingleton()->GetWaitFlag()) {
            // if (cutinArg.size() != variableArray.size()) {
            //     cutinArg.resize(variableArray.size());
            // }
            // for (int i = 0; i < cutinArg.size(); i++) {
            //     cutinArg[i] = variableArray[i];
            // }
            // cutinArg = std::move(variableArray);

            // executor.submit(CutIn, &cutinArg).wait_for(std::chrono::seconds(0));
        }
    }

    void KMCEventThread::TryShowProfile() {
        executor.submit(InterruptProfileEventManager).wait_for(std::chrono::seconds(0));
    }

    void KMCEventThread::Reset() {
        KMCCT::KMCCutin::GetSingleton()->Reset();
        KMCCT::KMCProfile::GetSingleton()->Reset();
        KMCCT::KMCOAR::GetSingleton()->Reset();
        KMCCT::KMCCutinCondition::GetSingleton()->ResetAll();
        KMCCT::KMCWaitTask::GetSingleton()->Reset();
        KMCCT::KMCTempKeywordManager::GetSingleton()->Reset();
        KMCCT::KMCStateManager::GetSingleton()->Reset();
        KMCCT::KMCExpression::GetSingleton()->Reset();
    }

    bool KMCEventThread::GetProfileInitEnd() { return isProfileInitEnd; }
    bool KMCEventThread::GetInitFirstFlag() { return init_first; }
    bool KMCEventThread::GetEnableProfileFlag() { return enable_profile; }
    bool KMCEventThread::GetInitEndFlag() { return isInitEnd; }

    int KMCEventThread::wrap_WaitLoadNamePlate(int &wid) {
        std::future<int> lwID;
        lwID = executor.submit(WaitLoadNamePlate, &wid);
        lwID.wait();
        return lwID.get();
    }

    int KMCEventThread::wrap_WaitMultLoad(int &wid, uint64_t &rand, int &index,
                                          std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> *loadedWedget) {
        auto lwID = executor.submit(WaitMultLoad, &wid, &rand, &aaaakmcroot, &index, &loadedWedget);
        lwID.wait();
        return lwID.get();
    }

    int KMCEventThread::wrap_WaitLoad(int &wid, uint64_t &rand,
                                      std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> *loadedWedget) {
        auto lwID = executor.submit(WaitLoad, &wid, &rand, &aaaakmcroot, &loadedWedget);
        lwID.wait();
        return lwID.get();
    }

    int KMCEventThread::wrap_WaitLoadText(int &wid, uint64_t &rand, std::vector<std::pair<uint64_t, int>> *loadedText) {
        auto lwID = executor.submit(WaitLoadText, &wid, &rand, &aaaakmcroot, &loadedText);
        lwID.wait();
        return lwID.get();
    }

}