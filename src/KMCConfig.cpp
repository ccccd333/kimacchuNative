#include "KMCConfig.h"

#include "KMCEventThread.h"
#include <yaml-cpp/yaml.h>


SINGLETONBODY(KMCCT::KMCConfig)

bool g_enableLog = false;

namespace KMCCT {
    using namespace boost::property_tree;

    void KMCConfig::Setup() {
        try {
            // ファイルパスを指定してロード
            YAML::Node config = YAML::LoadFile("Data\\skse\\plugins\\kimacchuNative.yaml");

            // General: EnableOutputLog: の階層を辿る
            if (config["General"] && config["General"]["EnableOutputLog"]) {
                g_enableLog = config["General"]["EnableOutputLog"].as<bool>();
                _unhook_papyrus = config["General"]["UnhookPapyrus"].as<bool>(true);
            }
        } catch (const std::exception &e) {
            // ファイルがない、またはパースエラー時は安全のためログ無効
            // ここでのエラーは SKSE ログに直接出す
            SKSE::log::error("Failed to load kimacchuNative.yaml: {}", e.what());
            g_enableLog = false;
        }

        // ここから LOG マクロが有効になる
        LOG("KMCConfig: Setup started. Logging is {}", g_enableLog ? "enabled" : "disabled");


        player = RE::PlayerCharacter::GetSingleton();


        if (!SetupJsonSimpleNodes(&ISetting, SETTING_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            ERROR("setting.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IDetectionKeyword, DETECTION_KEYWORD_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            ERROR("DetectionKeyword.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IDetectionFaction, DETECTION_FACTION_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            ERROR("DetectionFaction.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IDetectionMagicEffectKeyword, DETECTION_MAGIC_EFFECT_KEYWORD_FILE_NAME,
                                  JSON_ROOT_KEY_STRING)) {
            ERROR("DetectionMagicEffectKeyword.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IDetectionGlobal, DETECTION_GLOBAL_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            ERROR("DetectionGlobal.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IDetectionStorageUtil, DETECTION_STORAGE_UTIL_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            ERROR("DetectionStorageUtil.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IInvisibleTimingSetting, PROFILE_PATH + "/" + INVISIBLE_TIMING_SETTING_FILE_NAME,
                                  JSON_ROOT_KEY_STRING)) {
            ERROR("Profile InvisibleTimingSetting.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IProfileSoundEffect, PROFILE_PATH + "/" + PROFILE_SOUND_EFFECT_FILE_NAME,
                                  JSON_ROOT_KEY_STRING)) {
            ERROR("Profile SoundEffect.json json description error.");
        }

        ISetup(PLAYER_WORD_PATH, &IConditions);

        ProfilSetup(PLAYER_WORD_PATH, &IWidgetSetting, &ITextSetting, &IProfileText);

        if (!SetupJsonSimpleNodes(&IManagedFollower, MANAGED_FOLLOWER_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            ERROR("ManagedFollower.json json description error.");
        } else {
            int i = 0;
            for (auto [key, value] : IManagedFollower) {
                try {
                    std::vector<std::string> fc = KMCSplit(value, ',');
                    std::string fkey = std::to_string(i + 1);
                    IFollower.push_back(KMCFollower(fc.at(0), fc.at(1)));

                    KMCFollower *target = &(IFollower[i]);
                    ISetup(FOLLOWER_WORD_PATH + fkey, &(target->IConditions));

                    if (!SetupJsonSimpleNodes(&(target->ISpeachTiming),
                                              FOLLOWER_WORD_PATH + fkey + "/" + SPEACH_TIMING_FILE_NAME,
                                              JSON_ROOT_KEY_STRING)) {
                        ERROR("SpeachTiming.json json description error.");
                    }

                    ++i;
                } catch (...) {
                    ERROR("ERROR Setup Follower AutoWordWFConfig etc. The number of elements in the value is wrong.");
                }
            }
        }
    }

    std::string KMCConfig::ISetup(std::string target,std::vector<std::pair<std::string, std::string>> *cond) {

        if (!SetupJsonSimpleNodes(cond, target + "/" + CONDITIONS_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            ERROR("{} ConditionsAndKeywords.json ERROR.", target);
        }

        return target;
    }

    void KMCConfig::ProfilSetup(std::string target, std::vector<std::pair<std::string, std::string>> *pws,
                                std::vector<std::pair<std::string, std::string>> *ts, std::vector<std::string> *pt) {
        // WidgetSetting.json
        if (!SetupJsonSimpleNodes(pws, PROFILE_PATH + "/" + target + "/" + WIDGET_SETTING_FILE_NAME,
                                  JSON_ROOT_KEY_STRING)) {
            ERROR("{} WidgetSetting.json ERROR.", target);
        }

        // TextSetting.json
        if (!SetupJsonSimpleNodes(ts, PROFILE_PATH + "/" + target + "/" + TEXT_SETTING_FILE_NAME,
                                  JSON_ROOT_KEY_STRING)) {
            ERROR("{} TextSetting.json ERROR.", target);
        }

        if (!SetupJsonSimpleArray(pt, PROFILE_PATH + "/" + target + "/" + PROFILE_TEXT_FILE_NAME,
                                  JSON_ROOT_KEY_STRING)) {
            ERROR("{} ProfileText.json ERROR.", target);
        }
    }

    void KMCConfig::Init() {
        player = RE::PlayerCharacter::GetSingleton();

        int i = 0;
        // try {
        for (; i < IFollower.size(); i++) {
            KMCFollower *target = &(IFollower[i]);
            std::string formid = target->formId;
            std::string pluginname = target->pluginName;
            IFollower[i].follower =
                (RE::Actor *)RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(formid, NULL, 16), pluginname);
            IFollower[i].index = i;
        }

        for (i = 0; i < IFollower.size(); i++) {
            for (auto [ckey, cvalue] : IFollower[i].IConditions) {
                try {
                    std::vector<std::string> fcond = KMCSplit(cvalue, ',');
                    long long formid = std::stoll(fcond.at(0), NULL, 16);
                    std::string pluginname = fcond.at(1);

                    LOG("formid {} pluginname {}", formid, pluginname);

                    IFollower[i].IKeywords.push_back(std::make_pair(
                        ckey, (RE::BGSKeyword *)RE::TESDataHandler::GetSingleton()->LookupForm(formid, pluginname)));
                } catch (...) {
                    ERROR("ERROR Setup ConditionsAndKeywords.json. The number of elements in the value is wrong.");
                }
            }
        }
        //} catch (...) {
        // ERROR("ERROR Setup Follower not found. Specify the FormID of the NPC placed in the world space.");
        //}
    }

    bool KMCConfig::SetupJsonSimpleNodes(std::vector<std::pair<std::string, std::string>> *configs,
                                         std::string jsonFileName, std::string rootKeyName) {
        ptree pt;
        try {
            read_json(COMMON_PATH + jsonFileName, pt);
            LOG("JsonFileName = {}", jsonFileName);
            for (auto &&child : pt.get_child(rootKeyName)) {
                const std::string key = child.first;
                std::string keylower = key;
                std::transform(key.begin(), key.end(), keylower.begin(), [](char c) { return std::tolower(c); });

                const ptree &info = child.second;

                if (rootKeyName.compare(JSON_ROOT_KEY_STRING) == 0) {
                    if (boost::optional<std::string> name = info.get_value_optional<std::string>()) {
                        std::string value = name.get();

                        configs->push_back(std::make_pair(keylower, value));
                        LOG(" key = {} value = {}", keylower, value);
                    } else {
                        ERROR(" Bad key or value.");
                        return false;
                    }
                } else if (rootKeyName.compare(JSON_ROOT_KEY_INT) == 0) {
                    if (boost::optional<int> name = info.get_value_optional<int>()) {
                        std::string value = std::to_string(name.get());
                        configs->push_back(std::make_pair(keylower, value));
                        LOG(" key = {} value = {}", keylower, value);
                    } else {
                        ERROR(" Bad key or value.");
                        return false;
                    }
                } else {
                    ERROR(" Bad root key.");
                    return false;
                }
            }
        } catch (...) {
            ERROR("ERROR LOADING");
            return false;
        }

        return true;
    }

    bool KMCConfig::SetupJsonSimpleArray(std::vector<std::string> *configs, std::string jsonFileName,
                                         std::string rootKeyName) {
        ptree pt;
        int row = 1;
        try {
            read_json(COMMON_PATH + jsonFileName, pt);

            ptree pt_array = pt.get_child(rootKeyName);
            LOG("JsonFileName = {}", jsonFileName);
            for (auto it = pt_array.begin(); it != pt_array.end(); ++it) {
                const std::string text = it->second.data();
                configs->push_back(text);
                LOG(" row = {} text = {}", row, text);
                ++row;
            }
        } catch (...) {
            ERROR("ERROR LOADING");
            return false;
        }

        return true;
    }

    bool KMCConfig::SetupJsonNestedNodes(
        std::vector<std::pair<std::string, std::map<std::string, std::string>>> *configs, std::string jsonFileName,
        std::string rootKeyName) {
        ptree pt;
        // ptree npt;
        try {
            read_json(COMMON_PATH + jsonFileName, pt);
            LOG("JsonFileName = {}", jsonFileName);
            for (auto &&child : pt.get_child(rootKeyName)) {
                const std::string key = child.first;

                if (child.second.empty()) {
                    ERROR("nested node empty.");
                    continue;
                }

                for (auto &&childn : child.second) {
                    const ptree &info = childn.second;

                    std::map<std::string, std::string> mp;
                    if (boost::optional<std::string> name = info.get_value_optional<std::string>()) {
                        std::string value = name.get();
                        mp.insert(std::make_pair(childn.first, value));
                        LOG("key = {} nkey = {} value = {}", key, childn.first, value);
                    } else {
                        ERROR(" Bad key or value.");
                        return false;
                    }

                    if (mp.size() != 0) {
                        configs->push_back(std::make_pair(key, mp));
                    }
                }
            }
        } catch (...) {
            ERROR("ERROR LOADING");
            return false;
        }

        return true;
    }
}