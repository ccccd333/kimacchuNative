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
    std::vector<RE::BGSKeyword*> WeaponKeywordTypeSlash;
    std::vector<RE::BGSKeyword*> WeaponKeywordTypeBlunt;
    bool isTravel = false;
    bool isTravelFar = false;

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
        if (st.threshold < any) {
            return "";
        }

        const auto activeEffects = st.actor->GetMagicTarget()->GetActiveEffectList();
        if (activeEffects) {
            for (auto const& ae : *activeEffects) {
                const auto mgef = ae ? ae->GetBaseObject() : nullptr;
                if (mgef != nullptr) {
                    KMC_LOG("Active Effect Name {} search target {}", mgef->GetName(), st.keyword->GetFormEditorID());
                    if (mgef->HasKeyword(st.keyword)) {
                        return st.cutinName;
                    }
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

        if (isTravelFar) {
            isTravelFar = false;
            return "FTEndFar";
        }

        if (isTravel) {
            isTravel = false;
            return "FTEndClose";
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
        LocTypeDungeon = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x000130DB);
        LocTypeClearable = (RE::BGSKeyword*)RE::TESForm::LookupByID(0x000F5E80);

        WeaponKeywordTypeSlash = {
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0001E711, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0001E713, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0006D932, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0006D931, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0001E712, "Skyrim.esm")};
        WeaponKeywordTypeBlunt = {
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0006D930, "Skyrim.esm"),
            (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(0x0001E714, "Skyrim.esm")};

        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();

        int cutin_chance_fasttravel = 0;

        for (const auto& entry : action_states) {
            if (entry.name == StateNames::FTEndFar) {
                states.emplace(entry.priority, StateControll(STFTEnd, KMCCT::State(player, "FTEndFar", entry.chance)));
                cutin_chance_fasttravel = entry.chance;
            } else if (entry.name == StateNames::Injury) {
                states.emplace(entry.priority, StateControll(STInjury, KMCCT::State(player, "Injury", entry.chance)));
            } else if (entry.name == StateNames::Combat) {
                states.emplace(entry.priority, StateControll(STCombat, KMCCT::State(player, "Combat", entry.chance)));
            } else if (entry.name == StateNames::Sneak) {
                states.emplace(entry.priority, StateControll(STSneak, KMCCT::State(player, "Sneak", entry.chance)));
            } else if (entry.name == StateNames::Mount) {
                states.emplace(entry.priority, StateControll(STMount, KMCCT::State(player, "Mount", entry.chance)));
            } else if (entry.name == StateNames::Dungeon) {
                states.emplace(entry.priority, StateControll(STDungeon, KMCCT::State(player, "InDungeon", entry.chance)));
            } else if (entry.name == StateNames::Idle) {
                states.emplace(entry.priority, StateControll(STIdle, KMCCT::State(player, "Idle", entry.chance)));
            }
        }

        auto setting = KMCCT::KMCConfig::GetSingleton()->getISetting();
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
            isTravelFar = true;
        } else {
            isTravel = true;
        }
    }

    
    std::string KMCContextManager::GetPlayerState() {
        std::string state = GetHighestPriorityPlayer();

        if (state != "") {
            isTravelFar = false;
            isTravel = false;
        }

        return state;
    }

    std::string KMCContextManager::GetHighestPriorityPlayer() {
        auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();

        if (KMCStateManager::GetSingleton()->GetStoppingState()) {
            return "";
        }

        if (player == nullptr) return "";

        std::random_device rnd;
        std::mt19937 mt(rnd());
        std::uniform_int_distribution<> rand100(1, 100);
        int r = (int)rand100(mt);

        std::string result;

        for (auto [key, value] : states) {
            result = value.func(r, value.state);
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

                switch (type) {
                    case KMCDetectionType::keyword:
                        keyword = (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        keyword != nullptr ? found = true : found = false;
                        break;
                    case KMCDetectionType::faction:
                        faction = (RE::TESFaction*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        faction != nullptr ? found = true : found = false;
                        break;
                    case KMCDetectionType::magic_effect_keyword:
                        keyword = (RE::BGSKeyword*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        keyword != nullptr ? found = true : found = false;
                        break;
                    case KMCDetectionType::global:
                        global = (RE::TESGlobal*)RE::TESDataHandler::GetSingleton()->LookupForm(
                            std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                        global != nullptr ? found = true : found = false;
                        break;
                    default:
                        break;
                }

                if (found) {
                    int priority = std::stoi(spvalue.at(3));
                    if (states.contains(priority)) {
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

                        states.insert(std::make_pair(priority, StateControll(f, state)));
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