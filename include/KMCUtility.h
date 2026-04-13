#pragma once
#include <chrono>

using Clock = std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::time_point;
using namespace std::literals::chrono_literals;

namespace KMCCT {
    // Frame Rate Setting
    const int ANIMATION_FRAME_MS = 60;
    const int CALL_LOAD_FRAME_MS = 8;
    const int CALL_INVISIBLE_MS = 8;
    const int WHILE_WAIT_TIME = 1000;
    const int CUT_IN_COND_WHILE_WAIT_TIME = 1000;
    const int WAIT_CYCLE_MS = 3000;
    const int CALL_DESTROY_MS = 4;
    const int SET_POS_MS = 8;
    const int SET_ALPHA_MS = 8;
    const int WHILE_PROFILE_SHOW_WAIT_TIME = 500;
    const int INSPECTION_LOOP_MS = 1000;
    const int PAPYRUS_UPDATE_WHILE_WAIT_TIME = 1000;
    const int WHILE_WAIT_TIME_EXP = 500;
    // ms
    const long long TIME_SCALE_MS = 1000;

    const float F_CUT_IN_COND_WHILE_WAIT_TIME = (float)CUT_IN_COND_WHILE_WAIT_TIME / 1000.0f;
    const float DELAY_THRESHHOLD = 0.01f;
    const double ROUND_N = 4;

    const std::string IS_LOOP = "loop";
    const std::string IS_NO_LOOP = "noloop";

    template <class Event>
    class CallbackEventSink : public RE::BSTEventSink<Event> {
        std::function<void(const Event *)> _callback;

