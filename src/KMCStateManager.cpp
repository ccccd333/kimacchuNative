#include "KMCStateManager.h"
#include "KMCConfig.h"
#include "KMCEventThread.h"
#include "KMCUtility.h"
#include "KMCWaitTask.h"
#include "KMCCutin.h"
#include "KMCGameEventListener.h"

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

    bool isTravel;
    bool isTravelFar;
    bool isAddItem;
    bool isStoppingState = false;
    long long limit = 0;
    long long inSceneMS = 5000;
    std::string addItemCutinValue = "100";
    float isinsceneDetectRangeExterior = 2000.0f;
    float isinsceneDetectRangeInterior = 1000.0f;
    float fhuVag = 0.0f;
    float fhuAnal = 0.0f;
    float fhuOral = 0.0f;
    std::map<int, StateControll> states;
    RE::BGSKeyword* LocTypeDungeon;
    RE::BGSKeyword* LocTypeClearable;
    std::vector<RE::BGSKeyword*> WeaponKeywordTypeSlash;
    std::vector<RE::BGSKeyword*> WeaponKeywordTypeBlunt;
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
#pragma region State Functions
    std::string STHasKeyword(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->HasKeyword(st.keyword)) {
            return st.cutinName;
        }

        return "";
    }

    std::string STHasMagicEffectKeyword(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        const auto activeEffects = st.actor->GetMagicTarget()->GetActiveEffectList();
        if (activeEffects) {
            for (auto const& ae : *activeEffects) {
                const auto mgef = ae ? ae->GetBaseObject() : nullptr;
                if (mgef != nullptr) {
                    KMC_LOG("Active Effect Name {} search target {}", mgef->GetName(), st.keyword->GetFormEditorID());
                    if (mgef->HasKeyword(st.keyword)) {
                        return st.cutinName;
                    }
                }
            }
        }

        return "";
    }

    std::string STHasFaction(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->IsInFaction(st.faction)) {
            return st.cutinName;
        }

        return "";
    }

    std::string STGlobal(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (JudgeKMCInequalitySign(st.gparam.isign, st.global->value, st.gparam.base_value)) {
            return st.cutinName;
        }

        return "";
    }

    std::string STStrageUtil(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if(!StrageUtilReady.load(std::memory_order_acquire)) {
            return "";
        }

        auto result = KMCCT::KMCStateManager::GetSingleton()->GetStrageUtilResult();
        int offset = st.stuparam.offset;
        if (result->size() <= offset) {
            return "";
        }

        MultiTypeValue diff_value = result->at(offset);
        MultiTypeValue base_value = st.stuparam.base_value;

        if (JudgeKMCInequalitySign(st.stuparam.isign, diff_value, base_value)) {
            return st.cutinName;
        }

        return "";
    }

    //std::string STFhu(int any, State st) {
    //    if (fhuVag + fhuAnal + fhuOral > 0.1f) {
    //        if (fhuVag > 0.1f) {
    //            return "fhu_vag";
    //        } else if (fhuAnal > fhuOral) {
    //            return "fhu_anal";
    //        } else {
    //            return "fhu_oral";
    //        }
    //    }

    //    return "";
    //}

    std::string STFTEnd(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (isTravelFar) {
            isTravelFar = false;
            return "FTEndFar";
        }

        if (isTravel) {
            isTravel = false;
            return "FTEndClose";
        }

        return "";
    }
    //std::string STFTEndClose(int any, State st) {
    //    if (isTravel) {
    //        isTravel = false;
    //        return "FTEndClose";
    //    }

    //    return "";
    //}
    std::string STInjury(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        float nowhealth = st.actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
        float damage = st.actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth) * -1;
        if (damage > nowhealth) {
            return "Injury";
        }

        return "";
    }
    std::string STCombat(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->IsInCombat()) {
            return "Combat";
        }

        return "";
    }
    std::string STSneak(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->IsSneaking()) {
            return "Sneak";
        }

        return "";
    }
    std::string STMount(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->IsOnMount()) {
            return "Mount";
        }

        return "";
    }
    std::string STIdle(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        auto calendar = RE::Calendar::GetSingleton();
        if (calendar != nullptr && calendar->gameHour != nullptr) {
            float hour = calendar->gameHour->value;
            if (hour >= 22) {
                return "IdleMidnight";
            } else if (hour >= 13) {
                return "IdlePM";
            } else if (hour >= 6) {
                return "Idle";
            } else {
                return "IdleMidnight";
            }
        }

        return "";
    }

    std::string STDungeon(int any, State st) { 
        if (st.threshold < any) {
            return "";
        }

        static std::vector<RE::BGSKeyword*> v = {LocTypeClearable, LocTypeDungeon};
        auto cell = st.actor->GetParentCell();
        if (cell != nullptr && cell->IsInteriorCell()) {
            auto loc = st.actor->GetCurrentLocation();
            if (loc != nullptr) {
                if (loc->HasKeywordInArray(v, true)) {
                    return "InDungeon";
                }
            }
        }

        return "";
    }
