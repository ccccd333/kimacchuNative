#include "KMCCutin.h"
#include "KMCStateManager.h"
#include "KMCConfig.h"
#include "KMCSound.h"
#include "KMCEventThread.h"
#include "KMCOAR.h"
#include "KMCWaitTask.h"
#include "KMCExpression.h"

SINGLETONBODY(KMCCT::KMCCutin)

namespace KMCCT {
    std::map<int, KMCCutinOrder> AnimFtoAnimP{
                                                     {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)},
                                                     {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},
                                                     {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 1)},
                                                     {11, KMCCutinOrder(KMCCT::AnimWidgetVisible, 1)},
                                                     {12, KMCCutinOrder(KMCCT::TextVisible, 1)},
                                                     {13, KMCCutinOrder(KMCCT::KMCPlayAnim, 1)},
                                                     {14, KMCCutinOrder(KMCCT::TextInVisible, 1)},
                                                     {15, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 1)},
                                                     {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 1)},   
                                                     {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},
                                                     {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                     {20, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                                     {21, KMCCutinOrder(KMCCT::AnimWidgetVisible, 0)},
                                                     {22, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                                     {23, KMCCutinOrder(KMCCT::KMCPlayAnim, 0)},
                                                     {24, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                                     {25, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 0)},
                                                     {26, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)}};
    std::map<int, KMCCutinOrder> AnimPtoAnimF{
                                                     {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},
                                                     {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                     {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                                     {11, KMCCutinOrder(KMCCT::AnimWidgetVisible, 0)},
                                                     {12, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                                     {13, KMCCutinOrder(KMCCT::KMCPlayAnim, 0)},
                                                     {14, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                                     {15, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 0)},
                                                     {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)},   
                                                     {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)},
                                                     {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},
                                                     {20, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 1)},
                                                     {21, KMCCutinOrder(KMCCT::AnimWidgetVisible, 1)},
                                                     {22, KMCCutinOrder(KMCCT::TextVisible, 1)},
                                                     {23, KMCCutinOrder(KMCCT::KMCPlayAnim, 1)},
                                                     {24, KMCCutinOrder(KMCCT::TextInVisible, 1)},
                                                     {25, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 1)},
                                                     {26, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 1)}};
    std::map<int, KMCCutinOrder> FtoAnimP{
                                                 {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)}, 
                                                 {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},
                                                 {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 1)},
                                                 {11, KMCCutinOrder(KMCCT::TextVisible, 1)},
                                                 {12, KMCCutinOrder(KMCCT::WidgetVisible, 1)},
                                                 {13, KMCCutinOrder(KMCCT::KMCPlay, 1)},
                                                 {14, KMCCutinOrder(KMCCT::TextInVisible, 1)},
                                                 {15, KMCCutinOrder(KMCCT::WidgetInVisible, 1)},
                                                 {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 1)},
                                                 {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},
                                                 {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                 {20, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                                 {21, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                                 {22, KMCCutinOrder(KMCCT::AnimWidgetVisible, 0)},
                                                 {23, KMCCutinOrder(KMCCT::KMCPlayAnim, 0)},
                                                 {24, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                                 {25, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 0)},
                                                 {26, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)}};

    std::map<int, KMCCutinOrder> AnimPtoF{
                                                 {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},       
                                                 {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                 {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                                 {11, KMCCutinOrder(KMCCT::AnimWidgetVisible, 0)},
                                                 {12, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                                 {13, KMCCutinOrder(KMCCT::KMCPlayAnim, 0)},
                                                 {14, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                                 {15, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 0)},
                                                 {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)},
                                                 {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)},
                                                 {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},
                                                 {20, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 1)},
                                                 {21, KMCCutinOrder(KMCCT::TextVisible, 1)},
                                                 {22, KMCCutinOrder(KMCCT::WidgetVisible, 1)},
                                                 {23, KMCCutinOrder(KMCCT::KMCPlay, 1)},
                                                 {24, KMCCutinOrder(KMCCT::TextInVisible, 1)},
                                                 {25, KMCCutinOrder(KMCCT::WidgetInVisible, 1)},
                                                 {26, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 1)}};
    std::map<int, KMCCutinOrder> AnimFtoP{
                                                 {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)},        
                                                 {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},
                                                 {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 1)},
                                                 {11, KMCCutinOrder(KMCCT::AnimWidgetVisible, 1)},
                                                 {12, KMCCutinOrder(KMCCT::TextVisible, 1)},
                                                 {13, KMCCutinOrder(KMCCT::KMCPlayAnim, 1)},
                                                 {14, KMCCutinOrder(KMCCT::TextInVisible, 1)},
                                                 {15, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 1)},
                                                 {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 1)},
                                                 {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},
                                                 {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                 {20, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                                 {21, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                                 {22, KMCCutinOrder(KMCCT::WidgetVisible, 0)},
                                                 {23, KMCCutinOrder(KMCCT::KMCPlay, 0)},
                                                 {24, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                                 {25, KMCCutinOrder(KMCCT::WidgetInVisible, 0)},
                                                 {26, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)}};
    std::map<int, KMCCutinOrder> PtoAnimF{
                                                 {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},
                                                 {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                 {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                                 {11, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                                 {12, KMCCutinOrder(KMCCT::WidgetVisible, 0)},
                                                 {13, KMCCutinOrder(KMCCT::KMCPlay, 0)},
                                                 {14, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                                 {15, KMCCutinOrder(KMCCT::WidgetInVisible, 0)},
                                                 {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)},   
                                                 {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)},
                                                 {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},
                                                 {20, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 1)},
                                                 {21, KMCCutinOrder(KMCCT::AnimWidgetVisible, 1)},
                                                 {22, KMCCutinOrder(KMCCT::TextVisible, 1)},
                                                 {23, KMCCutinOrder(KMCCT::KMCPlayAnim, 1)},
                                                 {24, KMCCutinOrder(KMCCT::TextInVisible, 1)},
                                                 {25, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 1)},
                                                 {26, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 1)}};
    std::map<int, KMCCutinOrder> FtoP{
                                             {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)},        
                                             {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},
                                             {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 1)},
                                             {11, KMCCutinOrder(KMCCT::TextVisible, 1)},
                                             {12, KMCCutinOrder(KMCCT::WidgetVisible, 1)},
                                             {13, KMCCutinOrder(KMCCT::KMCPlay, 1)},
                                             {14, KMCCutinOrder(KMCCT::TextInVisible, 1)},
                                             {15, KMCCutinOrder(KMCCT::WidgetInVisible, 1)},
                                             {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 1)},
                                             {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},  
                                             {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                             {20, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                             {21, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                             {22, KMCCutinOrder(KMCCT::WidgetVisible, 0)},
                                             {23, KMCCutinOrder(KMCCT::KMCPlay, 0)},
                                             {24, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                             {25, KMCCutinOrder(KMCCT::WidgetInVisible, 0)},
                                             {26, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)}};
    std::map<int, KMCCutinOrder> PtoF{
                                             {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},        
                                             {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                             {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                             {11, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                             {12, KMCCutinOrder(KMCCT::WidgetVisible, 0)},
                                             {13, KMCCutinOrder(KMCCT::KMCPlay, 0)},
                                             {14, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                             {15, KMCCutinOrder(KMCCT::WidgetInVisible, 0)},
                                             {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)},
                                             {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)}, 
                                             {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},
                                             {20, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 1)},
                                             {21, KMCCutinOrder(KMCCT::TextVisible, 1)},
                                             {22, KMCCutinOrder(KMCCT::WidgetVisible, 1)},
                                             {23, KMCCutinOrder(KMCCT::KMCPlay, 1)},
                                             {24, KMCCutinOrder(KMCCT::TextInVisible, 1)},
                                             {25, KMCCutinOrder(KMCCT::WidgetInVisible, 1)},
                                             {26, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 1)}};
    std::map<int, KMCCutinOrder> AnimPlayerOnly{
                                                       {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},        
                                                       {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                       {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                                       {11, KMCCutinOrder(KMCCT::AnimWidgetVisible, 0)},
                                                       {12, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                                       {13, KMCCutinOrder(KMCCT::KMCPlayAnim, 0)},
                                                       {14, KMCCutinOrder(KMCCT::AnimWidgetInVisible, 0)},
                                                       {15, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                                       {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)}};
    std::map<int, KMCCutinOrder> WidgetPlayerOnly{
                                                         {0, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},        
                                                         {1, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                         {10, KMCCutinOrder(KMCCT::KMCNamePlateStartAnim, 0)},
                                                         {11, KMCCutinOrder(KMCCT::TextVisible, 0)},
                                                         {12, KMCCutinOrder(KMCCT::WidgetVisible, 0)},
                                                         {13, KMCCutinOrder(KMCCT::KMCPlay, 0)},
                                                         {14, KMCCutinOrder(KMCCT::TextInVisible, 0)},
                                                         {15, KMCCutinOrder(KMCCT::WidgetInVisible, 0)},
                                                         {16, KMCCutinOrder(KMCCT::KMCNamePlateEndAnim, 0)}};

    std::string ct_aaaakmcroot = "";
    KMCInterruptPushCutInData aaaakmcInterruptData;
    float aaaakmcvolum = 0.5f;
    long long aaaakmcAnimtime = 4;
    long long aaaakmctime = 5;
    long long aaaakmcCycle = 10;

    std::string aaaaPlayerTextX = "0";
    std::string aaaaPlayerTextY = "0";
    std::string aaaaPlayerWidgetX = "0";
    std::string aaaaPlayerWidgetY = "0";

    std::string aaaaPlayerNTextX = "0";
    std::string aaaaPlayerNTextY = "0";
    std::string aaaaPlayerNWidgetX = "0";
    std::string aaaaPlayerNWidgetY = "0";

    std::string aaaaFollowerTextX = "0";
    std::string aaaaFollowerTextY = "0";
    std::string aaaaFollowerWidgetX = "0";
    std::string aaaaFollowerWidgetY = "0";

    std::string aaaaFollowerNTextX = "0";
    std::string aaaaFollowerNTextY = "0";
    std::string aaaaFollowerNWidgetX = "0";
    std::string aaaaFollowerNWidgetY = "0";

    std::string KMC_DEFAULT_FONT = "$EverywhereFont";
    int KMC_DEFAULT_FONT_SIZE = 24;
    int KMC_DEFAULT_NAME_POSX = 640;
    int KMC_DEFAULT_NAME_POSY = 320;
    int KMC_DEFAULT_WIDGET_POS_X = 640;
    int KMC_DEFAULT_WIDGET_POS_Y = 460;
    int KMC_DEFAULT_TEXT_POS_X = 640;
    int KMC_DEFAULT_TEXT_POS_Y = 535;

    void KMCCutin::InitCutin(std::string skyroot, std::vector<float> *floatArray) {

        animnow = false;

        ct_aaaakmcroot = skyroot;
        aaaakmcvolum = (*floatArray)[0];
        aaaakmcAnimtime = (*floatArray)[1];
        aaaakmctime = (*floatArray)[2];
        aaaakmcCycle = (*floatArray)[3];
        aaaaPlayerTextX = std::to_string((int)((*floatArray)[4]));
        aaaaPlayerTextY = std::to_string((int)((*floatArray)[5]));
        aaaaPlayerWidgetX = std::to_string((int)((*floatArray)[6]));
        aaaaPlayerWidgetY = std::to_string((int)((*floatArray)[7]));

        aaaaPlayerNTextX = std::to_string((int)((*floatArray)[8]));
        aaaaPlayerNTextY = std::to_string((int)((*floatArray)[9]));
        aaaaPlayerNWidgetX = std::to_string((int)((*floatArray)[10]));
        aaaaPlayerNWidgetY = std::to_string((int)((*floatArray)[11]));

        aaaaFollowerTextX = std::to_string((int)((*floatArray)[12]));
        aaaaFollowerTextY = std::to_string((int)((*floatArray)[13]));
        aaaaFollowerWidgetX = std::to_string((int)((*floatArray)[14]));
        aaaaFollowerWidgetY = std::to_string((int)((*floatArray)[15]));

        aaaaFollowerNTextX = std::to_string((int)((*floatArray)[16]));
        aaaaFollowerNTextY = std::to_string((int)((*floatArray)[17]));
        aaaaFollowerNWidgetX = std::to_string((int)((*floatArray)[18]));
        aaaaFollowerNWidgetY = std::to_string((int)((*floatArray)[19]));

        auto settings = KMCCT::KMCConfig::GetSingleton()->getISetting();
        std::string FollowerDefaultRange = "";
        // setting string
        std::vector<std::pair<std::string, std::string>> ssetting{
            std::make_pair("font", "ERROR SETTING setting.json font"),
            std::make_pair("follower_detect_range", "ERROR SETTING setting.json default follower detect range")};

        std::vector<std::string *> starget = {
            &KMC_DEFAULT_FONT,
            &FollowerDefaultRange,
        };

        for (int i = 0; i < ssetting.size(); i++) {
            std::string k = ssetting[i].first;
            std::string v = ssetting[i].second;
            *(starget[i]) = LoadDefaultSetting(k, v, settings);
        }
        KMC_DEFAULT_FONT = *(starget[0]);
        followerDetectRange = std::stof(*(starget[1]));

        // setting int
        std::vector<std::pair<std::string, std::string>> isetting{
            std::make_pair("fontsize", "ERROR SETTING setting.json font size"),
            std::make_pair("nameposx", "ERROR SETTING setting.json nameposx"),
            std::make_pair("nameposy", "ERROR SETTING setting.json nameposy"),
            std::make_pair("w_posx", "ERROR SETTING setting.json w_posx"),
            std::make_pair("w_posy", "ERROR SETTING setting.json w_posy"),
            std::make_pair("t_posx", "ERROR SETTING setting.json t_posx"),
            std::make_pair("t_posy", "ERROR SETTING setting.json t_posy")};
        std::vector<int *> target = {&KMC_DEFAULT_FONT_SIZE,    &KMC_DEFAULT_NAME_POSX,    &KMC_DEFAULT_NAME_POSY,
                                     &KMC_DEFAULT_WIDGET_POS_X, &KMC_DEFAULT_WIDGET_POS_Y, &KMC_DEFAULT_TEXT_POS_X,
                                     &KMC_DEFAULT_TEXT_POS_Y};

        for (int i = 0; i < isetting.size(); i++) {
            std::string k = isetting[i].first;
            std::string v = isetting[i].second;
            *(target[i]) = std::stoi(LoadDefaultSetting(k, v, settings));
        }



        CategoryRandomizer();

        InitNamePlate();
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        // player
        auto *animationRange = KMCCT::KMCConfig::GetSingleton()->getIAnimationRange();
        auto *autoWordWF = KMCCT::KMCConfig::GetSingleton()->getIAutoWordWFConfigs();
        auto *playerAutoWord = KMCCT::KMCConfig::GetSingleton()->getIAutoWordConfigs();
        InitLoop(animationRange, autoWordWF, playerAutoWord, &LoadedWedget,
                 &LoadedText, &LoadedTIDsConfigs, &LoadedWIDsConfigs,
                 std::stoi(aaaaPlayerTextX), std::stoi(aaaaPlayerTextY), std::stoi(aaaaPlayerWidgetX),
                 std::stoi(aaaaPlayerWidgetY), &KMCCT::PLAYER_WORD_PATH);
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        auto *followers = KMCCT::KMCConfig::GetSingleton()->getFollowers();
        for (int i = 0; i < followers->size(); i++) {
            KMCFollower f = (*followers)[i];
            auto *fanimationRange = &(f.IAnimationRange);
            auto *fautoWordWF = &(f.IAutoWordWFConfigs);
            auto *fAutoWord = &(f.IAutoWordConfigs);
            std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> lw;
            std::vector<std::pair<uint64_t, int>> lt;

            std::string fwp = KMCCT::FOLLOWER_WORD_PATH + std::to_string(i + 1);

            InitLoop(fanimationRange, fautoWordWF, fAutoWord, &lw, &lt, &FLoadedTIDsConfigs, &FLoadedWIDsConfigs,
                     std::stoi(aaaaFollowerTextX), std::stoi(aaaaFollowerTextY), std::stoi(aaaaFollowerWidgetX),
                     std::stoi(aaaaFollowerWidgetY), &fwp);
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }
            FLoadedWT.push_back(std::make_pair(std::to_string(i + 1), KMCFLoadedWidget(lw, lt)));
        }
    }

    void KMCCutin::InterruptCutInEventManager(KMCInterruptPushCutInData data) {
        static long long event_cool_time =
            KMCFindVector(KMCCT::KMCConfig::GetSingleton()->getISetting(), KMCCT::STATE_MANAGER_CONFIG_KEY,
                          KMCCT::INTERRUPT_EVENT_COOL_TIME) *
            KMCCT::TIME_SCALE_MS;

        if (!KMCCT::KMCEventThread::GetSingleton()->forceendanim &&
            KMCCT::KMCEventThread::GetSingleton()->GetInitEndFlag() &&
            !KMCCT::KMCCutin::GetSingleton()->GetAnimNow() &&
            !KMCCT::KMCWaitTask::GetSingleton()->GetWaitFlag()) {

            time_point<Clock> interrupt_time = Clock::now();
            milliseconds diff = duration_cast<milliseconds>(interrupt_time - event_start);
            long long dur = diff.count();
            if (dur < event_cool_time) {
                LOG("InterruptCutInEventManager cool time {} {}", dur, event_cool_time);
                return;
            }
            event_start = Clock::now();
            aaaakmcInterruptData = data;
            // LOG("InterruptCutInEventManager interrupt");

            switch (aaaakmcInterruptData.interruptType) {
                case KMCInterruptType::add_item:
                    KMCCT::wrap_InterruptCutInEventManager(InterruptEventAddItem);
                    break;
                case KMCInterruptType::on_hit:
                    KMCCT::wrap_InterruptCutInEventManager(InterruptEventOnHit);
                    break;
            }
        }
    }

    int KMCCutin::WaitLoad(int* wid, uint64_t* rand, std::string* root,
        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>** loadedWedget) {
        std::string result = IWW::MainFunctions::GetSingleton()->GetOutput(*wid, "-1");
        while (result == "W8") {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return -1;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));
            result = IWW::MainFunctions::GetSingleton()->GetOutput(*wid, "-1");
        }
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return -1;
        }
        int loadedwid = std::stoi(result);
        //LoadedWidgetsFactory.push_back(loadedwid);
        uint64_t r = *rand;
        LOG("KMCEventThread::WaitLoad WID = {} TYPE ID = {}", result, r);
        (*loadedWedget)->push_back(std::make_pair(r, KMCLoadedWidgetData(false, loadedwid, 0, "")));

        return loadedwid;
    }

    int KMCCutin::WaitMultLoad(int* wid, uint64_t* rand, std::string* root, int* index,
        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>** loadedWedget) {
        std::string result = IWW::MainFunctions::GetSingleton()->GetOutput(*wid, "-1");
        while (result == "W8") {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return -1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));
            result = IWW::MainFunctions::GetSingleton()->GetOutput(*wid, "-1");
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return -1;
        }

        int r = std::stoi(result);
        //LoadedWidgetsFactory.push_back(r);

        LOG("KMCEventThread::WaitMultLoad WID = {} ", result);
        uint64_t k = *rand;
        auto it =
            std::find_if((*loadedWedget)->begin(), (*loadedWedget)->end(), [k](const auto &p) { return p.first == k; });
        it->second.animWedget[*index] = r;
        //}
        return r;
    }

    int KMCCutin::WaitLoadText(int* wid, uint64_t* rand, std::string* root,
        std::vector<std::pair<uint64_t, int>>** loadedText) {
        std::string result = IWW::MainFunctions::GetSingleton()->GetOutput(*wid, "-1");
        while (result == "W8") {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return -1;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));
            result = IWW::MainFunctions::GetSingleton()->GetOutput(*wid, "-1");
        }
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return -1;
        }

        int loadedwid = std::stoi(result);
        //LoadedWidgetsFactory.push_back(loadedwid);

        LOG("KMCEventThread::WaitLoadText TID = {} ", result);
        uint64_t k = *rand;
        (*loadedText)->push_back(std::make_pair(k, loadedwid));

        return loadedwid;
    }
    
    int KMCCutin::WaitLoadNamePlate(int* wid) {
        std::string result = IWW::MainFunctions::GetSingleton()->GetOutput(*wid, "-1");
        while (result == "W8") {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return -1;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));
            result = IWW::MainFunctions::GetSingleton()->GetOutput(*wid, "-1");
        }
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return -1;
        }
        int loadedwid = std::stoi(result);
        //LoadedWidgetsFactory.push_back(loadedwid);
        LOG("KMCEventThread::WaitLoadNamePlate WID = {} ", result);

        return loadedwid;
    }


    int KMCCutin::CondCutIn(KMCCutinValues val) {
        auto node = KMCCT::KMCCutinCondition::GetSingleton()->GetTaskResult();

        if (node) {
            // guard
            {
                std::lock_guard<std::mutex> lock(animnow_mtx);
                if (animnow) return -1;
                animnow = true;
            }

            node->Tuning(val);

            KMCCT::KMCCutinCondition::GetSingleton()->Completed(node);

            // qu exp
            if (KMCCT::KMCExpression::GetSingleton()->OnStandby() < 0) {
                return 99;
            }

            int rslt = CutIn(val);

            // pop exp
            KMCCT::KMCExpression::GetSingleton()->OnFinished();

            return rslt;
        }

        return 99;
    }
    int KMCCutin::IterCutIn(KMCCutinValues val) {
        // guard
        {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            if (animnow) return -1;
            animnow = true;
        }

        return CutIn(val);
    }

    // return = -1 : 他のカットイン要求を処理中
    // return = -2 : StateManager(プレイヤーの状態管理)により拒否された(プレイヤーが話し中など)
    // return = 2 : rand生成したが対象がいなかった
    // return = 0 : 正常終了
    int KMCCutin::CutIn(KMCCutinValues val) {


        // init
        std::string aaaakmctype = val.aaaakmctype;
        float c_time = val.aaaakmctime;
        float c_animtime = val.aaaakmcAnimtime;
        float c_volum = val.aaaakmcvolum;
        float c_oar_time = val.aaaakmcoar;
        float c_exp_time = val.aaaakmcexp;
        // ct_aaaakmcroot = (*variableArray)[0].c_str();
        // std::string aaaakmctype = (*variableArray)[1].c_str();
        // std::string aaaakmctime = (*variableArray)[2].c_str();
        // std::string aaaakmcposx = (*variableArray)[3].c_str();
        // std::string aaaakmcposy = (*variableArray)[4].c_str();
        // std::string aaaakmctposx = (*variableArray)[5].c_str();
        // std::string aaaakmctposy = (*variableArray)[6].c_str();
        // std::string aaaakmcvolum = (*variableArray)[7].c_str();
        long long t = c_time * KMCCT::TIME_SCALE_MS;
        bool isAnim = false;

        if (aaaakmctype == "") {
            aaaakmctype = KMCCT::KMCStateManager::GetSingleton()->GetPlayerState();

            if (aaaakmctype == "") {
                std::lock_guard<std::mutex> lock(animnow_mtx);
                animnow = false;
                return -2;
            }
        }
        LOG("CutIn type {}", aaaakmctype);

        int exp_rand = -1;
        std::string exp_type = val.aaaakmcExptype;
        LOG("EXP type ===> {}, CUTIN type ===> {}", exp_type, aaaakmctype);
        if (val.overri_fc_exp) {
            LOG("[EXP type] ===> {}", exp_type);
            if (aaaakmctype != exp_type) {
                exp_rand = KMCCT::KMCExpression::GetSingleton()->GetCutInID(exp_type);
                if (exp_rand == 0) {
                    LOG("EXP type ===> not found", exp_type, aaaakmctype);
                    exp_rand = -1;
                }
            }
        }

        std::string skyUiBase = ct_aaaakmcroot;
        int wid = -1;
        int tid = -1;

        int fwid = -1;
        int ftid = -1;
        bool isfAnim = false;
        bool fSpeachFlag = true;
        RE::Actor *follower = nullptr;
        std::vector<std::pair<std::string, std::string>> *speakTiming = nullptr;
        KMCCompsFlag pcf;
        KMCCompsFlag fcf;
        std::string precord = "";
        std::string frecord = "";

        // player
        int rand = GetCutInID(aaaakmctype);
        if (rand < 0) {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            animnow = false;
            return 2;
        }
        std::string srand = std::to_string(rand);

        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator target;
        // LOG("KMCEventThread:CutIn PARAM root = {} type = {} aaaakmctime = {} aaaakmcposx = {} aaaakmcposy = {}",
        // skyUiBase, aaaakmctype, aaaakmctime, aaaakmcposx, aaaakmcposy);

        //auto *loadedWedget = KMCCT::KMCEventThread::GetSingleton()->getLoadedWedget();
        auto *HideComponents = KMCCT ::KMCConfig::GetSingleton()->getIHideComponents();

        auto itPlayerHideComponents = std::find_if(HideComponents->begin(), HideComponents->end(),
                                                   [srand](const auto &p) { return p.first == srand; });
        if (itPlayerHideComponents != HideComponents->end()) {
            pcf = itPlayerHideComponents->second;
        }

        if (pcf.Widget) {
            auto itPlayerLoadedW = std::find_if(LoadedWedget.begin(), LoadedWedget.end(),
                                                [rand](const auto &p) { return p.first == rand; });
            if (itPlayerLoadedW != LoadedWedget.end()) {
                if (itPlayerLoadedW->second.isAnim) {
                    isAnim = true;
                    t = c_animtime * KMCCT::TIME_SCALE_MS;
                } else {
                    if (itPlayerLoadedW->second.wedget > 0) {
                        wid = itPlayerLoadedW->second.wedget;
                    } else {
                        pcf.Widget = false;
                    }
                }
                target = itPlayerLoadedW;
            } else {
                pcf.Widget = false;
            }
        }
        if (pcf.Word) {
            // player text
            auto itPlayerText =
                std::find_if(LoadedText.begin(), LoadedText.end(), [rand](const auto &p) { return p.first == rand; });
            if (itPlayerText != LoadedText.end()) {
                tid = itPlayerText->second;
            } else {
                pcf.Word = false;
            }
        }
        if (pcf.Sound) {
            pcf.Sound = KMCCT::KMCSound::GetSingleton()->IsPlayableSoundEx(srand, -1);
        }
        if (pcf.SE) {
            pcf.SE = KMCCT::KMCSound::GetSingleton()->GetFirstSEIndexEx(srand, -1, &precord);
        }
        if (pcf.NamePlate || pcf.NamePlateName) {
            if (pcf.Widget == false && pcf.Word == false/* && pcf.Sound == false && pcf.SE == false*/) {
                pcf.NamePlate = false;
                pcf.NamePlateName = false;
            }
        }

        // follower
        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator ftarget;
        int frand = -1;
        auto f = FollowerEffectiveDistance(KMCCT::KMCConfig::GetSingleton()->getPlayer(),
                                           KMCCT::KMCConfig::GetSingleton()->getFollowers(), followerDetectRange, rand);
        if (f.size() != 0) {
            frand = f.begin()->index;
            int target_index = 0;
            LOG("fsize {}", f.size());
            if (f.size() > 1) {
                std::random_device rnd;
                std::mt19937 mt(rnd());
                std::uniform_int_distribution<> randhl(0, (int)(f.size()) - 1);
                target_index = randhl(mt);
                frand = f[target_index].index;
            }

            auto fHideComponents = f[target_index].IHideComponents;

            auto itfHideComponents = std::find_if(fHideComponents.begin(), fHideComponents.end(),
                                                  [srand](const auto &p) { return p.first == srand; });
            if (itfHideComponents != fHideComponents.end()) {
                fcf = itfHideComponents->second;
            }

            if (fcf.Widget) {
                auto *floadedWedget = &((FLoadedWT)[frand].second.LoadedWedget);
                auto itFLoadedW = std::find_if(floadedWedget->begin(), floadedWedget->end(),
                                               [rand](const auto &p) { return p.first == rand; });
                if (itFLoadedW != floadedWedget->end()) {
                    if (itFLoadedW->second.isAnim) {
                        isfAnim = true;
                        t = c_animtime * KMCCT::TIME_SCALE_MS;
                    } else {
                        if (itFLoadedW->second.wedget > 0) {
                            fwid = itFLoadedW->second.wedget;
                        } else {
                            fcf.Widget = false;
                        }
                    }
                    ftarget = itFLoadedW;

                    follower = f[target_index].follower;
                    speakTiming = &(f[target_index].ISpeachTiming);
                } else {
                    fcf.Widget = false;
                }
            }
            if (fcf.Word) {
                auto floadedText = (FLoadedWT)[frand].second.LoadedText;
                auto itFLoadedT = std::find_if(floadedText.begin(), floadedText.end(),
                                               [rand](const auto &p) { return p.first == rand; });
                if (itFLoadedT != floadedText.end()) {
                    ftid = itFLoadedT->second;
                } else {
                    fcf.Word = false;
                }
            }
            if (fcf.Sound) {
                fcf.Sound = KMCCT::KMCSound::GetSingleton()->IsPlayableSoundEx(srand, frand);
            }
            if (fcf.SE) {
                fcf.SE = KMCCT::KMCSound::GetSingleton()->GetFirstSEIndexEx(srand, frand, &frecord);
            }
            if (fcf.NamePlate || fcf.NamePlateName) {
                if (fcf.Widget == false && fcf.Word == false /*&& fcf.Sound == false && fcf.SE == false*/) {
                    fcf.NamePlate = false;
                    fcf.NamePlateName = false;
                }
            }

            if (fcf.IsAllDisable()) {
                fSpeachFlag = false;
            }
        } else {
            fSpeachFlag = false;
        }

        if (fSpeachFlag) {
            KMCAnimST st = KMCAnimST();
            st.t = target;
            st.ft = ftarget;
            st.tid = tid;
            st.ftid = ftid;
            st.time = t;
            st.isAnim = isAnim;
            st.isfAnim = isfAnim;
            st.rand = rand;
            st.frand = frand;
            st.volum = c_volum;
            st.ISpeechTiming = speakTiming;
            st.speakerp = KMCCT::KMCConfig::GetSingleton()->getPlayer();
            st.speakerf = follower;
            st.pcf = pcf;
            st.fcf = fcf;
            st.precord = precord;
            st.frecord = frecord;
            st.nppw = PlayerNamePlate;
            st.oar_time = c_oar_time;
            st.exp_time = c_exp_time;
            st.overri_oar_time = val.overri_oar_time;
            st.overri_exp_time = val.overri_exp_time;
            st.exp_rand = exp_rand;
            try {
                auto fpl = (FNamePlate).at(st.frand);
                st.npfw = fpl;
            } catch (...) {
                ERROR("There is no follower nameplate setting; please check NamePlate.json. Follower ID {}", st.frand);
            }

            DispPFWidget(st);
        } else {
            KMCAnimST st = KMCAnimST();
            st.t = target;
            st.tid = tid;
            st.time = t;
            st.isAnim = isAnim;
            st.rand = rand;
            st.frand = frand;
            st.volum = c_volum;
            st.speakerp = KMCCT::KMCConfig::GetSingleton()->getPlayer();
            st.pcf = pcf;
            st.precord = precord;
            st.nppw = PlayerNamePlate;
            st.oar_time = c_oar_time;
            st.exp_time = c_exp_time;
            st.overri_oar_time = val.overri_oar_time;
            st.overri_exp_time = val.overri_exp_time;
            st.exp_rand = exp_rand;
            DispPWidget(st);
        }

        {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            animnow = false;
        }

        return 0;
    }

    bool KMCCutin::GetAnimNow() { 
        {
            std::lock_guard<std::mutex> lock(animnow_mtx); 
            return animnow;
        }
    }
    void KMCCutin::SetAnimNow(bool set) {
        {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            animnow = set;
        }
    }

    void KMCCutin::InitNamePlate() {
        // NamePlate
        
        // player
        auto namep = KMCCT::KMCConfig::GetSingleton()->getINamePlate();
        if (namep->size() == 0) {
            WARN("NamePlate.json undefine");
            return;
        }

        KMCNamePlate pnpsetting = (*namep)[0].second;
        int rwid = -1;
        std::string filename = KMCCT::PICT_PATH1 + "/" + KMCCT::NAME_PLATE_PICT_NAME + KMCCT::PICT_TYPE;
        if (IsFileExist(KMCCT::PICT_ROOT + filename)) {
            int wid = IWW::MainFunctions::GetSingleton()->LoadWidget(ct_aaaakmcroot, filename, 10000, 10000, false);
            rwid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoadNamePlate(wid);
        } else {
            WARN("File path not found. If not intended, no problem. {}", filename);
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        std::string pn = KMCCT::KMCConfig::GetSingleton()->getPlayer()->GetName();
        // LOG("NamePlate name = {}", pn);
        //  player name plate widget
        int tid = IWW::MainFunctions::GetSingleton()->LoadText(ct_aaaakmcroot, pn, pnpsetting.font, pnpsetting.fontsize,
                                                               10000, 10000, false);
        int rtid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoadNamePlate(tid);
        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        if (rwid > 0) {
            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, rwid, true);

            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, rtid, true);

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));

            IWW::MainFunctions::GetSingleton()->SendToFront(ct_aaaakmcroot, rtid);

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));

            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, rwid, false);

            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, rtid, false);

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));
        }

        // LOG("NamePlate player rtid = {}", rtid);
        int pname_x = pnpsetting.name_x + std::stoi(aaaaPlayerNWidgetX);
        int pname_y = pnpsetting.name_y + std::stoi(aaaaPlayerNWidgetY);
        int pfont_x = pnpsetting.font_x + std::stoi(aaaaPlayerNTextX);
        int pfont_y = pnpsetting.font_y + std::stoi(aaaaPlayerNTextY);

        if (rwid > 0) {
            IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, rwid, pname_x);
            IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, rwid, pname_y);
        }

        IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, rtid, pfont_x);
        IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, rtid, pfont_y);
        IWW::MainFunctions::GetSingleton()->SetRGB(ct_aaaakmcroot, rtid, pnpsetting.r, pnpsetting.g, pnpsetting.b);

        PlayerNamePlate.set(rtid, rwid, pfont_x, pfont_y, pname_x, pname_y);

        LoadedNTIDsConfigs.push_back(
            std::make_pair(rtid, KMCDispConfigs(rtid, pfont_x, pfont_y, pnpsetting.r, pnpsetting.g, pnpsetting.b)));

        if (rwid > 0) {
            LoadedNWIDsConfigs.push_back(std::make_pair(rwid, KMCDispConfigs(rwid, pname_x, pname_y)));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));

        // follower
        auto followers = KMCCT::KMCConfig::GetSingleton()->getFollowers();
        KMCNamePlate fnpsetting = (*namep)[1].second;
        std::string key;
        KMCFollower value;
        for (int i = 0; i < (*followers).size(); i++) {
            if ((*followers)[i].follower == nullptr) continue;

            std::string fname = (*followers)[i].follower->GetName();

            // LOG("Follower NamePlate name = {}", fname);

            // LOG("NamePlate follower rtid = {}", rftid);
            int rfwid = -1;
            if (IsFileExist(KMCCT::PICT_ROOT + filename)) {
                int fwid = IWW::MainFunctions::GetSingleton()->LoadWidget(ct_aaaakmcroot, filename, 10000, 10000, false);
                rfwid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoadNamePlate(fwid);
            } else {
                WARN("File path not found. If not intended, no problem. {}", filename);
            }

            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            int ftid = IWW::MainFunctions::GetSingleton()->LoadText(ct_aaaakmcroot, fname, fnpsetting.font,
                                                                    fnpsetting.fontsize, 10000, 10000, false);
            int rftid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoadNamePlate(ftid);
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            if (rfwid > 0) {
                IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, rfwid, true);
                IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, rftid, true);

                std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));

                IWW::MainFunctions::GetSingleton()->SendToFront(ct_aaaakmcroot, rftid);

                std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));

                IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, rfwid, false);
                IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, rftid, false);

                std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_LOAD_FRAME_MS));
            }

            int fname_x = fnpsetting.name_x + std::stoi(aaaaFollowerNWidgetX);
            int fname_y = fnpsetting.name_y + std::stoi(aaaaFollowerNWidgetY);
            int ffont_x = fnpsetting.font_x + std::stoi(aaaaFollowerNTextX);
            int ffont_y = fnpsetting.font_y + std::stoi(aaaaFollowerNTextY);

            IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, rftid, ffont_x);
            IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, rftid, ffont_y);
            IWW::MainFunctions::GetSingleton()->SetRGB(ct_aaaakmcroot, rftid, fnpsetting.r, fnpsetting.g, fnpsetting.b);

            if (rfwid > 0) {
                IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, rfwid, fname_x);
                IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, rfwid, fname_y);
                // IWW::MainFunctions::GetSingleton()->SendToBack(ct_aaaakmcroot, rfwid);
            }

            FNamePlate.push_back(KMCNPLoadedWidget(rftid, rfwid, ffont_x, ffont_y, fname_x, fname_y));

            FLoadedNTIDsConfigs.push_back(std::make_pair(
                rftid, KMCDispConfigs(rftid, ffont_x, ffont_y, fnpsetting.r, fnpsetting.g, fnpsetting.b)));
            if (rfwid > 0) {
                FLoadedNWIDsConfigs.push_back(std::make_pair(rfwid, KMCDispConfigs(rfwid, fname_x, fname_y)));
            }
        }
    }


    void KMCCutin::InitLoop(std::vector<std::pair<std::string, std::string>> *ar,
                  std::vector<std::pair<std::string, std::string>> *awwf,
                  std::vector<std::pair<std::string, std::string>> *aw,
                  std::vector<std::pair<uint64_t, KMCLoadedWidgetData>> *loadedWedget,
                  std::vector<std::pair<uint64_t, int>> *loadedText,
                  std::vector<std::pair<int, KMCDispConfigs>> *loadedTIDsConfigs,
                  std::vector<std::pair<int, KMCDispConfigs>> *loadedWIDsConfigs, int offset_tx, int offset_ty,
                  int offset_wx, int offset_wy, const std::string *target) {
        auto *autoWordCategories = KMCCT::KMCConfig::GetSingleton()->getIAutoWordCategoriesConfigs();
        std::set<uint64_t> pushbackResult;

        LOG("Load animation widget");
        // animation widget
        for (auto [key, value] : *ar) {
            try {
                auto be = KMCSplit(value, ',');
                uint64_t awcKey = std::stoi(key);
                uint64_t k = std::stoi(key);
                loadedWedget->push_back(std::make_pair(k, KMCLoadedWidgetData(true, 0, std::stoi(be.at(1)), be.at(2))));
                LOG("InitLW push_back key: {} size: {}", k, be.at(1));

                auto itf = std::find_if(autoWordCategories->begin(), autoWordCategories->end(),
                                        [key](const auto &p) { return p.first == key; });
                if (itf != autoWordCategories->end()) {
                    for (int i = 0; i < std::stoi(be.at(1)); i++) {
                        std::string k1 = itf->second;
                        std::string k2 = k1;
                        std::transform(k1.begin(), k1.end(), k2.begin(), [](char c) { return std::tolower(c); });

                        int wx = KMC_DEFAULT_WIDGET_POS_X;
                        int wy = KMC_DEFAULT_WIDGET_POS_Y;

                        auto fontmap =
                            std::find_if(awwf->begin(), awwf->end(), [key](const auto &p) { return p.first == key; });

                        if (fontmap != awwf->end()) {
                            std::string fontSettings = fontmap->second;
                            std::vector<std::string> fsettings = KMCSplit(fontSettings, ',');
                            wx = std::stoi(fsettings.at(2));
                            wy = std::stoi(fsettings.at(3));
                        }

                        std::string filename = KMCCT::PICT_PATH1 + "/" + *target + "/" + key + "/" + KMCCT::PICT_PATH2 +
                                               k2 + std::to_string(i + 1) + KMCCT::PICT_TYPE;
                        if (IsFileExist(KMCCT::PICT_ROOT + filename)) {
                            int wid = IWW::MainFunctions::GetSingleton()->LoadWidget(ct_aaaakmcroot, filename, 10000,
                                                                                     10000, false);
                            LOG("filename: {} root: {} category: {}", filename, ct_aaaakmcroot, k1);
                            wid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitMultLoad(wid, k, i,
                                                                                           loadedWedget);
                            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                                return;
                            }

                            IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, wid, wx + offset_wx);
                            IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, wid, wy + offset_wy);

                            loadedWIDsConfigs->push_back(
                                std::make_pair(wid, KMCDispConfigs(wid, wx + offset_wx, wy + offset_wy)));
                        } else {
                            WARN("File path not found. If not intended, no problem. {}", filename);
                        }
                    }
                    pushbackResult.insert(awcKey);
                } else {
                    ERROR("ERROR LOADING AnimationRange.json. Not Found autoWordCategories key: {}", key);
                }
            } catch (...) {
                ERROR(
                    "ERROR AnimationRange.json or AutoWordWFConfig.json. The number of elements in the value is wrong. "
                    ": {}",
                    *target);
                // return;
            }
        }

        LOG("Load widget");
        // widget
        for (auto [key, value] : *autoWordCategories) {
            try {
                uint64_t k = std::stoi(key);

                if ((pushbackResult.find(k) != pushbackResult.end()) == false) {
                    std::string k1 = value;
                    std::string k2 = k1;
                    std::transform(k1.begin(), k1.end(), k2.begin(), [](char c) { return std::tolower(c); });

                    int wx = KMC_DEFAULT_WIDGET_POS_X;
                    int wy = KMC_DEFAULT_WIDGET_POS_Y;

                    auto fontmap =
                        std::find_if(awwf->begin(), awwf->end(), [key](const auto &p) { return p.first == key; });

                    if (fontmap != awwf->end()) {
                        std::string fontSettings = fontmap->second;
                        std::vector<std::string> fsettings = KMCSplit(fontSettings, ',');
                        wx = std::stoi(fsettings.at(2));
                        wy = std::stoi(fsettings.at(3));
                    }

                    std::string filename =
                        KMCCT::PICT_PATH1 + "/" + *target + "/" + key + "/" + KMCCT::PICT_PATH2 + k2 + KMCCT::PICT_TYPE;
                    if (IsFileExist(KMCCT::PICT_ROOT + filename)) {
                        int wid =
                            IWW::MainFunctions::GetSingleton()->LoadWidget(ct_aaaakmcroot, filename, 10000, 10000, false);

                        wid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoad(wid, k, loadedWedget);
                        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                            return;
                        }
                        IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, wid, wx + offset_wx);
                        IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, wid, wy + offset_wy);

                        loadedWIDsConfigs->push_back(
                            std::make_pair(wid, KMCDispConfigs(wid, wx + offset_wx, wy + offset_wy)));
                        LOG("KMCEventThread::LOAD fileName = {} id = {}", filename, wid);
                    } else {
                        loadedWedget->push_back(std::make_pair(k, KMCLoadedWidgetData(false, -1, 0, "")));
                        WARN("File path not found. If not intended, no problem. {}", filename);
                    }
                }
            } catch (...) {
                ERROR("ERROR AutoWordWFConfig.json. The number of elements in the value is wrong. : {}", *target);
                // return;
            }
        }

        LOG("Load text");
        // Text
        for (auto [key, value] : *aw) {
            try {
                uint64_t k = std::stoi(key);

                std::string font = KMC_DEFAULT_FONT;
                int fontsize = KMC_DEFAULT_FONT_SIZE;
                int tx = KMC_DEFAULT_TEXT_POS_X;
                int ty = KMC_DEFAULT_TEXT_POS_Y;
                int r = 0, g = 0, b = 0;

                auto fontmap =
                    std::find_if(awwf->begin(), awwf->end(), [key](const auto &p) { return p.first == key; });

                if (fontmap != awwf->end()) {
                    std::string fontSettings = fontmap->second;
                    std::vector<std::string> fsettings = KMCSplit(fontSettings, ',');
                    font = fsettings.at(0);
                    fontsize = std::stoi(fsettings.at(1));
                    tx = std::stoi(fsettings.at(4));
                    ty = std::stoi(fsettings.at(5));
                    r = std::stoi(fsettings.at(6));
                    g = std::stoi(fsettings.at(7));
                    b = std::stoi(fsettings.at(8));
                }
                int wid = IWW::MainFunctions::GetSingleton()->LoadText(ct_aaaakmcroot, value, font, fontsize, 10000, 10000,
                                                                       false);

                wid = KMCCT::KMCEventThread::GetSingleton()->wrap_WaitLoadText(wid, k, loadedText);
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    return;
                }
                IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, wid, tx + offset_tx);
                IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, wid, ty + offset_ty);
                IWW::MainFunctions::GetSingleton()->SetRGB(ct_aaaakmcroot, wid, r, g, b);

                loadedTIDsConfigs->push_back(
                    std::make_pair(wid, KMCDispConfigs(wid, tx + offset_tx, ty + offset_ty, r, g, b)));
                LOG("KMCEventThread::LOAD TEXT = {} id = {} FONT = {} FONT SIZE = {}", value, wid, font, fontsize);
            } catch (...) {
                ERROR("ERROR AutoWordWFConfig.json. The number of elements in the value is wrong. : {}", *target);
                // return;
            }
        }
    }

    int KMCCutin::GetCutInID(std::string aaaakmctype) {
        int rand = -1;

        auto findit = aaaakmcCategoryRandMap.find(aaaakmctype);
        if (findit != aaaakmcCategoryRandMap.end()) {
            auto *randData = &(findit->second);
            int of = randData->offset;
            auto r = randData->randValues;
            rand = r[of];
            ++of;
            int mi = randData->maxIndex;
            if (of > mi) {
                of = 0;
                auto randomValue = MakeRandArraySelect(randData->size, randData->low, randData->high);
                randData->randValues = std::move(randomValue);
            }

            randData->offset = of;
        }
        return rand;
    }

