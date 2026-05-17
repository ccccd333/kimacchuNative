#include "KMCProfile.h"
#include "KMCConfig.h"
#include "KMCEventThread.h"
#include "KMCSound.h"
#include "KMCStateManager.h"
#include "KMCWaitTask.h"
#include "KMCPrismaUIBridge.h"
#include "KMCStorageUtilTracker.h"
#include "KMCContextManager.h"
#include <filesystem>

namespace fs = std::filesystem;

SINGLETONBODY(KMCCT::KMCProfile)

namespace KMCCT {

    void KMCProfile::Init() {
        StrageUtilStartIndex = 0;
        StrageUtilEndIndex = 0;

        try {
            if (!Parse(COMMON_PATH + PROFILE_PATH + "/" + DISPLAY_PROFILE_PATH)) {
                loaded = false;
            }

            if (loaded) {
                mod_end_index = mod_start_index + profil_ex_data.format_id_num;
                is_live_data_only = true;
                if (mod_start_index != mod_end_index) {
                    for (const auto &fm : profil_ex_data.format_maps) {
                        for (const auto &index : fm.placeholder_indices) {
                            if (fm.live_map_keys.contains(index)) {
                                modified_container.push_back(fm.live_map_keys.at(index));
                            } else {
                                modified_container.push_back(fm.format_strings.at(index));
                                is_live_data_only = false;
                            }
                        }
                    }
                }

                if (is_live_data_only) {
                    dummy_modified_container.clear();
                }
            } else {
                KMC_ERROR("[Error] Failed to parse profile configuration JSON. The profile function will be disabled.");
            }
        } catch (std::runtime_error ex) {
            loaded = false;
            KMC_ERROR("ERROR LOADING {}", ex.what());
        }
    }

    std::vector<std::string> KMCProfile::GetModifiedContainer() { 
        if (is_live_data_only) {
            return dummy_modified_container;
        } else {
            return modified_container;
        }
    
    }

    void KMCProfile::SetModifiedContainer(std::vector<std::string> container) {
        {
            std::lock_guard<std::mutex> lock(pr_mtx);

            if (update_prifile) return;
            update_prifile = true;
            if (is_live_data_only) {
                ResultModifiedContainer = modified_container;
            } else {
                ResultModifiedContainer = container;
            }

            //if (BefResultModifiedContainer.size() > 0 && BefResultModifiedContainer == ResultModifiedContainer) {
            //    update_prifile = false;
            //    return;
            //}
            //BefResultModifiedContainer = ResultModifiedContainer;
        }

        KMCCT::wrap_UpdateModifiedContainer(ResultModifiedContainer, StrageUtilEndIndex, mod_start_index, mod_end_index,
                                            profil_ex_data);
    }

