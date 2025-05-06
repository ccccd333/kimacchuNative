#pragma once
#include "KMCUtility.h"

namespace KMCCT {
    struct KMCCutinValues {
    public:
        KMCCutinValues() {}
        KMCCutinValues(std::string type, float time, float animtime, float vol, float oar_time, bool overri_oar_flag) { 
            aaaakmctype = type;
            aaaakmctime = time;
            aaaakmcAnimtime = animtime;
            aaaakmcvolum = vol;
            aaaakmcoar = oar_time;
            overri_oar_time = overri_oar_flag;
        }
        KMCCutinValues(std::string type, float time, float animtime, float vol) {
            aaaakmctype = type;
            aaaakmctime = time;
            aaaakmcAnimtime = animtime;
            aaaakmcvolum = vol;
            overri_oar_time = false;
        }
    public:
        std::string aaaakmctype = "";
        float aaaakmctime = 1.0f;
        float aaaakmcAnimtime = 1.0f;
        float aaaakmcvolum = 1.0f;
        float aaaakmcoar = 1.0f;
        bool overri_oar_time = false;
    };

    void WidgetVisible(KMCAnimST *st, int &playerorfollower);
    void WidgetInVisible(KMCAnimST *st, int &playerorfollower);
    void AnimWidgetVisible(KMCAnimST *st, int &playerorfollower);
    void AnimWidgetInVisible(KMCAnimST *st, int &playerorfollower);
    void TextVisible(KMCAnimST *st, int &playerorfollower);
    void TextInVisible(KMCAnimST *st, int &playerorfollower);
    void KMCPlayAnim(KMCAnimST *st, int &playerorfollower);
    void KMCPlay(KMCAnimST *st, int &playerorfollower);
    void KMCNamePlateStartAnim(KMCAnimST *st, int &playerorfollower);
    void KMCNamePlateEndAnim(KMCAnimST *st, int &playerorfollower);
    void KMCOARFuncStart(KMCAnimST *st, int &playerorfollower);
    void KMCExpFuncStart(KMCAnimST *st, int &playerorfollower);

    void KMCMoveWidgetWaitTask(KMCWaitTaskParam st);

    class KMCCutin {
        SINGLETONHEADER(KMCCutin)
    public:
        ~KMCCutin();
        void InitCutin(std::string skyroot, std::vector<float> *floatArray);

        void InterruptCutInEventManager(KMCInterruptPushCutInData data);

        int WaitLoad(int *wid, uint64_t *rand, std::string *root,
                     std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> **loadedWedget);
        int WaitMultLoad(int *wid, uint64_t *rand, std::string *root, int *index,
                         std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> **loadedWedget);
        int WaitLoadText(int *wid, uint64_t *rand, std::string *root,
                         std::vector<std::pair<uint64_t, int>> **loadedText);
        int WaitLoadNamePlate(int *wid);

        int CondCutIn(KMCCutinValues val);
        int IterCutIn(KMCCutinValues val);
        
        
        bool GetAnimNow();
        void SetAnimNow(bool set);
        void AnimationLoop(long long time, std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator it);
        void AnimationLoop(long long time, std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator it,
                           std::string trackid, int frand, std::string record, float volume);
        void PlaySE(long long time, std::string trackid, int frand, std::string record, float volume);
        void MCMSettingChange(std::vector<float> *floatArray);

        bool ExistCategory(std::string t) { 
            return aaaakmcCategoryRandMap.contains(t);
        }

        void Reset() {
            ResetLoadedWedget();
            ResetLoadedText();
            ResetFLoadedWT();
            ResetFNamePlate();
            PlayerNamePlate.LoadedText = 0;
            PlayerNamePlate.LoadedWidget = 0;
            ResetIDsConfigs();
        }

