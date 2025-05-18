#pragma once
namespace KMCCT {

    class KMCTempKeywordManager {
        SINGLETONHEADER(KMCTempKeywordManager)
    public:
        bool HasTempKeyword(const std::set<std::string>& sorce, bool a_match_all, std::set<bool> &match);
        bool HasTempKeyword(const std::set<std::string>& sorce, bool a_match_all);

        bool NHasTempKeyword(const std::set<std::string>& sorce, bool a_nmatch_all);

        int HasTempKeyword(const std::set<std::string>& sorce);
        bool AddTempKeyword(const std::set<std::string>& sorce);
        bool RemoveTempKeyword(const std::set<std::string>& sorce);
        void ToLog();
        void Reset();

    private:
        std::set<std::string> temp_keywords;
        std::mutex aaaakmc_temp_keyword_mtx_;
        
    };
}