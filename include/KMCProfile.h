#pragma once
#include "KMCUtility.h"

namespace KMCCT {

    namespace ProfileSymbols {
        
        // 内部置換用
        static constexpr std::string_view PLACEHOLDER_VALUE = "[{value}]";
        static constexpr std::string_view PLACEHOLDER_DEFAULT = "default";
    }

    class KMCProfile {
        SINGLETONHEADER(KMCProfile)
    public:
        const std::string DISPLAY_PROFILE_PATH = "DisplayProfile.json";
        const std::string LIVE_DATA_PREFIX = "CppLiveData:";
        const size_t LIVE_DATA_PREFIX_LEN = LIVE_DATA_PREFIX.length();

        void Init();
        void Reset() {
            //BefResultModifiedContainer.clear();
            //ResultModifiedContainer.clear();
            
            show_prifile = false;
            showing_profile = false;
            interrupt_show_profile = false;
            switch_disp_profile_flag = false;
        }

        std::vector<std::string> GetModifiedContainer();
        void SetModifiedContainer(std::vector<std::string> container);
        void UpdateModifiedContainer(std::vector<std::string> *mod_container, int *SUtilEndIndex, int *ModStIndex,
                                     int *ModEnIndex, KMCProfil *profile);
        void KMCResetProfileContainer();
        void InterruptProfileEventManager();
        void ShowProfile(bool visible);
        void TryShowProfile();
        int GetStateProfileEvent();
        bool GetUpdateProfile() { return update_prifile; }
        void Set_update_prifile(bool set) { update_prifile = set; }
        bool Get_first_profile_update() { return first_profile_update; }
        void Set_first_profile_update(bool set) { first_profile_update = set; }
        bool GetShowProfile() { return show_prifile; }
        void Set_show_prifile(bool set) { show_prifile = set; }
        bool GetShowingProfile() { return showing_profile; }
        void Set_showing_profile(bool set) { showing_profile = set; }
        bool Get_interrupt_show_profile() { return interrupt_show_profile; }
        void Set_interrupt_show_profile(bool set) { interrupt_show_profile = set; }
        bool Get_switch_disp_profile_flag() { return switch_disp_profile_flag; }
        void Set_switch_disp_profile_flag(bool set) { switch_disp_profile_flag = set; }
    private:
        // レガシー
        /*void ProfileInit(KMCProfil &profil, std::string target, std::vector<std::pair<std::string, std::string>> *ws,
                         std::vector<std::pair<std::string, std::string>> *ts, std::vector<std::string> *pt);
        */
        
        bool Parse(std::string path);
    private:
        // player
        KMCProfil profil_ex_data;
        std::mutex pr_mtx;

        // ModifiedContainer
        std::vector<std::string> modified_container;
        std::vector<std::string> ResultModifiedContainer;
        std::vector<std::string> BefResultModifiedContainer;
        int StrageUtilStartIndex = 0;
        int StrageUtilEndIndex = 0;
        int mod_start_index = 0;
        int mod_end_index = 0;

        std::string aaaakmcroot = "";
        float aaaakmcvolum = 1.0f;
        bool update_prifile = false;
        bool first_profile_update = false;
        bool show_prifile = false;
        bool showing_profile = false;
        bool interrupt_show_profile = false;
        bool switch_disp_profile_flag = false;

        bool is_missing_file = false;

    };
}