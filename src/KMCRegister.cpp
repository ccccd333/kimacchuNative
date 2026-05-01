#include "KMCRegister.h"
#include "KMCConfig.h"
#include "KMCOAR.h"
#include "KMCSound.h"
#include "KMCPapyrus.h"
#include "KMCEventThread.h"
#include "KMCStateManager.h"
#include "KMCProfile.h"
#include "KMCCutinCondition.h"
#include "KMCGameEventListener.h"
#include "KMCExpression.h"
#include "KMCStorageUtilTracker.h"
#include "KMCPrismaUIBridge.h"
#include "KMCDisplayWordAndTexture.h"
#include "KMCDisplayAddon.h"
#include "KMCContextManager.h"

SINGLETONBODY(KMCCT::KMCRegister)

namespace KMCCT {

    void KMCCT::OnMessageReceived(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
            case SKSE::MessagingInterface::kPostLoad: 

                // この地点ではフォロワーのindex番号は決定しない
                KMCCT::KMCConfig::GetSingleton()->Setup();
                

                KMCCT::KMCCutinCondition::GetSingleton()->Setup();
                
                KMCCT::KMCExpression::GetSingleton()->Setup();

                KMCCT::KMCContextManager::GetSingleton()->Setup();
                
                break;
            case SKSE::MessagingInterface::kDataLoaded:
                // この地点でフォロワーのindexが決定
                KMCCT::KMCConfig::GetSingleton()->Init();
                KMCCT::KMCDisplayAddon::GetSingleton()->Init();
                KMCCT::KMCExpression::GetSingleton()->Prepare();

                KMCPrismaUIBridge::GetSingleton()->Init();

                KMCCT::KMCContextManager::GetSingleton()->Init();

                KMCCT::KMCStateManager::GetSingleton()->Register();

                
                KMCCT::KMCOAR::GetSingleton()->Init();
                KMCCT::KMCSound::GetSingleton()->Init();
                KMCCT::KMCStateManager::GetSingleton()->Init();
                
                StorageUtilTracker::Init();
                KMCCT::KMCCutinCondition::GetSingleton()->Init();

                
                KMCCT::KMCGameEventListener::GetSingleton()->Init();
                KMCCT::KMCEventThread::GetSingleton()->Init();

                break;
            case SKSE::MessagingInterface::kPreLoadGame:  // set reload flag, so we can prevent in papyrus calls of
                                                          // native function untill view get reset by invoking _reset
                KMC_LOG("kPreLoadGame")
                KMCCT::KMCEventThread::GetSingleton()->SetForceEndAnim(true);
                // Wait for animation process to finish.
                //KMCLoadedWidget();
                KMCCT::KMCSetInitFlag();
                KMCCT::KMCOAR::GetSingleton()->Reset();

                KMCCT::KMCProfile::GetSingleton()->KMCResetProfileContainer();

                //KMCCT::KMCEventThread::GetSingleton()->Reset();
                break;
            case SKSE::MessagingInterface::kPostLoadGame:  // for loading existing game
            case SKSE::MessagingInterface::kSaveGame:
                KMC_LOG("kPostLoadGame | kSaveGame")
                                
                break;
        }
    }

    bool KMCCT::PapyrusRegister(RE::BSScript::IVirtualMachine* vm) {
        const bool loc_unhook = KMCConfig::GetSingleton()->IsUnhookEnabled();

    #define REGISTERPAPYRUSFUNC2(name, unhook) \
        { vm->RegisterFunction(#name, "aaaKimachuuCutInScripts_native", KMCCT::name, unhook&& loc_unhook); }

        REGISTERPAPYRUSFUNC2(CutInCreate, true);
        REGISTERPAPYRUSFUNC2(Init, true);
        //REGISTERPAPYRUSFUNC2(SetFHUValues, true);
        REGISTERPAPYRUSFUNC2(MCMSettingChage, true);
        REGISTERPAPYRUSFUNC2(IsAlreadyInited, true);
        REGISTERPAPYRUSFUNC2(GetModifiedContainer, true);
        REGISTERPAPYRUSFUNC2(SetModifiedContainer, true);
        REGISTERPAPYRUSFUNC2(TryShowProfile, true);
        REGISTERPAPYRUSFUNC2(GetStateProfileEvent, true);
        REGISTERPAPYRUSFUNC2(IsCapturedForm, true);
        REGISTERPAPYRUSFUNC2(CacheForms, true);
        REGISTERPAPYRUSFUNC2(GetCachedForms, true);
        REGISTERPAPYRUSFUNC2(IsCachedTagID, true);
        REGISTERPAPYRUSFUNC2(CreateTagID, true);
        REGISTERPAPYRUSFUNC2(GetCachedTagID, true);
        REGISTERPAPYRUSFUNC2(IsCachedModNames, true);
        REGISTERPAPYRUSFUNC2(CreateCacheModNames, true);
        REGISTERPAPYRUSFUNC2(GetCachedeModNames, true);
        REGISTERPAPYRUSFUNC2(GetCutinCondition, true);
        REGISTERPAPYRUSFUNC2(GetCutinConditionNode, true);
        REGISTERPAPYRUSFUNC2(GetPathNumber, true);
        REGISTERPAPYRUSFUNC2(SetIDContainer, true);
        REGISTERPAPYRUSFUNC2(SetEndPapurusExp, true);
        REGISTERPAPYRUSFUNC2(SetFLEndPapurusExp, true);
        

        REGISTERPAPYRUSFUNC2(GetMArray, true);
        REGISTERPAPYRUSFUNC2(GetNArray, true);

        REGISTERPAPYRUSFUNC2(SearchID, true);
        REGISTERPAPYRUSFUNC2(SetResultMCM, true);
        REGISTERPAPYRUSFUNC2(IsUpdateMCM, true);
        REGISTERPAPYRUSFUNC2(SaveKMCMCM, true);


#undef REGISTERPAPYRUSFUNC
        return true;
    }
}