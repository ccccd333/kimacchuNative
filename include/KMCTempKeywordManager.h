#pragma once
namespace KMCCT {

    class KMCTempKeywordManager {
        SINGLETONHEADER(KMCTempKeywordManager)
    public:
        bool HasTempKeyword(const std::set<std::string>& sorce, bool a_match_all, std::set<bool> &match);
        bool HasTempKeyword(const std::set<std::string>& sorce, bool a_match_all);
        int HasTempKeyword(const std::set<std::string>& sorce);
        bool AddTempKeyword(const std::set<std::string>& sorce);
        bool RemoveTempKeyword(const std::set<std::string>& sorce);

        void Reset() { 
            temp_keywords.clear();
        }

    private:
        std::set<std::string> temp_keywords;
        std::mutex aaaakmc_temp_keyword_mtx_;
        
    };
}