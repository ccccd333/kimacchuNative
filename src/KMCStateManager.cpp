#include "KMCStateManager.h"
#include "KMCConfig.h"
#include "KMCEventThread.h"
#include "KMCUtility.h"
#include "KMCWaitTask.h"
#include "KMCCutin.h"
#include "KMCGameEventListener.h"
#include "KMCContextManager.h"

SINGLETONBODY(KMCCT::KMCStateManager)

// struct RE::TESTopicInfoEvent {
//     RE::Actor* speaker;      // 00 - NiTPointer<Actor>
//     void* unk04;             // 08 - BSTSmartPointer<REFREventCallbacks::IEventCallback>
//     RE::FormID topicInfoID;  // 10
//     bool flag;               // 18

//    inline bool IsStarting() { return !flag; }
//    inline bool IsStopping() { return flag; }
//};

namespace KMCCT {


    bool isAddItem;
    bool isStoppingState = false;
    long long limit = 0;
    long long inSceneMS = 5000;
    float isinsceneDetectRangeExterior = 2000.0f;
    float isinsceneDetectRangeInterior = 1000.0f;
    float fhuVag = 0.0f;
    float fhuAnal = 0.0f;
    float fhuOral = 0.0f;
    

    //int cutin_chance_fhu = 100;
    int cutin_chance_fasttravel = 100;
    int cutin_chance_injury = 100;
    int cutin_chance_combat = 100;
    int cutin_chance_sneak = 100;
    int cutin_chance_mount = 100;
    int cutin_chance_idle = 100;
    int cutin_chance_isindungeon = 100;

    //int cutin_priority_fhu = 10;
    int cutin_priority_fasttravel = 20;
    int cutin_priority_injury = 30;
    int cutin_priority_combat = 40;
    int cutin_priority_sneak = 50;
    int cutin_priority_mount = 60;
    int cutin_priority_isindungeon = 65;
    int cutin_priority_idle = 70;
    
    std::atomic<bool> StrageUtilReady(false);

    // void PushEventAddItem() {

    //    static time_point<Clock> start = Clock::now();
    //    end = Clock::now();
    //    milliseconds diff = duration_cast<milliseconds>(end - start);
    //    dur = diff.count();
    //    if (dur >= limit) {
    //        ::KMCCT::KMCEventThread::GetSingleton()->CutInCreate({"AddItem"});
    //        start = Clock::now();
    //        //isAddItem = false;
    //    }
    //}


