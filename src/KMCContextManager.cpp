#include "KMCContextManager.h"
#include "KMCConfig.h"
#include "KMCCutin.h"
#include "KMCGameEventListener.h"
#include "KMCStateManager.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <boost/algorithm/string/trim.hpp>

SINGLETONBODY(KMCCT::KMCContextManager)

namespace KMCCT {
    using json = nlohmann::json;

    RE::BGSKeyword* LocTypeDungeon;
    RE::BGSKeyword* LocTypeClearable;
    RE::BGSKeyword* LocTypeInn;
    std::vector<RE::BGSKeyword*> WeaponKeywordTypeSlash;
    std::vector<RE::BGSKeyword*> WeaponKeywordTypeBlunt;
    std::atomic<bool> is_travel{false};
    std::atomic<bool> is_travel_far{false};

    std::atomic<bool> is_p_travel{false};
    std::atomic<bool> is_p_travel_far{false};

    std::unordered_map<int, bool> me_result_cache;
    std::unordered_map<int, bool> me_p_result_cache;

    std::string STHasKeyword(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->HasKeyword(st.keyword)) {
            return st.cutinName;
        }

        return "";
    }

    std::string STHasMagicEffectKeyword(int any, State st) {
        
        if (st.is_me_representative) {
            std::vector<MagicEffectTarget*> target;
            for (auto& v : st.me_keywords) {
                // randの値以下なので、確率に選ばれなかった
                if (v.threshold < any) {
                    continue;
                }

                target.push_back(&v);
            }

            if (target.empty()) {
                return "";
            }

            const auto active_effects = st.actor->GetMagicTarget()->GetActiveEffectList();
            if (active_effects) {
                for (auto const& ae : *active_effects) {
                    const auto mgef = ae ? ae->GetBaseObject() : nullptr;
                    if (mgef != nullptr) {
                        KMC_LOG("Active Effect Name {} search target {}", mgef->GetName(),
                                st.keyword->GetFormEditorID());
                        for (auto* v : target) {
                            if (mgef->HasKeyword(v->me_keyword)) {
                                if (st.is_profile) {
                                    me_p_result_cache[v->priority] = true;
                                } else {
                                    // magic effectのN番目かわからないし、リストの最後なのかわからないので入れてく
                                    me_result_cache[v->priority] = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (st.is_profile) {
            if (me_p_result_cache.contains(st.me_priority)) {
                if (me_p_result_cache[st.me_priority]) {
                    return st.cutinName;
                }
            }
        } else {
            if (me_result_cache.contains(st.me_priority)) {
                if (me_result_cache[st.me_priority]) {
                    return st.cutinName;
                }
            }
        }
        return "";
    }

    std::string STHasFaction(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->IsInFaction(st.faction)) {
            return st.cutinName;
        }

        return "";
    }

    std::string STGlobal(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (JudgeKMCInequalitySign(st.gparam.isign, st.global->value, st.gparam.base_value)) {
            return st.cutinName;
        }

        return "";
    }

    std::string STFTEnd(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.is_profile) {
            if (is_p_travel_far.load(std::memory_order_relaxed)) {
                is_p_travel_far.store(false, std::memory_order_relaxed);
                return "FTEndFar";
            }

            if (is_p_travel.load(std::memory_order_relaxed)) {
                is_p_travel.store(false, std::memory_order_relaxed);
                return "FTEndClose";
            }
        } else {
            if (is_travel_far.load(std::memory_order_relaxed)) {
                is_travel_far.store(false, std::memory_order_relaxed);
                return "FTEndFar";
            }

            if (is_travel.load(std::memory_order_relaxed)) {
                is_travel.store(false, std::memory_order_relaxed);
                return "FTEndClose";
            }
        }
        return "";
    }

    std::string STInjury(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        float nowhealth = st.actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
        float damage = st.actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth) * -1;
        if (damage > nowhealth) {
            return "Injury";
        }

        return "";
    }
    std::string STCombat(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->IsInCombat()) {
            return "Combat";
        }

        return "";
    }
    std::string STSneak(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->IsSneaking()) {
            return "Sneak";
        }

        return "";
    }
    std::string STMount(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        if (st.actor->IsOnMount()) {
            return "Mount";
        }

        return "";
    }
    std::string STIdle(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        auto calendar = RE::Calendar::GetSingleton();
        if (calendar != nullptr && calendar->gameHour != nullptr) {
            float hour = calendar->gameHour->value;
            if (hour >= 22) {
                return "IdleMidnight";
            } else if (hour >= 13) {
                return "IdlePM";
            } else if (hour >= 6) {
                return "Idle";
            } else {
                return "IdleMidnight";
            }
        }

        return "";
    }

    std::string STDungeon(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        static std::vector<RE::BGSKeyword*> v = {LocTypeClearable, LocTypeDungeon};
        auto cell = st.actor->GetParentCell();
        if (cell != nullptr && cell->IsInteriorCell()) {
            auto loc = st.actor->GetCurrentLocation();
            if (loc != nullptr) {
                if (loc->HasKeywordInArray(v, true)) {
                    return "InDungeon";
                }
            }
        }

        return "";
    }

    std::string STInn(int any, State st) {
        if (st.threshold < any) {
            return "";
        }

        auto cell = st.actor->GetParentCell();
        if (cell != nullptr && cell->IsInteriorCell()) {
            auto loc = st.actor->GetCurrentLocation();
            if (loc != nullptr) {
                if (loc->HasKeyword(LocTypeInn)) {
                    return "Inn";
                }
            }
        }

        return "";
    }
#pragma endregion 

    void KMCContextManager::Setup() {
        try {
            if (!Parse(COMMON_PATH + CUTIN_CONTEXT_PATH)) {
                loaded = false;
            }
        } catch (std::runtime_error ex) {
            loaded = false;
            KMC_ERROR("ERROR LOADING {}", ex.what());
        }
    }

    void KMCContextManager::Init() {
        LocTypeDungeon = RE::TESForm::LookupByID<RE::BGSKeyword>(0x000130DB);
        LocTypeClearable = RE::TESForm::LookupByID<RE::BGSKeyword>(0x000F5E80);
        LocTypeInn = RE::TESForm::LookupByID<RE::BGSKeyword>(0x0001CB87);
        WeaponKeywordTypeSlash = {
            RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x0001E711, "Skyrim.esm"),
            RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x0001E713, "Skyrim.esm"),
            RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x0006D932, "Skyrim.esm"),
            RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x0006D931, "Skyrim.esm"),
            RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x0001E712, "Skyrim.esm")};
        WeaponKeywordTypeBlunt = {
            RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x0006D930, "Skyrim.esm"),
            RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x0001E714, "Skyrim.esm")};

        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();

        int cutin_chance_fasttravel = 0;

        for (const auto& entry : action_states) {
            if (entry.name == StateNames::FTEndFar) {
                cutin_states.emplace(entry.priority, StateControll(STFTEnd, KMCCT::State(player, "FTEndFar", entry.chance)));
                cutin_chance_fasttravel = entry.chance;
            } else if (entry.name == StateNames::Injury) {
                cutin_states.emplace(entry.priority, StateControll(STInjury, KMCCT::State(player, "Injury", entry.chance)));
            } else if (entry.name == StateNames::Combat) {
                cutin_states.emplace(entry.priority, StateControll(STCombat, KMCCT::State(player, "Combat", entry.chance)));
            } else if (entry.name == StateNames::Sneak) {
                cutin_states.emplace(entry.priority, StateControll(STSneak, KMCCT::State(player, "Sneak", entry.chance)));
            } else if (entry.name == StateNames::Mount) {
                cutin_states.emplace(entry.priority, StateControll(STMount, KMCCT::State(player, "Mount", entry.chance)));
            } else if (entry.name == StateNames::Dungeon) {
                cutin_states.emplace(entry.priority, StateControll(STDungeon, KMCCT::State(player, "InDungeon", entry.chance)));
            } else if (entry.name == StateNames::Idle) {
                cutin_states.emplace(entry.priority, StateControll(STIdle, KMCCT::State(player, "Idle", entry.chance)));
            } else if (entry.name == StateNames::Inn) {
                cutin_states.emplace(entry.priority, StateControll(STInn, KMCCT::State(player, "Inn", entry.chance)));
            }
        }

        auto setting = KMCCT::KMCConfig::GetSingleton()->GetKMCSetting();
        int interrupt_event_OnHit = KMCFindVector(setting, INTERRUPT_EVENT_ONHIT, 1);

        if (interrupt_event_OnHit > 0) {
            KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
                [this](const RE::TESHitEvent* event) { this->OnHitEvent(event); });
        }

        if (cutin_chance_fasttravel > 0) {
            KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
                [this](const RE::TESFastTravelEndEvent* event) { this->FastTravelEndEvent(event); });
        }

        for (auto& [key, entry] : extended_context_data) {
            if (key == ExtendedContextNames::Keyword) {
                InitDetectionItems(&entry, player, STHasKeyword, KMCDetectionType::keyword,
                                   {"DetectionKeyword.json Duplicate keyword priority",
                                    "DetectionKeyword.json Keyword could not be loaded. Please review form IDs, etc.",
                                    "DetectionKeyword.json", "DetectionKeyword.json error param 6"});
            } else if (key == ExtendedContextNames::Faction) {
                InitDetectionItems(&entry, player, STHasFaction, KMCDetectionType::faction,
                                   {"DetectionFaction.json Duplicate keyword priority",
                                    "DetectionFaction.json Keyword could not be loaded. Please review form IDs, etc.",
                                    "DetectionFaction.json", ""});
            } else if (key == ExtendedContextNames::DetectionMagicEffectKeyword) {
                InitDetectionItems(
                    &entry, player, STHasMagicEffectKeyword, KMCDetectionType::magic_effect_keyword,
                    {"DetectionMagicEffectKeyword.json Duplicate keyword priority",
                     "DetectionMagicEffectKeyword.json Keyword could not be loaded. Please review form IDs, etc.",
                     "DetectionMagicEffectKeyword.json", ""});
            } else if (key == ExtendedContextNames::Global) {
                InitDetectionItems(&entry, player, STGlobal, KMCDetectionType::global,
                                   {"DetectionGlobal.json Duplicate keyword priority",
                                    "DetectionGlobal.json Keyword could not be loaded. Please review form IDs, etc.",
                                    "DetectionGlobal.json", "DetectionGlobal.json error param 6 or 7 "});
            }
        }

        State* first_magic_effect_state = nullptr;

        for (auto& [priority, control] : cutin_states) {
            if (control.state.is_magic_effect) {
                if (first_magic_effect_state == nullptr) {
                    first_magic_effect_state = &control.state;
                    first_magic_effect_state->is_me_representative = true;
                    MagicEffectTarget met = {.me_keyword = control.state.keyword,
                                             .threshold = control.state.threshold,
                                             .priority = control.state.me_priority};
                    first_magic_effect_state->me_keywords.push_back(met);
                } else {
                    MagicEffectTarget met = {.me_keyword = control.state.keyword,
                                             .threshold = control.state.threshold,
                                             .priority = control.state.me_priority};
                    first_magic_effect_state->me_keywords.push_back(met);
                    control.state.is_me_representative = false;
                }
            }
        }
    }

    void KMCContextManager::ProfileStateSetup(std::set<std::string> monitor_target) {
        for (const auto& [priority, control] : cutin_states) {
            if (monitor_target.contains(control.state.cutinName)) {
                profile_states.emplace(priority, control);
            }
        }

        State* first_magic_effect_state = nullptr;

        for (auto& [priority, control] : profile_states) {
            control.state.is_profile = true;

            if (control.state.is_magic_effect) {
                if (first_magic_effect_state == nullptr) {
                    
                    first_magic_effect_state = &control.state;
                    first_magic_effect_state->ResetME();
                    first_magic_effect_state->is_me_representative = true;
                    MagicEffectTarget met = {.me_keyword = control.state.keyword,
                                             .threshold = control.state.threshold,
                                             .priority = control.state.me_priority};
                    first_magic_effect_state->me_keywords.push_back(met);
                } else {
                    MagicEffectTarget met = {.me_keyword = control.state.keyword,
                                             .threshold = control.state.threshold,
                                             .priority = control.state.me_priority};
                    first_magic_effect_state->me_keywords.push_back(met);
                    control.state.is_me_representative = false;
                }
            }
        }
    }

    void KMCContextManager::OnHitEvent(const RE::TESHitEvent* event) {
        if (event->target != nullptr && event->target->formID == 0x14 &&
            !event->flags.any(RE::TESHitEvent::Flag::kHitBlocked)) {
            RE::TESForm* root = RE::TESForm::LookupByID(event->source);
            if (root != nullptr) {
                KMC_LOG("TESHitEvent item {} source {} ", root->GetName(), event->source);
                std::string power_attack = "0";
                if (event->flags.any(RE::TESHitEvent::Flag::kPowerAttack)) {
                    power_attack = "1";
                }

                if (root->HasKeywordInArray(WeaponKeywordTypeBlunt, false)) {
                    KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(
                        KMCInterruptPushCutInData(KMCInterruptType::on_hit, "HitBluntWeapon", power_attack, "Power"));
                } else if (root->HasKeywordInArray(WeaponKeywordTypeSlash, false)) {
                    KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(
                        KMCInterruptPushCutInData(KMCInterruptType::on_hit, "HitSlashWeapon", power_attack, "Power"));
                }
            }
        } else if (event->target != nullptr && event->target->formID == 0x14 &&
                   event->flags.any(RE::TESHitEvent::Flag::kHitBlocked)) {
            KMC_LOG("TESHitEvent Blocked source {} ", event->source);
            std::string power_attack = "0";
            if (event->flags.any(RE::TESHitEvent::Flag::kPowerAttack)) {
                power_attack = "1";
            }
            KMCCT::KMCCutin::GetSingleton()->InterruptCutInEventManager(
                KMCInterruptPushCutInData(KMCInterruptType::on_hit, "HitBlocked", power_attack, "Power"));
        }
    }
    void KMCContextManager::FastTravelEndEvent(const RE::TESFastTravelEndEvent* event) {
        if (event->fastTravelEndHours > 1.5f) {
            is_travel_far.store(true, std::memory_order_relaxed);
            is_p_travel_far.store(true, std::memory_order_relaxed);
        } else {
            is_travel.store(true, std::memory_order_relaxed);
            is_p_travel.store(true, std::memory_order_relaxed);
        }
    }

    
    std::string KMCContextManager::GetPlayerState() {
        std::string state = GetHighestPriorityPlayer();

        if (state != "") {
            is_travel_far.store(false, std::memory_order_relaxed);
            is_travel.store(false, std::memory_order_relaxed);
        }

        return state;
    }

    std::string KMCContextManager::GetStateForProfile() {
        std::string state = "default";

        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
        if (player == nullptr) return "default";

        std::random_device rnd;
        std::mt19937 mt(rnd());
        std::uniform_int_distribution<> rand100(1, 100);
        int r = (int)rand100(mt);

        me_p_result_cache.clear();
        for (const auto& [key, value] : profile_states) {
            state = value.func(r, value.state);
            if (state != "") {
                is_p_travel_far.store(false, std::memory_order_relaxed);
                is_p_travel.store(false, std::memory_order_relaxed);
                return state;
            }
        }

        return "default";
    }

    std::string KMCContextManager::GetHighestPriorityPlayer() {
        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
        if (player == nullptr || KMCStateManager::GetSingleton()->GetStoppingState()) {
            return "";
        }

        std::random_device rnd;
        std::mt19937 mt(rnd());
        std::uniform_int_distribution<> rand100(1, 100);
        int r = rand100(mt);

        me_result_cache.clear();

        for (const auto& [key, value] : cutin_states) {
            std::string result = value.func(r, value.state);
            if (result != "") {
                return result;
            }
        }
        return "";
    }

    void KMCContextManager::InitDetectionItems(std::vector<std::pair<int, std::string>>* dconfig,
                                             RE::Actor* actor, StateFunc f, KMCDetectionType type,
                                             std::vector<std::string> messages) {
        for (auto [key, value] : *dconfig) {
            try {
                auto spvalue = KMCSplit(value, ',');

                RE::BGSKeyword* keyword = nullptr;
                // RE::BGSKeyword* magic_effect_keyword = nullptr;
                RE::TESFaction* faction = nullptr;
                RE::TESGlobal* global = nullptr;
                bool found = false;
                bool is_magic_effect = false;

                switch (type) {
                    case KMCDetectionType::keyword:
                        keyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        keyword != nullptr ? found = true : found = false;
                        break;
                    case KMCDetectionType::faction:
                        faction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        faction != nullptr ? found = true : found = false;
                        break;
                    case KMCDetectionType::magic_effect_keyword:
                        keyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        keyword != nullptr ? found = true : found = false;
                        is_magic_effect = true;
                        break;
                    case KMCDetectionType::global:
                        global = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        global != nullptr ? found = true : found = false;
                        break;
                    default:
                        break;
                }

                if (found) {
                    int priority = std::stoi(spvalue.at(3));
                    if (cutin_states.contains(priority)) {
                        KMC_ERROR("{}", messages.at(0));
                    } else {
                        KMCCT::State state =
                            KMCCT::State(actor, spvalue.at(2), std::stoi(spvalue.at(4)), keyword, faction, global);
                        std::string exc_key = "";
                        try {
                            if (type == KMCDetectionType::keyword) {
                                if (keyword != nullptr && std::stoi(spvalue.at(5)) > 0) {
                                    KMCStateManager::GetSingleton()->AddSceneExclusionKeyword(keyword);
                                }
                            } else if (type == KMCDetectionType::global) {
                                if (global != nullptr) {
                                    std::string inequality_sign = spvalue.at(5);
                                    std::string global_diff_value = spvalue.at(6);

                                    state.gparam = KMCTESGlobalParam(StringToKMCInequalitySign(inequality_sign),
                                                                     std::stof(global_diff_value));
                                }
                            }
                        } catch (...) {
                            KMC_ERROR("{}", messages.at(3));
                        }
                        if (is_magic_effect) {
                            state.is_magic_effect = true;
                            state.me_priority = priority;
                        }
                        cutin_states.insert(std::make_pair(priority, StateControll(f, state)));
                    }
                } else {
                    KMC_ERROR("{}", messages.at(1));
                }
            } catch (...) {
                KMC_ERROR("ERROR {} key: {} value : {}", messages.at(2), key, value);
            }
        }
    }

    bool KMCContextManager::Parse(std::string path) {
        std::ifstream stream(path);

        if (!stream.is_open()) {
            throw std::runtime_error("Failed open file. Path ==> " + path);
        }

        if (!json::accept(stream)) {
            throw std::runtime_error("Incorrect json format. Path ==> " + path);
        }

        stream.seekg(0, std::ios::beg);

        json j = json::parse(stream);

        for (const auto& line : j["action_context"]["state"]) {
            ActionStateEntry ase;
            auto sp = KMCSplit(line.get<std::string>(), ',');
            if (sp.size() != 3) {
                // jsonの読み取ったサイズと想定が違う
                return false;
            }

            std::string v1 = sp.at(0);
            std::string v2 = sp.at(1);
            std::string v3 = sp.at(2);
            boost::algorithm::trim(v1);
            boost::algorithm::trim(v2);
            boost::algorithm::trim(v3);
            ase.name = v1;
            ase.priority = std::stoi(v2);
            ase.chance = std::stoi(v3);

            action_states.push_back(ase);
        }

        for (auto& [key, entry] : j["extended_context"].items()) {
            int index = 0;
            for (const auto& line : entry) {
                extended_context_data[key].push_back({index++, line.get<std::string>()});
            }
        }

        return true;
    }
}