    void KMCProfile::UpdateModifiedContainer(std::vector<std::string> *mod_container, int *SUtilEndIndex,
                                             int *ModStIndex, int *ModEnIndex, KMCProfil *profile) {

        if (mod_container->size() <= *ModEnIndex && *ModStIndex != *ModEnIndex) {
            if (!KMCCT::KMCEventThread::GetSingleton()->GetProfileInitEnd()) {
                {
                    std::lock_guard<std::mutex> lock(pr_mtx);
                    ResultModifiedContainer.clear();
                    //BefResultModifiedContainer.clear();
                    update_prifile = false;
                }

                return;
            }

            try {
                json js = json::object();
                std::unordered_map<std::string, std::map<std::string, std::string>> formated_map;
                if (profil_ex_data.format_id_num != mod_container->size()) {
                    KMC_ERROR(
                        "UpdateModifiedContainer: Data count mismatch. Expected (format_id_num): {}, Actual "
                        "(mod_container size): {}",
                        profil_ex_data.format_id_num, mod_container->size());
                } else {
                    int fm_index = 0;
                    for (int i = 0; i < mod_container->size();) {
                        const auto &cross_ref = profile->format_maps.at(fm_index);

                        int j = i;
                        std::string edited_sring = "";
                        for (int index = 0; index < cross_ref.format_strings.size(); index++) {
                            if (std::ranges::contains(cross_ref.placeholder_indices, index)) {
                                std::string tag_or_edited_string = mod_container->at(j);
                                if (tag_or_edited_string.starts_with(LIVE_DATA_PREFIX)) {
                                    // ライブデータの処理
                                    const auto &live = profil_ex_data.live_data.at(cross_ref.live_map_keys.at(index));
                                    MultiTypeValue sov_return_value;
                                    const VMObjectHandleInfo *vmhandle = nullptr;
                                    if (!live.sov_is_null) {
                                        vmhandle = &live.sov.vm_object;
                                    }
                                    if (StorageUtilTracker::GetValue(live.default_value, live.access_key, vmhandle,
                                                                     sov_return_value)) {
                                        bool edited = false;
                                        for (const auto &v : live.pod) {
                                            if (v.sign == KMCInequalitySign::def) {
                                                edited = true;
                                            } else if (JudgeKMCInequalitySign(v.sign, sov_return_value, v.comp_value)) {
                                                edited = true;
                                            }

                                            if (edited) {
                                                if (v.result == ProfileSymbols::PLACEHOLDER_VALUE) {
                                                    edited_sring += sov_return_value.ToStringMV();
                                                } else {
                                                    edited_sring += v.result;
                                                }
                                                break;
                                            }
                                        }

                                        if (!edited) {
                                            edited_sring += "<N/A>";
                                        }
                                    } else {
                                        // StrageUtilから値が取れない場合
                                        KMC_WARN(
                                            "UpdateModifiedContainer: StorageUtil failed to fetch data. "
                                            "Tag: [{}], AccessKey: [{}], VMHandle: {}. Using default value.",
                                            live.tag, live.access_key, live.sov.vm_object.handle);
                                        if (live.default_op_return_value == ProfileSymbols::PLACEHOLDER_VALUE) {
                                            edited_sring += sov_return_value.ToStringMV();
                                        } else {
                                            edited_sring += live.default_op_return_value;
                                        }
                                        
                                    }
                                } else {
                                    edited_sring += mod_container->at(j);
                                }
                                ++j;
                            } else {
                                edited_sring += cross_ref.format_strings.at(index);
                            }
                        }
                        formated_map[cross_ref.id][std::to_string(cross_ref.row)] = edited_sring;

                        i = j;
                        ++fm_index;
                    }
                    js = formated_map;
                    KMCPrismaUIBridge::GetSingleton()->KMCUpdateProfileText(js);
                }
            } catch (std::exception &e) {
                KMC_ERROR("UpdateModifiedContainer Error {}", e.what());
            }
        }

        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            update_prifile = false;
            first_profile_update = true;
        }
    }

    void KMCProfile::KMCResetProfileContainer() {
        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            //BefResultModifiedContainer.clear();
            ResultModifiedContainer.clear();
            first_profile_update = false;
            update_prifile = false;
        }
    }

    void KMCProfile::InterruptProfileEventManager() {
        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            interrupt_show_profile = true;
        }
       
        static long long event_cool_time =
            KMCFindVector(KMCCT::KMCConfig::GetSingleton()->GetKMCSetting(), KMCCT::PROFILE_DELAY_TIME_CONFIG_KEY,
                          KMCCT::INTERRUPT_SHOW_PROFILE_DELAY_TIME) *
            (float)KMCCT::TIME_SCALE_MS;
        static int polling_count = KMCFindVector(KMCCT::KMCConfig::GetSingleton()->GetKMCSetting(),
                                                 KMCCT::PROFILE_POLLING_COUNT_CONFIG_KEY, KMCCT::PROFILE_POLLING_COUNT);
        
        auto *thread = KMCCT::KMCEventThread::GetSingleton();
        if (!thread->IsShuttingDown() && !thread->GetForceEndAnim()) {
            for (int i = 0; i < polling_count; i++) {
                time_point<Clock> start = Clock::now();
                time_point<Clock> end;
                long long dur = 0;
                if (KMCCT::KMCStateManager::GetSingleton()->GetProfileInvisibleState(
                        KMCCT::KMCWaitTask::GetSingleton()->GetWaitFlag())) {
                    break;
                }

                bool sh_p = false;
                bool u_p = false;
                bool fpu = false;

                {
                    std::lock_guard<std::mutex> lock(pr_mtx);
                    sh_p = showing_profile;
                    u_p = update_prifile;
                    fpu = first_profile_update;
                }

                if (!u_p && fpu && !sh_p) {
                    KMC_LOG("Show Profile {} {} {}", u_p, fpu, sh_p);
                    ShowProfile(!switch_disp_profile_flag);
                    switch_disp_profile_flag = !switch_disp_profile_flag;
                    break;
                } else {
                    KMC_WARN("Profile Stack !!!! {} {} {}", u_p, fpu, sh_p);
                }
                // sleep
                while (true) {
                    if (thread->IsShuttingDown() || thread->GetForceEndAnim()) {
                        return;
                    }

                    end = Clock::now();
                    milliseconds diff = duration_cast<milliseconds>(end - start);
                    dur = diff.count();
                    if (dur >= event_cool_time) {
                        break;
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::WHILE_PROFILE_SHOW_WAIT_TIME));
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            interrupt_show_profile = false;
        }
    }
    void KMCProfile::ShowProfile(bool visible) {
    //void KMCProfile::ShowProfile(bool visible, std::string category) {

        auto *thread = KMCCT::KMCEventThread::GetSingleton();

        if (thread->IsShuttingDown() && thread->GetForceEndAnim() &&
            !KMCCT::KMCEventThread::GetSingleton()->GetProfileInitEnd()) {
            return;
        }

        bool show_p = false;
        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            show_p = show_prifile;

            if (show_p && visible) return;
            if (update_prifile) return;
            show_prifile = true;
            showing_profile = true;
        }
        // player only
        if (!loaded) return;
        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
        if (player == nullptr) return;

        //static float text_fade_in_out_time = KMCFindVector(KMCCT::KMCConfig::GetSingleton()->getIProfileAnimTextFade(),
        //                                                   KMCCT::TEXT_FADE_IN_OUT_TIME_SETTING, 1.0f);
        //static float widget_fade_in_out_time = KMCFindVector(
        //    KMCCT::KMCConfig::GetSingleton()->getIProfileAnimTextFade(), KMCCT::WIDGET_FADE_IN_OUT_TIME_SETTING, 1.0f);

        int time = 0;
        //text_fade_in_out_time > widget_fade_in_out_time ? time = text_fade_in_out_time * (float)KMCCT::TIME_SCALE_MS
        //                                                : time = widget_fade_in_out_time * (float)KMCCT::TIME_SCALE_MS;

        if (visible) {
            KMCCT::KMCSound::GetSingleton()->PlayProfileSE(KMCProfileSEType::open, aaaakmcvolum, player);

            // visible
            std::string category = KMCContextManager::GetSingleton()->GetStateForProfile();
            KMCPrismaUIBridge::GetSingleton()->KMCShowProfile(category);
            //KMCPrismaUIBridge::GetSingleton()->KMCShowProfile(category);

            std::this_thread::sleep_for(std::chrono::milliseconds(time));
        } else {
            KMCCT::KMCSound::GetSingleton()->PlayProfileSE(KMCProfileSEType::end, aaaakmcvolum, player);

            // invisible
            KMCPrismaUIBridge::GetSingleton()->KMCHideProfile();

            std::this_thread::sleep_for(std::chrono::milliseconds(time));

            {
                std::lock_guard<std::mutex> lock(pr_mtx);
                show_prifile = false;
            }
        }

        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            showing_profile = false;
        }
    }

    void KMCProfile::TryShowProfile() {
        KMCCT::KMCEventThread::GetSingleton()->TryShowProfile();
    }

    int KMCProfile::GetStateProfileEvent() {
        bool sh_p = false;
        bool up = false;
        bool fpu = false;
        bool isp = false;
        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            sh_p = showing_profile;
            up = update_prifile;
            fpu = first_profile_update;
            isp = interrupt_show_profile;
        }

        //if (!KMCCT::KMCEventThread::GetSingleton()->GetInitFirstFlag()) {
        //    return -4;  // not work iwant widget ng
        //} else 
            if (!KMCCT::KMCEventThread::GetSingleton()->GetEnableProfileFlag()) {
            return -5;  // profile disable
        } else if (KMCCT::KMCEventThread::GetSingleton()->IsShuttingDown() ||
                   !KMCCT::KMCEventThread::GetSingleton()->GetProfileInitEnd()) {
            return -3;  // init now
        } else if (isp || sh_p) {
            return -2;  // interrupt now
        } else if (up) {
            return -1;  // profile update now
        } else if (!fpu) {
            return 0;
        }               // else if (!show_prifile) {
        return 1;  // profile updated
        //} else {
        //    return 0;
        //}
    }

    bool KMCProfile::Parse(std::string path) {
        std::ifstream stream(path);

        if (!stream.is_open()) {
            throw std::runtime_error("Failed open file. Path ==> " + path);
        }

        if (!json::accept(stream)) {
            throw std::runtime_error("Incorrect json format. Path ==> " + path);
        }

        stream.seekg(0, std::ios::beg);

        json j = json::parse(stream);

        /*
        {PlayerSLSValidFreedomLic}みたいなのを蓄積していく。
        linesへstd::unordered_map<id, std::map<row, std::string>>でアクセスする。
        papyrus側で{PlayerSLSValidFreedomLic}を文字列に置き換えた後の値を
        */
        //std::vector<KMCProfileReplaceMap> format_map;
        std::set<std::string> monitor_target;

        for (auto &[key, profile] : j.items()) {
            if (key.empty()) continue;

            char type = key[0];
            if (type == 'T') {
                if (profile.contains("lines") && profile["lines"].is_array()) {
                    int current_row = 0;

                        for (auto &line_item : profile["lines"]) {
                        if (!line_item.is_string()) {
                            current_row++;
                            continue;
                        }

                        std::string original_text = line_item.get<std::string>();
                        profil_ex_data.profile_lines[key][current_row] = original_text;

                        static const std::regex re_placeholder(R"(\{([a-zA-Z0-9]+)\})");
                        std::smatch match;

                        if (!std::regex_search(original_text, match, re_placeholder)) {
                            // {~}中括弧のものがなければそのままテキストのみを保管する
                            current_row++;
                            continue;
                        }

                        KMCProfileReplaceMap rep_map;
                        rep_map.id = key;
                        rep_map.row = current_row;

                        std::string::const_iterator search_start(original_text.cbegin());

                        while (std::regex_search(search_start, original_text.cend(), match, re_placeholder)) {
                            std::string prefix(search_start, match[0].first);
                            if (!prefix.empty()) {
                                rep_map.format_strings.push_back(prefix);
                            }

                            rep_map.placeholder_indices.push_back(static_cast<int>(rep_map.format_strings.size()));
                            rep_map.format_strings.push_back(match[0].str());

                            search_start = match[0].second;
                        }

                        std::string suffix(search_start, original_text.cend());
                        if (!suffix.empty()) {
                            rep_map.format_strings.push_back(suffix);
                        }

                        profil_ex_data.format_id_num += static_cast<int>(rep_map.placeholder_indices.size());
                        profil_ex_data.format_maps.push_back(std::move(rep_map));

                        current_row++;
                    }
                } 
                
                if (profile.contains("bg_path") && profile["bg_path"].is_string()) {
                    std::string bg_path = profile.value("bg_path", "");
                    if (!fs::exists(PRISMA_UI_HTML_PATH + bg_path)) {
                        std::string error_path = PRISMA_UI_HTML_PATH + bg_path;
                        KMC_ERROR("Missing back ground file: {}", error_path);
                        is_missing_file = true;
                    }

                    profil_ex_data.bg_map.emplace(key, bg_path);
                }
            } else if (type == 'D') {
                KMCProfileDrawingData data;

                std::string bg_path = profile.value("bg_path", "");
                if (!fs::exists(PRISMA_UI_HTML_PATH + bg_path)) {
                    std::string error_path = PRISMA_UI_HTML_PATH + bg_path;
                    KMC_ERROR("Missing back ground file: {}", error_path);
                    is_missing_file = true;
                }
                profil_ex_data.bg_map.emplace(key, bg_path);

                data.base_path =  profile.value("base_path", "");

                if (profile.contains("texture_range") && profile["texture_range"].is_object()) {
                    data.start = profile["texture_range"].value("start", 1);
                    data.end = profile["texture_range"].value("end", 1);
                } else {
                    data.start = 1;
                    data.end = 1;
                }

                for (int i = data.start; i <= data.end; i++) {
                    std::string file_path = data.base_path + key + "/" + std::to_string(i) + ".png";
                    if (!fs::exists(PRISMA_UI_HTML_PATH + file_path)) {
                        std::string error_path = PRISMA_UI_HTML_PATH + file_path;
                        KMC_ERROR("Missing file: {}", error_path);
                        is_missing_file = true;
                    }
                }

                // category別texture_range
                if (profile.contains("category") && profile["category"].is_object()) {
                    for (auto& [cat_name, cat_val] : profile["category"].items()) {
                        if (cat_val.contains("texture_range") && cat_val["texture_range"].is_object()) {
                            int cat_start = cat_val["texture_range"].value("start", 1);
                            int cat_end   = cat_val["texture_range"].value("end", 1);
                            data.category_tex_range[cat_name] = {.start = cat_start, .end = cat_end};

                            monitor_target.insert(cat_name);

                            for (int i = cat_start; i <= cat_end; i++) {
                                std::string file_path =
                                    data.base_path + key + "/" + cat_name + "/" + std::to_string(i) + ".png";
                                if (!fs::exists(PRISMA_UI_HTML_PATH + file_path)) {
                                    std::string error_path = PRISMA_UI_HTML_PATH + file_path;
                                    KMC_ERROR("Missing category file: {} cat={}", error_path, cat_name);
                                    is_missing_file = true;
                                }
                            }
                        }
                    }
                }

                profil_ex_data.drawing_data[key] = data;
            }
        }
        bool is_error_strage_util = false;
        if (j.contains("strage_util_tags") && j["strage_util_tags"].is_array()) {

            for (auto &tag_item : j["strage_util_tags"]) {
                KMCProfileStorageUtilLiveData live_item;
                bool error = false;

                std::string tag_name = tag_item.value("tag", "");
                if (tag_name.empty()) continue;

                live_item.tag = tag_name;
                std::string live_key = LIVE_DATA_PREFIX + tag_name;

                live_item.type = tag_item.value("type", "int");
                std::string ref_formid = tag_item.value("ref", "");
                auto spvalue = KMCSplit(ref_formid, ',');
                if (spvalue.size() >= 1) {
                    if (spvalue.at(0) != "null") {
                        auto form = RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        if (form) {
                            live_item.sov.vm_object = StorageUtilTracker::BuildHandleFromStackPointer(form);
                            live_item.sov_is_null = false;
                        } else {
                            is_error_strage_util = true;
                            error = true;
                            KMC_ERROR(
                                "strage_util_tags: Failed to LookupForm for tag [{}]. FormID: {}, Plugin: {}. Check if "
                                "the "
                                "plugin is loaded or FormID is correct.",
                                tag_name, spvalue.at(0), spvalue.at(1));
                        }
                    } else {
                        live_item.sov_is_null = true;
                    }

                    live_item.access_key = tag_item.value("access_key", "");

                    live_item.default_value.SetValue(live_item.type, tag_item.value("default_value", "0"));

                    if (tag_item.contains("rules") && tag_item["rules"].is_array()) {
                        for (auto &rule_item : tag_item["rules"]) {
                            KMCProfileOperatorData op_data;
                            op_data.op = rule_item.value("op", "");
                            op_data.sign = StringToKMCInequalitySign(op_data.op);
                            if (op_data.sign != KMCInequalitySign::unk) {
                                op_data.comp_value.SetValue(live_item.type, rule_item.value("value", ""));

                                op_data.result = rule_item.value("result", "");
                                
                                if (op_data.sign == KMCInequalitySign::def) {
                                    live_item.default_op_return_value = op_data.result;
                                }

                                live_item.pod.push_back(std::move(op_data));
                            } else {
                                KMC_ERROR(
                                    "strage_util_tags: Invalid operator '{}' in tag [{}]. "
                                    "Available operators: '==', '!=', '>', '<', '>=', '<=', 'default'",
                                    op_data.op, tag_name);
                                error = true;
                                is_error_strage_util = true;
                                break;
                            }
                        }
                    } else {
                        // ルール無しはだめ
                        KMC_ERROR(
                            "strage_util_tags: Invalid 'ref' format in tag [{}]. Expected 'FormID,PluginName' "
                            "(e.g. '14,Skyrim.esm'), but got: '{}'",
                            tag_name, ref_formid);
                        error = true;
                        is_error_strage_util = true;
                    }
                    bool found = false;
                    // T01とかのやつのformat用文字列変換クラスをポインタでもらっとく
                    for (auto &prm : profil_ex_data.format_maps) {
                        
                        int post_index = 0;
                        for (const auto &index : prm.placeholder_indices) {
                            if (prm.format_strings.at(index) == tag_name) {
                                found = true;
                                post_index = index;
                                break;
                            }
                        }

                        if (found) {
                            prm.live_map_keys[post_index] = live_key;
                            break;
                        }
                    }

                    if (!found) {
                        KMC_ERROR("Failed to get live key for StorageUtil tag: {}", tag_name);
                        error = true;
                    }
                } else {
                    error = true;
                    is_error_strage_util = true;
                    KMC_ERROR(
                        "strage_util_tags: Missing 'rules' array for tag [{}]. Logic requires at least one comparison "
                        "rule or a 'default' result.",
                        tag_name);
                }

                if (!error) {
                    profil_ex_data.live_data[live_key] = std::move(live_item);
                }
            }
        }

        if (is_missing_file) {
            // 1つでもpngが無ければNG、JS側でエラーになる
            KMC_ERROR("[Error]Some image files could not be loaded. Therefore, the profile function will be disabled.");
            return false;
        }

        if (is_error_strage_util) {
            KMC_ERROR(
                "[StorageUtil] Profile loading failed due to invalid 'strage_util_tags' configuration. Check previous "
                "error logs for specific tag failures.");
            return false;
        }


        KMCContextManager::GetSingleton()->ProfileStateSetup(monitor_target);

        KMCPrismaUIBridge::GetSingleton()->KMCSetupProfile(j);

        return true;
    }
    // レガシー