#pragma region Randomizer Widget Function
    void KMCCutin::CategoryRandomizer() {
        aaaakmcCategoryRandMap.clear();

        std::vector<std::pair<std::string, std::string>> *autoWordRangeConfigs =
            KMCCT::KMCConfig::GetSingleton()->getIAutoWordRangeConfigs();
        std::vector<std::pair<std::string, std::string>> *autoWordCategoriesConfigs =
            KMCCT::KMCConfig::GetSingleton()->getIAutoWordCategoriesConfigs();

        // animation widget
        for (auto [key, value] : *autoWordCategoriesConfigs) {
            auto findit = aaaakmcCategoryRandMap.find(value);
            if (findit != aaaakmcCategoryRandMap.end()) {
                continue;
            }

            std::string k1 = value;
            std::string k2 = k1;
            std::transform(k1.begin(), k1.end(), k2.begin(), [](char c) { return std::tolower(c); });
            std::string tLow = k2 + "low";
            std::string tHigh = k2 + "high";
            auto it1 = std::find_if(autoWordRangeConfigs->begin(), autoWordRangeConfigs->end(),
                                    [tLow](const auto &p) { return p.first == tLow; });
            auto it2 = std::find_if(autoWordRangeConfigs->begin(), autoWordRangeConfigs->end(),
                                    [tHigh](const auto &p) { return p.first == tHigh; });
            if (it1 != autoWordRangeConfigs->end() && it2 != autoWordRangeConfigs->end()) {
                int l = std::stoi(it1->second);
                int h = std::stoi(it2->second);

                if ((h - l) + 1 <= 0) {
                    ERROR("Error KMCEventThread::CategoryRandomizer range error type = {} low = {} high = {}", k2, tLow,
                          tHigh);
                }

                size_t size = (h - l) + 1;
                auto randomValue = MakeRandArraySelect(size, l, h);

                aaaakmcCategoryRandMap.insert(std::make_pair(value, KMCRandomData(0, h - l, randomValue, h, l, size)));

                // for (int i = 0; i < randomValue.size(); i++) {
                //     LOG("KMCEventThread::CategoryRandomizer type = {} rand = {}", value, randomValue[i]);
                // }

            } else {
                ERROR("Error KMCEventThread::CategoryRandomizer auto word range configs type = {} low = {} high = {}",
                      k2, tLow, tHigh);
            }
        }
    }
