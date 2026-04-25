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
        if (this->sub_task_source.disable_keyword_check) {
            return true;
        }

        if (sub_task_source.adrm == AdRm::both) {
            KMC_LOG("[TEMP_KEYWORD_CHECK] add remove both");
            int adm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(sub_task_source.ad);
            int rmm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(sub_task_source.rm);
            if ((adm == 1 || adm == 3) || (rmm == 1 || rmm == 2)) {
                KMC_LOG("[TEMP_KEYWORD_CHECK] add remove both true");
                return true;
            }

            KMC_LOG("[TEMP_KEYWORD_CHECK] add remove both false ADM ==> {} RMM ==> {}", adm, rmm);
        } else if (sub_task_source.adrm == AdRm::add) {
            KMC_LOG("[TEMP_KEYWORD_CHECK] add");
            int adm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(sub_task_source.ad);
            if ((adm == 1 || adm == 3)) {
                KMC_LOG("[TEMP_KEYWORD_CHECK] add true");
                return true;
            }
            KMC_LOG("[TEMP_KEYWORD_CHECK] add false ADM ==> {}", adm);
        } else if (sub_task_source.adrm == AdRm::remove) {
            KMC_LOG("[TEMP_KEYWORD_CHECK] remove");
            int rmm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(sub_task_source.rm);
            if ((rmm == 1 || rmm == 2)) {
                KMC_LOG("[TEMP_KEYWORD_CHECK] remove true");
                return true;
            }
            KMC_LOG("[TEMP_KEYWORD_CHECK] remove false RMM ==> {}", rmm);
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