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

#include <IWWPapyrus.h>

SINGLETONBODY(KMCCT::KMCRegister)

namespace KMCCT {

	//std::string GetJContainersPluginName() {
 //       auto patchVersion = REL::Module::get().version().patch();

 //       std::string pluginName{"JContainers64"};
 //       if (REL::Module::IsVR()) {
 //           pluginName = "JContainersVR";
 //       } else if (patchVersion == 659 || patchVersion == 1179) {
 //           pluginName = "JContainersGOG";
 //       }

 //       return pluginName;
 //   }

    void KMCCT::OnMessageReceived(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
            case SKSE::MessagingInterface::kPostLoad: 
                //std::string pluginName = GetJContainersPluginName();
          

                //SKSE::GetMessagingInterface()->RegisterListener(
                //    pluginName.c_str(), [](SKSE::MessagingInterface::Message* a_msg) {
                //        LOG("a_msg={}, msgtype={}", (void*)a_msg,
                //                     a_msg ? a_msg->type : -1);
                //        if (a_msg && a_msg->type == KMCCT::message_root_interface) {
                //            const root_interface* root = root_interface::from_void(a_msg->data);
                //            LOG("root_interface={}", (void*)root);
                //            if (root) {
                //                KMCCT::JCWrapper::GetSingleton()->PreInit(root);
                //            }
                //        }
                //    });
                //}

                IWW::Config::GetSingleton()->Setup();
                KMCCT::KMCConfig::GetSingleton()->Setup();
                KMCCT::KMCCutinCondition::GetSingleton()->Setup();
                KMCCT::KMCExpression::GetSingleton()->Setup();

                break;
            case SKSE::MessagingInterface::kDataLoaded:
                //KMCCT::JCWrapper::GetSingleton()->Init();

                KMCCT::KMCStateManager::GetSingleton()->Register();

                KMCCT::KMCConfig::GetSingleton()->Init();
                KMCCT::KMCOAR::GetSingleton()->Init();
                KMCCT::KMCSound::GetSingleton()->Init();
                KMCCT::KMCStateManager::GetSingleton()->Init();
                KMCCT::KMCEventThread::GetSingleton()->Init();
                KMCCT::KMCProfile::GetSingleton()->Init();
                KMCCT::KMCCutinCondition::GetSingleton()->Init();

                KMCCT::KMCGameEventListener::GetSingleton()->Init();
                KMCCT::KMCExpression::GetSingleton()->Init();
                break;
            case SKSE::MessagingInterface::kPreLoadGame:  // set reload flag, so we can prevent in papyrus calls of
                                                          // native function untill view get reset by invoking _reset
                LOG("kPreLoadGame")
                KMCCT::KMCEventThread::GetSingleton()->forceendanim = true;
                // Wait for animation process to finish.
                //KMCLoadedWidget();
                KMCCT::KMCSetInitFlag();
                KMCCT::KMCOAR::GetSingleton()->Reset();

                KMCCT::KMCProfile::GetSingleton()->KMCResetProfileContainer();
                //while (KMCGetAnimNow()) {}
                
                IWW::MainFunctions::GetSingleton()->SetReseting(true);

                //KMCCT::KMCEventThread::GetSingleton()->Reset();
                break;
            case SKSE::MessagingInterface::kPostLoadGame:  // for loading existing game
            case SKSE::MessagingInterface::kSaveGame:
                // Wait for animation process to finish.
                //KMCCT::KMCEventThread::GetSingleton()->forceendanim = true;
                //while (KMCGetAnimNow()) {}

                IWW::MainFunctions::GetSingleton()->UpdateHud();
                LOG("kPostLoadGame | kSaveGame")
                                
                break;
        }
    }

    bool KMCCT::PapyrusRegister(RE::BSScript::IVirtualMachine* vm) {
        const bool loc_unhook = IWW::Config::GetSingleton()->GetVariable<bool>("General.UnhookPapyrus", true);
#if (PAPYRUSUNHOOKFPSALL == 1)
    #define REGISTERPAPYRUSFUNC(name, unhook) \
        { vm->RegisterFunction(#name, "iwant_widgets_native", IWW::name, loc_unhook); }
#else
    #define REGISTERPAPYRUSFUNC(name, unhook) \
        { vm->RegisterFunction(#name, "iwant_widgets_native", IWW::name, unhook&& loc_unhook); }
    #define REGISTERPAPYRUSFUNC2(name, unhook) \
        { vm->RegisterFunction(#name, "aaaKimachuuCutInScripts_native", KMCCT::name, unhook&& loc_unhook); }
#endif
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

        REGISTERPAPYRUSFUNC(LoadMeter, true)
        REGISTERPAPYRUSFUNC(LoadText, true)
        REGISTERPAPYRUSFUNC(LoadWidget, true)

        REGISTERPAPYRUSFUNC(SetPos, true)
        REGISTERPAPYRUSFUNC(SetPosX, true)
        REGISTERPAPYRUSFUNC(SetPosY, true)
        REGISTERPAPYRUSFUNC(SetSize, true)
        REGISTERPAPYRUSFUNC(SetSizeH, true)
        REGISTERPAPYRUSFUNC(SetSizeW, true)
        REGISTERPAPYRUSFUNC(GetSize, true)
        REGISTERPAPYRUSFUNC(SetZoom, true)
        REGISTERPAPYRUSFUNC(SetZoomX, true)
        REGISTERPAPYRUSFUNC(SetZoomY, true)
        REGISTERPAPYRUSFUNC(SetVisible, true)
        REGISTERPAPYRUSFUNC(SetRotation, true)
        REGISTERPAPYRUSFUNC(SetTransparency, true)
        REGISTERPAPYRUSFUNC(SetRGB, true)
        REGISTERPAPYRUSFUNC(Destroy, true)

        REGISTERPAPYRUSFUNC(SetMeterPercent, true)
        REGISTERPAPYRUSFUNC(SetMeterFillDirection, true)
        REGISTERPAPYRUSFUNC(SendToBack, true)
        REGISTERPAPYRUSFUNC(SendToFront, true)
        REGISTERPAPYRUSFUNC(DoMeterFlash, true)
        REGISTERPAPYRUSFUNC(SetMeterRGB, true)

        REGISTERPAPYRUSFUNC(SetText, true)
        REGISTERPAPYRUSFUNC(AppendText, true)
        REGISTERPAPYRUSFUNC(SwapDepths, true)

        REGISTERPAPYRUSFUNC(DrawShapeLine, true)
        REGISTERPAPYRUSFUNC(DrawShapeCircle, true)
        REGISTERPAPYRUSFUNC(DrawShapeOrbit, true)

        REGISTERPAPYRUSFUNC(DoTransitionByTime, true)

        REGISTERPAPYRUSFUNC(IsHudReady, true)
        REGISTERPAPYRUSFUNC(Reset, true)
        REGISTERPAPYRUSFUNC(IsResetting, true)
        REGISTERPAPYRUSFUNC(GetOutput, true)

#undef REGISTERPAPYRUSFUNC
        return true;
    }
}