        void ResetLoadedWedget() { LoadedWedget.clear(); }
        void ResetLoadedText() { LoadedText.clear(); }
        void ResetFLoadedWT() {
            for (int i = 0; i < FLoadedWT.size(); i++) {
                FLoadedWT[i].second.ResetLW();
            }
            FLoadedWT.clear();
        }
        void ResetFNamePlate() { FNamePlate.clear(); }
        void ResetIDsConfigs() {
            LoadedTIDsConfigs.clear();
            FLoadedTIDsConfigs.clear();
            LoadedWIDsConfigs.clear();
            FLoadedWIDsConfigs.clear();
            LoadedNWIDsConfigs.clear();
            LoadedNWIDsConfigs.clear();
            FLoadedNTIDsConfigs.clear();
            FLoadedNWIDsConfigs.clear();
        }

        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> *GetLoadedWedget() { return &LoadedWedget; }
        std::vector<std::pair<uint64_t, int>> *GetLoadedText() { return &LoadedText; }
        std::vector<std::pair<int, KMCDispConfigs>> *GetLoadedTIDsConfigs() { return &LoadedTIDsConfigs; }
        std::vector<std::pair<int, KMCDispConfigs>> *GetLoadedWIDsConfigs() { return &LoadedWIDsConfigs; }

        std::vector<std::pair<int, KMCDispConfigs>> *GetLoadedNTIDsConfigs() { return &LoadedNTIDsConfigs; }
        std::vector<std::pair<int, KMCDispConfigs>> *GetLoadedNWIDsConfigs() { return &LoadedNWIDsConfigs; }

        KMCNPLoadedWidget *GetPlayerNamePlate() { return &PlayerNamePlate; }

        std::vector<std::pair<std::string, KMCFLoadedWidget>> *GetFLoadedWT() { return &FLoadedWT; }
        std::vector<KMCNPLoadedWidget> *GetFNamePlate() { return &FNamePlate; };
        std::vector<std::pair<int, KMCDispConfigs>> *GetFLoadedTIDsConfigs() { return &FLoadedTIDsConfigs; }
        std::vector<std::pair<int, KMCDispConfigs>> *GetFLoadedWIDsConfigs() { return &FLoadedWIDsConfigs; }

        std::vector<std::pair<int, KMCDispConfigs>> *GetFLoadedNTIDsConfigs() { return &FLoadedNTIDsConfigs; }
        std::vector<std::pair<int, KMCDispConfigs>> *GetFLoadedNWIDsConfigs() { return &FLoadedNWIDsConfigs; }

    private:
        int CutIn(KMCCutinValues val);
        void InitNamePlate();
        void InitLoop(std::vector<std::pair<std::string, std::string>> *ar,
                      std::vector<std::pair<std::string, std::string>> *awwf,
                      std::vector<std::pair<std::string, std::string>> *aw,
                      std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> *loadedWedget,
                      std::vector<std::pair<uint64_t, int>> *loadedText,
                      std::vector<std::pair<int, KMCDispConfigs>> *loadedTIDsConfigs,
                      std::vector<std::pair<int, KMCDispConfigs>> *loadedWIDsConfigs, int offset_tx, int offset_ty,
                      int offset_wx, int offset_wy, const std::string *target);

        uint64_t GetCutInID(std::string aaaakmctype);
        void CategoryRandomizer();
        void DispPFWidget(KMCAnimST st);
        void DispPWidget(KMCAnimST st);

    private:
        bool animnow = false;
        std::mutex animnow_mtx;
        time_point<Clock> event_start = Clock::now();
        float followerDetectRange = 1000.0;
        std::unordered_map<std::string, KMCRandomData> aaaakmcCategoryRandMap;

        // player
        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> LoadedWedget;
        std::vector<std::pair<uint64_t, int>> LoadedText;
        std::vector<std::pair<int, KMCDispConfigs>> LoadedTIDsConfigs;
        std::vector<std::pair<int, KMCDispConfigs>> LoadedWIDsConfigs;
        std::vector<std::pair<int, KMCDispConfigs>> LoadedNTIDsConfigs;
        std::vector<std::pair<int, KMCDispConfigs>> LoadedNWIDsConfigs;

        KMCNPLoadedWidget PlayerNamePlate;

        // follower
        std::vector<std::pair<std::string, KMCFLoadedWidget>> FLoadedWT;
        std::vector<KMCNPLoadedWidget> FNamePlate;
        std::vector<std::pair<int, KMCDispConfigs>> FLoadedTIDsConfigs;
        std::vector<std::pair<int, KMCDispConfigs>> FLoadedWIDsConfigs;
        std::vector<std::pair<int, KMCDispConfigs>> FLoadedNTIDsConfigs;
        std::vector<std::pair<int, KMCDispConfigs>> FLoadedNWIDsConfigs;
    };
}