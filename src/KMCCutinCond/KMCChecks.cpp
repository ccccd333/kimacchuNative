#include "KMCCutinCond/KMCChecks.h"

#include <boost/any.hpp>

#include "KMCConfig.h"
#include "KMCTempKeywordManager.h"

namespace KMCCT {
    KMCCCTNothing nothi;
    KMCCCTPlayerMove pm;
    KMCCCTPlayerCombat pc;
    KMCCCTPlayerRunning pr;
    KMCCCTPlayerSneaking psn;
    KMCCCTPlayerIdle pid;
    KMCCCTPlayerKeyword pk;
    KMCCCTLocationKeyword lk;
    KMCCCTPlayerKill pkill;
    KMCCCTTempKeyword tkey;
    KMCCCTCrossHair ctch;
    KMCCCTBodySlot ctbs;
    KMCCCTMagicEffectKeyword ctmek;
    KMCCCTFormula ctform;
    std::vector<KMCCustomCondCheckHub*> Tasks = {&nothi, &pm,    &pc,   &pr,   &pid,  &psn,   &pk,
                                                 &lk,    &pkill, &tkey, &ctch, &ctbs, &ctmek, &ctform};

    bool GetCheckTaskDetail(KMCCCheckSource source, std::unique_ptr<KMCCustomCondCheckHub>* hub) {
        for (auto t : Tasks) {
            *hub = t->GetDetail(source);
            if (*hub) {
                return true;
            }
        }

        return false;
    }

    bool KMCCCTNothing::Check(KMCCCheckSource source) { return true; }