//    void KMCProfile::ProfileInit(KMCProfil &profil, std::string target,
//                                 std::vector<std::pair<std::string, std::string>> *ws,
//                                 std::vector<std::pair<std::string, std::string>> *ts, std::vector<std::string> *pt) {
//        KMCProfil result;
//        try {
//            for (auto &[key, value] : *ws) {
//                auto spresult = KMCSplit(value, ',');
//                KMCDispConfigs wid;
//                wid.defx = std::stoi(spresult.at(0));
//                wid.defy = std::stoi(spresult.at(1));
//                wid.defsizex = std::stoi(spresult.at(2));
//                wid.defsizey = std::stoi(spresult.at(3));
//                spresult.at(4) == "1" ? wid.widget_visible = true : wid.widget_visible = false;
//                try {
//                    std::string any = spresult.at(5);
//                    if (any != "") {
//                        wid.any1 = any;
//                    } else {
//                        wid.any1 = KMCCT::PROFILE_PICT_NAME;
//                    }
//                } catch (...) {
//                    wid.any1 = KMCCT::PROFILE_PICT_NAME;
//                }
//                result.wids.push_back(wid);
//            }
//        } catch (std::exception &e) {
//            ERROR("{} ProfileInit WidgetSetting Error out of range {}", target, e.what());
//        }
//
//        std::map<int, std::vector<int>> XGroup;
//        try {
//            for (int row = 0; row < pt->size(); row++) {
//                std::string text = pt->at(row);
//
//                std::regex pattern(R"(\{([0-9]|[a-z]|[A-Z])+\})");
//                std::sregex_iterator it(text.begin(), text.end(), pattern);
//                std::sregex_iterator end;
//
//                if (it != end) {
//                    // LOG("match {}", text);
//                    KMCProfilFormatIdMap pfim;
//                    std::string fids = result.format_id_strings;
//                    while (it != end) {
//                        std::string format_id = (*it).str();
//                        pfim.row = row;
//                        pfim.row_string = text;
//                        fids = fids + format_id + ",";
//                        pfim.format_id.push_back(format_id);
//                        result.format_id_num += 1;
//                        result.map_index.push_back(std::make_pair(row, -1));
//                        it++;
//                    }
//                    result.isFormat = true;
//                    result.format_id_strings = fids;
//                    result.format_map.insert(std::make_pair(row, pfim));
//                }
//                result.tids.insert(std::make_pair(row, KMCDispConfigs((int)text.size())));
//                result.row_string.push_back(text);
//            }
//        } catch (std::exception &e) {
//            ERROR("{} ProfileInit ProfileText Error out of range {}", target, e.what());
//            return;
//        }
//
//        try {
//            int group_id = 0;
//            std::set<int> skip_row;
//            std::vector<std::string> bsp;
//
//            auto fill_disp_config = [&](int to_end) {
//                if (bsp.size() == 0) {
//                    return;
//                }
//                int bef_end = std::stoi(bsp.at(7));
//
//                for (int i = bef_end; i < to_end; i++) {
//                    if (skip_row.size() > 0 && skip_row.find(i) != skip_row.end()) {
//                        continue;
//                    }
//                    auto fmprofile = &result.tids.at(i);
//                    fmprofile->defx = std::stoi(bsp.at(0));
//                    fmprofile->defy = std::stoi(bsp.at(1));
//                    fmprofile->font = bsp.at(2);
//                    fmprofile->font_size = std::stoi(bsp.at(3));
//                    fmprofile->r = std::stoi(bsp.at(4));
//                    fmprofile->g = std::stoi(bsp.at(5));
//                    fmprofile->b = std::stoi(bsp.at(6));
//
//                    skip_row.insert(i);
//
//                    if (XGroup.contains(group_id)) {
//                        XGroup.at(group_id).push_back(i);
//                    } else {
//                        XGroup.insert(std::make_pair(group_id, std::vector<int>()));
//                        XGroup.at(group_id).push_back(i);
//                    }
//                }
//            };
//            bool fill = false;
//            for (const auto &[key, value] : *ts) {
//                if (key == PROFILE_SETTING_KEY_NAME_OTHER) {
//                    if (value == "") {
//                        fill = true;
//                    } else {
//                        if (fill) {
//                            fill_disp_config((int)result.tids.size());
//                            fill = false;
//                        }
//
//                        ++group_id;
//
//                        std::vector<std::string> sp = KMCSplit(value, ',');
//                        for (int i = 0; i < result.tids.size(); i++) {
//                            if (skip_row.size() > 0 && skip_row.find(i) != skip_row.end()) {
//                                continue;
//                            }
//
//                            auto fmprofile = &result.tids.at(i);
//                            fmprofile->defx = std::stoi(sp.at(0));
//                            fmprofile->defy = std::stoi(sp.at(1));
//                            fmprofile->font = sp.at(2);
//                            fmprofile->font_size = std::stoi(sp.at(3));
//                            fmprofile->r = std::stoi(sp.at(4));
//                            fmprofile->g = std::stoi(sp.at(5));
//                            fmprofile->b = std::stoi(sp.at(6));
//
//                            if (XGroup.contains(group_id)) {
//                                XGroup.at(group_id).push_back(i);
//                            } else {
//                                XGroup.insert(std::make_pair(group_id, std::vector<int>()));
//                                XGroup.at(group_id).push_back(i);
//                            }
//                        }
//                    }
//                } else {
//                    std::vector<std::string> sp = KMCSplit(value, ',');
//
//                    if (key.contains('-')) {
//                        std::vector<std::string> krange = KMCSplit(key, '-');
//                        int start = std::stoi(krange.at(0)) - 1;
//                        int end = std::stoi(krange.at(1));
//
//                        if (fill) {
//                            fill_disp_config(start);
//                            fill = false;
//                        }
//
//                        ++group_id;
//
//                        for (int i = start; i < end; i++) {
//                            if (result.tids.contains(i)) {
//                                auto tid_setting = &result.tids.at(i);
//                                tid_setting->defx = std::stoi(sp.at(0));
//                                tid_setting->defy = std::stoi(sp.at(1));
//                                tid_setting->font = sp.at(2);
//                                tid_setting->font_size = std::stoi(sp.at(3));
//                                tid_setting->r = std::stoi(sp.at(4));
//                                tid_setting->g = std::stoi(sp.at(5));
//                                tid_setting->b = std::stoi(sp.at(6));
//                                skip_row.insert(i);
//
//                                if (XGroup.contains(group_id)) {
//                                    XGroup.at(group_id).push_back(i);
//                                } else {
//                                    XGroup.insert(std::make_pair(group_id, std::vector<int>()));
//                                    XGroup.at(group_id).push_back(i);
//                                }
//                            }
//                        }
//
//                        bsp = sp;
//                        bsp.push_back(krange.at(1));
//                    } else {
//                        int row = std::stoi(key) - 1;
//
//                        if (fill) {
//                            fill_disp_config(row);
//                            fill = false;
//                        }
//
//                        ++group_id;
//
//                        if (result.tids.contains(row)) {
//                            auto tid_setting = &result.tids.at(row);
//                            tid_setting->defx = std::stoi(sp.at(0));
//                            tid_setting->defy = std::stoi(sp.at(1));
//                            tid_setting->font = sp.at(2);
//                            tid_setting->font_size = std::stoi(sp.at(3));
//                            tid_setting->r = std::stoi(sp.at(4));
//                            tid_setting->g = std::stoi(sp.at(5));
//                            tid_setting->b = std::stoi(sp.at(6));
//                            skip_row.insert(row);
//
//                            if (XGroup.contains(group_id)) {
//                                XGroup.at(group_id).push_back(row);
//                            } else {
//                                XGroup.insert(std::make_pair(group_id, std::vector<int>()));
//                                XGroup.at(group_id).push_back(row);
//                            }
//                        }
//
//                        bsp = sp;
//                        bsp.push_back(key);
//                    }
//                }
//            }
//        } catch (std::exception &e) {
//            ERROR("{} ProfileInit TextSetting.json Error out of range", target, e.what());
//            return;
//        }
//#pragma region Legacy
//        // try {
//        //     int group_id = 0;
//        //     std::set<int> skip_row;
//        //     std::vector<std::string> bsp;
//
//        //    auto fill_disp_pos_config = [&](int to_end) {
//        //        if (bsp.size() == 0) {
//        //            return;
//        //        }
//        //        int bef_end = std::stoi(bsp.at(2));
//        //        int font_size = 0;
//        //        int base_font_size = std::stoi(bsp.at(3));
//        //        int y = std::stoi(bsp.at(1));
//        //        for (int i = bef_end; i < to_end; i++) {
//        //            if (skip_row.size() > 0 && skip_row.find(i) != skip_row.end()) {
//        //                continue;
//        //            }
//        //            auto fmprofile = &result.tids.at(i);
//        //            int befpoint = i - 1;
//        //            if (result.tids.contains(befpoint)) {
//        //                auto tid_setting_pos = &result.tids.at(befpoint);
//        //                if (tid_setting_pos->font_size > fmprofile->font_size) {
//        //                    font_size = tid_setting_pos->font_size;
//        //                } else {
//        //                    font_size = fmprofile->font_size;
//        //                }
//        //            } else {
//        //                font_size = base_font_size;
//        //            }
//
//        //
//        //            fmprofile->defx = std::stoi(bsp.at(0));
//        //            y += font_size;
//        //            fmprofile->defy = y;
//        //            skip_row.insert(i);
//        //            if (XGroup.contains(group_id)) {
//        //                XGroup.at(group_id).push_back(i);
//        //            } else {
//        //                XGroup.insert(std::make_pair(group_id, std::vector<int>()));
//        //                XGroup.at(group_id).push_back(i);
//        //            }
//        //        }
//        //    };
//        //    bool fill = false;
//        //    for (const auto &[key, value] : *ps) {
//        //        if (key == PROFILE_SETTING_KEY_NAME_OTHER) {
//        //            if (value == "") {
//        //                fill = true;
//        //            } else {
//        //                if (fill) {
//        //                    fill_disp_pos_config((int)result.tids.size());
//        //                    fill = false;
//        //                }
//
//        //                ++group_id;
//        //                std::vector<std::string> sp = KMCSplit(value, ',');
//        //                int y = std::stoi(sp.at(1));
//        //                int bef_font_size = 0;
//        //                for (int i = 0; i < result.tids.size(); i++) {
//        //                    if (skip_row.size() > 0 && skip_row.find(i) != skip_row.end()) {
//        //                        continue;
//        //                    }
//        //
//        //                    auto fmprofile = &result.tids.at(i);
//        //                    fmprofile->defx = std::stoi(sp.at(0));
//        //                    if (bef_font_size != 0 && fmprofile->font_size > bef_font_size) {
//        //                        y += fmprofile->font_size;
//        //                    } else {
//        //                        y += bef_font_size;
//        //                    }
//        //
//        //                    fmprofile->defy = y;
//
//        //                    bef_font_size = fmprofile->font_size;
//
//        //                    if (XGroup.contains(group_id)) {
//        //                        XGroup.at(group_id).push_back(i);
//        //                    } else {
//        //                        XGroup.insert(std::make_pair(group_id, std::vector<int>()));
//        //                        XGroup.at(group_id).push_back(i);
//        //                    }
//        //
//        //                }
//        //            }
//        //        } else {
//        //            std::vector<std::string> sp = KMCSplit(value, ',');
//        //            if (key.contains('-')) {
//
//        //                std::vector<std::string> krange = KMCSplit(key, '-');
//        //                int start = std::stoi(krange.at(0)) - 1;
//        //                int end = std::stoi(krange.at(1));
//
//        //                if (fill) {
//        //                    fill_disp_pos_config(start);
//        //                    fill = false;
//        //                }
//
//        //                ++group_id;
//        //                int y = std::stoi(sp.at(1));
//        //                int font_size = 0;
//        //                int last_posy = 0;
//        //                int bef_font_size = 0;
//        //                for (int i = start; i < end; i++) {
//        //                    if (result.tids.contains(i)) {
//        //                        auto tid_setting_pos = &result.tids.at(i);
//        //                        tid_setting_pos->defx = std::stoi(sp.at(0));
//        //                        if (bef_font_size != 0 && tid_setting_pos->font_size > bef_font_size) {
//        //                            y += tid_setting_pos->font_size;
//        //                        } else {
//        //                            y += bef_font_size;
//        //                        }
//
//        //                        tid_setting_pos->defy = y;
//        //                        bef_font_size = tid_setting_pos->font_size;
//
//        //                        font_size = tid_setting_pos->font_size;
//        //                        last_posy = tid_setting_pos->defy;
//        //                        skip_row.insert(i);
//        //                        if (XGroup.contains(group_id)) {
//        //                            XGroup.at(group_id).push_back(i);
//        //                        } else {
//        //                            XGroup.insert(std::make_pair(group_id, std::vector<int>()));
//        //                            XGroup.at(group_id).push_back(i);
//        //                        }
//        //                    }
//        //                }
//
//        //                sp[1] = std::to_string(last_posy);
//        //                bsp = sp;
//        //                bsp.push_back(krange.at(1));
//        //                bsp.push_back(std::to_string(font_size));
//        //            } else {
//        //                int row = std::stoi(key) - 1;
//
//        //                if (fill) {
//        //                    fill_disp_pos_config(row);
//        //                    fill = false;
//        //                }
//
//        //                ++group_id;
//        //                int font_size = 0;
//        //                int last_posy = 0;
//        //                if (result.tids.contains(row)) {
//        //                    auto tid_setting_pos = &result.tids.at(row);
//        //                    tid_setting_pos->defx = std::stoi(sp.at(0));
//        //                    tid_setting_pos->defy = std::stoi(sp.at(1));
//        //                    font_size = tid_setting_pos->font_size;
//        //                    last_posy = tid_setting_pos->defy;
//        //                    skip_row.insert(row);
//        //                    if (XGroup.contains(group_id)) {
//        //                        XGroup.at(group_id).push_back(row);
//        //                    } else {
//        //                        XGroup.insert(std::make_pair(group_id, std::vector<int>()));
//        //                        XGroup.at(group_id).push_back(row);
//        //                    }
//        //                }
//
//        //                sp[1] = std::to_string(last_posy);
//        //                bsp = sp;
//        //                bsp.push_back(key);
//        //                bsp.push_back(std::to_string(font_size));
//        //            }
//        //        }
//        //    }
//        //} catch (std::exception &e) {
//        //    ERROR("{} ProfileInit PositionSetting.json Error out of range {}", target, e.what());
//        //}
//#pragma endregion
//        std::string cat = "";
//        int format_id_num = result.format_id_num;
//        std::string format_id_strings = result.format_id_strings;
//        std::map<int, KMCProfilFormatIdMap> format_map;
//        std::map<int, KMCDispConfigs> tids;
//        std::vector<std::pair<int, int>> map_index;
//
//        int push_index = -1;
//        int bef_row = -1;
//        for (auto &[key, value] : XGroup) {
//            int row = key - 1;
//            int count = 0;
//
//            if (bef_row != row) {
//                ++push_index;
//            }
//
//            for (auto &k : value) {
//                // LOG("XGroup : {} {}", key, k);
//                if (count == 0) {
//                    cat = result.row_string[k];
//                    ++count;
//                } else {
//                    cat = cat + "\n" + result.row_string[k];
//                }
//
//                for (auto it = result.map_index.begin(); it != result.map_index.end(); ++it) {
//                    if (it->first == k) {
//                        map_index.push_back(std::make_pair(k, push_index));
//                    }
//                }
//
//                if (result.format_map.contains(k)) {
//                    if (format_map.contains(push_index)) {
//                        auto rfm = result.format_map[k];
//                        for (int i = 0; i < rfm.format_id.size(); i++) {
//                            format_map[push_index].format_id.push_back(rfm.format_id[i]);
//                        }
//                    } else {
//                        format_map.insert(std::make_pair(push_index, result.format_map[k]));
//                    }
//                }
//
//                if (result.tids.contains(k)) {
//                    if (!tids.contains(push_index)) {
//                        tids.insert(std::make_pair(push_index, result.tids[k]));
//                    }
//                }
//            }
//            format_map[push_index].row_string = cat;
//            profil.row_string.push_back(cat);
//
//            bef_row = row;
//        }
//        sort(map_index.begin(), map_index.end());
//        profil.map_index = map_index;
//        profil.format_id_num = format_id_num;
//        profil.format_id_strings = format_id_strings;
//        profil.format_id_strings_array = KMCSplit(format_id_strings, ',');
//        profil.isFormat = result.isFormat;
//        profil.tids = tids;
//        profil.format_map = format_map;
//        profil.wids = result.wids;
//    }

}