    void KMCStateManager::Register() {

        //auto lambda_function = [this]() { this->Register(); };

        auto setting = KMCCT::KMCConfig::GetSingleton()->getISetting();

        limit = KMCFindVector(setting, STATE_MANAGER_CONFIG_KEY, (long long)1) * KMCCT::TIME_SCALE_MS;
        inSceneMS = KMCFindVector(setting, IN_SCENE_CONFIG_KEY, (long long)10) * KMCCT::TIME_SCALE_MS;
        isinsceneDetectRangeExterior = KMCFindVector(setting, ISINSCENE_NPC_DETECT_RANGE_EXTERIOR, 2000.0f);
        isinsceneDetectRangeInterior = KMCFindVector(setting, ISINSCENE_NPC_DETECT_RANGE_INTERIOR, 1000.0f);




        KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
            [this](const RE::MenuOpenCloseEvent* event) { this->MenuOpenCloseEvent(event); });

        
        auto profile_invis = KMCCT::KMCConfig::GetSingleton()->getIInvisibleTimingSetting();
        int profile_inv_combat = KMCFindVector(profile_invis, PROFILE_INVISI_COMBAT, 0);
        //int profile_inv_idle = KMCFindVector(profile_invis, PROFILE_INVISI_IDLE, 0);
        int profile_inv_scene = KMCFindVector(profile_invis, PROFILE_INVISI_SCENE, 0);
        int profile_inv_move = KMCFindVector(profile_invis, PROFILE_INVISI_MOVE, 0);
        profile_invisible_status = ProfileInvisibleStat(profile_inv_combat, profile_inv_scene, profile_inv_move);
    }

    void KMCStateManager::Init() {
        actorNPC = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x00013794);
    }

    void KMCStateManager::Reset() { 
        current_state.clear();
    }


    void KMCStateManager::MenuOpenCloseEvent(const RE::MenuOpenCloseEvent* event) {
        if (event != nullptr) {
            std::string menu_name = event->menuName.c_str();
            if (stopping_state.contains(menu_name) && event->opening) {
                current_state.insert(menu_name);
                KMC_LOG("KMCStateManager::MenuOpenCloseEvent true ==> {}", menu_name);
                isStoppingState = true;
            } else if (stopping_state.contains(menu_name) && !event->opening) {
                current_state.erase(menu_name);
                KMC_LOG("KMCStateManager::MenuOpenCloseEvent false ==> {}", menu_name);
                if (current_state.empty()) {
                    isStoppingState = false;
                }
            }
        }
    }
    
    int KMCStateManager::IsInScene() { 
        if (actorNPC == nullptr) return -1;
        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
        if (player == nullptr || !player->Is3DLoaded()) {
            return -3;
        }

        if (scene_exclusion_keywords.size() != 0) {
            if (player->HasKeywordInArray(scene_exclusion_keywords, false)) return 1;
        }

        float isinsceneDetectRange = isinsceneDetectRangeExterior;
        auto cell = player->GetParentCell();
        if (cell != nullptr && cell->IsInteriorCell()) {
            isinsceneDetectRange = isinsceneDetectRangeInterior;
        }

        auto pcell = player->GetParentCell();

        if (pcell == nullptr) {
            // If the cell is not attached, such as during the main menu
            //KMCCT::KMCEventThread::GetSingleton()->forceendanim = true;
            return -3;
        }

        auto processLists = RE::ProcessLists::GetSingleton();
        if (processLists) {
            float detectRangeSq = isinsceneDetectRange * isinsceneDetectRange;
            processLists->ForEachHighActor([&](RE::Actor& act_ref) {
                RE::Actor* act = &act_ref;
                if (act && act->Is3DLoaded() && !act->IsPlayerRef()) {
                    if (act->GetPosition().GetSquaredDistance(player->GetPosition()) <= detectRangeSq) {
                        if (act->HasKeyword(actorNPC) && act->GetCurrentScene() != nullptr) {
                            auto charc = act->As<RE::Character>();
                            if (charc && KMCCT::IsTalking(charc)) {
                                KMC_LOG("[CURRENT SCENE] name {} range {}", act->GetName(), isinsceneDetectRange);

                                KMCCT::KMCWaitTask::GetSingleton()->KMCPushWaitTask(
                                    KMCWaitType::in_scene,
                                    KMCWaitConfigs(inSceneMS, Clock::now(), KMCWaitType::in_scene, true));
                                return RE::BSContainer::ForEachResult::kStop;
                            }
                        }
                    }
                }
                return RE::BSContainer::ForEachResult::kContinue;
            });
        }

        return 0;
    }

    bool KMCStateManager::GetProfileInvisibleState(bool suspend) {
        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();

        if (player == nullptr) return false;

        if (profile_invisible_status.combat) {
            if (player->IsInCombat()) {
                return true;
            }
        }

        //if (profile_invisible_status.idle) {
        //    if (!player->IsMoving()) {
        //        return true;
        //    }
        //}

       if (profile_invisible_status.move) {
            if (player->IsMoving()) {
                return true;
            }
       }

       if (profile_invisible_status.scene) {
           if (suspend) {
               return true;
           }
       }

       return false;
    }

    bool KMCStateManager::GetStoppingState() { 
        return isStoppingState;
    }

    bool KMCStateManager::GetWhetherThereNoState() {
        if (GetStoppingState()) {
            // 話中の場合
            return false;
        }

        std::string cutin_name = KMCContextManager::GetSingleton()->GetPlayerState();
        if (cutin_name == "") {
            return true;
        }

        if (KMCCT::KMCCutin::GetSingleton()->ExistCategory(cutin_name)) {
            return true;
        } else {
            // auto wordとして存在するのでカットイン回避用のcut in名ではないと判断する
            return false;
        }
    }

}