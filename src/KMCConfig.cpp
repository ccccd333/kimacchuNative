#include "KMCConfig.h"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "KMCEventThread.h"

SINGLETONBODY(KMCCT::KMCConfig)

bool g_enableLog = false;

namespace KMCCT {
    using namespace boost::property_tree;
    using json = nlohmann::json;

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
        KMC_LOG("KMCConfig: Setup started. Logging is {}", g_enableLog ? "enabled" : "disabled");

        player = RE::PlayerCharacter::GetSingleton();

        if (!SetupJsonSimpleNodes(&kmc_setting, SETTING_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            KMC_ERROR("setting.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IInvisibleTimingSetting, PROFILE_PATH + "/" + INVISIBLE_TIMING_SETTING_FILE_NAME,
                                  JSON_ROOT_KEY_STRING)) {
            KMC_ERROR("Profile InvisibleTimingSetting.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&IProfileSoundEffect, PROFILE_PATH + "/" + PROFILE_SOUND_EFFECT_FILE_NAME,
                                  JSON_ROOT_KEY_STRING)) {
            KMC_ERROR("Profile SoundEffect.json json description error.");
        }

        if (!SetupJsonSimpleNodes(&kmc_managed_followers_p, MANAGED_FOLLOWER_FILE_NAME, JSON_ROOT_KEY_STRING)) {
            KMC_ERROR("ManagedFollower.json json description error.");
        } else {
            int i = 0;
            for (auto [key, value] : kmc_managed_followers_p) {
                try {
                    std::vector<std::string> fc = KMCSplit(value, ',');
                    std::string fkey = std::to_string(i + 1);
                    kmc_managed_followers.push_back(KMCFollower(fc.at(0), fc.at(1)));

                    KMCFollower *target = &(kmc_managed_followers[i]);
                    ParseFollowerBehaviors(COMMON_PATH + FOLLOWER_WORD_PATH + fkey + "/" + FOLLOWER_BEHAVIORS_FILE_NAME,
                                           target);

                    ++i;
                } catch (...) {
                    KMC_ERROR(
                        "ERROR Setup Follower AutoWordWFConfig etc. The number of elements in the value is wrong.");
                }
            }
        }
    }

    bool KMCConfig::ParseFollowerBehaviors(std::string path, KMCFollower *f) {
        std::ifstream stream(path);

        if (!stream.is_open()) {
            throw std::runtime_error("Failed open file. Path ==> " + path);
        }

        if (!json::accept(stream)) {
            throw std::runtime_error("Incorrect json format. Path ==> " + path);
        }

        stream.seekg(0, std::ios::beg);

        json j = json::parse(stream);

        try {
            // 1. restrict_keywords のパース (vector<pair>)
            if (j.contains("restrict_keywords")) {
                for (auto &[key, value] : j["restrict_keywords"].items()) {
                    // key: "181", value: "12B24,SexLab.esm"
                    f->restrict_keywords.push_back({key, value.get<std::string>()});
                }
            }

            if (j.contains("playback_priority")) {
                for (auto &[key, value] : j["playback_priority"].items()) {
                    try {
                        int category_id = std::stoi(key);

                        int p_val = value.is_number() ? value.get<int>() : 0;
                        if (p_val > 0) {
                            f->playback_priority[category_id] = KMCCT::ST_BEFORE;
                        } else {
                            f->playback_priority[category_id] = KMCCT::ST_AFTER;
                        }
                    } catch (const std::exception &e) {
                        KMC_ERROR("Failed to parse priority key: {} - {}", key, e.what());
                    }
                }
            }
        } catch (const std::exception &e) {
            KMC_ERROR("JSON Load Error: {}", e.what());
        }

        return true;
    }