#pragma endregion

    void KMCCutin::DispPFWidget(KMCAnimST st) {
        int r = st.rand;
        std::string srnd = std::to_string(r);
        std::string timing = KMCCT::ST_AFTER;

        auto speachTiming = st.ISpeechTiming;
        if (speachTiming != nullptr) {
            auto stimingit = std::find_if(speachTiming->begin(), speachTiming->end(),
                                          [srnd](const auto &p) { return p.first == srnd; });
            if (stimingit != speachTiming->end()) {
                timing = stimingit->second;
            }
        }

        // auto ppl = KMCCT::KMCEventThread::GetSingleton()->getPlayerNamePlate();
        // auto fpl = (*KMCCT::KMCEventThread::GetSingleton()->getFNamePlate())[st.frand];

        // st.nppw = *ppl;
        // st.npfw = fpl;
        st.wt = simply;
        st.wet = end_fadeout;

        std::map<int, KMCCutinOrder> *animmap = nullptr;

        // disp widget
        if (st.isAnim || st.isfAnim) {
            if (st.isAnim && st.isfAnim) {
                // player follower anim widget
                if (timing == KMCCT::ST_BEFORE) {
                    animmap = &AnimFtoAnimP;
                } else {
                    animmap = &AnimPtoAnimF;
                }
            } else if (st.isAnim) {
                if (timing == KMCCT::ST_BEFORE) {
                    animmap = &FtoAnimP;
                } else {
                    animmap = &AnimPtoF;
                }
            } else if (st.isfAnim) {
                if (timing == KMCCT::ST_BEFORE) {
                    animmap = &AnimFtoP;
                } else {
                    animmap = &PtoAnimF;
                }
            }
        } else {
            if (timing == KMCCT::ST_BEFORE) {
                animmap = &FtoP;
            } else {
                animmap = &PtoF;
            }
        }

        if (animmap == nullptr) {
            ERROR("not found cutin type");
            return;
        }

        for (auto [key, value] : *animmap) {
            value.func(&st, value.playerorfollower);
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }
        }
    }

    void KMCCutin::DispPWidget(KMCAnimST st) {
        st.wt = simply;
        st.wet = end_fadeout;
        std::map<int, KMCCutinOrder> *animmap;

        // disp widget
        if (st.isAnim) {
            animmap = &AnimPlayerOnly;
        } else {
            animmap = &WidgetPlayerOnly;
        }

        if (animmap == nullptr) {
            ERROR("not found cutin type");
            return;
        }

        for (auto [key, value] : *animmap) {
            value.func(&st, value.playerorfollower);
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }
        }
    }


    void KMCCutin::AnimationLoop(long long time, std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator it) {
        time_point<Clock> start = Clock::now();
        time_point<Clock> end;
        long long dur = 0;

        LOG("--------------KMCEventThread::AnimationLoop START------------");

        bool isloop = it->second.isLoop;
        std::vector<int>::iterator anim;

        while (true) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            if (isloop == true) {
                for (anim = it->second.animWedget.begin(); anim != it->second.animWedget.end(); ++anim) {
                    if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                        return;
                    }
                    // int wid = *anim;
                    if (*anim > 0) {
                        IWW::MainFunctions::GetSingleton()->SendToFront(ct_aaaakmcroot, *anim);
                        LOG("KMCEventThread::AnimationLoop wid = {}", *anim);
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));
                }
            }

            end = Clock::now();
            milliseconds diff = duration_cast<milliseconds>(end - start);
            dur = diff.count();
            if (dur >= time) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));
        }

        LOG("--------------KMCEventThread::AnimationLoop END------------");

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));
    }

    void KMCCutin::AnimationLoop(long long time, std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator it,
                       std::string trackid, int frand, std::string record, float volume) {
        time_point<Clock> start = Clock::now();
        long long now = 0;

        time_point<Clock> end;
        long long dur = 0;

        LOG("--------------KMCEventThread::AnimationLoop START------------");

        bool isloop = it->second.isLoop;
        std::vector<int>::iterator anim;

        std::string nrecord = record;
        while (true) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            if (isloop == true) {
                for (anim = it->second.animWedget.begin(); anim != it->second.animWedget.end(); ++anim) {
                    if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                        return;
                    }
                    // int wid = *anim;
                    if (*anim > 0) {
                        IWW::MainFunctions::GetSingleton()->SendToFront(ct_aaaakmcroot, *anim);
                        LOG("KMCEventThread::AnimationLoop wid = {}", *anim);
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));

                    now += KMCCT::ANIMATION_FRAME_MS;
                    if (nrecord != "" && now >= time * std::stof(nrecord)) {
                        LOG(" time ms {} record {} dur {}", time, nrecord, now);
                        KMCCT::KMCSound::GetSingleton()->PlaySEEx(trackid, frand, &nrecord, volume);
                    }
                }
            }

            end = Clock::now();
            milliseconds diff = duration_cast<milliseconds>(end - start);
            dur = diff.count();
            if (dur >= time) {
                break;
            }

            now = dur;
            if (nrecord != "" && now >= time * std::stof(nrecord)) {
                LOG(" time ms {} record {} dur {}", time, nrecord, now);
                KMCCT::KMCSound::GetSingleton()->PlaySEEx(trackid, frand, &nrecord, volume);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));
        }

        LOG("--------------KMCEventThread::AnimationLoop END------------");

        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));
    }

    void KMCCutin::PlaySE(long long time, std::string trackid, int frand, std::string record, float volume) {
        time_point<Clock> start = Clock::now();
        long long now = 0;

        time_point<Clock> end;
        long long dur = 0;

        std::vector<int>::iterator anim;

        std::string nrecord = record;
        while (true) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            end = Clock::now();
            milliseconds diff = duration_cast<milliseconds>(end - start);
            dur = diff.count();
            if (dur >= time) {
                break;
            }

            now = dur;
            if (nrecord != "" && now >= time * std::stof(nrecord)) {
                LOG(" time ms {} record {} dur {}", time, nrecord, now);
                KMCCT::KMCSound::GetSingleton()->PlaySEEx(trackid, frand, &nrecord, volume);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));
        }
    }

    void KMCCutin::MCMSettingChange(std::vector<float> *floatArray) {
        aaaakmcvolum = (*floatArray)[0];
        aaaakmcAnimtime = (*floatArray)[1];
        aaaakmctime = (*floatArray)[2];
        aaaakmcCycle = (*floatArray)[3];

        struct internal_config {
        public:
            internal_config() {}
            internal_config(std::string ex1, std::string ey1, std::string ex2, std::string ey2, std::string *ox,
                            std::string *oy) {
                x1 = ex1;
                y1 = ey1;
                x2 = ex2;
                y2 = ey2;
                outx = ox;
                outy = oy;
            }
            std::string x1, y1, x2, y2, *outx, *outy;
        };

        std::string playerTX = std::to_string((int)(*floatArray)[4]);
        std::string playerTY = std::to_string((int)(*floatArray)[5]);
        std::string playerWX = std::to_string((int)(*floatArray)[6]);
        std::string playerWY = std::to_string((int)(*floatArray)[7]);

        std::string playerNTX = std::to_string((int)(*floatArray)[8]);
        std::string playerNTY = std::to_string((int)(*floatArray)[9]);
        std::string playerNWX = std::to_string((int)(*floatArray)[10]);
        std::string playerNWY = std::to_string((int)(*floatArray)[11]);

        std::string followerTX = std::to_string((int)(*floatArray)[12]);
        std::string followerTY = std::to_string((int)(*floatArray)[13]);
        std::string followerWX = std::to_string((int)(*floatArray)[14]);
        std::string followerWY = std::to_string((int)(*floatArray)[15]);

        std::string followerNTX = std::to_string((int)(*floatArray)[16]);
        std::string followerNTY = std::to_string((int)(*floatArray)[17]);
        std::string followerNWX = std::to_string((int)(*floatArray)[18]);
        std::string followerNWY = std::to_string((int)(*floatArray)[19]);

        LOG("MCMSettingChange aaaakmcvolum {}", (*floatArray)[0]);
        LOG("MCMSettingChange aaaakmcAnimtime {}", (*floatArray)[1]);
        LOG("MCMSettingChange aaaakmctime {}", (*floatArray)[2]);
        LOG("MCMSettingChange aaaakmcCycle {}", (*floatArray)[3]);

        std::vector<std::pair<std::string, internal_config>> posconfigs_loop = {
            std::make_pair("PT", internal_config(playerTX, playerTY, aaaaPlayerTextX, aaaaPlayerTextY, &aaaaPlayerTextX,
                                                 &aaaaPlayerTextY)),
            std::make_pair("PW", internal_config(playerWX, playerWY, aaaaPlayerWidgetX, aaaaPlayerWidgetY,
                                                 &aaaaPlayerWidgetX, &aaaaPlayerWidgetY)),
            std::make_pair("FT", internal_config(followerTX, followerTY, aaaaFollowerTextX, aaaaFollowerTextY,
                                                 &aaaaFollowerTextX, &aaaaFollowerTextY)),
            std::make_pair("FW", internal_config(followerWX, followerWY, aaaaFollowerWidgetX, aaaaFollowerWidgetY,
                                                 &aaaaFollowerWidgetX, &aaaaFollowerWidgetY)),
            std::make_pair("NPT", internal_config(playerNTX, playerNTY, aaaaPlayerNTextX, aaaaPlayerNTextY,
                                                  &aaaaPlayerNTextX, &aaaaPlayerNTextY)),
            std::make_pair("NPW", internal_config(playerNWX, playerNWY, aaaaPlayerNWidgetX, aaaaPlayerNWidgetY,
                                                  &aaaaPlayerNWidgetX, &aaaaPlayerNWidgetY)),
            std::make_pair("NFT", internal_config(followerNTX, followerNTY, aaaaFollowerNTextX, aaaaFollowerNTextY,
                                                  &aaaaFollowerNTextX, &aaaaFollowerNTextY)),
            std::make_pair("NFW", internal_config(followerNWX, followerNWY, aaaaFollowerNWidgetX, aaaaFollowerNWidgetY,
                                                  &aaaaFollowerNWidgetX, &aaaaFollowerNWidgetY))};

        KMCWaitConfigs task;
        for (int i = 0; i < posconfigs_loop.size(); i++) {
            auto conf = posconfigs_loop.at(i);
            std::string key = conf.first;
            internal_config value = conf.second;
            std::string x, y, x1, y1;
            x = value.x1;
            y = value.y1;
            x1 = value.x2;
            y1 = value.y2;
            if (x != x1 || y != y1) {
                *(value.outx) = x;
                *(value.outy) = y;
                task.SetCallBackTask(KMCMoveWidgetWaitTask, KMCWaitTaskParam(x, y, key));
            }
        }

        if (task.isCBTask) {
            KMCCT::KMCWaitTask::GetSingleton()->KMCPushWaitTask(KMCWaitType::move_widget, task);
        }
    }

