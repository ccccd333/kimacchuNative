#include "KMCUtility.h"

#include <IWWConfig.h>
#include <IWWFunctions.h>

#include "KMCEventThread.h"

std::mt19937 create_rand_engine() {
    std::random_device rnd;
    std::vector<std::uint_least32_t> v(10);
    std::generate(v.begin(), v.end(), std::ref(rnd));
    std::seed_seq seed(v.begin(), v.end());
    return std::mt19937(seed);
}
namespace KMCCT {
    namespace detail {
        template <typename T>
        auto diff(T n1, T n2) -> typename std::make_unsigned<T>::type {
            static_assert(std::is_integral<T>::value, "T is not integral.");
            if (n1 < n2) std::swap(n1, n2);
            return static_cast<typename std::make_unsigned<T>::type>(n1 - n2);
        }
    }

    std::vector<std::string> KMCSplit(std::string str, char del) {
        size_t first = 0;
        size_t last = str.find_first_of(del);
        std::vector<std::string> result;
        if (str.find(del) != std::string::npos) {
            while (first < str.size()) {
                result.push_back(str.substr(first, last - first));
                first = last + 1;
                last = str.find_first_of(del, first);
                if (last == std::string::npos) last = str.size();
            }
        } else {
            result.push_back(str);
        }

        return result;
    }

    float round_n(float number, double n) {
        number = number * pow(10, n - 1);
        number = round(number);
        number /= pow(10, n - 1);
        return number;
    }

    // bool KMCFindVector(std::vector<std::pair<std::string, std::string>>* target, std::string key, bool defaultValue)
    // {
    //     bool result = defaultValue;
    //     auto it = std::find_if(target->begin(), target->end(), [key](const auto &p) { return p.first == key; });
    //     if (it != target->end()) {
    //         int value = std::stoi(it->second);
    //         if (value == 0) {
    //             result = false;
    //         }
    //     }
    //     return result;
    // }