    void KMCConfig::Init() {
        player = RE::PlayerCharacter::GetSingleton();

        int i = 0;
        // try {
        for (; i < kmc_managed_followers.size(); i++) {
            KMCFollower *target = &(kmc_managed_followers[i]);
            std::string ak_formid = target->formId;
            std::string pluginname = target->pluginName;
            try {
                if (auto form =
                        RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(ak_formid, NULL, 16), pluginname)) {
                    if (auto actor = form->As<RE::Actor>()) {
                        kmc_managed_followers[i].follower_handle = actor->GetHandle();
                    }
                }
            } catch (...) {
                KMC_ERROR("Invalid Follower FormID: {}", ak_formid);
            }

            for (auto [ckey, cvalue] : kmc_managed_followers[i].restrict_keywords) {
                try {
                    int category_id = std::stoi(ckey);

                    std::vector<std::string> fcond = KMCSplit(cvalue, ',');
                    long long kw_formid = std::stoll(fcond.at(0), NULL, 16);
                    std::string kw_pluginname = fcond.at(1);

                    KMC_LOG("formid {} pluginname {}", kw_formid, kw_pluginname);
                    auto keyword_form =
                        RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(kw_formid, kw_pluginname);
                    if (keyword_form) {
                        kmc_managed_followers[i].restrict_keywords_map[category_id] = keyword_form;
                    } else {
                        KMC_ERROR("Keyword not found or type mismatch: [Category: {}] [FormID: {}] [Plugin: {}]",
                                  category_id, kw_formid, pluginname);
                    }
                } catch (...) {
                    KMC_ERROR("ERROR Setup ConditionsAndKeywords.json. The number of elements in the value is wrong.");
                }
            }

            kmc_managed_followers[i].index = i;
        }
    }

    bool KMCConfig::SetupJsonSimpleNodes(std::vector<std::pair<std::string, std::string>> *configs,
                                         std::string jsonFileName, std::string rootKeyName) {
        ptree pt;
        try {
            read_json(COMMON_PATH + jsonFileName, pt);
            KMC_LOG("JsonFileName = {}", jsonFileName);
            for (auto &&child : pt.get_child(rootKeyName)) {
                const std::string key = child.first;
                std::string keylower = key;
                std::transform(key.begin(), key.end(), keylower.begin(), [](char c) { return std::tolower(c); });

                const ptree &info = child.second;

                if (rootKeyName.compare(JSON_ROOT_KEY_STRING) == 0) {
                    if (boost::optional<std::string> name = info.get_value_optional<std::string>()) {
                        std::string value = name.get();

                        configs->push_back(std::make_pair(keylower, value));
                        KMC_LOG(" key = {} value = {}", keylower, value);
                    } else {
                        KMC_ERROR(" Bad key or value.");
                        return false;
                    }
                } else if (rootKeyName.compare(JSON_ROOT_KEY_INT) == 0) {
                    if (boost::optional<int> name = info.get_value_optional<int>()) {
                        std::string value = std::to_string(name.get());
                        configs->push_back(std::make_pair(keylower, value));
                        KMC_LOG(" key = {} value = {}", keylower, value);
                    } else {
                        KMC_ERROR(" Bad key or value.");
                        return false;
                    }
                } else {
                    KMC_ERROR(" Bad root key.");
                    return false;
                }
            }
        } catch (...) {
            KMC_ERROR("ERROR LOADING");
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
            KMC_LOG("JsonFileName = {}", jsonFileName);
            for (auto it = pt_array.begin(); it != pt_array.end(); ++it) {
                const std::string text = it->second.data();
                configs->push_back(text);
                KMC_LOG(" row = {} text = {}", row, text);
                ++row;
            }
        } catch (...) {
            KMC_ERROR("ERROR LOADING");
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
            KMC_LOG("JsonFileName = {}", jsonFileName);
            for (auto &&child : pt.get_child(rootKeyName)) {
                const std::string key = child.first;

                if (child.second.empty()) {
                    KMC_ERROR("nested node empty.");
                    continue;
                }

                for (auto &&childn : child.second) {
                    const ptree &info = childn.second;

                    std::map<std::string, std::string> mp;
                    if (boost::optional<std::string> name = info.get_value_optional<std::string>()) {
                        std::string value = name.get();
                        mp.insert(std::make_pair(childn.first, value));
                        KMC_LOG("key = {} nkey = {} value = {}", key, childn.first, value);
                    } else {
                        KMC_ERROR(" Bad key or value.");
                        return false;
                    }

                    if (mp.size() != 0) {
                        configs->push_back(std::make_pair(key, mp));
                    }
                }
            }
        } catch (...) {
            KMC_ERROR("ERROR LOADING");
            return false;
        }

        return true;
    }
}