#pragma region interrupt event
    void KMCCT::InterruptEventAddItem() {
        int32_t goldvalue = std::stoll(aaaakmcInterruptData.condition1);
        int32_t value = std::stoll(aaaakmcInterruptData.condition2);

        if (goldvalue >= value) {
            KMCCT::KMCCutin::GetSingleton()->IterCutIn(
                KMCCutinValues(aaaakmcInterruptData.type, aaaakmctime, aaaakmcAnimtime, aaaakmcvolum));
            //::KMCCT::KMCEventThread::GetSingleton()->CutInCreate({aaaakmcInterruptData.type});
        }
    }

    void KMCCT::InterruptEventOnHit() {
        std::string is_power_attack = aaaakmcInterruptData.condition1;
        std::string mod = aaaakmcInterruptData.condition2;
        std::string type = aaaakmcInterruptData.type;

        if (is_power_attack == "1") {
            KMCCT::KMCCutin::GetSingleton()->IterCutIn(
                KMCCutinValues(type + mod, aaaakmctime, aaaakmcAnimtime, aaaakmcvolum));
            //::KMCCT::KMCEventThread::GetSingleton()->CutInCreate({type + mod});
        } else {
            KMCCT::KMCCutin::GetSingleton()->IterCutIn(
                KMCCutinValues(type, aaaakmctime, aaaakmcAnimtime, aaaakmcvolum));
            //::KMCCT::KMCEventThread::GetSingleton()->CutInCreate({type});
        }
    }
