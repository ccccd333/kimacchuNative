#pragma once
#include <boost/any.hpp>

namespace KMCCT {
    const std::string CUTIN_CONDITION_INTERVAL_VALUE = "ConditionIntervalValue.json";

    class KMCCJsonValueInterval {
    public:
        KMCCJsonValueInterval() {}
        ~KMCCJsonValueInterval() {}

        void Setup();

        std::string GetInterval(int level, std::string name);
    private :
        bool SetupJsonSimpleNodes(std::string jsonFileName);

    public:
        std::map<std::string, std::string> interval;
        std::set<std::string> checked;
    };

}