    public:
        CallbackEventSink(std::function<void(const Event *)> callback) : _callback(callback) {}
        RE::BSEventNotifyControl ProcessEvent(const Event *event, RE::BSTEventSource<Event> *) override {
            _callback(event);
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    template <typename EventType>
    void On(std::function<void(const EventType *)> callback) {
        RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<EventType>(
            new CallbackEventSink<EventType>(callback));
    }

    template <typename TopicEventType>
    void OnUI(std::function<void(const TopicEventType *)> callback) {
        RE::UI::GetSingleton()->AddEventSink<TopicEventType>(new CallbackEventSink<TopicEventType>(callback));
    }

    template <class... Args>
    void ConsoleLog(fmt::format_string<Args...> text, Args &&...args) {
        auto *log = RE::ConsoleLog::GetSingleton();
        if (log)
            log->Print(fmt::format(text, std::forward<Args>(args)...).c_str());
        else
            SKSE::log::error(
                "Failed to get RE::ConsoleLog::GetSingleton(). Did you call ConsoleLog() before OnDataLoaded?");
    }

    enum KMCWipeType { simply, end_fadeout };

    enum class KMCValueType { KM_INT, KM_FLOAT, KM_STRING, KM_LONG, UNK };

    enum class KMCWaitType { in_scene, move_widget, max };

    enum class KMCInterruptType { add_item, on_hit };

    enum class KMCDetectionType { keyword, faction, magic_effect_keyword, global };

    enum class KMCInequalitySign {
        more,  // TESGlbal >= base_value
        less,
        more_than,  // TESGlobal > base_value
        less_than,
        equal,  // TESGlobal == base_value
        unk
    };

    enum class WidgetType { player_widget, player_text_widget, follower_widget, follower_text_widget, none };

    enum class KMCProfileSEType { open, end };

    struct MultiTypeValue {
    public:
        float GetValue(float &o) { return fv; }
        std::string GetValue(std::string &o) { return sv; }
        long GetValue(long &o) { return lv; }
        int GetValue(int &o) { return iv; }

        bool SetValue(std::string type, std::string value) {
            if (type == "float") {
                fv = std::stof(value);
                value_type = KMCValueType::KM_FLOAT;
                return true;
            } else if (type == "string") {
                sv = value;
                value_type = KMCValueType::KM_STRING;
                return true;
            } else if (type == "long") {
                lv = std::stol(value);
                value_type = KMCValueType::KM_LONG;
                return true;
            } else if (type == "int") {
                iv = std::stoi(value);
                value_type = KMCValueType::KM_INT;
                return true;
            }

            return false;
        }

        std::string GetMod(std::string AMod) {
            if (KMCValueType::UNK == value_type) {
                return "";
            }

            std::string result = "";

            switch (value_type) {
                case KMCValueType::KM_INT:
                    result = AMod + "i";
                    break;
                case KMCValueType::KM_FLOAT:
                    result = AMod + "f";
                    break;
                case KMCValueType::KM_STRING:
                    result = AMod + "s";
                    break;
                case KMCValueType::KM_LONG:
                    result = AMod + "l";
                    break;
            }

            return result;
        }

        void SetIntValue(int value) {
            iv = value;
            value_type = KMCValueType::KM_INT;
        }


        void SetFloatValue(float value) {
            fv = value;
            value_type = KMCValueType::KM_FLOAT;
        }

        void SetStringValue(std::string value) {
            sv = value;
            value_type = KMCValueType::KM_STRING;
        }

    public:
        KMCValueType value_type = KMCValueType::UNK;
        float fv = 0.0f;
        std::string sv = "";
        long lv = 0;
        int iv = 0;
    };

    struct KMCTESGlobalParam {
    public:
        KMCTESGlobalParam() {}

        KMCTESGlobalParam(KMCInequalitySign si, float bv) {
            isign = si;
            base_value = bv;
        }

    public:
        KMCInequalitySign isign = KMCInequalitySign::equal;
        float base_value = 0.0f;
    };

    struct OARCompDetail {
    public:
        OARCompDetail() {
            global = nullptr;
            time = 0.0f;
        }

        OARCompDetail(RE::TESGlobal *g, float t) {
            global = g;
            time = t;
        }

        RE::TESGlobal *global;
        float time;
    };

    struct STMFGPair {
    public:
        STMFGPair() {}
        std::vector<int> type;
        std::vector<int> exp;
        std::vector<int> str;
        RE::Actor *ac = nullptr;
        int time = 3;
    };

    struct KMCStrageUtilParam {
    public:
        KMCStrageUtilParam() {}

        KMCStrageUtilParam(KMCInequalitySign si, MultiTypeValue mtv, int os) {
            isign = si;
            base_value = mtv;
            offset = os;
        }

    public:
        KMCInequalitySign isign = KMCInequalitySign::equal;
        int offset = 0;
        MultiTypeValue base_value;
    };

    class KMCCompsFlag {
    public:
        KMCCompsFlag() {}

        KMCCompsFlag(bool hwid, bool hword, bool hnp, bool hnpn, bool hs, bool hse) {
            Widget = hwid;
            Word = hword;
            NamePlate = hnp;
            NamePlateName = hnpn;
            Sound = hs;
            SE = hse;
        }

        bool IsAllDisable() {
            if (Widget == false && Word == false && NamePlate == false && NamePlateName == false && Sound == false &&
                SE == false) {
                return true;
            }
            return false;
        }

    public:
        bool Widget = true;
        bool Word = true;
        bool NamePlate = true;
        bool NamePlateName = true;
        bool Sound = true;
        bool SE = true;
    };

    // follower
    class KMCFollower {
    public:
        KMCFollower(){};

        KMCFollower(std::string f, std::string p) {
            formId = f;
            pluginName = p;
        }

    public:
        std::vector<std::pair<std::string, std::string>> IAutoWordConfigs;
        std::vector<std::pair<std::string, std::string>> IAutoWordWFConfigs;
        std::vector<std::pair<std::string, std::string>> IAnimationRange;
        std::vector<std::pair<std::string, std::string>> ISpeachTiming;
        std::vector<std::pair<std::string, std::string>> IConditions;
        std::vector<std::pair<std::string, RE::BGSKeyword *>> IKeywords;
        std::vector<std::pair<std::string, KMCCompsFlag>> IHideComponents;
        RE::Actor *follower = nullptr;
        std::string formId = "";
        std::string pluginName = "";
        int index = 0;
    };

    struct KMCInterruptPushCutInData {
    public:
        KMCInterruptPushCutInData() {}
        KMCInterruptPushCutInData(KMCInterruptType itt, std::string t, std::string cond1 = "", std::string cond2 = "",
                                  std::string cond3 = "", RE::TESForm *condf1 = nullptr) {
            interruptType = itt;
            type = t;
            condition1 = cond1;
            condition2 = cond2;
            condition3 = cond3;
            conditionf1 = condf1;
        }

    public:
        KMCInterruptType interruptType;
        std::string type = "";
        std::string condition1 = "";
        std::string condition2 = "";
        std::string condition3 = "";
        RE::TESForm *conditionf1;
    };

    struct KMCUpdateProfileData {
    public:
        int tid = -1;
        std::string format_data = "";
    };

    struct KMCWaitTaskParam {
    public:
        KMCWaitTaskParam() {}

        KMCWaitTaskParam(std::string v1, std::string v2 = "", std::string v3 = "") {
            value1 = v1;
            value2 = v2;
            value3 = v3;
        }

    public:
        std::string value1 = "";
        std::string value2 = "";
        std::string value3 = "";
    };

    typedef void (*CallBackWaitTask)(KMCWaitTaskParam st);
    class KMCWaitConfigs {
    public:
        KMCWaitConfigs() {
            waittime_ms = 0;
            enable = false;
            isCBTask = false;
        }

        KMCWaitConfigs(long long wt, time_point<Clock> et, KMCWaitType t, bool e) {
            waittime_ms = wt;
            entrytime = et;
            type = t;
            enable = e;
        }

        void SetCallBackTask(CallBackWaitTask cb, KMCWaitTaskParam wtp) {
            tasks.push_back(std::make_pair(cb, wtp));
            isCBTask = true;
        }

        void ClearCallBackTask() {
            tasks.clear();
            isCBTask = false;
        }

    public:
        long long waittime_ms = 0;
        time_point<Clock> entrytime;
        KMCWaitType type;
        bool enable = false;
        std::vector<std::pair<CallBackWaitTask, KMCWaitTaskParam>> tasks;
        // CallBackWaitTask cbtask;
        bool isCBTask = false;
        // KMCWaitTaskParam wtparam;
    };

    struct KMCNamePlateAnimationSettings {
    public:
        std::vector<std::pair<std::string, std::string>> settings;
    };

    struct KMCRandomData {
    public:
        KMCRandomData(int of, int in, std::vector<int> r, int h, int l, size_t s) {
            offset = of;
            maxIndex = in;
            rand_values = r;
            high = h;
            low = l;
            size = s;
        }

    public:
        int offset = 0;
        int maxIndex = 0;
        std::vector<int> rand_values;
        int high = 0, low = 0;
        size_t size = 0;
    };

    struct KMCNamePlate {
    public:
        KMCNamePlate(std::vector<std::string> conf) {
            conf.at(0) == "disable" ? isActive = false : isActive = true;
            font = conf.at(1);
            fontsize = std::stoi(conf.at(2));
            r = std::stoi(conf.at(3));
            g = std::stoi(conf.at(4));
            b = std::stoi(conf.at(5));

            name_x = std::stoi(conf.at(6));
            name_y = std::stoi(conf.at(7));

            font_x = std::stoi(conf.at(8));
            font_y = std::stoi(conf.at(9));
        }

        bool isActive;
        std::string font;
        int fontsize;
        int r, g, b;
        int name_x, name_y;
        int font_x, font_y;
    };

    struct KMCOutputContainer {
    public:
        KMCOutputContainer() {}
        KMCOutputContainer(int o, WidgetType w) { 
            output = o;
            wt = w;
        }


        int output = -99;
        WidgetType wt = WidgetType::none;
    };
    // animation setting

    class KMCLoadedWidgetData {
    public:
        KMCLoadedWidgetData(){};

        KMCLoadedWidgetData(bool isanim, int wid, int size, std::string loop) {
            isAnim = isanim;
            if (isAnim) {
                animWedget.resize(size);
                wedget = -1;
                loop == IS_LOOP ? isLoop = true : isLoop = false;
            } else {
                wedget = wid;
            }
        }

        ~KMCLoadedWidgetData() { animWedget.clear(); }

    public:
        bool isAnim = false;
        bool isLoop = false;
        int wedget = -1;
        int bef_widget_id = -1;
        std::vector<int> animWedget;
        std::vector<int> bef_animWedget;
        bool init = false;
    };

    struct KMCDispConfigs {
    public:
        KMCDispConfigs() {}

        KMCDispConfigs(int i, int dx, int dy, int al, int r, int g, int b) {
            id = i;
            defx = dx;
            defy = dy;
            alpha = al;
            this->r = r;
            this->g = g;
            this->b = b;
        }

        KMCDispConfigs(int i, int dx, int dy) {
            id = i;
            defx = dx;
            defy = dy;
            alpha = 255;
            this->r = 255;
            this->g = 255;
            this->b = 255;
        }

        KMCDispConfigs(int i, int dx, int dy, int r, int g, int b) {
            id = i;
            defx = dx;
            defy = dy;
            alpha = 255;
            this->r = r;
            this->g = g;
            this->b = b;
        }

        KMCDispConfigs(int bl) { length = bl; }

    public:
        int id = -1;
        int defx = 0, defy = 0;
        int defsizex = 100, defsizey = 100;
        int alpha = 255;
        int r = 0, g = 0, b = 0;
        std::string font = "";
        int font_size = 0;
        int length = 0;
        bool widget_visible = true;
        std::string any1 = "";
    };

    struct KMCNPLoadedWidget {
    public:
        KMCNPLoadedWidget() {}

        KMCNPLoadedWidget(int lt, int lw, int tx, int ty, int wx, int wy) {
            LoadedText = lt;
            LoadedWidget = lw;
            defaultTX = tx;
            defaultTY = ty;
            defaultWX = wx;
            defaultWY = wy;
        }

        void set(int lt, int lw, int tx, int ty, int wx, int wy) {
            LoadedText = lt;
            LoadedWidget = lw;
            defaultTX = tx;
            defaultTY = ty;
            defaultWX = wx;
            defaultWY = wy;
        }

    public:
        int LoadedText = -1;
        int LoadedWidget = -1;

        int defaultTX = 0, defaultTY = 0;
        int defaultWX = 0, defaultWY = 0;
    };

    struct KMCProfilFormatIdMap {
    public:
        KMCProfilFormatIdMap() {}
        KMCProfilFormatIdMap(int td, int r, std::string rs, std::vector<std::string> fi) {
            tid = td;
            row = r;
            row_string = rs;
            format_id = fi;
        }

    public:
        int tid = -1;
        int row = 0;
        std::string row_string = "";
        std::vector<std::string> format_id;
    };

    struct KMCProfil {
    public:
        KMCProfil() {}

    public:
        std::vector<KMCDispConfigs> wids;
        std::vector<std::string> row_string;
        int format_id_num = 0;
        std::string format_id_strings = "";
        std::vector<std::string> format_id_strings_array;
        std::vector<std::pair<int, int>> map_index;
        std::map<int, KMCDispConfigs> tids;
        std::map<int, KMCProfilFormatIdMap> format_map;
        bool isFormat = false;
    };

    struct STNodeRelations {
    public:
        STNodeRelations() {}

    public:
        std::string target_name = "";
        std::string escape_name = "";
        std::string key_name = "";
        int timing = 0;
        bool calc = false;
        float add_value = 0.0f;
        float subtract_value = 0.0f;
        float div_value = 0.0f;
        float mult_value = 1.0f;
        float mult_to_commited_value = 1.0f;
    };

    struct STPolling {
    public:
        STPolling() {}

    public:
        float stay_time = 0.0f;
        // float put_on_hold = 0.0f;
        time_point<Clock> timer;
        // time_point<Clock> put_on_hold_timer;
        // int poh_set = 0;
    };

    struct STCutinSetting {
    public:
        STCutinSetting() {}

    public:
        bool override_setting = false;
        float time = 4.0f;
        float anim_time = 5.0f;
        float volume = 1.0f;
        float oar_time = 7.0f;
        float exp_time = 4.0f;
    };

    struct KMCAnimST {
    public:
        KMCLoadedWidgetData t;
        KMCLoadedWidgetData ft;
        int tid = -1;
        int ftid = -1;
        long long time = 0;
        bool isAnim = false;
        bool isfAnim = false;
        int rand = -1;
        int frand = -1;
        float volum = 0;
        float oar_time = 0.0f;
        bool overri_oar_time = false;
        bool overri_exp_time = false;
        float exp_time = 0.0f;
        int exp_rand = -1;
        std::vector<std::pair<std::string, std::string>> *ISpeechTiming;
        RE::TESObjectREFR *speakerp;
        RE::TESObjectREFR *speakerf;
        KMCNPLoadedWidget nppw;
        KMCNPLoadedWidget npfw;
        KMCWipeType wt;
        KMCWipeType wet;
        KMCCompsFlag pcf;
        KMCCompsFlag fcf;
        std::string precord = "";
        std::string frecord = "";
    };

    class KMCFLoadedWidget {
    public:
        KMCFLoadedWidget() {}

        KMCFLoadedWidget(std::unordered_map<int, KMCLoadedWidgetData> lw,
                         std::unordered_map<int, KMCLoadedWidgetData> lt) {
            LoadedWedget = std::move(lw);
            LoadedText = std::move(lt);
        }

        void ResetLW() {
            LoadedWedget.clear();
            LoadedText.clear();
        }

    public:
        std::unordered_map<int, KMCLoadedWidgetData> LoadedWedget;
        std::unordered_map<int, KMCLoadedWidgetData> LoadedText;
    };
    typedef void (*CutInFunction)(KMCAnimST *st, int &playerorfollower);
    struct KMCCutinOrder {
    public:
        KMCCutinOrder() {}
        KMCCutinOrder(CutInFunction f, int fop) {
            func = f;
            playerorfollower = fop;
        }

    public:
        CutInFunction func;
        int playerorfollower;
    };

    std::vector<std::string> KMCSplit(std::string str, char del);

    float round_n(float number, double n);

    // bool KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key, bool defaultValue);
    int KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key, int defaultValue);
    long long KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key,
                            long long defaultValue);
    std::string KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key,
                              std::string defaultValue);
    float KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key, float defaultValue);

    std::vector<KMCFollower> FollowerEffectiveDistance(RE::PlayerCharacter *player, std::vector<KMCFollower> *followers,
                                                       float effectiveDistance, uint64_t rand);

    std::string LoadDefaultSetting(std::string confkey, std::string e_message,
                                   std::vector<std::pair<std::string, std::string>> *defaultSet);

    std::vector<int> MakeRandArraySelect(const size_t size, int rand_min, int rand_max);

    KMCInequalitySign StringToKMCInequalitySign(std::string target);

    bool JudgeKMCInequalitySign(KMCInequalitySign sign, MultiTypeValue v1, MultiTypeValue v2);
    bool JudgeKMCInequalitySign(KMCInequalitySign sign, float v1, float v2);
    bool JudgeKMCInequalitySign(KMCInequalitySign sign, int v1, int v2);
    bool JudgeKMCInequalitySign(KMCInequalitySign sign, std::string v1, std::string v2);
    bool JudgeKMCInequalitySign(KMCInequalitySign sign, long v1, long v2);

    bool BuildMultTypeValues(std::vector<std::string> *source, std::vector<KMCValueType> *results_value_type,
                             std::vector<MultiTypeValue> *out, int source_index = 0);
    bool IsFileExist(const std::string &name);

    void GetLeftPaddingString(std::string &str, int n, char padChar = ' ');

    void GetRightPaddingString(std::string &str, int n, char padChar = ' ');

    void Replace(std::string &stringreplace, const std::string &origin, const std::string &dest);

    RE::ObjectRefHandle GetSelectedRefHandle();

    RE::NiPointer<RE::TESObjectREFR> GetSelectedRef();

    void CompileAndRunImpl(RE::Script *script, RE::ScriptCompiler *compiler, RE::COMPILER_NAME name,
                           RE::TESObjectREFR *targetRef);

    void CompileAndRun(RE::Script *script, RE::TESObjectREFR *targetRef, const RE::COMPILER_NAME name);

    void RunConsoleCommand(const std::string &command);

    template <class... Args>
    void PapyrusFuncCall(std::string script_n, std::string func_n, RE::TESForm *form, Args... a_args) {
        if (form && form->formType) {
            auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
            auto policy = vm->GetObjectHandlePolicy();
            RE::VMHandle handle = policy->GetHandleForObject(form->GetFormType(), form);

            if (handle == policy->EmptyHandle()) {
                return;
            }

            RE::BSFixedString scriptName = script_n;
            RE::BSFixedString functionName = func_n;

            RE::BSTSmartPointer<RE::BSScript::Object> object;
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;

            if (vm->FindBoundObject(handle, scriptName.c_str(), object)) {
                auto args = RE::MakeFunctionArguments(std::forward<Args>(a_args)...);
                vm->DispatchMethodCall1(object, functionName, args, result);
            }
        }
    }

    uint64_t StorageUtilCalcID(void *stack_id);

    bool IsTalking(RE::Character *a_character);

    void KMCIsWorn(RE::Actor *actor, std::vector<std::uint32_t> worn_slot, std::vector<bool> &result);
    bool KMCIsWorn(RE::Actor *actor, RE::TESObjectARMO *armo);

    std::string EscapeStringForJavaScript(const std::string &input);

    void NamePlateSimplyWipe(KMCNPLoadedWidget id, std::string aaaakmcroot);
    void NamePlateFadeOut(KMCNPLoadedWidget id, std::string aaaakmcroot);

    // std::vector<RE::TESObjectREFR *> FindAllReferencesWithKeyword(RE::TESObjectREFR *a_ref, RE::TESForm
    // *a_formOrList,
    //                                                              float a_radius, bool a_matchAll);
}