    bool KMCCCTPlayerMove::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            if (player->IsMoving()) {
                return true;
            }
        }

        return false;
    }

    bool KMCCCTPlayerCombat::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            if (player->IsInCombat()) {
                return true;
            }
        }

        return false;
    }

    bool KMCCCTPlayerRunning::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            if (player->IsRunning()) {
                return true;
            }
        }

        return false;
    }

    bool KMCCCTPlayerSneaking::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            if (player->IsSneaking()) {
                return true;
            }
        }

        return false;
    }

    bool KMCCCTPlayerIdle::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            if (!player->IsMoving()) {
                return true;
            }
        }

        return false;
    }

    bool KMCCCTPlayerKeyword::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            if (source.hsnhs == HasNHan::both) {
                if (player->HasKeywordInArray(source.has, true) && !player->HasKeywordInArray(source.nhas, true)) {
                    return true;
                }
            } else if (source.hsnhs == HasNHan::has) {
                if (player->HasKeywordInArray(source.has, true)) {
                    return true;
                }
            } else if (source.hsnhs == HasNHan::nhas) {
                if (!player->HasKeywordInArray(source.nhas, true)) {
                    return true;
                }
            }
        }

        return false;
    }

    bool KMCCCTLocationKeyword::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            auto cell = player->GetParentCell();
            if (cell != nullptr) {
                auto loc = player->GetCurrentLocation();
                if (loc != nullptr) {
                    if (source.hsnhs == HasNHan::both) {
                        if (loc->HasKeywordInArray(source.has, true) && !loc->HasKeywordInArray(source.nhas, true)) {
                            return true;
                        }
                    } else if (source.hsnhs == HasNHan::has) {
                        if (loc->HasKeywordInArray(source.has, true)) {
                            return true;
                        }
                    } else if (source.hsnhs == HasNHan::nhas) {
                        if (!loc->HasKeywordInArray(source.nhas, true)) {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    bool KMCCCTPlayerKill::Init() {
        time_in_kill_count =
            KMCFindVector(KMCCT::KMCConfig::GetSingleton()->getISetting(), TIME_TO_INCLUDE_IN_KILL_COUNT, 3) *
            KMCCT::TIME_SCALE_MS;

        KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
            [this](const RE::TESDeathEvent* event) { this->OnDeathEvent(event); });
        KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
            [this](const RE::TESHitEvent* event) { this->OnHitEvent(event); });

        return true;
    }

    bool KMCCCTPlayerKill::Check(KMCCCheckSource source) {
        // LOG("[KMCChecks] kill count {}", kill_count);
        if (hit_and_kill_list.size() > 0) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                time_point<Clock> now = Clock::now();

                std::map<unsigned long long, KillDetails>::iterator it = hit_and_kill_list.begin();
                while (it != hit_and_kill_list.end()) {
                    milliseconds diff = duration_cast<milliseconds>(now - it->second.latest_hit_time);
                    long long dur = diff.count();
                    if (dur > time_in_kill_count) {
                        // ERROR("[OnDeathEvent POP] {}", it->first);
                        hit_and_kill_list.erase(it++);
                    } else {
                        ++it;
                    }
                }
            }
        }
        if (kill_count > 0) {
            --kill_count;
            return true;
        }

        return false;
    }

    void KMCCCTPlayerKill::OnDeathEvent(const RE::TESDeathEvent* event) {
        if (event && event->dead == false /* && event->actorKiller->IsPlayerRef()*/) {
            if (event->actorDying) {
                auto target = (unsigned long long)event->actorDying.get();
                // ERROR("[OnDeathEvent] {}", target);
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (hit_and_kill_list.contains(target)) {
                        auto task = hit_and_kill_list.extract(target);
                        auto kill_detail = task.mapped();
                        time_point<Clock> now = Clock::now();

                        milliseconds diff = duration_cast<milliseconds>(now - kill_detail.latest_hit_time);
                        long long dur = diff.count();
                        if (dur <= time_in_kill_count) {
                            // ERROR("[OnDeathEvent] kill count {}, {}", kill_count, target);
                            ++kill_count;
                        }
                        // LOG("[PlayerKill] {}, {}", hit_and_kill_list.size(), target);
                        // LOG("[PlayerKill] [target addres] {}, [killer] {}, [dying] {}, [player?] {} [dead] {}",
                        //     target, event->actorKiller->GetName(),
                        //     event->actorDying->GetName(), event->actorKiller->IsPlayerRef(), event->dead);

                    }  // else {
                       // LOG("[PlayerNoKill] {} {}", hit_and_kill_list.size(), target);
                    //   LOG("[killer] {}, [target addres] {}, [dying] {}, [player?] {} [dead] {}",
                    //   event->actorKiller->GetName(), target, event->actorDying->GetName(),
                    //        event->actorKiller->IsPlayerRef(), event->dead);
                    //}
                }
            }
        }
    }

    void KMCCCTPlayerKill::OnHitEvent(const RE::TESHitEvent* event) {
        if (event && event->cause && event->cause->IsPlayerRef()) {
            {
                // onhitで2回くらい入ってくるのはプレイヤーの付呪で
                // 剣＋魔法攻撃となっているから
                std::lock_guard<std::mutex> lock(mtx);
                auto target = (unsigned long long)event->target.get();
                // ERROR("[OnHitEvent] {}", target);
                if (hit_and_kill_list.contains(target)) {
                    hit_and_kill_list[target].latest_hit_time = Clock::now();
                    hit_and_kill_list[target].hit_count += 1;
                    // ERROR("[OnHitEvent] already {}, count {}", target, hit_and_kill_list[target].hit_count);
                } else {
                    hit_and_kill_list.emplace(target, KillDetails(1, Clock::now()));
                    // ERROR("[OnHitEvent] new {}, count {}", target, hit_and_kill_list[target].hit_count);
                }
                // LOG("[PlayerAttack] {}, {}, {}", hit_and_kill_list.size(), target);
                // auto cause = (unsigned long long)event->cause.get();
                // LOG("[Player] TESHitEvent target addres {}, target name {}, cause addres {}, cause name {}, isDead?
                // {} ", target, event->target->GetName(), cause, event->cause->GetName(), event->target->IsDead());
            }
        }
    }

    bool KMCCCTTempKeyword::Check(KMCCCheckSource source) {
        if (source.thsnhs == HasNHan::both) {
            bool hasm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(source.thas, true);
            bool nhasm = KMCCT::KMCTempKeywordManager::GetSingleton()->NHasTempKeyword(source.tnhas, true);
            if (hasm && nhasm) {
                return true;
            }
            LOG("[TEMP_KEYWORD_CHECK[BOTH]] no match");
            if (IWW::Config::GetSingleton()->GetVariable<int>("General.Logging", 1) >= 2) {
                for (auto& has : source.thas) {
                    LOG("[HAS] ==> {}", has);
                }

                for (auto& nhas : source.tnhas) {
                    LOG("[NHAS] ==> {}", nhas);
                }

                KMCCT::KMCTempKeywordManager::GetSingleton()->ToLog();
            }
        } else if (source.thsnhs == HasNHan::has) {
            bool hasm = KMCCT::KMCTempKeywordManager::GetSingleton()->HasTempKeyword(source.thas, true);
            if (hasm) {
                return true;
            }
            LOG("[TEMP_KEYWORD_CHECK[HAS]] no match");
            if (IWW::Config::GetSingleton()->GetVariable<int>("General.Logging", 1) >= 2) {
                for (auto& has : source.thas) {
                    LOG("[HAS] ==> {}", has);
                }

                KMCCT::KMCTempKeywordManager::GetSingleton()->ToLog();
            }
        } else if (source.thsnhs == HasNHan::nhas) {
            bool nhasm = KMCCT::KMCTempKeywordManager::GetSingleton()->NHasTempKeyword(source.tnhas, true);
            if (nhasm) {
                return true;
            }

            LOG("[TEMP_KEYWORD_CHECK [NHAS]] no match");
            if (IWW::Config::GetSingleton()->GetVariable<int>("General.Logging", 1) >= 2) {
                for (auto& nhas : source.tnhas) {
                    LOG("[NHAS] ==> {}", nhas);
                }

                KMCCT::KMCTempKeywordManager::GetSingleton()->ToLog();
            }
        }

        return false;
    }

    bool KMCCCTCrossHair::Init() {
        std::regex pattern(R"(^(?![^!"#$%&'()\*\+\-\.,\/:;<=>?@\[\\\]^_{|}~]+).*$)");
        try {
            for (auto& chrn : o_source.cross_hair_ref_name) {
                auto sp = KMCSplit(chrn, '>');
                if (sp.size() >= 2) {
                    std::smatch sm;
                    KMCPattern kmc_pattern;
                    bool hit = false;
                    for (auto& tag : tags) {
                        if (sp.at(0) == tag) {
                            if (tag == GET_CELL_NAME_TELEPORT_LINKED_DOOR) {
                                kmc_pattern.kmcif = kmc_if::get_cell_name_teleport_linked_door;
                            } else if (tag == GET_NAME) {
                                kmc_pattern.kmcif = kmc_if::get_name;
                            } else {
                                ERROR("KMCCCTCrossHair::Init tag name NG");
                                return false;
                            }

                            std::string ref_name = sp.back();
                            kmc_pattern.work = ref_name;
                            if (std::regex_match(ref_name, sm, pattern)) {
                                // 記号ありの場合
                                kmc_pattern.kpm = kmc_pattern_mat::pattern;
                                kmc_pattern.pattern = std::regex(ref_name);
                            } else {
                                kmc_pattern.kpm = kmc_pattern_mat::norm;
                                kmc_pattern.norm = ref_name;
                            }

                            ifs.push_back(kmc_pattern);
                            hit = true;
                        }
                    }

                    if (!hit) {
                        ERROR("KMCCCTCrossHair::Init NG");
                        return false;
                    }
                } else {
                    ERROR("KMCCCTCrossHair::Init Format NG [{}]", chrn);
                }
            }
        } catch (std::exception ex) {
            ERROR("KMCCCTCrossHair::Init Regex Error wt:{}", ex.what());
            return false;
        }

        return true;
    }

    bool KMCCCTCrossHair::Check(KMCCCheckSource source) {
        if (!cpd) {
            cpd = RE::CrosshairPickData::GetSingleton();

            if (!cpd) {
                ERROR("KMCCCTCrossHair::Init NG[RE::CrosshairPickData::GetSingleton()->not found]");
                return false;
            }
        }

        if (cpd) {
            if (cpd->target.get()) {
                std::string get_cell_name_tld = "";
                std::string gn = "";

                for (auto& i : ifs) {
                    if (get_cell_name_tld != NOT_FOUND && i.kmcif == kmc_if::get_cell_name_teleport_linked_door) {
                        if (GetCellNameTeleportLinkedDoor(i, get_cell_name_tld)) {
                            return true;
                        }
                    } else if (gn != NOT_FOUND && i.kmcif == kmc_if::get_name) {
                        if (GetName(i, gn)) {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    bool KMCCCTCrossHair::GetCellNameTeleportLinkedDoor(const KMCPattern& p, std::string& Obtained) {
        if (Obtained != "") {
            return IsMatch(Obtained, p);
        } else if (auto target = cpd->target.get(); target) {
            if (auto door = target->extraList.GetTeleportLinkedDoor().get(); door) {
                if (auto gpc = door.get()->GetParentCell(); gpc) {
                    Obtained = gpc->GetName();
                    LOG("KMCCCTCrossHair::GetCellNameTeleportLinkedDoor [{}]", Obtained);
                    return IsMatch(Obtained, p);
                }
            }
        }

        LOG("KMCCCTCrossHair::GetCellNameTeleportLinkedDoor NOT FOUND [{}]", Obtained);
        Obtained = NOT_FOUND;
        return false;
    }
    bool KMCCCTCrossHair::GetName(const KMCPattern& p, std::string& Obtained) {
        if (Obtained != "") {
            return IsMatch(Obtained, p);
        } else if (auto target = cpd->target.get(); target) {
            Obtained = target.get()->GetName();
            LOG("KMCCCTCrossHair::GetName [{}]", Obtained);
            return IsMatch(Obtained, p);
        }

        LOG("KMCCCTCrossHair::GetName NOT FOUND [{}]", Obtained);
        Obtained = NOT_FOUND;

        return false;
    }

    bool KMCCCTCrossHair::IsMatch(const std::string& name, const KMCPattern& p) {
        if (p.kpm == kmc_pattern_mat::norm) {
            return name == p.norm;
        } else {
            std::smatch sm;
            if (std::regex_match(name, sm, p.pattern)) {
                return true;
            }
        }
        return false;
    }

    bool KMCCCTBodySlot::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            std::vector<bool> res;
            KMCIsWorn(player, source.body_slot.slots, res);

            if (res.size() == source.body_slot.check_value.size()) {
                int i = 0;
                bool hresult = false;
                bool nhresult = false;
                for (auto& [key, value] : source.body_slot.check_value) {
                    bool rv = res.at(i);
                    if (rv) {
                        if (value == 1) {
                            hresult = true;
                            LOG("[IsWorn] body slot ===> {}, flag:{}", key, value);
                        } else {
                            LOG("[IsWorn] body slot ===> {}, flag:{}", key, value);
                            nhresult = true;
                        }
                    } else {
                        if (value == 0) {
                            LOG("[IsNotWorn] body slot ===> {}, flag:{}", key, value);
                            hresult = true;
                        } else {
                            LOG("[IsNotWorn] body slot ===> {}, flag:{}", key, value);
                            nhresult = true;
                        }
                    }

                    i++;
                }

                if (source.body_slot.match == 0) {
                    // any
                    return hresult;

                } else {
                    // all
                    if (nhresult) return false;

                    return true;
                }
            }
        }

        return false;
    }

    bool KMCCCTMagicEffectKeyword::Check(KMCCCheckSource source) {
        auto player = KMCCT::KMCConfig::GetSingleton()->getPlayer();
        if (player) {
            const auto activeEffects = player->GetMagicTarget()->GetActiveEffectList();
            if (activeEffects) {
                for (auto const& ae : *activeEffects) {
                    const auto mgef = ae ? ae->GetBaseObject() : nullptr;
                    if (mgef) {
                        LOG("[Active Effect] Name {}, FormId {}", mgef->GetName(), mgef->GetFormID());
                        if (source.hsnhs == HasNHan::both) {
                            if (mgef->HasKeywordInArray(source.has, true) &&
                                !mgef->HasKeywordInArray(source.nhas, true)) {
                                return true;
                            }
                        } else if (source.hsnhs == HasNHan::has) {
                            if (mgef->HasKeywordInArray(source.has, true)) {
                                return true;
                            }
                        } else if (source.hsnhs == HasNHan::nhas) {
                            if (!mgef->HasKeywordInArray(source.nhas, true)) {
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    bool KMCCCTFormula::Check(KMCCCheckSource source) {
        for (auto& [key, value] : source.cond_formula) {
            bool t = false;
            bool f = false;
            for (auto& formv : value) {
                LOG("[Evaluate] EntryNo ==> {} Formula ==> {} comp1 ==> {} comp2 ==> {}", key, formv->cond,
                    formv->comp1(), formv->comp2());
                if (JudgeKMCInequalitySign(formv->isign, formv->comp1(), formv->comp2())) {
                    t = true;
                } else {
                    f = true;
                }
            }

            if (t && !f) {
                LOG("[OK] Formula ==>  EntryNo ==> {}", key);
                return true;
            }
        }
        return false;
    }
}