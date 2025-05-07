#include "KMCProfile.h"

#include "KMCConfig.h"
#include "KMCEventThread.h"
#include "KMCSound.h"
#include "KMCStateManager.h"
#include "KMCWaitTask.h"

SINGLETONBODY(KMCCT::KMCProfile)

namespace KMCCT {

    void KMCProfile::Init() {
        StrageUtilStartIndex = 0;
        ModifiedContainer = KMCCT::KMCStateManager::GetSingleton()->GetStrageUtilAccessKeys();
        StrageUtilEndIndex = (int)ModifiedContainer.size() - 1;
        if (StrageUtilEndIndex < 0) StrageUtilEndIndex = 0;

        // Player Profile
        auto pws = KMCCT::KMCConfig::GetSingleton()->getIWidgetSetting();
        auto pts = KMCCT::KMCConfig::GetSingleton()->getITextSetting();
        auto ppt = KMCCT::KMCConfig::GetSingleton()->getIProfileText();

        if (StrageUtilEndIndex == 0) {
            ModStartIndex = 0;
        } else {
            ModStartIndex = StrageUtilEndIndex + 1;
        }

        ProfileInit(PlayerProfil, KMCCT::PLAYER_WORD_PATH, pws, pts, ppt);
        ModEndIndex = ModStartIndex + PlayerProfil.format_id_num;

        if (ModStartIndex != ModEndIndex) {
            std::vector<std::string> sp_format_id_string = KMCSplit(PlayerProfil.format_id_strings, ',');
            for (int i = 0; i < PlayerProfil.format_id_num; i++) {
                try {
                    std::string fid = sp_format_id_string.at(i);
                    ModifiedContainer.push_back(fid);
                } catch (...) {
                    ERROR("KMCEventThread::Init not found format id key : {}", i);
                    ModEndIndex = ModStartIndex + i;
                    break;
                }
            }
        }

        // for (auto &[key, value] : PlayerProfil.tids) {
        //     LOG("KMCEventThread::Init ROW : {} size {}", key, PlayerProfil.tids.size());
        //     LOG("Params x {} y {} font {} fsize {} r {} g {} b {}", value.defx, value.defy, value.font,
        //     value.font_size, value.r, value.g, value.b);
        // }

        // for (auto &[key, value] : PlayerProfil.format_map) {
        //     LOG("KMCEventThread::Init format_map ROW : {}", key);
        //     LOG("Params row {} row_string {} tid {} ", value.row, value.row_string, value.tid);
        // }

        // for (auto &[key, value] : PlayerProfil.map_index) {
        //     LOG("KMCEventThread::Init map_index before : {} after : {}", key, value);
        // }

        // for (auto &value : PlayerProfil.row_string) {
        //     LOG("KMCEventThread::Init row_string value : {}", value);
        // }
        //
        // LOG("KMCEventThread::Init format_id_strings {}", PlayerProfil.format_id_strings);
    }