#pragma endregion

    void KMCStateManager::Register() {

        //auto lambda_function = [this]() { this->Register(); };

        auto setting = KMCCT::KMCConfig::GetSingleton()->getISetting();
        //int interrupt_event_AddItem = KMCFindVector(setting, INTERRUPT_EVENT_ADDITEM, 1);
        int interrupt_event_OnHit = KMCFindVector(setting, INTERRUPT_EVENT_ONHIT, 1);
        limit = KMCFindVector(setting, STATE_MANAGER_CONFIG_KEY, (long long)1) * KMCCT::TIME_SCALE_MS;
        inSceneMS = KMCFindVector(setting, IN_SCENE_CONFIG_KEY, (long long)10) * KMCCT::TIME_SCALE_MS;
        addItemCutinValue = KMCFindVector(setting, ADD_ITEM_CUTIN_VALUE, "0");
        isinsceneDetectRangeExterior = KMCFindVector(setting, ISINSCENE_NPC_DETECT_RANGE_EXTERIOR, 2000.0f);
        isinsceneDetectRangeInterior = KMCFindVector(setting, ISINSCENE_NPC_DETECT_RANGE_INTERIOR, 1000.0f);
        // cutin_chance_fhu = KMCFindVector(setting, CUTIN_CHANCE_FHU, 100);
        cutin_chance_fasttravel = KMCFindVector(setting, CUTIN_CHANCE_FASTTRAVEL, 100);
        cutin_chance_injury = KMCFindVector(setting, CUTIN_CHANCE_INJURY, 100);
        cutin_chance_combat = KMCFindVector(setting, CUTIN_CHANCE_COMBAT, 100);
        cutin_chance_sneak = KMCFindVector(setting, CUTIN_CHANCE_SNEAK, 100);
        cutin_chance_mount = KMCFindVector(setting, CUTIN_CHANCE_MOUNT, 100);
        cutin_chance_isindungeon = KMCFindVector(setting, CUTIN_CHANCE_ISINDUNGEON, 100);
        cutin_chance_idle = KMCFindVector(setting, CUTIN_CHANCE_IDLE, 100);

        //if (interrupt_event_AddItem > 0) {
        //    KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
        //        [this](const RE::TESContainerChangedEvent* event) { this->AddItemEvent(event); });
        //}

        //On<RE::TESDeathEvent>([](const RE::TESDeathEvent* event) {
        //    if (event->actorKiller && event->actorDying) {
        //        LOG("[killer] {}, [dying] {}, [player?] {} [dead] {}", event->actorKiller->GetName(),
        //            event->actorDying->GetName(), event->actorKiller->IsPlayerRef(), event->dead);
        //        
        //    }
        //});

        if (interrupt_event_OnHit > 0) {
            KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
                [this](const RE::TESHitEvent* event) { this->OnHitEvent(event); });

            //On<RE::TESHitEvent>([](const RE::TESHitEvent* event) {
            //    if (event->target != nullptr && event->target->formID == 0x14 &&
            //        !event->flags.any(RE::TESHitEvent::Flag::kHitBlocked)) {
            //        RE::TESForm* root = RE::TESForm::LookupByID(event->source);
            //        if (root != nullptr) {
            //            LOG("TESHitEvent item {} source {} ", root->GetName(), event->source);
            //            std::string power_attack = "0";
            //            if (event->flags.any(RE::TESHitEvent::Flag::kPowerAttack)) {
            //                power_attack = "1";
            //            }

            //            if (root->HasKeywordInArray(WeaponKeywordTypeBlunt, false)) {
            //                KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(KMCInterruptPushCutInData(
            //                    KMCInterruptType::on_hit, "HitBluntWeapon", power_attack, "Power"));
            //            } else if (root->HasKeywordInArray(WeaponKeywordTypeSlash, false)) {
            //                KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(KMCInterruptPushCutInData(
            //                    KMCInterruptType::on_hit, "HitSlashWeapon", power_attack, "Power"));
            //            }
            //        }
            //    } else if (event->target != nullptr && event->target->formID == 0x14 &&
            //               event->flags.any(RE::TESHitEvent::Flag::kHitBlocked)) {
            //        LOG("TESHitEvent Blocked source {} ", event->source);
            //        std::string power_attack = "0";
            //        if (event->flags.any(RE::TESHitEvent::Flag::kPowerAttack)) {
            //            power_attack = "1";
            //        }
            //        KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(
            //            KMCInterruptPushCutInData(KMCInterruptType::on_hit, "HitBlocked", power_attack, "Power"));
            //    }
            //});
        }

        if (cutin_chance_fasttravel > 0) {
            KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
                [this](const RE::TESFastTravelEndEvent* event) { this->FastTravelEndEvent(event); });
            //On<RE::TESFastTravelEndEvent>([](const RE::TESFastTravelEndEvent* event) {
            //    if (event->fastTravelEndHours > 1.5f) {
            //        isTravelFar = true;
            //    } else {
            //        isTravel = true;
            //    }
            //});
        }

        KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
            [this](const RE::MenuOpenCloseEvent* event) { this->MenuOpenCloseEvent(event); });

        //OnUI<RE::MenuOpenCloseEvent>([](const RE::MenuOpenCloseEvent* event) {

        //    if (event != nullptr) {
        //        isStoppingState = event->opening;
        //    }
        //});

        cutin_priority_fasttravel = KMCFindVector(setting, CUTIN_PRIORITY_FASTTRAVEL, 20);
        cutin_priority_injury = KMCFindVector(setting, CUTIN_PRIORITY_INJURY, 30);
        cutin_priority_combat = KMCFindVector(setting, CUTIN_PRIORITY_COMBAT, 40);
        cutin_priority_sneak = KMCFindVector(setting, CUTIN_PRIORITY_SNEAK, 50);
        cutin_priority_mount = KMCFindVector(setting, CUTIN_PRIORITY_MOUNT, 60);
        cutin_priority_isindungeon = KMCFindVector(setting, CUTIN_PRIORITY_ISINDUNGEON, 65);
        cutin_priority_idle = KMCFindVector(setting, CUTIN_PRIORITY_IDLE, 70);
        
        auto profile_invis = KMCCT::KMCConfig::GetSingleton()->getIInvisibleTimingSetting();
        int profile_inv_combat = KMCFindVector(profile_invis, PROFILE_INVISI_COMBAT, 0);
        //int profile_inv_idle = KMCFindVector(profile_invis, PROFILE_INVISI_IDLE, 0);
        int profile_inv_scene = KMCFindVector(profile_invis, PROFILE_INVISI_SCENE, 0);
        int profile_inv_move = KMCFindVector(profile_invis, PROFILE_INVISI_MOVE, 0);
        profile_invisible_status = ProfileInvisibleStat(profile_inv_combat, profile_inv_scene, profile_inv_move);
    }

    void KMCStateManager::Init() {
        actorNPC = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x00013794);
        LocTypeDungeon = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x000130DB);
        LocTypeClearable = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x000F5E80);

        //LocSetCave = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x000130EF);
        //LocSetNordicRuin = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x000130F0);
        //LocSetDwarvenRuin = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x000130F2);

        WeaponKeywordTypeSlash = {
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0001E711, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0001E713, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0006D932, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0006D931, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0001E712, "Skyrim.esm")
        };
        WeaponKeywordTypeBlunt = {
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0006D930, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0001E714, "Skyrim.esm")
        };

        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
        
        states = {
            //{cutin_priority_fhu, StateControll(STFhu, KMCCT::State(player, "FHU", cutin_chance_fhu))},
            {cutin_priority_fasttravel, StateControll(STFTEnd, KMCCT::State(player, "FTEndFar", cutin_chance_fasttravel))},
            {cutin_priority_injury, StateControll(STInjury, KMCCT::State(player, "Injury", cutin_chance_injury))},
            {cutin_priority_combat, StateControll(STCombat, KMCCT::State(player, "Combat", cutin_chance_combat))},
            {cutin_priority_sneak, StateControll(STSneak, KMCCT::State(player, "Sneak", cutin_chance_sneak))},
            {cutin_priority_mount, StateControll(STMount, KMCCT::State(player, "Mount", cutin_chance_mount))},
            {cutin_priority_isindungeon, StateControll(STDungeon, KMCCT::State(player, "InDungeon", cutin_chance_isindungeon))},
            {cutin_priority_idle, StateControll(STIdle, KMCCT::State(player, "Idle", cutin_chance_idle))}
        };

        auto dk = KMCCT::KMCConfig::GetSingleton()->getIDetectionKeyword();
        auto fk = KMCCT::KMCConfig::GetSingleton()->getIDetectionFaction();
        auto mek = KMCCT::KMCConfig::GetSingleton()->getIDetectionMagicEffectKeyword();
        auto glob = KMCCT::KMCConfig::GetSingleton()->getIDetectionGlobal();
        //auto stu = KMCCT::KMCConfig::GetSingleton()->getIDetectionStorageUtil();

        InitDetectionItems(dk, player, STHasKeyword, KMCDetectionType::keyword,
                           {"DetectionKeyword.json Duplicate keyword priority",
                            "DetectionKeyword.json Keyword could not be loaded. Please review form IDs, etc.", 
                            "DetectionKeyword.json", "DetectionKeyword.json error param 6"});
        InitDetectionItems(fk, player, STHasFaction, KMCDetectionType::faction,
                           {"DetectionFaction.json Duplicate keyword priority",
                            "DetectionFaction.json Keyword could not be loaded. Please review form IDs, etc.",
                            "DetectionFaction.json", ""});
        InitDetectionItems(mek, player, STHasMagicEffectKeyword, KMCDetectionType::magic_effect_keyword,
                           {"DetectionMagicEffectKeyword.json Duplicate keyword priority",
                            "DetectionMagicEffectKeyword.json Keyword could not be loaded. Please review form IDs, etc.",
                            "DetectionMagicEffectKeyword.json", ""});
        InitDetectionItems(glob, player, STGlobal, KMCDetectionType::global,
                           {"DetectionGlobal.json Duplicate keyword priority",
                            "DetectionGlobal.json Keyword could not be loaded. Please review form IDs, etc.",
                            "DetectionGlobal.json", "DetectionGlobal.json error param 6 or 7 "});
        //InitDetectionStrageUtil(stu, player);
    }

    void KMCStateManager::Reset() { 
        current_state.clear();
    }

    void KMCStateManager::AddItemEvent(const RE::TESContainerChangedEvent* event) {
        //if (event->newContainer == 0x14 && event->oldContainer != 0x00) {
        //    // If you exit this function, you will not be able to find it. Besides, the pointer reference will
        //    // be broken.
        //    RE::TESForm* root = RE::TESForm::LookupByID(event->baseObj);
        //    if (root != nullptr) {
        //        LOG("TESContainerChangedEvent item {} value {} diff value {}", root->GetName(), root->GetGoldValue(),
        //            addItemCutinValue);
        //        KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(KMCInterruptPushCutInData(
        //            KMCInterruptType::add_item, "AddItem", std::to_string(root->GetGoldValue()), addItemCutinValue));
        //    }
        //}
    }
    void KMCStateManager::OnHitEvent(const RE::TESHitEvent* event) {
        if (event->target != nullptr && event->target->formID == 0x14 &&
            !event->flags.any(RE::TESHitEvent::Flag::kHitBlocked)) {
            RE::TESForm* root = RE::TESForm::LookupByID(event->source);
            if (root != nullptr) {
                KMC_LOG("TESHitEvent item {} source {} ", root->GetName(), event->source);
                std::string power_attack = "0";
                if (event->flags.any(RE::TESHitEvent::Flag::kPowerAttack)) {
                    power_attack = "1";
                }

                if (root->HasKeywordInArray(WeaponKeywordTypeBlunt, false)) {
                    KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(
                        KMCInterruptPushCutInData(KMCInterruptType::on_hit, "HitBluntWeapon", power_attack, "Power"));
                } else if (root->HasKeywordInArray(WeaponKeywordTypeSlash, false)) {
                    KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(
                        KMCInterruptPushCutInData(KMCInterruptType::on_hit, "HitSlashWeapon", power_attack, "Power"));
                }
            }
        } else if (event->target != nullptr && event->target->formID == 0x14 &&
                   event->flags.any(RE::TESHitEvent::Flag::kHitBlocked)) {
            KMC_LOG("TESHitEvent Blocked source {} ", event->source);
            std::string power_attack = "0";
            if (event->flags.any(RE::TESHitEvent::Flag::kPowerAttack)) {
                power_attack = "1";
            }
            KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(
                KMCInterruptPushCutInData(KMCInterruptType::on_hit, "HitBlocked", power_attack, "Power"));
        }
    }
    void KMCStateManager::FastTravelEndEvent(const RE::TESFastTravelEndEvent* event) {
        if (event->fastTravelEndHours > 1.5f) {
            isTravelFar = true;
        } else {
            isTravel = true;
        }
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

        if (keywordsExcludeDuringScene.size() != 0) {
            if (player->HasKeywordInArray(keywordsExcludeDuringScene, false)) return 1;
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

    //std::vector<std::string> KMCStateManager::GetStrageUtilAccessKeys() {
    //    StrageUtilReady = false;
    //    return strage_util_access_map.access_keys;
    //}
    //void KMCStateManager::SetResultStrageUtil(std::vector<std::string> result) { 
    //    StrageUtilReady = false;

    //    if (!BuildMultTypeValues(&result, &strage_util_access_map.results_value_type, &strage_util_access_map.results)) {
    //        ERROR("Attempt to retrieve StrageUtil results failed. Please review DetectionStorageUtil.json.");
    //        return;
    //    }
    //    StrageUtilReady = true;
    //}

    //void KMCStateManager::AllowDialogue(RE::Actor* a_speaker, RE::TESTopic* a_topic) {
    //    using DialogueData = RE::DIALOGUE_DATA::Subtype;
    //    using DialogueType = RE::DIALOGUE_TYPE;

    //    bool isFollower = a_speaker->IsPlayer();
    //    if (!a_speaker->IsPlayer()) {
    //        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
    //        float isinsceneDetectRange = isinsceneDetectRangeExterior;
    //        auto cell = player->GetParentCell();
    //        if (cell != nullptr && cell->IsInteriorCell()) {
    //            isinsceneDetectRange = isinsceneDetectRangeInterior;
    //        }

    //        float dist = a_speaker->data.location.GetDistance(player->data.location);

    //        if (dist < isinsceneDetectRange) {
    //            LOG("[CURRENT SCENE] name {} range {}", a_speaker->GetName(), isinsceneDetectRange);

    //            KMCCT::KMCWaitTask::GetSingleton()->KMCPushWaitTask(
    //                KMCWaitType::in_scene, KMCWaitConfigs(inSceneMS, Clock::now(), KMCWaitType::in_scene, true));
    //            return;
    //        } else {
    //            LOG("[N CURRENT SCENE] name {} range {} dist {}", a_speaker->GetName(), isinsceneDetectRange, dist);
    //        }
    //    }
    //}

    std::string KMCStateManager::GetPlayerState() {
        std::string state = GetHighestPriorityPlayer();

        if (state != "") {
            isTravelFar = false;
            isAddItem = false;
            isTravel = false;
        }

        return state;
    }

    std::string KMCStateManager::GetHighestPriorityPlayer() {
        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();

        if (isStoppingState) {
            return "";
        }

        if (player == nullptr) return "";

        std::random_device rnd;
        std::mt19937 mt(rnd());
        std::uniform_int_distribution<> rand100(1, 100);
        int r = (int)rand100(mt);

        std::string result;

        for (auto [key, value] : states) {
            result = value.func(r, value.state);
            if (result != "") {
                return result;
            }
        }

        return "";
    }

    bool KMCStateManager::GetStoppingState() { 
        return isStoppingState;
    }

    bool KMCStateManager::GetWhetherThereNoState() {
        if (GetStoppingState()) {
            // 話中の場合
            return false;
        }

        std::string cutin_name = GetPlayerState();
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


    void KMCStateManager::InitDetectionItems(std::vector<std::pair<std::string, std::string>>* dconfig,
                                             RE::Actor* actor, StateFunc f, KMCDetectionType type,
                                             std::vector<std::string> messages) {
        for (auto [key, value] : *dconfig) {
            try {
                auto spvalue = KMCSplit(value, ',');

                RE::BGSKeyword* keyword = nullptr;
                // RE::BGSKeyword* magic_effect_keyword = nullptr;
                RE::TESFaction* faction = nullptr;
                RE::TESGlobal* global = nullptr;
                bool found = false;

                switch (type) {
                    case KMCDetectionType::keyword:
                        keyword = (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        keyword != nullptr ? found = true : found = false;
                        break;
                    case KMCDetectionType::faction:
                        faction = (RE::TESFaction*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        faction != nullptr ? found = true : found = false;
                        break;
                    case KMCDetectionType::magic_effect_keyword:
                        keyword = (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        keyword != nullptr ? found = true : found = false;
                        break;
                    case KMCDetectionType::global:
                        global = (RE::TESGlobal*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        global != nullptr ? found = true : found = false;
                        break;
                    default:
                        break;
                }

                if (found) {
                    int priority = std::stoi(spvalue.at(3));
                    if (states.contains(priority)) {
                        KMC_ERROR("{}", messages.at(0));
                    } else {
                        KMCCT::State state =
                            KMCCT::State(actor, spvalue.at(2), std::stoi(spvalue.at(4)), keyword, faction, global);
                        std::string exc_key = "";
                        try {
                            if (type == KMCDetectionType::keyword) {
                                if (keyword != nullptr && std::stoi(spvalue.at(5)) > 0) {
                                    keywordsExcludeDuringScene.push_back(keyword);
                                }
                            } else if (type == KMCDetectionType::global) {
                                if (global != nullptr) {
                                    std::string inequality_sign = spvalue.at(5);
                                    std::string global_diff_value = spvalue.at(6);

                                    state.gparam = KMCTESGlobalParam(StringToKMCInequalitySign(inequality_sign),
                                                                     std::stof(global_diff_value));
                                }                   
                            }
                        } catch (...) {
                            KMC_ERROR("{}", messages.at(3));
                        }

                        states.insert(std::make_pair(priority, StateControll(f, state)));
                    }
                } else {
                    KMC_ERROR("{}", messages.at(1));
                }
            } catch (...) {
                KMC_ERROR("ERROR {} key: {} value : {}", messages.at(2), key, value);
            }
        }
    }

    //void KMCStateManager::InitDetectionStrageUtil(std::vector<std::pair<std::string, std::string>>* dconfig,
    //                                              RE::Actor* actor) {
    //    int offset = 0;
    //    for (auto [key, value] : *dconfig) {
    //        try {
    //            auto spvalue = KMCSplit(value, ',');

    //            int priority = std::stoi(spvalue.at(2));
    //            if (states.contains(priority)) {
    //                ERROR("DetectionGlobal.json Duplicate keyword priority");
    //            } else {
    //                
    //                std::string access_key = spvalue.at(0);
    //                std::string event_name = spvalue.at(1);
    //                int th = std::stoi(spvalue.at(3));

    //                std::string inequality_sign = spvalue.at(4);
    //                std::string value_type = spvalue.at(5);
    //                std::string base_value = spvalue.at(6);

    //                MultiTypeValue mtv;
    //                if (!mtv.SetValue(value_type, base_value)) {
    //                    ERROR("DetectionGlobal.json value type error. Please choose one of the following (int or float or string or long) key: {} value : {} value type : {}", key, value,
    //                          value_type);
    //                    continue;
    //                }

    //                std::string t_mod = mtv.GetMod(PAPYRUS_TYPE_MOD_NAME);

    //                KMCCT::State state = KMCCT::State(actor, event_name, th);
    //                
    //                state.stuparam = KMCStrageUtilParam(StringToKMCInequalitySign(inequality_sign), mtv, offset);

    //                states.insert(std::make_pair(priority, StateControll(STStrageUtil, state)));

    //                strage_util_access_map.access_keys.push_back(t_mod + access_key);
    //                strage_util_access_map.results_value_type.push_back(mtv.value_type);

    //                ++offset;
    //            }
    //        } catch (...) {
    //            ERROR("ERROR DetectionStorageUtil.json key: {} value : {}", key, value);
    //        }
    //    }

    //}
}