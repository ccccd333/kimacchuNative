#include "KMCCutinCond/KMCCJsonValueInterval.h"
#include "KMCConfig.h"
#include <IWWFunctions.h>

namespace KMCCT {
    using namespace boost::property_tree;

    void KMCCJsonValueInterval::Setup() { 
        SetupJsonSimpleNodes(CUTIN_CONDITION_INTERVAL_VALUE);
    }

    std::string KMCCJsonValueInterval::GetInterval(int level, std::string name) {
        std::string path = std::to_string(level) + "." + name;
        if (interval.contains(path)) {
            return interval[path];
        }
        
        ERROR("KMCCJsonValueInterval::GetInterval ERROR [{}]", path);
        return "";
    }

    bool KMCCJsonValueInterval::SetupJsonSimpleNodes(std::string jsonFileName) {
        ptree pt;
        try {
            read_json(COMMON_PATH + jsonFileName, pt);
            LOG("JsonFileName = {}", jsonFileName);
            for (auto& element : pt) {
                const std::string key = element.first;
                auto t = element.second;

                std::string push_value = "";


/*                if (auto vi = t.get_value_optional<int>()) {
                    push_value = vi.get();
                    LOG("KMCCCJsonDefaultValues int key {} value {}", key, boost::any_cast<int>(push_value));
                } else if (auto vf = t.get_value_optional<float>()) {
                    push_value = vf.get();
                    LOG("KMCCCJsonDefaultValues float key {} value {}", key, boost::any_cast<float>(push_value));
                } else if (auto vd = t.get_value_optional<double>()) {
                    push_value = vd.get();
                    LOG("KMCCCJsonDefaultValues double key {} value {}", key, boost::any_cast<double>(push_value));
                } else if (auto vb = t.get_value_optional<bool>()) {
                    push_value = vb.get();
                    LOG("KMCCCJsonDefaultValues bool key {} value {}", key, boost::any_cast<bool>(push_value));
                } else */if (auto vs = t.get_value_optional<std::string>()) {
                    push_value = vs.get();
                    LOG("KMCCJsonValueInterval string key {} value {}", key, push_value);
                } else {
                    ERROR(" Bad value. key {}", key);
                    continue;
                }

                interval.insert(std::make_pair(key, push_value));
            }
        } catch (...) {
            ERROR("ERROR LOADING");
            return false;
        }

        return true;
    }
}