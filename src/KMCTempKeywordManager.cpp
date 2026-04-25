#include "KMCTempKeywordManager.h"

SINGLETONBODY(KMCCT::KMCTempKeywordManager)

namespace KMCCT {

    bool KMCTempKeywordManager::HasTempKeyword(const std::set<std::string> &sorce, bool a_match_all,
                                               std::set<bool> &match) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_temp_keyword_mtx_);
            bool m = false;
            bool nm = false;

            for (auto &sk : sorce) {
                if (temp_keywords.contains(sk)) {
                    match.emplace(true);
                    m = true;
                } else {
                    match.emplace(false);
                    nm = true;
                }
            }

            if (a_match_all) {
                if (nm)
                    return false;
                else if (!m)
                    return false;
                else
                    return true;
            } else {
                if (m)
                    return true;
                else
                    return false;
            }
        }
    }

    bool KMCTempKeywordManager::HasTempKeyword(const std::set<std::string> &sorce, bool a_match_all) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_temp_keyword_mtx_);
            bool m = false;
            bool nm = false;

            for (auto &sk : sorce) {
                if (temp_keywords.contains(sk)) {
                    m = true;
                } else {
                    nm = true;
                }
            }

            if (a_match_all) {
                if (nm)
                    return false;
                else if (!m)
                    return false;
                else
                    return true;
            } else {
                if (m)
                    return true;
                else
                    return false;
            }
        }
    }

    bool KMCTempKeywordManager::NHasTempKeyword(const std::set<std::string> &sorce, bool a_nmatch_all) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_temp_keyword_mtx_);
            bool m = false;
            bool nm = false;

            for (auto &sk : sorce) {
                if (temp_keywords.contains(sk)) {
                    m = true;
                } else {
                    nm = true;
                }
            }

            if (a_nmatch_all) {
                if (nm && m)
                    return false;
                else if (m)
                    return false;
                else
                    return true;
            } else {
                if (!nm)
                    return false;
                else
                    return true;
            }
        }
    }

    int KMCTempKeywordManager::HasTempKeyword(const std::set<std::string> &sorce) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_temp_keyword_mtx_);
            bool m = false;
            bool nm = false;

            for (auto &sk : sorce) {
                if (temp_keywords.contains(sk)) {
                    m = true;
                } else {
                    nm = true;
                }
            }

            if (m && nm) {
                return 1;
            } else if (m) {
                return 2;
            } else if (nm) {
                return 3;
            } else {
                return 4;
            }
        }
    }

    bool KMCTempKeywordManager::AddTempKeyword(const std::set<std::string> &sorce) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_temp_keyword_mtx_);
            bool ok = true;
            for (auto &sk : sorce) {
                if (!temp_keywords.contains(sk)) {
                    temp_keywords.emplace(sk);
                } else {
                    ok = false;
                }

                KMC_LOG("[TEMP KEYWORD] ADD {}", sk);
            }

            return ok;
        }
    }
    bool KMCTempKeywordManager::RemoveTempKeyword(const std::set<std::string> &sorce) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_temp_keyword_mtx_);
            bool ok = true;

            for (auto &sk : sorce) {
                if (temp_keywords.contains(sk)) {
                    temp_keywords.erase(sk);
                } else {
                    ok = false;
                }

                KMC_LOG("[TEMP KEYWORD] REMOVE {}", sk);
            }

            return ok;
        }
    }

    void KMCTempKeywordManager::ToLog() {
        for (auto &tk : temp_keywords) {
            KMC_LOG("[TEMP KEYWORD CONTAINER] {}", tk);
        }
    }

    void KMCTempKeywordManager::Reset() { 
        KMC_LOG("[TEMP KEYWORD] RESET");
        temp_keywords.clear(); 
    }
}