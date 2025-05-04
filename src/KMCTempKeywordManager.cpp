#include "KMCTempKeywordManager.h"
#include "IWWFunctions.h"

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
            }

            return ok;
        }
    }
}