#pragma endregion

#pragma region function pointer
    void WidgetVisible(KMCAnimST *st, int &playerorfollower) {
        int wid;
        if (playerorfollower == 0) {
            if (!st->pcf.Widget) return;
            wid = st->t->second.wedget;
        } else {
            if (!st->fcf.Widget) return;
            wid = st->ft->second.wedget;
        }

        if (wid > 0) {
            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, wid, true);
        }
    }

    void WidgetInVisible(KMCAnimST *st, int &playerorfollower) {
        int wid;
        if (playerorfollower == 0) {
            if (!st->pcf.Widget) return;
            wid = st->t->second.wedget;
        } else {
            if (!st->fcf.Widget) return;
            wid = st->ft->second.wedget;
        }

        if (wid > 0) {
            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, wid, false);
        }
    }

    void AnimWidgetVisible(KMCAnimST *st, int &playerorfollower) {
        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator it;
        if (playerorfollower == 0) {
            if (!st->pcf.Widget) return;
            it = st->t;
        } else {
            if (!st->fcf.Widget) return;
            it = st->ft;
        }

        std::vector<int>::iterator anim;
        for (anim = it->second.animWedget.begin(); anim != it->second.animWedget.end(); ++anim) {
            // int wid = *anim;
            if (*anim > 0) {
                IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, *anim, true);
                std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));
            }
        }
    }

    void AnimWidgetInVisible(KMCAnimST *st, int &playerorfollower) {
        std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator it;
        if (playerorfollower == 0) {
            if (!st->pcf.Widget) return;
            it = st->t;
        } else {
            if (!st->fcf.Widget) return;
            it = st->ft;
        }

        std::vector<int>::iterator anim;
        for (anim = it->second.animWedget.begin(); anim != it->second.animWedget.end(); ++anim) {
            if (*anim > 0) {
                IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, *anim, false);
                std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_INVISIBLE_MS));
            }

            // LOG("KMCEventThread::AnimationLoop visible false id = {}", *anim);
        }
    }

    void TextVisible(KMCAnimST *st, int &playerorfollower) {
        int tid;
        if (playerorfollower == 0) {
            if (!st->pcf.Word) return;
            tid = st->tid;
        } else {
            if (!st->fcf.Word) return;
            tid = st->ftid;
        }

        // sub title
        if (tid > 0) {
            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, tid, true);
        }
    }

    void TextInVisible(KMCAnimST *st, int &playerorfollower) {
        int tid;
        if (playerorfollower == 0) {
            if (!st->pcf.Word) return;
            tid = st->tid;
        } else {
            if (!st->fcf.Word) return;
            tid = st->ftid;
        }

        // sub title
        if (tid > 0) {
            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, tid, false);
        }
    }

    void KMCPlayAnim(KMCAnimST *st, int &playerorfollower) {
        bool isanim = false;

        if (playerorfollower == 0) {
            if (st->pcf.Widget) {
                isanim = true;
            }
        } else {
            if (st->fcf.Widget) {
                isanim = true;
            }
        }
        if (isanim) {
            std::string r;
            float volum;
            RE::TESObjectREFR *target;
            KMCCompsFlag cf;
            int frand;
            long long time;
            std::vector<std::pair<uint64_t, KMCLoadedWidgetData>>::iterator t;
            std::string record = "0.0";
            if (playerorfollower == 0) {
                // player
                r = std::to_string(st->rand);
                volum = st->volum;
                target = st->speakerp;
                frand = -1;
                time = st->time;
                t = st->t;
                cf = st->pcf;
                record = st->precord;
            } else {
                r = std::to_string(st->rand);
                volum = st->volum;
                target = st->speakerf;
                frand = st->frand;
                time = st->time;
                t = st->ft;
                cf = st->fcf;
                record = st->frecord;
            }

            // play sound
            if (cf.Sound) {
                KMCCT::KMCSound::GetSingleton()->PlayEx(r, volum, target, frand);
            }

            if (cf.SE) {
                KMCCT::KMCCutin::GetSingleton()->AnimationLoop(time, t, r, frand, record, volum);
            } else {
                KMCCT::KMCCutin::GetSingleton()->AnimationLoop(time, t);
            }
        } else {
            KMCPlay(st, playerorfollower);
        }
    }

    void KMCPlay(KMCAnimST *st, int &playerorfollower) {
        std::string r;
        float volum;
        RE::TESObjectREFR *target;
        int frand;
        long long time;
        KMCCompsFlag cf;
        std::string record = "";
        if (playerorfollower == 0) {
            // player
            r = std::to_string(st->rand);
            volum = st->volum;
            target = st->speakerp;
            frand = -1;
            time = st->time;
            cf = st->pcf;
            record = st->precord;
        } else {
            r = std::to_string(st->rand);
            volum = st->volum;
            target = st->speakerf;
            frand = st->frand;
            time = st->time;
            cf = st->fcf;
            record = st->frecord;
        }

        if (cf.Sound) {
            KMCCT::KMCSound::GetSingleton()->PlayEx(r, volum, target, frand);
        }

        if (cf.SE) {
            KMCCT::KMCCutin::GetSingleton()->PlaySE(time, r, frand, record, volum);
        } else {
            KMCTimer(time);
        }
    }

    void KMCNamePlateStartAnim(KMCAnimST *st, int &playerorfollower) {
        KMCNPLoadedWidget id;
        KMCWipeType type;
        KMCCompsFlag cf;

        if (playerorfollower == 0) {
            id = st->nppw;
            type = st->wt;
            cf = st->pcf;
        } else {
            id = st->npfw;
            type = st->wt;
            cf = st->fcf;
        }

        if (cf.NamePlate && id.LoadedWidget != -1) {
            // IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, id.LoadedWidget, id.defaultWX);
            // IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, id.LoadedWidget, id.defaultWY);

            switch (type) {
                case simply:
                    NamePlateSimplyWipe(id, ct_aaaakmcroot);
                    break;
            }
        }

        if (cf.NamePlateName && id.LoadedText != -1) {
            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, id.LoadedText, true);
        }
    }

    void KMCNamePlateEndAnim(KMCAnimST *st, int &playerorfollower) {
        KMCNPLoadedWidget id;
        KMCWipeType type;
        KMCCompsFlag cf;

        if (playerorfollower == 0) {
            id = st->nppw;
            type = st->wet;
            cf = st->pcf;
        } else {
            id = st->npfw;
            type = st->wet;
            cf = st->fcf;
        }

        if (cf.NamePlate && id.LoadedWidget != -1) {
            switch (type) {
                case end_fadeout:
                    NamePlateFadeOut(id, ct_aaaakmcroot);
                    break;
            }
        }

        if (cf.NamePlateName && id.LoadedText != -1) {
            IWW::MainFunctions::GetSingleton()->SetVisible(ct_aaaakmcroot, id.LoadedText, false);
        }
    }

    void KMCOARFuncStart(KMCAnimST *st, int &playerorfollower) {
        int frand;
        int rand;

        rand = st->rand;

        if (playerorfollower == 0) {
            frand = -1;
        } else {
            frand = st->frand;
        }

        KMCCT::KMCOAR::GetSingleton()->PushOARFunc(rand, frand, st->overri_oar_time, st->oar_time);
    }

    void KMCExpFuncStart(KMCAnimST* st, int& playerorfollower) {
        int frand;
        int rand;

        rand = st->exp_rand == -1 ? st->rand : st->exp_rand;

        if (playerorfollower == 0) {
            frand = -1;
        } else {
            frand = st->frand;
        }

        KMCCT::KMCExpression::GetSingleton()->PushExpFunc(rand, frand, st->overri_exp_time, st->exp_time);
    }