    void KMCProfile::InitProfile(std::string skyroot, std::vector<float> *floatArray) {
        aaaakmcroot = skyroot;
        aaaakmcvolum = (*floatArray)[0];

        if (PlayerProfil.tids.size() == 0) return;

        if (PlayerProfil.wids.size() != 0) {
            for (auto &values : PlayerProfil.wids) {
                int rwid = -1;
                std::string filename = KMCCT::PICT_PATH1 + "/" + values.any1 + KMCCT::PICT_TYPE;
                if (IsFileExist(KMCCT::PICT_ROOT + filename)) {
                    int wid =
                        IWW::MainFunctions::GetSingleton()->LoadWidget(aaaakmcroot, filename, 10000, 10000, false);
                    rwid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoadNamePlate(wid);

                    values.id = rwid;
                    if (rwid > 0) {
                        IWW::MainFunctions::GetSingleton()->SetPosX(aaaakmcroot, rwid, values.defx);
                        IWW::MainFunctions::GetSingleton()->SetPosY(aaaakmcroot, rwid, values.defy);
                        IWW::MainFunctions::GetSingleton()->SetSizeW(aaaakmcroot, rwid, values.defsizex);
                        IWW::MainFunctions::GetSingleton()->SetSizeH(aaaakmcroot, rwid, values.defsizey);
                    }
                } else {
                    WARN("File path not found. If not intended, no problem. {}", filename);
                }
            }
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
            KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
            return;
        }

        try {
            int row_string_index_count = 0;
            std::map<int, KMCDispConfigs> copy_tids = PlayerProfil.tids;
            for (auto &[key, value] : copy_tids) {
                std::string text = PlayerProfil.row_string.at(row_string_index_count);
                auto tid_form = &(PlayerProfil.tids.at(key));
                KMCProfilFormatIdMap *format_map = nullptr;

                if (PlayerProfil.format_map.contains(key)) {
                    format_map = &(PlayerProfil.format_map.at(key));
                }

                int tid = IWW::MainFunctions::GetSingleton()->LoadText(aaaakmcroot, text, tid_form->font,
                                                                       tid_form->font_size, 10000, 10000, false);
                int rtid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoadNamePlate(tid);
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }
                tid_form->id = rtid;
                if (format_map != nullptr) {
                    format_map->tid = rtid;
                }
                IWW::MainFunctions::GetSingleton()->SetPosX(aaaakmcroot, rtid, tid_form->defx);
                IWW::MainFunctions::GetSingleton()->SetPosY(aaaakmcroot, rtid, tid_form->defy);
                IWW::MainFunctions::GetSingleton()->SetRGB(aaaakmcroot, rtid, tid_form->r, tid_form->g, tid_form->b);
                ++row_string_index_count;
            }
        } catch (std::exception &e) {
            ERROR("InitProfile error {}", e.what());
        }

