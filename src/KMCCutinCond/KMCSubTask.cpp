#include <boost/any.hpp>

#include "KMCConfig.h"
#include "KMCCutinCond/KMCSubTask.h"
#include "KMCTempKeywordManager.h"

namespace KMCCT {
    KMCCCKeyword adrm_key;
    std::vector<KMCCustomCondSubTaskHub *> Tasks = {&adrm_key};

    bool GetCheckSubTaskDetail(CutinCondSubType source, std::unique_ptr<KMCCustomCondSubTaskHub> *hub) {
        for (auto t : Tasks) {
            *hub = t->GetDetail(source);
            if (*hub) {
                return true;
            }
        }

        return false;
    }

    bool KMCCCKeyword::Check() { 
        if (sub_task_source.adrm == AdRm::both) {
            int adm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(sub_task_source.ad);
            int rmm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(sub_task_source.rm);
            if ((adm == 1 || adm == 3) || (rmm == 1 || rmm == 2)) {
                return true;
            }
        } else if (sub_task_source.adrm == AdRm::add) {
            int adm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(sub_task_source.ad);
            if ((adm == 1 || adm == 3)) {
                return true;
            }
        } else if (sub_task_source.adrm == AdRm::remove) {
            int rmm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(sub_task_source.rm);
            if ((rmm == 1 || rmm == 2)) {
                return true;
            }
        } else {
            return false;
        }
        return false;

        
    }

    bool KMCCCKeyword::PushTask() {
        if (sub_task_source.adrm == AdRm::both) {
            KMCCT::KMCTempKeywordManager::GetSingleton()->AddTempKeyword(sub_task_source.ad);
            KMCCT::KMCTempKeywordManager::GetSingleton()->RemoveTempKeyword(sub_task_source.rm);
        } else if (sub_task_source.adrm == AdRm::add) {
            KMCCT::KMCTempKeywordManager::GetSingleton()->AddTempKeyword(sub_task_source.ad);
        } else if (sub_task_source.adrm == AdRm::remove) {
            KMCCT::KMCTempKeywordManager::GetSingleton()->RemoveTempKeyword(sub_task_source.rm);
        } else {
            return false;
        }
        return true;
    }
}