    int KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key, int defaultValue) {
        int result = defaultValue;
        auto it = std::find_if(target->begin(), target->end(), [key](const auto &p) { return p.first == key; });
        if (it != target->end()) {
            result = std::stoi(it->second);
        }
        return result;
    }

    long long KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key,
                            long long defaultValue) {
        long long result = defaultValue;
        auto it = std::find_if(target->begin(), target->end(), [key](const auto &p) { return p.first == key; });
        if (it != target->end()) {
            result = std::stoll(it->second);
        }
        return result;
    }

    std::string KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key,
                              std::string defaultValue) {
        std::string result = defaultValue;
        auto it = std::find_if(target->begin(), target->end(), [key](const auto &p) { return p.first == key; });
        if (it != target->end()) {
            result = it->second;
        }
        return result;
    }
    float KMCFindVector(std::vector<std::pair<std::string, std::string>> *target, std::string key, float defaultValue) {
        float result = defaultValue;
        auto it = std::find_if(target->begin(), target->end(), [key](const auto &p) { return p.first == key; });
        if (it != target->end()) {
            result = std::stof(it->second);
        }
        return result;
    }

    std::string LoadDefaultSetting(std::string confkey, std::string e_message,
                                   std::vector<std::pair<std::string, std::string>> *defaultSet) {
        std::string k = confkey;
        auto it = std::find_if(defaultSet->begin(), defaultSet->end(), [k](const auto &p) { return p.first == k; });
        if (it != defaultSet->end()) {
            std::string f = it->second;
            return it->second;
        } else {
            ERROR("{}", e_message);
        }

        return "";
    }

    std::vector<int> MakeRandArraySelect(const size_t size, int rand_min, int rand_max) {
        if (rand_min > rand_max) std::swap(rand_min, rand_max);
        const auto max_min_diff = detail::diff(rand_max, rand_min) + 1;
        if (max_min_diff < size) throw std::runtime_error("Invalid argument");

        std::vector<int> tmp;
        tmp.reserve(max_min_diff);

        for (auto i = rand_min; i <= rand_max; ++i) tmp.push_back(i);

        auto engine = create_rand_engine();

        for (size_t cnt = 0; cnt < size; ++cnt) {
            size_t pos = std::uniform_int_distribution<size_t>(cnt, tmp.size() - 1)(engine);

            if (cnt != pos) std::swap(tmp[cnt], tmp[pos]);
        }
        tmp.erase(std::next(tmp.begin(), size), tmp.end());

        return tmp;
    }

    std::vector<KMCFollower> FollowerEffectiveDistance(RE::PlayerCharacter *player, std::vector<KMCFollower> *followers,
                                                       float effectiveDistance, uint64_t rand) {
        std::vector<KMCFollower> sr;
        if (player == nullptr or followers == nullptr) return sr;

        // RE::BGSKeyword a;

        // std::string pln = pl->GetName();
        RE::NiPoint3 ppos = player->GetPosition();

        for (int i = 0; i < followers->size(); i++) {
            RE::Actor *f = (*followers)[i].follower;
            if (f != nullptr) {
                bool checkOK = false;
                float dist = f->GetPosition().GetDistance(ppos);
                LOG("FPos PPos dist = {}", dist);
                if (effectiveDistance > dist) {
                    checkOK = true;
                }

                if (checkOK) {
                    // check keyword
                    std::string r = std::to_string(rand);
                    auto condKeywords = (*followers)[i].IKeywords;
                    auto fmkeyword = std::find_if(condKeywords.begin(), condKeywords.end(),
                                                  [r](const auto &p) { return p.first == r; });
                    if (fmkeyword != condKeywords.end()) {
                        f->HasKeyword(fmkeyword->second) ? checkOK = true : checkOK = false;
                    } else {
                        checkOK = true;
                    }
                }

                if (checkOK) {
                    sr.push_back((*followers)[i]);
                }
            }
        }

        return sr;
    }

    KMCInequalitySign StringToKMCInequalitySign(std::string target) {
        if (target == ">=") {
            return KMCInequalitySign::more;
        } else if (target == "<=") {
            return KMCInequalitySign::less;
        } else if (target == ">") {
            return KMCInequalitySign::more_than;
        } else if (target == "<") {
            return KMCInequalitySign::less_than;
        } else if (target == "==") {
            return KMCInequalitySign::equal;
        }

        return KMCInequalitySign::unk;
    }

    bool JudgeKMCInequalitySign(KMCInequalitySign sign, float v1, float v2) {
        bool result = false;
        switch (sign) {
            case KMCInequalitySign::more:
                v1 >= v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::less:
                v1 <= v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::more_than:
                v1 > v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::less_than:
                v1 < v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::equal:
                v1 == v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::unk:
                break;
            default:
                break;
        }

        return result;
    }
    bool JudgeKMCInequalitySign(KMCInequalitySign sign, MultiTypeValue v1, MultiTypeValue v2) {
        bool result = false;
        auto v1t = v1.value_type;
        auto v2t = v2.value_type;

        if (v1t != v2t) return result;

        switch (v1t) {
            case KMCValueType::KM_INT: {
                int seed = 0;
                result = JudgeKMCInequalitySign(sign, v1.GetValue(seed), v2.GetValue(seed));
            } break;
            case KMCValueType::KM_FLOAT: {
                float seed = 0.0f;
                result = JudgeKMCInequalitySign(sign, v1.GetValue(seed), v2.GetValue(seed));
            } break;
            case KMCValueType::KM_STRING: {
                std::string seed = "";
                result = JudgeKMCInequalitySign(sign, v1.GetValue(seed), v2.GetValue(seed));
            } break;
            case KMCValueType::KM_LONG: {
                long seed = 0;
                result = JudgeKMCInequalitySign(sign, v1.GetValue(seed), v2.GetValue(seed));
            } break;
        }

        return result;
    }

    bool JudgeKMCInequalitySign(KMCInequalitySign sign, int v1, int v2) {
        bool result = false;
        switch (sign) {
            case KMCInequalitySign::more:
                v1 >= v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::less:
                v1 <= v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::more_than:
                v1 > v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::less_than:
                v1 < v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::equal:
                v1 == v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::unk:
                break;
            default:
                break;
        }

        return result;
    }
    bool JudgeKMCInequalitySign(KMCInequalitySign sign, std::string v1, std::string v2) {
        bool result = false;
        switch (sign) {
            case KMCInequalitySign::more:
                v1 >= v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::less:
                v1 <= v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::more_than:
                v1 > v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::less_than:
                v1 < v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::equal:
                v1 == v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::unk:
                break;
            default:
                break;
        }

        return result;
    }
    bool JudgeKMCInequalitySign(KMCInequalitySign sign, long v1, long v2) {
        bool result = false;
        switch (sign) {
            case KMCInequalitySign::more:
                v1 >= v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::less:
                v1 <= v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::more_than:
                v1 > v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::less_than:
                v1 < v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::equal:
                v1 == v2 ? result = true : result = false;
                break;
            case KMCInequalitySign::unk:
                break;
            default:
                break;
        }

        return result;
    }

    bool BuildMultTypeValues(std::vector<std::string> *source, std::vector<KMCValueType> *results_value_type,
                             std::vector<MultiTypeValue> *out, int source_index) {
        if (source->empty()) {
            return false;
        }

        if (out->size() < results_value_type->size()) {
            out->resize(results_value_type->size());
        }

        for (int i = 0; i < out->size(); i++) {
            std::string v = "";
            try {
                v = source->at(i + source_index);
            } catch (...) {
                ERROR("BuildMultTypeValues out of range");
                return false;
            }
            auto value_type = results_value_type->at(i);
            auto mtv = &(out->at(i));

            if (v == "") {
                if (value_type != KMCValueType::KM_STRING) {
                    WARN("BuildMultTypeValues another type");
                }

                continue;
            }

            switch (value_type) {
                case KMCValueType::KM_INT:
                    mtv->iv = std::stoi(v);
                    mtv->value_type = KMCValueType::KM_INT;
                    break;
                case KMCValueType::KM_FLOAT:
                    mtv->fv = std::stof(v);
                    mtv->value_type = KMCValueType::KM_FLOAT;
                    break;
                case KMCValueType::KM_STRING:
                    mtv->sv = v;
                    mtv->value_type = KMCValueType::KM_STRING;
                    break;
                case KMCValueType::KM_LONG:
                    mtv->lv = std::stol(v);
                    mtv->value_type = KMCValueType::KM_LONG;
                    break;
            }
        }

        return true;
    }

    bool IsFileExist(const std::string &name) { return std::filesystem::is_regular_file(name); }

    void GetLeftPaddingString(std::string &str, int n, char padChar) {
        if (n <= str.size()) {
            return;
        }

        str.insert(0, n - str.size(), padChar);
    }

    void GetRightPaddingString(std::string &str, int n, char padChar) {
        if (n <= str.size()) {
            return;
        }

        str.insert(str.size(), n - str.size(), padChar);
    }

    void Replace(std::string &stringreplace, const std::string &origin, const std::string &dest) {
        size_t pos = stringreplace.find(origin);
        size_t len = origin.length();
        stringreplace.replace(pos, len, dest);
    }

    RE::ObjectRefHandle GetSelectedRefHandle() {
        const REL::Relocation<RE::ObjectRefHandle *> selectedRef{
            RELOCATION_ID(519394, REL::Module::get().version().patch() < 1130 ? 405935 : 504099)};
        return *selectedRef;
    }

    RE::NiPointer<RE::TESObjectREFR> GetSelectedRef() {
        const auto handle = GetSelectedRefHandle();
        return handle.get();
    }

    void CompileAndRunImpl(RE::Script *script, RE::ScriptCompiler *compiler, RE::COMPILER_NAME name,
                           RE::TESObjectREFR *targetRef) {
        using func_t = decltype(CompileAndRunImpl);
        const REL::Relocation<func_t> func{
            RELOCATION_ID(21416, REL::Module::get().version().patch() < 1130 ? 21890 : 441582)};
        return func(script, compiler, name, targetRef);
    }

    void CompileAndRun(RE::Script *script, RE::TESObjectREFR *targetRef, const RE::COMPILER_NAME name) {
        RE::ScriptCompiler compiler;
        CompileAndRunImpl(script, &compiler, name, targetRef);
    }

    void RunConsoleCommand(const std::string &command) {
        const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
        if (const auto script = scriptFactory ? scriptFactory->Create() : nullptr) {
            script->SetCommand(command);
            CompileAndRun(script, GetSelectedRef().get(), RE::COMPILER_NAME::kSystemWindowCompiler);
            delete script;
        }
    }

    // template <class... Args>
    // void PapyrusFuncCall(std::string script_n, std::string func_n, Args ...a_args) {
    //     auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
    //
    //     RE::TESForm *form = RE::TESDataHandler::GetSingleton()->LookupForm(0x806, "KimachuuCutIn.esp");
    //     if (form) {
    //         auto policy = vm->GetObjectHandlePolicy();
    //         RE::VMHandle handle = policy->GetHandleForObject(form->GetFormType(), form);
    //
    //         if (handle == policy->EmptyHandle()) {
    //             return;
    //         }
    //
    //         RE::BSFixedString scriptName = script_n;
    //         RE::BSFixedString functionName = func_n;
    //
    //         RE::BSTSmartPointer<RE::BSScript::Object> object;
    //         RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;
    //
    //         if (vm->FindBoundObject(handle, scriptName.c_str(), object)) {
    //             auto args = RE::MakeFunctionArguments(a_args);
    //             vm->DispatchMethodCall1(object, functionName, args, result);
    //         }
    //     }
    // }

    using Slot = RE::BIPED_MODEL::BipedObjectSlot;
    bool IsTalking(RE::Character *a_character) {
        using func_t = decltype(&IsTalking);
        static REL::Relocation<func_t> func{RELOCATION_ID(36277, 37266)};
        return func(a_character);
    }

    uint64_t StorageUtilCalcID(void *stack_id) {
        uint64_t strage_util_key_id = 0;
        uint32_t low = 0;
        if (stack_id) {
            low = *(uint32_t *)((uintptr_t)stack_id + 0x14);
            auto high = *(uint8_t *)((uintptr_t)stack_id + 0x1A);

            strage_util_key_id = ((uint64_t)high << 32) | low;
        }

        return strage_util_key_id;
    }

    void KMCIsWorn(RE::Actor *actor, std::vector<std::uint32_t> worn_slot, std::vector<bool> &result) {
        
        if (worn_slot.empty()) {
            return;
        }
        result.assign(worn_slot.size(), false);
        // const auto armorType = static_cast<RE::BGSBipedObjectForm::ArmorType>(a_armorType);

        auto inv = actor->GetInventory([/*armorType,*/ worn_slot](RE::TESBoundObject &a_object) {
            const auto armor = a_object.As<RE::TESObjectARMO>();
            if (armor /*&& armor->GetArmorType() == armorType*/) {
                if (worn_slot.empty() || std::ranges::any_of(worn_slot, [&](const auto &slot) {
                        return armor->HasPartOf(static_cast<Slot>(slot));
                    })) {
                    return true;
                }
            }
            return false;
        });

        for (auto &[item, data] : inv) {
            const auto &[count, entry] = data;
            if (count > 0 && entry->IsWorn()) {
                const auto armor = entry->GetObject()->As<RE::TESObjectARMO>();
                if (armor) {
                    for (int i = 0; i < worn_slot.size(); i++) {
                        auto slt = static_cast<Slot>(worn_slot.at(i));
                        if (armor->HasPartOf(static_cast<Slot>(slt))) {
                            result.at(i) = true;
                        }
                    }
                }
            }
        }
    }

    bool KMCIsWorn(RE::Actor* actor, RE::TESObjectARMO* armo) {
        auto inv = actor->GetInventory();

        for (auto &[item, data] : inv) {
            const auto &[count, entry] = data;
            if (count > 0 && entry->IsWorn()) {
                const auto armor = entry->GetObject()->As<RE::TESObjectARMO>();
                if (armor->GetFormID() == armo->GetFormID()) {
                    return true;
                }
            }
        }

        return false;
    }


    std::string EscapeStringForJavaScript(const std::string &input) {
        std::ostringstream ss;
        for (char c : input) {
            switch (c) {
                case '\'':
                    ss << "\\'";
                    break;
                case '\"':
                    ss << "\\\"";
                    break;
                case '\\':
                    ss << "\\\\";
                    break;
                case '\n':
                    ss << "\\n";
                    break;
                case '\r':
                    ss << "\\r";
                    break;
                case '\t':
                    ss << "\\t";
                    break;
                case '\b':
                    ss << "\\b";
                    break;
                case '\f':
                    ss << "\\f";
                    break;
                case '/':
                    ss << "\\/";
                    break;
                default:
                    ss << c;
                    break;
            }
        }
        return ss.str();
    }

    void NamePlateSimplyWipe(KMCNPLoadedWidget id, std::string aaaakmcroot) {
        auto npasimple = KMCCT::KMCConfig::GetSingleton()->getINamePlateAnimation((int)simply);

        const static int tox = KMCFindVector(&(npasimple->settings), "tox", -300);
        const static int strength = KMCFindVector(&(npasimple->settings), "basevelocity", 1);
        const static int acceleration = KMCFindVector(&(npasimple->settings), "acceleration", 1);
        const static int stAddTiming = KMCFindVector(&(npasimple->settings), "switchgearsMS", 50);
        const static int t = KMCFindVector(&(npasimple->settings), "switchgearsMS", 1) * KMCCT::TIME_SCALE_MS;

        int wid = id.LoadedWidget;
        // int tid = id.LoadedText;
        int diffX = (tox + id.defaultWX) - id.defaultWX;
        int length = 0;
        int add = acceleration;
        tox < 0 ? length = tox *-1 : length = tox;

        long long freq = 2;

        int stock = 0;

        int nowp = 0;

        int transX = 0;
        if (diffX > 0) {
            transX = strength;
        } else if (diffX < 0) {
            transX = -strength;
        } else {
            if (wid != -1) {
                IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, wid, true);
            }
            return;
        }

        if (wid != -1) {
            IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, wid, true);
            int x = id.defaultWX;
            time_point<Clock> start = Clock::now();

            while (true) {
                ++stock;
                if (stock >= stAddTiming) {
                    stock -= stAddTiming;
                    add += acceleration;
                }
                x += (transX * add);
                nowp += (strength * add);

                if (nowp > length) {
                    IWW::MainFunctions::GetSingleton()->SetPosX(aaaakmcroot, wid, (tox + id.defaultWX));
                    break;
                } else {
                    IWW::MainFunctions::GetSingleton()->SetPosX(aaaakmcroot, wid, x);
                }

                time_point<Clock> end;
                long long dur = 0;

                end = Clock::now();
                milliseconds diff = duration_cast<milliseconds>(end - start);
                dur = diff.count();
                if (dur >= t) {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(freq));
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    break;
                }
            }
        }
    }

    void NamePlateFadeOut(KMCNPLoadedWidget id, std::string aaaakmcroot) {
        auto npafade = KMCCT::KMCConfig::GetSingleton()->getINamePlateAnimation((int)end_fadeout);

        const static int strength = KMCFindVector(&(npafade->settings), "fadeoutspeed", 1);
        const static int speedupvalue = KMCFindVector(&(npafade->settings), "speedupvalue", 1);
        const static int startalpha = KMCFindVector(&(npafade->settings), "startalpha", 255);
        const static int stAddTiming = KMCFindVector(&(npafade->settings), "switchgearsMS", 50);
        const static int t = KMCFindVector(&(npafade->settings), "animationtime", 1) * KMCCT::TIME_SCALE_MS;

        long long freq = 2;
        int add = speedupvalue;
        int wid = id.LoadedWidget;
        // int tid = id.LoadedText;

        int alpha = startalpha;
        int stock = 0;

        if (wid != -1) {
            IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, wid, true);
            time_point<Clock> start = Clock::now();

            while (true) {
                ++stock;
                if (stock >= stAddTiming) {
                    stock -= stAddTiming;
                    add += speedupvalue;
                }
                alpha -= (strength * add);

                if (alpha > 0) {
                    IWW::MainFunctions::GetSingleton()->SetTransparency(aaaakmcroot, wid, alpha);
                } else {
                    break;
                }

                time_point<Clock> end;
                long long dur = 0;

                end = Clock::now();
                milliseconds diff = duration_cast<milliseconds>(end - start);
                dur = diff.count();
                if (dur >= t) {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(freq));
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    break;
                }
            }
        }

        IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, wid, false);
        // std::this_thread::sleep_for(std::chrono::milliseconds(16));
        // IWW::MainFunctions::GetSingleton()->SetVisible(aaaakmcroot, tid, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_INVISIBLE_MS));
        IWW::MainFunctions::GetSingleton()->SetTransparency(aaaakmcroot, wid, 255);
        IWW::MainFunctions::GetSingleton()->SetPosX(aaaakmcroot, wid, id.defaultWX);
        IWW::MainFunctions::GetSingleton()->SetPosY(aaaakmcroot, wid, id.defaultWY);
        std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::CALL_INVISIBLE_MS));
        // IWW::MainFunctions::GetSingleton()->SetTransparency(aaaakmcroot, tid, 255);
        // IWW::MainFunctions::GetSingleton()->SetPosX(aaaakmcroot, tid, id.defaultTX);
        // IWW::MainFunctions::GetSingleton()->SetPosY(aaaakmcroot, tid, id.defaultTY);
    }
}