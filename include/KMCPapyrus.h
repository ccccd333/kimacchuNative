#pragma once
#include "KMCEventThread.h"
#include "KMCProfile.h"
#include "KMCStateManager.h"
#include "KMCCutinCondition.h"
#include "KMCExpression.h"

namespace KMCCT {

    class KMCPapyrus {
        SINGLETONHEADER(KMCPapyrus)
    public:
        ~KMCPapyrus() {}

        // form
        bool KMCIsCapturedForm();
        void KMCCacheForms(std::vector<std::string> form_names);
        std::vector<RE::TESForm *> KMCGetCachedForms();

        // tags
        bool KMCIsCachedIdentity();
        void KMCCreateIdentificationByDataAnalysis(std::vector<std::string> container, int start_index,
                                             std::vector<std::string> identitys, std::string identNameMismatch);
        std::vector<std::string> KMCGetCachedIdentificationByDataAnalysis();

        // mod name
        bool KMCIsCachedModNames();
        void KMCCacheModNames(std::vector<std::string> mod_names);
        std::vector<int> KMCGetCachedModNames();

    private:
        std::vector<RE::TESForm *> form_list;
        bool get_first_cforms = true;
        std::vector<std::string> IdentityDatas;
        bool get_init_first_ident = true;
        std::vector<int> ModNames;
        bool get_init_first_mod_names = true;
    };

    inline int IsAlreadyInited(RE::StaticFunctionTag*) { 
        return ::KMCCT::KMCEventThread::GetSingleton()->IsAlreadyInited();
    }

    inline bool Init(RE::StaticFunctionTag*, RE::BSFixedString skyroot, std::vector<float> floatArray) {
        ::KMCCT::KMCEventThread::GetSingleton()->InitWordsAndWidgets(skyroot, floatArray);
        return true;
    }

    inline bool CutInCreate(RE::StaticFunctionTag*, std::vector<RE::BSFixedString> variableArray) {
        //::KMCCT::KMCEventThread::GetSingleton()->CutInCreate(variableArray);
        return true;
    }

    //inline bool SetFHUValues(RE::StaticFunctionTag*, std::vector<float> floatArray) {
    //    ::KMCCT::KMCStateManager::GetSingleton()->SetFHUStatus(floatArray[0], floatArray[1], floatArray[2]);
    //    return true;
    //}

    inline std::vector<std::string> GetModifiedContainer(RE::StaticFunctionTag*) {
        return ::KMCCT::KMCProfile::GetSingleton()->GetModifiedContainer();
    }

    inline void SetModifiedContainer(RE::StaticFunctionTag*, std::vector<std::string> variableArray) {
        ::KMCCT::KMCProfile::GetSingleton()->SetModifiedContainer(variableArray);
    }

    inline bool MCMSettingChage(RE::StaticFunctionTag*, std::vector<float> floatArray) { 
        ::KMCCT::KMCEventThread::GetSingleton()->MCMSettingChange(floatArray);
        return true;
    }

    inline void TryShowProfile(RE::StaticFunctionTag*) { 
        ::KMCCT::KMCProfile::GetSingleton()->TryShowProfile();
    }

    inline int GetStateProfileEvent(RE::StaticFunctionTag*) {
        return ::KMCCT::KMCProfile::GetSingleton()->GetStateProfileEvent();
    }

    inline bool IsCapturedForm(RE::StaticFunctionTag *) { return KMCPapyrus::GetSingleton()->KMCIsCapturedForm(); }

    inline void CacheForms(RE::StaticFunctionTag *, std::vector<std::string> forms) {
        KMCPapyrus::GetSingleton()->KMCCacheForms(forms);
    }

    inline std::vector<RE::TESForm *> GetCachedForms(RE::StaticFunctionTag *) {
        return KMCPapyrus::GetSingleton()->KMCGetCachedForms();
    }

    inline bool IsCachedTagID(RE::StaticFunctionTag *) { return KMCPapyrus::GetSingleton()->KMCIsCachedIdentity(); }

    inline void CreateTagID(RE::StaticFunctionTag *, std::vector<std::string> container,
                                                   int start_index, std::vector<std::string> identitys,
                                                   std::string identNameMismatch) {
        KMCPapyrus::GetSingleton()->KMCCreateIdentificationByDataAnalysis(container, start_index, identitys, identNameMismatch);
    }

    inline std::vector<std::string> GetCachedTagID(RE::StaticFunctionTag *) {
        return KMCPapyrus::GetSingleton()->KMCGetCachedIdentificationByDataAnalysis();
    }

    inline bool IsCachedModNames(RE::StaticFunctionTag *) { return KMCPapyrus::GetSingleton()->KMCIsCachedModNames(); }

    inline void CreateCacheModNames(RE::StaticFunctionTag *, std::vector<std::string> mod_names) {
        KMCPapyrus::GetSingleton()->KMCCacheModNames(mod_names);
    }

    inline std::vector<int> GetCachedeModNames(RE::StaticFunctionTag *) {
        return KMCPapyrus::GetSingleton()->KMCGetCachedModNames();
    }

    inline std::vector<std::string> GetCutinCondition(RE::StaticFunctionTag *) {
        return KMCCutinCondition::GetSingleton()->GetCutinCondition();
    }

    inline std::vector<std::string> GetCutinConditionNode(RE::StaticFunctionTag *, std::string node_name) {
        return KMCCutinCondition::GetSingleton()->GetCutinConditionNode(node_name);
    }

    inline int GetPathNumber(RE::StaticFunctionTag *, std::string root_name,
                                                          std::string node_name) {
        return KMCCutinCondition::GetSingleton()->GetPathNumber(root_name, node_name);
    }
        
    inline void SetIDContainer(RE::StaticFunctionTag *, std::string root_name, std::string node_name,
                                            int index, int option) {
        return KMCCutinCondition::GetSingleton()->SetIDContainer(root_name, node_name, index, option);
    }


    inline std::vector<std::string> GetMArray(RE::StaticFunctionTag *, std::string root_name, int index) {
        return KMCCutinCondition::GetSingleton()->GetMArray(root_name, index);
    }

    inline std::vector<std::string> GetNArray(RE::StaticFunctionTag *, std::string root_name,
                                              std::string node_name,int index) {
        return KMCCutinCondition::GetSingleton()->GetNArray(root_name, node_name, index);
    }

    inline std::vector<std::string> SearchID(RE::StaticFunctionTag *, std::string root_name, std::string work_name,
                                             int option) {
        return KMCCutinCondition::GetSingleton()->SearchID(root_name, work_name, option);
    }

    inline int SetResultMCM(RE::StaticFunctionTag *, std::string root_name, std::string work_name, int option,
                             std::string result_value) {
        return KMCCutinCondition::GetSingleton()->SetResultMCM(root_name, work_name, option, result_value);
    }

    inline bool IsUpdateMCM(RE::StaticFunctionTag *) {
        return KMCCutinCondition::GetSingleton()->IsUpdateMCM();
    }

    inline std::string SaveKMCMCM(RE::StaticFunctionTag *) { return KMCCutinCondition::GetSingleton()->SaveKMCMCM();
    }

    inline void SetEndPapurusExp(RE::StaticFunctionTag *) { 
        return KMCExpression::GetSingleton()->SetEndPapurusExp();
    }

    inline void SetFLEndPapurusExp(RE::StaticFunctionTag *) { return KMCExpression::GetSingleton()->SetFLEndPapurusExp(); }
        //bool CutInCreate(RE::StaticFunctionTag*, std::vector<RE::BSFixedString> variableArray);
        //bool CutInDestroy(RE::StaticFunctionTag*);
}