        // for (int i = 0; i < ppro->tids.size(); i++) {
        //     std::string text = ppro->row_string.at(i);
        //     auto tid_form = &(ppro->tids[i]);
        //     KMCProfilFormatIdMap* format_map = nullptr;
        //
        //     if (ppro->format_map.contains(i)) {
        //         format_map = &(ppro->format_map[i]);
        //     }
        //
        //     int tid = IWW::MainFunctions::GetSingleton()->LoadText(aaaakmcroot, text, tid_form->font,
        //     tid_form->font_size,
        //                                                            10000, 10000, false);
        //     auto lwID = executor.submit(WaitLoadNamePlate, &tid);
        //     lwID.wait();
        //     int rtid = lwID.get();
        //     if (KMCCT::KMCEventThread::GetSingleton()->forceendanim || shutdown) {
        //         return;
        //     }
        //     tid_form->id = rtid;
        //     if (format_map != nullptr) {
        //         format_map->tid = rtid;
        //     }
        //     IWW::MainFunctions::GetSingleton()->SetPosX(aaaakmcroot, rtid, tid_form->defx);
        //     IWW::MainFunctions::GetSingleton()->SetPosY(aaaakmcroot, rtid, tid_form->defy);
        //     IWW::MainFunctions::GetSingleton()->SetRGB(aaaakmcroot, rtid, tid_form->r, tid_form->g, tid_form->b);
        // }
    }

    std::vector<std::string> KMCProfile::GetModifiedContainer() { return ModifiedContainer; }

    void KMCProfile::SetModifiedContainer(std::vector<std::string> container) {
        ResultModifiedContainer = std::move(container);

        if (BefResultModifiedContainer.size() > 0 && BefResultModifiedContainer == ResultModifiedContainer) {
            return;
        }
        BefResultModifiedContainer = ResultModifiedContainer;
        KMCCT::wrap_UpdateModifiedContainer(ResultModifiedContainer, StrageUtilEndIndex, ModStartIndex, ModEndIndex,
                                            PlayerProfil);
    }

    void KMCProfile::UpdateModifiedContainer(std::vector<std::string> *mod_container, int *SUtilEndIndex,
                                             int *ModStIndex, int *ModEnIndex, KMCProfil *profile) {
        int profile_start_index = 0;
        if (*SUtilEndIndex > 0) {
            profile_start_index += *SUtilEndIndex + 1;
            KMCCT::KMCStateManager::GetSingleton()->SetResultStrageUtil(*mod_container);
        } else {
            profile_start_index = 0;
        }
        if (mod_container->size() <= *ModEnIndex && *ModStIndex != *ModEnIndex) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                KMCCT::KMCEventThread::GetSingleton()->GetShutDown() ||
                !KMCCT::KMCEventThread::GetSingleton()->GetProfileInitEnd()) {
                return;
            }
            {
                std::lock_guard<std::mutex> lock(pr_mtx);
                update_prifile = true;
            }
            try {
                auto formmap = profile->format_map;
                std::map<int, KMCUpdateProfileData> formated_map;
                for (int i = profile_start_index; i < mod_container->size(); i++) {
                    std::string value = mod_container->at(i);
                    std::string format_id = profile->format_id_strings_array.at(i - profile_start_index);
                    int row = profile->map_index.at(i - profile_start_index).second;
                    if (formmap.contains(row)) {
                        auto pfm = formmap.at(row);
                        if (formated_map.contains(row)) {
                            std::string formated = formated_map.at(row).format_data;
                            Replace(formated, format_id, value);
                            formated_map.at(row).format_data = formated;
                        } else {
                            formated_map.insert(std::make_pair(row, KMCUpdateProfileData()));
                            std::string formated = pfm.row_string;
                            Replace(formated, format_id, value);
                            formated_map.at(row).format_data = formated;
                            formated_map.at(row).tid = pfm.tid;
                        }
                    }
                }

                for (auto &[key, value] : formated_map) {
                    IWW::MainFunctions::GetSingleton()->SetText(aaaakmcroot, value.tid, value.format_data);
                }
            } catch (std::exception &e) {
                ERROR("UpdateModifiedContainer Error {}", e.what());
            }
        }
        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            update_prifile = false;
        }
        first_profile_update = true;
    }

    void KMCProfile::KMCResetProfileContainer() {
        BefResultModifiedContainer.clear();
        ResultModifiedContainer.clear();
    }

    void KMCProfile::InterruptProfileEventManager() {
        interrupt_show_profile = true;
        static long long event_cool_time =
            KMCFindVector(KMCCT::KMCConfig::GetSingleton()->getISetting(), KMCCT::PROFILE_DELAY_TIME_CONFIG_KEY,
                          KMCCT::INTERRUPT_SHOW_PROFILE_DELAY_TIME) *
            (float)KMCCT::TIME_SCALE_MS;
        static int polling_count = KMCFindVector(KMCCT::KMCConfig::GetSingleton()->getISetting(),
                                                 KMCCT::PROFILE_POLLING_COUNT_CONFIG_KEY, KMCCT::PROFILE_POLLING_COUNT);
        if (!KMCCT::KMCEventThread::GetSingleton()->GetShutDown() &&
            !KMCCT::KMCEventThread::GetSingleton()->forceendanim &&
            KMCCT::KMCEventThread::GetSingleton()->GetProfileInitEnd()) {
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
                {
                    std::lock_guard<std::mutex> lock(pr_mtx);
                    sh_p = showing_profile;
                    u_p = update_prifile;
                }

                if (!u_p && first_profile_update && !sh_p) {
                    ShowProfile(!switch_disp_profile_flag);
                    switch_disp_profile_flag = !switch_disp_profile_flag;
                    break;
                }
                // sleep
                while (true) {
                    if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                        KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
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
        interrupt_show_profile = false;
    }

    void KMCProfile::ShowProfile(bool visible) {
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
        if (PlayerProfil.tids.size() == 0) return;
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player == nullptr) return;

        static float text_fade_in_out_time = KMCFindVector(KMCCT::KMCConfig::GetSingleton()->getIProfileAnimTextFade(),
                                                           KMCCT::TEXT_FADE_IN_OUT_TIME_SETTING, 1.0f);
        static float widget_fade_in_out_time = KMCFindVector(
            KMCCT::KMCConfig::GetSingleton()->getIProfileAnimTextFade(), KMCCT::WIDGET_FADE_IN_OUT_TIME_SETTING, 1.0f);

        int time = 0;
        text_fade_in_out_time > widget_fade_in_out_time ? time = text_fade_in_out_time * (float)KMCCT::TIME_SCALE_MS
                                                        : time = widget_fade_in_out_time * (float)KMCCT::TIME_SCALE_MS;

        if (visible) {
            KMCCT::KMCSound::GetSingleton()->PlayProfileSE(KMCProfileSEType::open, aaaakmcvolum, player);

            // visible
            for (auto &value : PlayerProfil.wids) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }

                if (value.id > 0 && value.widget_visible) {
                    IWW::MainFunctions::GetSingleton()->SetTransparency(aaaakmcroot, value.id, 0);
                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::SET_ALPHA_MS));
                    IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, value.id, true);
                }
            }

            for (auto &[key, value] : PlayerProfil.tids) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }

                if (value.id > 0) {
                    IWW::MainFunctions::GetSingleton()->SetTransparency(aaaakmcroot, value.id, 0);
                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::SET_ALPHA_MS));
                    IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, value.id, true);
                }
            }

            // disp anim
            for (auto &value : PlayerProfil.wids) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }

                if (value.id > 0 && value.widget_visible) {
                    IWW::MainFunctions::GetSingleton()->DoTransitionByTime(
                        aaaakmcroot, value.id, 100, widget_fade_in_out_time, "alpha", "none", "none", 0.0);
                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::SET_ALPHA_MS));
                }
            }

            for (auto &[key, value] : PlayerProfil.tids) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }

                if (value.id > 0) {
                    IWW::MainFunctions::GetSingleton()->DoTransitionByTime(
                        aaaakmcroot, value.id, 100, text_fade_in_out_time, "alpha", "none", "none", 0.0);
                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::SET_ALPHA_MS));
                    time += KMCCT::SET_ALPHA_MS;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(time));
        } else {
            KMCCT::KMCSound::GetSingleton()->PlayProfileSE(KMCProfileSEType::end, aaaakmcvolum, player);

            // invisible anim
            for (auto &value : PlayerProfil.wids) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }

                if (value.id > 0 && value.widget_visible) {
                    IWW::MainFunctions::GetSingleton()->DoTransitionByTime(
                        aaaakmcroot, value.id, 0, widget_fade_in_out_time, "alpha", "none", "none", 0.0);
                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::SET_ALPHA_MS));
                }
            }
            for (auto &[key, value] : PlayerProfil.tids) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }

                if (value.id > 0) {
                    IWW::MainFunctions::GetSingleton()->DoTransitionByTime(
                        aaaakmcroot, value.id, 0, text_fade_in_out_time, "alpha", "none", "none", 0.0);
                    time += KMCCT::SET_ALPHA_MS;
                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::SET_ALPHA_MS));
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(time));

            // invisible
            for (auto &value : PlayerProfil.wids) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }

                if (value.id > 0 && value.widget_visible) {
                    IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, value.id, false);
                }
            }

            for (auto &[key, value] : PlayerProfil.tids) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim ||
                    KMCCT::KMCEventThread::GetSingleton()->GetShutDown()) {
                    return;
                }

                if (value.id > 0) {
                    IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, value.id, false);
                }
            }
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
        if (!interrupt_show_profile) {
            KMCCT::KMCEventThread::GetSingleton()->TryShowProfile();
        }
    }

    int KMCProfile::GetStateProfileEvent() {
        bool sh_p = false;
        {
            std::lock_guard<std::mutex> lock(pr_mtx);
            sh_p = showing_profile;
        }

        if (!KMCCT::KMCEventThread::GetSingleton()->GetInitFirstFlag()) {
            return -4;  // not work iwant widget ng
        } else if (!KMCCT::KMCEventThread::GetSingleton()->GetEnableProfileFlag()) {
            return -5;  // profile disable
        } else if (KMCCT::KMCEventThread::GetSingleton()->GetShutDown() ||
                   !KMCCT::KMCEventThread::GetSingleton()->GetProfileInitEnd()) {
            return -3;  // init now
        } else if (interrupt_show_profile || sh_p) {
            return -2;  // interrupt now
        } else if (update_prifile) {
            return -1;  // profile update now
        } else if (!first_profile_update) {
            return 0;
        }          // else if (!show_prifile) {
        return 1;  // profile updated
        //} else {
        //    return 0;
        //}
    }

    void KMCProfile::ProfileInit(KMCProfil &profil, std::string target,
                                 std::vector<std::pair<std::string, std::string>> *ws,
                                 std::vector<std::pair<std::string, std::string>> *ts, std::vector<std::string> *pt) {
        KMCProfil result;
        try {
            for (auto &[key, value] : *ws) {
                auto spresult = KMCSplit(value, ',');
                KMCDispConfigs wid;
                wid.defx = std::stoi(spresult.at(0));
                wid.defy = std::stoi(spresult.at(1));
                wid.defsizex = std::stoi(spresult.at(2));
                wid.defsizey = std::stoi(spresult.at(3));
                spresult.at(4) == "1" ? wid.widget_visible = true : wid.widget_visible = false;
                try {
                    std::string any = spresult.at(5);
                    if (any != "") {
                        wid.any1 = any;
                    } else {
                        wid.any1 = KMCCT::PROFILE_PICT_NAME;
                    }
                } catch (...) {
                    wid.any1 = KMCCT::PROFILE_PICT_NAME;
                }
                result.wids.push_back(wid);
            }
        } catch (std::exception &e) {
            ERROR("{} ProfileInit WidgetSetting Error out of range {}", target, e.what());
        }

        std::map<int, std::vector<int>> XGroup;
        try {
            for (int row = 0; row < pt->size(); row++) {
                std::string text = pt->at(row);

                std::regex pattern(R"(\{([0-9]|[a-z]|[A-Z])+\})");
                std::sregex_iterator it(text.begin(), text.end(), pattern);
                std::sregex_iterator end;

                if (it != end) {
                    // LOG("match {}", text);
                    KMCProfilFormatIdMap pfim;
                    std::string fids = result.format_id_strings;
                    while (it != end) {
                        std::string format_id = (*it).str();
                        pfim.row = row;
                        pfim.row_string = text;
                        fids = fids + format_id + ",";
                        pfim.format_id.push_back(format_id);
                        result.format_id_num += 1;
                        result.map_index.push_back(std::make_pair(row, -1));
                        it++;
                    }
                    result.isFormat = true;
                    result.format_id_strings = fids;
                    result.format_map.insert(std::make_pair(row, pfim));
                }
                result.tids.insert(std::make_pair(row, KMCDispConfigs((int)text.size())));
                result.row_string.push_back(text);
            }
        } catch (std::exception &e) {
            ERROR("{} ProfileInit ProfileText Error out of range {}", target, e.what());
            return;
        }

        try {
            int group_id = 0;
            std::set<int> skip_row;
            std::vector<std::string> bsp;

            auto fill_disp_config = [&](int to_end) {
                if (bsp.size() == 0) {
                    return;
                }
                int bef_end = std::stoi(bsp.at(7));

                for (int i = bef_end; i < to_end; i++) {
                    if (skip_row.size() > 0 && skip_row.find(i) != skip_row.end()) {
                        continue;
                    }
                    auto fmprofile = &result.tids.at(i);
                    fmprofile->defx = std::stoi(bsp.at(0));
                    fmprofile->defy = std::stoi(bsp.at(1));
                    fmprofile->font = bsp.at(2);
                    fmprofile->font_size = std::stoi(bsp.at(3));
                    fmprofile->r = std::stoi(bsp.at(4));
                    fmprofile->g = std::stoi(bsp.at(5));
                    fmprofile->b = std::stoi(bsp.at(6));

                    skip_row.insert(i);

                    if (XGroup.contains(group_id)) {
                        XGroup.at(group_id).push_back(i);
                    } else {
                        XGroup.insert(std::make_pair(group_id, std::vector<int>()));
                        XGroup.at(group_id).push_back(i);
                    }
                }
            };
            bool fill = false;
            for (const auto &[key, value] : *ts) {
                if (key == PROFILE_SETTING_KEY_NAME_OTHER) {
                    if (value == "") {
                        fill = true;
                    } else {
                        if (fill) {
                            fill_disp_config((int)result.tids.size());
                            fill = false;
                        }

                        ++group_id;

                        std::vector<std::string> sp = KMCSplit(value, ',');
                        for (int i = 0; i < result.tids.size(); i++) {
                            if (skip_row.size() > 0 && skip_row.find(i) != skip_row.end()) {
                                continue;
                            }

                            auto fmprofile = &result.tids.at(i);
                            fmprofile->defx = std::stoi(sp.at(0));
                            fmprofile->defy = std::stoi(sp.at(1));
                            fmprofile->font = sp.at(2);
                            fmprofile->font_size = std::stoi(sp.at(3));
                            fmprofile->r = std::stoi(sp.at(4));
                            fmprofile->g = std::stoi(sp.at(5));
                            fmprofile->b = std::stoi(sp.at(6));

                            if (XGroup.contains(group_id)) {
                                XGroup.at(group_id).push_back(i);
                            } else {
                                XGroup.insert(std::make_pair(group_id, std::vector<int>()));
                                XGroup.at(group_id).push_back(i);
                            }
                        }
                    }
                } else {
                    std::vector<std::string> sp = KMCSplit(value, ',');

                    if (key.contains('-')) {
                        std::vector<std::string> krange = KMCSplit(key, '-');
                        int start = std::stoi(krange.at(0)) - 1;
                        int end = std::stoi(krange.at(1));

                        if (fill) {
                            fill_disp_config(start);
                            fill = false;
                        }

                        ++group_id;

                        for (int i = start; i < end; i++) {
                            if (result.tids.contains(i)) {
                                auto tid_setting = &result.tids.at(i);
                                tid_setting->defx = std::stoi(sp.at(0));
                                tid_setting->defy = std::stoi(sp.at(1));
                                tid_setting->font = sp.at(2);
                                tid_setting->font_size = std::stoi(sp.at(3));
                                tid_setting->r = std::stoi(sp.at(4));
                                tid_setting->g = std::stoi(sp.at(5));
                                tid_setting->b = std::stoi(sp.at(6));
                                skip_row.insert(i);

                                if (XGroup.contains(group_id)) {
                                    XGroup.at(group_id).push_back(i);
                                } else {
                                    XGroup.insert(std::make_pair(group_id, std::vector<int>()));
                                    XGroup.at(group_id).push_back(i);
                                }
                            }
                        }

                        bsp = sp;
                        bsp.push_back(krange.at(1));
                    } else {
                        int row = std::stoi(key) - 1;

                        if (fill) {
                            fill_disp_config(row);
                            fill = false;
                        }

                        ++group_id;

                        if (result.tids.contains(row)) {
                            auto tid_setting = &result.tids.at(row);
                            tid_setting->defx = std::stoi(sp.at(0));
                            tid_setting->defy = std::stoi(sp.at(1));
                            tid_setting->font = sp.at(2);
                            tid_setting->font_size = std::stoi(sp.at(3));
                            tid_setting->r = std::stoi(sp.at(4));
                            tid_setting->g = std::stoi(sp.at(5));
                            tid_setting->b = std::stoi(sp.at(6));
                            skip_row.insert(row);

                            if (XGroup.contains(group_id)) {
                                XGroup.at(group_id).push_back(row);
                            } else {
                                XGroup.insert(std::make_pair(group_id, std::vector<int>()));
                                XGroup.at(group_id).push_back(row);
                            }
                        }

                        bsp = sp;
                        bsp.push_back(key);
                    }
                }
            }
        } catch (std::exception &e) {
            ERROR("{} ProfileInit TextSetting.json Error out of range", target, e.what());
            return;
        }
#pragma region Legacy
        // try {
        //     int group_id = 0;
        //     std::set<int> skip_row;
        //     std::vector<std::string> bsp;

        //    auto fill_disp_pos_config = [&](int to_end) {
        //        if (bsp.size() == 0) {
        //            return;
        //        }
        //        int bef_end = std::stoi(bsp.at(2));
        //        int font_size = 0;
        //        int base_font_size = std::stoi(bsp.at(3));
        //        int y = std::stoi(bsp.at(1));
        //        for (int i = bef_end; i < to_end; i++) {
        //            if (skip_row.size() > 0 && skip_row.find(i) != skip_row.end()) {
        //                continue;
        //            }
        //            auto fmprofile = &result.tids.at(i);
        //            int befpoint = i - 1;
        //            if (result.tids.contains(befpoint)) {
        //                auto tid_setting_pos = &result.tids.at(befpoint);
        //                if (tid_setting_pos->font_size > fmprofile->font_size) {
        //                    font_size = tid_setting_pos->font_size;
        //                } else {
        //                    font_size = fmprofile->font_size;
        //                }
        //            } else {
        //                font_size = base_font_size;
        //            }

        //
        //            fmprofile->defx = std::stoi(bsp.at(0));
        //            y += font_size;
        //            fmprofile->defy = y;
        //            skip_row.insert(i);
        //            if (XGroup.contains(group_id)) {
        //                XGroup.at(group_id).push_back(i);
        //            } else {
        //                XGroup.insert(std::make_pair(group_id, std::vector<int>()));
        //                XGroup.at(group_id).push_back(i);
        //            }
        //        }
        //    };
        //    bool fill = false;
        //    for (const auto &[key, value] : *ps) {
        //        if (key == PROFILE_SETTING_KEY_NAME_OTHER) {
        //            if (value == "") {
        //                fill = true;
        //            } else {
        //                if (fill) {
        //                    fill_disp_pos_config((int)result.tids.size());
        //                    fill = false;
        //                }

        //                ++group_id;
        //                std::vector<std::string> sp = KMCSplit(value, ',');
        //                int y = std::stoi(sp.at(1));
        //                int bef_font_size = 0;
        //                for (int i = 0; i < result.tids.size(); i++) {
        //                    if (skip_row.size() > 0 && skip_row.find(i) != skip_row.end()) {
        //                        continue;
        //                    }
        //
        //                    auto fmprofile = &result.tids.at(i);
        //                    fmprofile->defx = std::stoi(sp.at(0));
        //                    if (bef_font_size != 0 && fmprofile->font_size > bef_font_size) {
        //                        y += fmprofile->font_size;
        //                    } else {
        //                        y += bef_font_size;
        //                    }
        //
        //                    fmprofile->defy = y;

        //                    bef_font_size = fmprofile->font_size;

        //                    if (XGroup.contains(group_id)) {
        //                        XGroup.at(group_id).push_back(i);
        //                    } else {
        //                        XGroup.insert(std::make_pair(group_id, std::vector<int>()));
        //                        XGroup.at(group_id).push_back(i);
        //                    }
        //
        //                }
        //            }
        //        } else {
        //            std::vector<std::string> sp = KMCSplit(value, ',');
        //            if (key.contains('-')) {

        //                std::vector<std::string> krange = KMCSplit(key, '-');
        //                int start = std::stoi(krange.at(0)) - 1;
        //                int end = std::stoi(krange.at(1));

        //                if (fill) {
        //                    fill_disp_pos_config(start);
        //                    fill = false;
        //                }

        //                ++group_id;
        //                int y = std::stoi(sp.at(1));
        //                int font_size = 0;
        //                int last_posy = 0;
        //                int bef_font_size = 0;
        //                for (int i = start; i < end; i++) {
        //                    if (result.tids.contains(i)) {
        //                        auto tid_setting_pos = &result.tids.at(i);
        //                        tid_setting_pos->defx = std::stoi(sp.at(0));
        //                        if (bef_font_size != 0 && tid_setting_pos->font_size > bef_font_size) {
        //                            y += tid_setting_pos->font_size;
        //                        } else {
        //                            y += bef_font_size;
        //                        }

        //                        tid_setting_pos->defy = y;
        //                        bef_font_size = tid_setting_pos->font_size;

        //                        font_size = tid_setting_pos->font_size;
        //                        last_posy = tid_setting_pos->defy;
        //                        skip_row.insert(i);
        //                        if (XGroup.contains(group_id)) {
        //                            XGroup.at(group_id).push_back(i);
        //                        } else {
        //                            XGroup.insert(std::make_pair(group_id, std::vector<int>()));
        //                            XGroup.at(group_id).push_back(i);
        //                        }
        //                    }
        //                }

        //                sp[1] = std::to_string(last_posy);
        //                bsp = sp;
        //                bsp.push_back(krange.at(1));
        //                bsp.push_back(std::to_string(font_size));
        //            } else {
        //                int row = std::stoi(key) - 1;

        //                if (fill) {
        //                    fill_disp_pos_config(row);
        //                    fill = false;
        //                }

        //                ++group_id;
        //                int font_size = 0;
        //                int last_posy = 0;
        //                if (result.tids.contains(row)) {
        //                    auto tid_setting_pos = &result.tids.at(row);
        //                    tid_setting_pos->defx = std::stoi(sp.at(0));
        //                    tid_setting_pos->defy = std::stoi(sp.at(1));
        //                    font_size = tid_setting_pos->font_size;
        //                    last_posy = tid_setting_pos->defy;
        //                    skip_row.insert(row);
        //                    if (XGroup.contains(group_id)) {
        //                        XGroup.at(group_id).push_back(row);
        //                    } else {
        //                        XGroup.insert(std::make_pair(group_id, std::vector<int>()));
        //                        XGroup.at(group_id).push_back(row);
        //                    }
        //                }

        //                sp[1] = std::to_string(last_posy);
        //                bsp = sp;
        //                bsp.push_back(key);
        //                bsp.push_back(std::to_string(font_size));
        //            }
        //        }
        //    }
        //} catch (std::exception &e) {
        //    ERROR("{} ProfileInit PositionSetting.json Error out of range {}", target, e.what());
        //}
#pragma endregion
        std::string cat = "";
        int format_id_num = result.format_id_num;
        std::string format_id_strings = result.format_id_strings;
        std::map<int, KMCProfilFormatIdMap> format_map;
        std::map<int, KMCDispConfigs> tids;
        std::vector<std::pair<int, int>> map_index;

        int push_index = -1;
        int bef_row = -1;
        for (auto &[key, value] : XGroup) {
            int row = key - 1;
            int count = 0;

            if (bef_row != row) {
                ++push_index;
            }

            for (auto &k : value) {
                // LOG("XGroup : {} {}", key, k);
                if (count == 0) {
                    cat = result.row_string[k];
                    ++count;
                } else {
                    cat = cat + "\n" + result.row_string[k];
                }

                for (auto it = result.map_index.begin(); it != result.map_index.end(); ++it) {
                    if (it->first == k) {
                        map_index.push_back(std::make_pair(k, push_index));
                    }
                }

                if (result.format_map.contains(k)) {
                    if (format_map.contains(push_index)) {
                        auto rfm = result.format_map[k];
                        for (int i = 0; i < rfm.format_id.size(); i++) {
                            format_map[push_index].format_id.push_back(rfm.format_id[i]);
                        }
                    } else {
                        format_map.insert(std::make_pair(push_index, result.format_map[k]));
                    }
                }

                if (result.tids.contains(k)) {
                    if (!tids.contains(push_index)) {
                        tids.insert(std::make_pair(push_index, result.tids[k]));
                    }
                }
            }
            format_map[push_index].row_string = cat;
            profil.row_string.push_back(cat);

            bef_row = row;
        }
        sort(map_index.begin(), map_index.end());
        profil.map_index = map_index;
        profil.format_id_num = format_id_num;
        profil.format_id_strings = format_id_strings;
        profil.format_id_strings_array = KMCSplit(format_id_strings, ',');
        profil.isFormat = result.isFormat;
        profil.tids = tids;
        profil.format_map = format_map;
        profil.wids = result.wids;
    }

}