#pragma endregion


    void KMCMoveWidgetWaitTask(KMCWaitTaskParam st) {
        std::vector<std::pair<int, KMCDispConfigs>> *loadedIDsConfigs = nullptr;
        KMCNPLoadedWidget *pnp = KMCCutin::GetSingleton()->GetPlayerNamePlate();
        std::vector<KMCNPLoadedWidget> *fnp = KMCCutin::GetSingleton()->GetFNamePlate();

        if (st.value3 == "PT") {
            loadedIDsConfigs = KMCCutin::GetSingleton()->GetLoadedTIDsConfigs();
        } else if (st.value3 == "PW") {
            loadedIDsConfigs = KMCCutin::GetSingleton()->GetLoadedWIDsConfigs();
        } else if (st.value3 == "FT") {
            loadedIDsConfigs = KMCCutin::GetSingleton()->GetFLoadedTIDsConfigs();
        } else if (st.value3 == "FW") {
            loadedIDsConfigs = KMCCutin::GetSingleton()->GetFLoadedWIDsConfigs();
        } else if (st.value3 == "NPT") {
            loadedIDsConfigs = KMCCutin::GetSingleton()->GetLoadedNTIDsConfigs();
        } else if (st.value3 == "NPW") {
            loadedIDsConfigs = KMCCutin::GetSingleton()->GetLoadedNWIDsConfigs();
        } else if (st.value3 == "NFT") {
            loadedIDsConfigs = KMCCutin::GetSingleton()->GetFLoadedNTIDsConfigs();
        } else if (st.value3 == "NFW") {
            loadedIDsConfigs = KMCCutin::GetSingleton()->GetFLoadedNWIDsConfigs();
        } else {
            return;
        }

        for (int i = 0; i < loadedIDsConfigs->size(); i++) {
            std::pair<int, KMCDispConfigs> *configp = &(loadedIDsConfigs->at(i));
            int id = configp->first;
            int defx = configp->second.defx;
            int defy = configp->second.defy;
            int tox = std::stoi(st.value1);
            int toy = std::stoi(st.value2);

            int posx = defx + tox;
            int posy = defy + toy;

            IWW::MainFunctions::GetSingleton()->SetPosX(ct_aaaakmcroot, id, defx + tox);
            IWW::MainFunctions::GetSingleton()->SetPosY(ct_aaaakmcroot, id, defy + toy);

            if (st.value3 == "NPT") {
                pnp->defaultTX = posx;
                pnp->defaultTY = posy;
            } else if (st.value3 == "NPW") {
                pnp->defaultWX = posx;
                pnp->defaultWY = posy;
            } else if (st.value3 == "NFT") {
                loadedIDsConfigs = KMCCutin::GetSingleton()->GetFLoadedNTIDsConfigs();
                for (int j = 0; j < fnp->size(); j++) {
                    auto *np = &((*fnp).at(j));
                    if (np->LoadedText == id) {
                        np->defaultTX = posx;
                        np->defaultTY = posy;
                    }
                }
            } else if (st.value3 == "NFW") {
                loadedIDsConfigs = KMCCutin::GetSingleton()->GetFLoadedNWIDsConfigs();
                for (int j = 0; j < fnp->size(); j++) {
                    auto *np = &((*fnp).at(j));
                    if (np->LoadedWidget == id) {
                        np->defaultWX = posx;
                        np->defaultWY = posy;
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::SET_POS_MS));
        }
    }
}