#include "KMCCutinCondition.h"

#include <nlohmann/json.hpp>

#include "KMCConfig.h"
#include "KMCCutin.h"
#include "KMCCutinCond/KMCCCJsonTags.h"
#include "KMCEventThread.h"
#include "KMCExpression.h"
#include "KMCStateManager.h"
#include "KMCTempKeywordManager.h"

SINGLETONBODY(KMCCT::KMCCutinCondition)
namespace KMCCT {
    using namespace boost::property_tree;
    using Clock = std::chrono::steady_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::time_point;
    using namespace std::literals::chrono_literals;

    KMCCCStartArg s_args;
    std::mutex aaaakmc_main_pushed_pop_mtx_;
    std::mutex aaaakmc_main_update_mtx_;
    using json = nlohmann::json;

    void KMCCutinCondition::Setup() {
        // default_value.Setup();
        key_detail.Setup();
        // CutinCondition.jsonを読み込む。formidとかの解決はInit()で行う
        SetupJsonNodes(CUT_IN_CONDITION_FILE_NAME);

        // debug
        // for (auto &ma : custom_cond.managers) {
        //    LOG("m c {} sc1 {} sc2 {}", ma.source.main_category, ma.source.sub1_category, ma.source.sub2_category);
        //    for (auto &no : ma.nodes) {
        //        LOG("n cct {} ty_add_v {} ty_tim_ct {}", (int)no.get()->cutin_cond_type,
        //            no.get()->task_hub.get()->add_value, no.get()->task_hub.get()->cutin_time);
        //    }
        //}

        // for (auto [k, vv] : work_nodes) {
        //     if (vv != nullptr) {
        //         LOG("wn key {} ", k);
        //         LOG("wn cct {} ty_add_v {} ty_tim_ct {}", (int)vv->cutin_cond_type, vv->task_hub.get()->add_value,
        //             vv->task_hub.get()->cutin_time);
        //     }
        // }

        // for (auto &ma : custom_cond.managers) {
        //     for (auto &no : ma.nodes) {
        //         for (auto &sub : no.get()->subtract) {
        //             LOG("project_name {} subtract_target_name {}", no.get()->project_name, sub.target_name);
        //         }
        //     }
        // }

        // for (auto &ma : custom_cond.managers) {
        //     for (auto &no : ma.nodes) {
        //         LOG("project_name {} ------ subtract relation ------", no->project_name);
        //         for (auto &rs : no.get()->node_relations) {
        //             LOG("[subtract relation] project_name {} ", rs->project_name);
        //         }

        //        LOG("project_name {} ------ subtract relation ------", no->project_name);
        //    }
        //}
    }

    void KMCCutinCondition::Init() {
        // formidなどの解決を行う

        // disable manager and nodes
        for (auto &manager : custom_cond.managers) {
            if (manager.get()->disable) {
                WARN("[Disable] Path : {}", manager.get()->cond_custom_pro_name);
            } else {
                WARN("[Enable] Path : {}", manager.get()->cond_custom_pro_name);
                for (auto &node : manager.get()->nodes) {
                    if (node.get()->disable) {
                        WARN("[Disable] Path : {}", node.get()->project_name);
                    } else {
                        WARN("[Enable] Path : {}", node.get()->project_name);
                    }
                }
            }
        }

        auto itr = custom_cond.managers.begin();
        while (itr != custom_cond.managers.end()) {
            if (!itr->get()->disable) {
                auto *mnger = itr->get();
                std::string pname = mnger->cond_custom_pro_name;
                auto source = mnger->source;

                // managerの管理がkeywordか
                if (mnger->source.sub1_category == KMCCCSubCategory::KEYWORD ||
                    mnger->source.sub1_category == KMCCCSubCategory::MAGIC_EFFECT_KEYWORD) {
                    mnger->m_type = ManageType::keyword;
                }

                if (mnger->m_type == ManageType::keyword) {
                    auto sp_keyword_formid = KMCSplit(source.keyword_formid, ',');
                    auto sp_keyword_plugin_name = KMCSplit(source.keyword_plugin_name, ',');
                    if (sp_keyword_formid.size() != sp_keyword_plugin_name.size()) {
                        mnger->disable = true;
                        ERROR(
                            "Keyword is specified, but the number of definitions of keyword_plugin_name and "
                            "keyword_formid "
                            "do not match. [{}]",
                            pname);
                    }

                    if (!mnger->disable) {
                        for (int i = 0; i < sp_keyword_formid.size(); i++) {
                            auto keyword_tmp = (RE::BGSKeyword *)RE::TESDataHandler::GetSingleton()->LookupForm(
                                std::stoll(sp_keyword_formid.at(i), NULL, 16), sp_keyword_plugin_name.at(i));
                            if (keyword_tmp == nullptr) {
                                mnger->disable = true;
                                ERROR(
                                    "keyword is specified, but formid could not be obtained with the combination of "
                                    "keyword_plugin_name and keyword_formid [{}]",
                                    pname);
                                break;
                            }
                            mnger->source.keywords.emplace_back(keyword_tmp);
                        }
                    }

                    if (!mnger->disable) {
                        // has nhasをすぐ使えるようにする
                        if (source.sub1_category == KMCCCSubCategory::KEYWORD ||
                            source.sub1_category == KMCCCSubCategory::MAGIC_EFFECT_KEYWORD) {
                            if (!mnger->source.keyword_has_nhas_build(source.sub2_category)) {
                                mnger->disable = true;
                                ERROR(
                                    "The number of (sub_category2) definitions for has and nhas does not match the "
                                    "number "
                                    "of definitions for keyword");
                            }
                        }
                    }
                }

                if (!mnger->disable) {
                    if (source.sub1_category == KMCCCSubCategory::BODY_SLOT) {
                        mnger->source.body_slot_build();
                    } else if (source.main_category == KMCCCMainCategory::FORMULA) {
                        int andor_c = 0;
                        for (int formi = 0; formi < mnger->source.formula.size(); formi++) {
                            auto fm = &mnger->source.formula.at(formi);
                            if (!fm->Build()) {
                                mnger->disable = true;
                                ERROR("The formula definition is incorrect. Please review it.");
                                break;
                            }

                            mnger->source.cond_formula[andor_c].emplace_back(fm);
                            if (fm->and_or == AndOr::isOr) {
                                LOG("[FROMULA OR] {} EntryNo ==> {}", fm->cond, andor_c);
                                ++andor_c;
                            } else {
                                LOG("[FROMULA AND] {} EntryNo ==> {}", fm->cond, andor_c);
                            }
                        }

                    } else if (source.main_category == KMCCCMainCategory::TEMP_KEYWORD) {
                        if (!mnger->source.temp_keyword_has_nhas_build(source.sub1_category)) {
                            mnger->disable = true;
                            ERROR(
                                "The number of (sub_category1) definitions for has and nhas does not match the "
                                "number "
                                "of definitions for keyword");
                        }
                    }
                }

                if (!mnger->disable) {
                    // node keyword 解決
                    auto itrn = mnger->nodes.begin();
                    while (itrn != mnger->nodes.end()) {
                        auto *node = itrn->get();
                        if (!node->disable) {
                            // type_temp_keywordの場合
                            if (node->sub_task_hub && node->push_type == PushType::keyword) {
                                if (!node->sub_task_hub->sub_task_source.keyword_adrm_build()) {
                                    node->disable = true;
                                    ERROR(
                                        "The number of definitions of the keyword_name and "
                                        "category of type_temp_keywords must match [{}]",
                                        pname);
                                }
                            }

                            if (!node->task_hub->PreProcessing()) {
                                node->disable = true;
                                ERROR("Task preprocessing failed. [{}]", pname);
                            }
                        }

                        if (!node->disable) {
                            itrn++;
                        } else {
                            itrn = mnger->nodes.erase(itrn);
                        }
                    }
                }

                // end
                if (mnger->nodes.size() == 0) {
                    // ノードがなくなった場合
                    itr = custom_cond.managers.erase(itr);
                } else {
                    if (!mnger->disable) {
                        // マネージャが無効ではない場合は次へ
                        itr++;
                    } else {
                        // 無効なのでeraseのポインターをもらう
                        itr = custom_cond.managers.erase(itr);
                    }
                }
            } else {
                itr = custom_cond.managers.erase(itr);
            }
        }

        // removed manager and nodes
        for (auto &manager : custom_cond.managers) {
            WARN("[Enable] Path : {}", manager.get()->cond_custom_pro_name);

            for (auto &node : manager.get()->nodes) {
                WARN("[Enable] Path : {}", node.get()->project_name);
            }
        }

        // std::map<std::string, std::vector<std::string>> project_name_dictionary;
        for (auto &manager : custom_cond.managers) {
            // project_root_names.insert(manager.get()->cond_custom_pro_name);
            auto *mnger = manager.get();
            work_managers.emplace(mnger->cond_custom_pro_name, mnger);
            for (int i = 0; i < mnger->nodes.size(); i++) {
                auto *node = mnger->nodes.at(i).get();
                std::string project_name = node->project_name;
                // project_name_dictionary[manager.get()->cond_custom_pro_name].emplace_back(
                //     node->cond_custom_node_name);
                work_nodes.emplace(project_name, node);

                name_mapping[mnger->cond_custom_pro_name].emplace_back(node->cond_custom_node_name);
            }
        }

        // relationsのノード間の関係を構築
        for (auto &manager : custom_cond.managers) {
            // std::string mk = manager.cond_custom_pro_name;
            auto *mnger = manager.get();
            for (int i = 0; i < mnger->nodes.size(); i++) {
                auto *node = mnger->nodes.at(i).get();
                auto relations = node->relations;
                std::string project_name = node->project_name;
                for (auto s : relations) {
                    if (s.target_name != "") {
                        auto sp = KMCSplit(s.escape_name, '@');
                        if (sp.size() == 2) {
                            std::string first = sp.at(0);
                            std::string second = sp.at(1);
                            for (auto &[pndkey, pndvalue] : name_mapping) {
                                try {
                                    std::regex pattern(first);
                                    std::sregex_iterator it(pndkey.begin(), pndkey.end(), pattern);
                                    std::sregex_iterator end;

                                    if (it != end) {
                                        for (auto &pndvecvalue : pndvalue) {
                                            std::regex pattern2(second);
                                            std::sregex_iterator it2(pndvecvalue.begin(), pndvecvalue.end(), pattern2);
                                            std::sregex_iterator end2;

                                            std::string tell = pndkey + "." + pndvecvalue;
                                            if (it2 != end2) {
                                                LOG("[NODE_RELATIONS] CutinCondition.json [NODE [{}]] ===> Found {} "
                                                    "===> "
                                                    "{}",
                                                    project_name, s.target_name, tell);
                                                node->node_relations.emplace_back(
                                                    KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::
                                                        RelationsData(work_nodes[tell], s));
                                            } /* else {
                                                    ERROR(
                                                        "[RELATION] CutinCondition.json {}, The target defined in "
                                                        "node_relations does not exist. [1] ===> {} / [0].[1] ===> {}",
                                                        s.target_name, pndvecvalue, tell);
                                                }*/
                                        }
                                    }
                                } catch (std::exception ex) {
                                    ERROR("[NODE_RELATIONS] CutinCondition.json [{}] : wt : {}", project_name,
                                          ex.what());
                                }
                            }
                        } else {
                            ERROR(
                                "[NODE_RELATIONS] CutinCondition.json [{}] : node_relations should be defined by root "
                                "name "
                                "and node name.",
                                project_name);
                        }
                    }
                }
            }
        }
    }

    int KMCCutinCondition::ToMove(const KMCCCStartArg &args) {
        int result = 0;
        s_args = args;
        if (!KMCCT::KMCEventThread::GetSingleton()->GetInitEndFlag()) {
            return result;
        }

        Update();

        if (!KMCCT::KMCEventThread::GetSingleton()->GetInitEndFlag()) {
            return result;
        }

        PreProcess();

        if (!KMCCT::KMCEventThread::GetSingleton()->GetInitEndFlag()) {
            return result;
        }

        if (!KMCCT::KMCStateManager::GetSingleton()->GetStoppingState()) {
            StartEvaluation();

            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return result;
            }

            Commit();

            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return result;
            }
        }

        // if (!stay) {
        //     result = custom_cond.CheckAndPopFirstTaskNode(cutin_name, cutin_setting);
        // }
        return result;
    }

    KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *KMCCutinCondition::GetTaskResult() {
        return custom_cond.Try2Get();
    }

    void KMCCutinCondition::Completed(KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        if (node) {
            std::string push_key = node->push_key;
            if (custom_cond.TryPopOut(push_key)) {
                LOG("[KMCCutinCondition::Completed] end of work {}", push_key);
            } else {
                ERROR("[KMCCutinCondition::Completed] not found push_key {}", push_key);
            }

        } else {
            ERROR("[KMCCutinCondition::Completed] Fatal : wtf");
        }
    }

    bool KMCCutinCondition::Contains(std::string project_name) { return custom_cond.Contains(project_name); }

    void KMCCutinCondition::SetMCMUpdate(KMCKDElement elem) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_main_update_mtx_);
            mcm_update.emplace_back(elem);
        }
    }

    void KMCCutinCondition::ResetAll() {
        for (auto &[k, wnv] : work_nodes) {
            wnv->Reset();
        }
    }

    std::vector<std::string> KMCCutinCondition::GetCutinCondition() {
        static std::vector<std::string> vreturn;
        if (vreturn.size() > 0) {
            return vreturn;
        } else {
            for (auto &m : custom_cond.managers) {
                vreturn.emplace_back(m->cond_custom_pro_name);
            }
        }

        return vreturn;
    }

    std::vector<std::string> KMCCutinCondition::GetCutinConditionNode(std::string root_name) {
        return name_mapping[root_name];
    }

    int KMCCutinCondition::GetPathNumber(std::string root_name, std::string work_name) {
        std::string ac_key = root_name;
        if (work_name != "") {
            ac_key += "." + work_name;
        }

        ids[ac_key].clear();

        if (element.contains(ac_key)) {
            return static_cast<int>(element[ac_key].size());
        }

        auto elm = path_mapping[ac_key];
        for (auto &e : elm) {
            std::vector<std::string> sp = KMCSplit(e.path, '/');
            /*if (e.escape) {
                sp = KMCSplit(e.escape_p, e.escape_char);
            } else {
                sp = KMCSplit(e.path, e.escape_char);
            }*/
            if (sp.size() == key_detail.ELEM_MAX) {
                KMCKDElement elem;
                elem.path = sp[0];
                elem.name = sp[1];
                elem.type = sp[2];

                auto range = KMCSplit(sp[3], '-');
                if (range.size() == key_detail.RANG_MAX) {
                    elem.range_min = range[0];
                    elem.range_max = range[1];
                    elem.interval = sp[4];
                    elem.origin_value = sp[5];
                    elem.kd_path = e;
                    element[ac_key].emplace_back(elem);
                } else {
                    ERROR("KMCCutinCondition::GetPathNumber NG {}", ac_key);
                }
            } else {
                ERROR("KMCCutinCondition::GetPathNumber NG {}", ac_key);
            }
        }

        return static_cast<int>(element[ac_key].size());
    }

    void KMCCutinCondition::SetIDContainer(std::string root_name, std::string work_name, int index, int option) {
        std::string ac_key = root_name;
        if (work_name != "") {
            ac_key += "." + work_name;
        }

        LOG("KMCCutinCondition::SetIDContainer: option[{}] : index[{}] : key[{}]", option, index, ac_key);
        ids[ac_key].emplace_back(KMCKDOption(option, index));
    }

    std::vector<std::string> KMCCutinCondition::GetMArray(std::string root_name, int index) {
        auto paths = element[root_name].at(index);
        marray.clear();

        marray.emplace_back(paths.path);
        marray.emplace_back(paths.name);
        marray.emplace_back(paths.type);
        marray.emplace_back(paths.range_min);
        marray.emplace_back(paths.range_max);
        marray.emplace_back(paths.interval);
        marray.emplace_back(paths.origin_value);

        // LOG("KMCCutinCondition::GetMArray key:{} path:{}", root_name, paths.path);
        return marray;
    }
    std::vector<std::string> KMCCutinCondition::GetNArray(std::string root_name, std::string work_name, int index) {
        auto paths = element[root_name + "." + work_name].at(index);
        narray.clear();

        narray.emplace_back(paths.path);
        narray.emplace_back(paths.name);
        narray.emplace_back(paths.type);
        narray.emplace_back(paths.range_min);
        narray.emplace_back(paths.range_max);
        narray.emplace_back(paths.interval);
        narray.emplace_back(paths.origin_value);

        // LOG("KMCCutinCondition::GetNArray key:{} path:{}", root_name + "." + work_name, paths.path);

        return narray;
    }

    std::vector<std::string> KMCCutinCondition::SearchID(std::string root_name, std::string work_name, int option) {
        oparray.clear();

        std::string ac_key = root_name;
        if (work_name != "") {
            ac_key += "." + work_name;
        }
        auto vec = ids.at(ac_key);
        // for (auto &value : ids.at(ac_key)) {
        auto it = std::find_if(vec.begin(), vec.end(), [option](const KMCKDOption &p) { return p.id == option; });
        if (it != vec.end()) {
            if (it->index != -1) {
                auto paths = element[ac_key].at(it->index);
                oparray.emplace_back(paths.path);
                oparray.emplace_back(paths.name);
                oparray.emplace_back(paths.type);
                oparray.emplace_back(paths.range_min);
                oparray.emplace_back(paths.range_max);
                oparray.emplace_back(paths.interval);
                oparray.emplace_back(paths.origin_value);
                LOG("KMCCutinCondition::SearchID: option[{}] : name[{}]", option, paths.name);
                return oparray;
            }
        }
        //}

        LOG("KMCCutinCondition::SearchID NG: option[{}]", option);

        return oparray;
    }

    int KMCCutinCondition::SetResultMCM(std::string root_name, std::string work_name, int option,
                                        std::string result_value) {
        std::string ac_key = root_name;
        if (work_name != "") {
            ac_key += "." + work_name;
        }
        auto vec = ids.at(ac_key);

        // for (auto &[key, value] : ids) {
        auto it = std::find_if(vec.begin(), vec.end(), [option](const KMCKDOption &p) { return p.id == option; });
        if (it != vec.end()) {
            if (it->index != -1) {
                auto paths = element[ac_key].at(it->index);
                paths.ac_key = ac_key;
                paths.index = it->index;
                if (paths.type == "i") {
                    int t = std::stoi(result_value);
                    int origin = std::stoi(paths.origin_value);
                    if (t != origin) {
                        paths.ivalue = t;
                        result_array.emplace(paths.path, paths);
                    } else {
                        result_array.erase(paths.path);
                    }
                } else if (paths.type == "s") {
                    std::string bef = result_value;
                    std::string af = bef;
                    std::transform(bef.begin(), bef.end(), af.begin(), [](char c) { return std::tolower(c); });

                    if (paths.origin_value != af) {
                        paths.value = af;
                        result_array.emplace(paths.path, paths);
                    } else {
                        result_array.erase(paths.path);
                    }
                } else if (paths.type == "f") {
                    double t = std::stod(result_value);
                    double origin = std::stod(paths.origin_value);
                    if (t != origin) {
                        paths.dvalue = t;
                        result_array.emplace(paths.path, paths);
                    } else {
                        result_array.erase(paths.path);
                    }
                } else if (paths.type == "l") {
                    long t = std::stol(result_value);
                    long origin = std::stol(paths.origin_value);
                    if (t != origin) {
                        paths.lvalue = t;
                        result_array.emplace(paths.path, paths);
                    } else {
                        result_array.erase(paths.path);
                    }
                } else {
                    ERROR("KMCCKeyDetail Build unkown type");
                    return -2;
                }

                LOG("KMCCutinCondition::SetResultMCM: option[{}] : name[{}]", option, paths.name);
                return 0;
            }
        }
        //}
        return -1;
        LOG("KMCCutinCondition::SetResultMCM NG: option[{}] : result_value[{}]", option, result_value);
    }

    bool KMCCutinCondition::IsUpdateMCM() { return result_array.size() > 0; }

    std::string KMCCutinCondition::SaveKMCMCM() {
        try {
            std::ifstream stream(COMMON_PATH + CUT_IN_CONDITION_FILE_NAME);

            if (!stream.is_open()) throw new std::exception("Failed open file.");

            if (!json::accept(stream)) throw new std::exception("Incorrect json format.");

            stream.seekg(0, std::ios::beg);

            json j = json::parse(stream);

            for (auto &[key, value] : result_array) {
                LOG("[SAVE] key:[{}]", key);
                std::vector<std::string> sp;
                if (key.contains('@')) {
                    sp = KMCSplit(key, '@');
                } else {
                    sp = KMCSplit(key, '.');
                }
                // auto sp = KMCSplit(key, '.');
                auto *n = &j.at(sp[0]);
                for (int i = 1; i < sp.size() - 1; i++) {
                    n = &n->at(sp[i]);
                }
                bool upd = false;
                std::string upd_value = "";
                value.end_name = sp.back();
                if (value.type == "i") {
                    (*n)[value.end_name] = value.ivalue;
                    upd_value = std::to_string(value.ivalue);
                    upd = true;
                    // pt.put(key, value.ivalue);
                    LOG("[SAVE] key:[{}] value:[{}]", key, value.ivalue);
                } else if (value.type == "f") {
                    // float res = round_n(value.fvalue, 2.0);
                    (*n)[value.end_name] = value.dvalue;
                    upd_value = std::to_string(value.dvalue);
                    upd = true;
                    // pt.put(key, value.fvalue);
                    LOG("[SAVE] key:[{}] value:[{}]", key, value.dvalue);
                } else if (value.type == "s") {
                    if (value.kd_path.is_push) {
                        (*n)[value.end_name].push_back(value.value);
                        upd_value = value.origin_value;
                    } else {
                        (*n)[value.end_name] = value.value;
                        upd_value = value.value;
                    }

                    upd = true;
                    // pt.put(key, value.value);
                    LOG("[SAVE] key:[{}] value:[{}]", key, value.value);
                } else if (value.type == "l") {
                    (*n)[value.end_name] = value.dvalue;
                    upd_value = std::to_string(value.dvalue);
                    upd = true;
                    // pt.put(key, value.dvalue);
                    LOG("[SAVE] key:[{}] value:[{}]", key, value.dvalue);
                } else {
                    ERROR("[SAVE ERROR] not found type : key[{}]", key);
                }

                if (upd && value.ac_key != "" && value.index != -1) {
                    // MCM側で読み取るものを更新しておく
                    element[value.ac_key].at(value.index).origin_value = upd_value;

                    // 更新後の値でゲーム中にアップデート可能なものをプッシュする
                    SetMCMUpdate(value);
                }
            }

            std::ofstream output_file(COMMON_PATH + CUT_IN_CONDITION_FILE_NAME);
            if (!output_file.is_open()) {
                // std::cout << "\n Failed to open output file";
            } else {
                output_file << std::setw(4) << j << std::endl;
                output_file.close();
            }

            result_array.clear();
        } catch (std::exception ex) {
            ERROR("[SAVE ERROR] wt:{}", ex.what());
            return ex.what();
        }

        return "";

        // ptree pt;
        // try {
        //     read_json(COMMON_PATH + CUT_IN_CONDITION_FILE_NAME, pt);
        //     LOG("[SAVE] JsonFileName = {}", CUT_IN_CONDITION_FILE_NAME);

        //    for (auto &[key, value] : result_array) {
        //
        //        if (value.type == "i") {
        //            pt.put(key, value.ivalue);
        //            LOG("[SAVE] key:[{}] value:[{}]", key, value.ivalue);
        //        } else if (value.type == "f") {
        //            pt.put(key, value.fvalue);
        //            LOG("[SAVE] key:[{}] value:[{}]", key, value.fvalue);
        //        } else if (value.type == "s") {
        //            pt.put(key, value.value);
        //            LOG("[SAVE] key:[{}] value:[{}]", key, value.value);
        //        } else if (value.type == "l") {
        //            pt.put(key, value.dvalue);
        //            LOG("[SAVE] key:[{}] value:[{}]", key, value.dvalue);
        //        } else {
        //            ERROR("[SAVE ERROR] not found type : key[{}]", key);
        //        }
        //
        //    }
        //    write_json(COMMON_PATH + CUT_IN_CONDITION_FILE_NAME, pt);
        //    result_array.clear();
        //} catch (std::exception ex) {
        //    ERROR("[SAVE ERROR] wt:{}", ex.what());
        //    return ex.what();
        //}

        return "";
    }

    void KMCCutinCondition::Update() {
        if (mcm_update.size() == 0) return;

        {
            std::lock_guard<std::mutex> lock(aaaakmc_main_update_mtx_);

            for (auto &mcm : mcm_update) {
                std::string ac_key = mcm.ac_key;
                try {
                    if (ac_key.contains(".")) {
                        work_nodes.at(ac_key)->Update(mcm);
                    } else {
                        work_managers.at(ac_key)->Update(mcm);
                    }
                } catch (std::exception ex) {
                    ERROR("KMCCutinCondition::Update MCM key:[{}] wt{}", ac_key, ex.what());
                }
            }

            mcm_update.clear();
        }
    }

    void KMCCutinCondition::PreProcess() { custom_cond.PlayFcExp(); }

    void KMCCutinCondition::StartEvaluation() {
        custom_cond.checked_nodes.clear();
        custom_cond.pre_commit_nodes.clear();
        custom_cond.pre_completed_nodes.clear();
        custom_cond.completed_nodes.clear();
        custom_cond.post_nodes.clear();

        for (auto &manager : custom_cond.managers) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            if (!manager->disable) {
                manager->Instruct();
            }
        }
    }

    void KMCCutinCondition::Commit() {
        if (custom_cond.checked_nodes.size() > 0) {
            custom_cond.PreCommit();

            if (custom_cond.pre_completed_nodes.size() > 0) {
                custom_cond.Commit();

                if (custom_cond.post_nodes.size() > 0) {
                    custom_cond.Post();
                }
            }
        }
    }

    void KMCCustomCondMain::PreCommit() {
        pre_commit_nodes = checked_nodes;

        std::chrono::duration<float> diff = Clock::now() - s_args.start_time;
        float dur = diff.count();
        float correction = 0.0f;
        if (dur > DELAY_THRESHHOLD) {
            correction = round_n(dur, ROUND_N);
            LOG("[CORRECTION] [CORRECTION][{}] [DUR][{}]", correction, dur);
        }

        for (auto &m_node : checked_nodes) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            auto node = m_node.second;
            // 補正
            LOG("[CORRECTION] TARGET_NODE ===> [{}] * [CORRECTION][{}] [DURATION][{}] ", m_node.first, correction, dur);
            node->task_hub->Correction(correction);

            for (auto &sbt : node->node_relations) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    return;
                }

                if (sbt.relations.timing == 1) {
                    // timing = 1は状態が続く限り 関係するノードの処理を行う
                    if (!sbt.rnode->task_hub->Completed()) {
                        LOG("[{}] CALC ==> [{}]", m_node.first, sbt.rnode->project_name);
                        if (sbt.relations.calc) {
                            sbt.rnode->task_hub->Add(sbt.relations.add_value, correction);
                            sbt.rnode->task_hub->Subtract(sbt.relations.subtract_value, correction);
                            sbt.rnode->task_hub->Div(sbt.relations.div_value, correction);
                            sbt.rnode->task_hub->Mult(sbt.relations.mult_value, correction);

                            if (!sbt.rnode->task_hub->IsEpsilon()) {
                                std::string pushk = sbt.rnode->post_commit_push_key;
                                if (!pre_commit_nodes.contains(pushk)) {
                                    pre_commit_nodes.emplace(sbt.rnode->post_commit_push_key, sbt.rnode);
                                }
                            }
                        }
                    }
                }
            }
        }

        float percent = 0.0f;
        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *> exp_work;

        for (auto &m_node : pre_commit_nodes) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }
            std::string mpushed_key = m_node.first;
            auto node = m_node.second;

            if (node->task_hub->IsEpsilon()) continue;

            LOG("[COMMIT1] [{}]", node->post_commit_push_key);
            node->task_hub->Commit();

            if (node->task_hub->CompPercent() > percent) {
                // カットインするものがどれくらいの進捗率となっているか
                percent = node->task_hub->CompPercent();
            }

            if (node->task_hub->Completed()) {
                pre_completed_nodes.emplace(mpushed_key, node);

            } else {
                // force expression
                if (node->force_exp_timing == 2) {
                    exp_work.emplace(node->post_commit_push_key, node);
                }
            }
        }

        // force exp
        if (pre_completed_nodes.size() > 0) {
            TryReleaseExp();
        } else {
            for (auto &[key, node] : exp_work) {
                if (node->stop_percentage >= percent) {
                    // 現在進行中のタスクがstop_percentage以上の進捗の場合以外はプッシュする
                    // 同着のものがある場合、優先順位はnodeのpriorityによる
                    TryPushExp(node->post_commit_push_key, node);
                }
            }
        }
    }

    void KMCCustomCondMain::Commit() {
        completed_nodes = pre_completed_nodes;

        std::chrono::duration<float> diff = Clock::now() - s_args.start_time;
        float dur = diff.count();
        float correction = 0.0f;
        if (dur > DELAY_THRESHHOLD) {
            correction = round_n(dur, ROUND_N);
            LOG("[correction] correction {} dur {}", correction, dur);
        }

        for (auto &m_node : pre_completed_nodes) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            std::string mpushed_key = m_node.first;
            auto node = m_node.second;

            for (auto &sbt : node->node_relations) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    return;
                }

                if (sbt.relations.timing == 0 && node->task_hub->Completed()) {
                    // timing = 0はタスク完了時1回目に限り 関係するノードの処理を行う
                    if (!sbt.rnode->task_hub->Completed()) {
                        LOG("[{}] CALC ==> [{}]", mpushed_key, sbt.rnode->project_name);
                        if (sbt.relations.calc) {
                            sbt.rnode->task_hub->Add(sbt.relations.add_value, correction);
                            sbt.rnode->task_hub->Subtract(sbt.relations.subtract_value, correction);
                            sbt.rnode->task_hub->Div(sbt.relations.div_value, correction);
                            sbt.rnode->task_hub->Mult(sbt.relations.mult_value, correction);

                            if (!sbt.rnode->task_hub->IsEpsilon()) {
                                std::string pushk = sbt.rnode->post_commit_push_key;
                                completed_nodes.emplace(sbt.rnode->post_commit_push_key, sbt.rnode);
                            }
                        }
                    }
                }
            }
        }

        for (auto &m_node : completed_nodes) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            std::string mpushed_key = m_node.first;
            auto node = m_node.second;

            // commit
            LOG("[COMMIT2] [{}]", node->post_commit_push_key);
            node->task_hub->Commit();

            if (!node->push_end && node->task_hub->Completed()) {
                // after process
                post_nodes.emplace(node->post_commit_push_key, node);
            }
        }
    }

    void KMCCustomCondMain::Post() {
        for (auto &m_node : post_nodes) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }

            std::string mpushed_key = m_node.first;
            auto node = m_node.second;

            // push
            if (node->not_cutin) {
                LOG("[PUSH] [NOT CUTIN] [{}], [NPUSH_KEY]===>[{}]", node->post_commit_push_key, node->push_key);
                node->polling.timer = Clock::now();
                node->Stop();
            } else if (node->PushTask()) {
                LOG("[PUSH] [{}], [PUSH_KEY]===>[{}]", node->post_commit_push_key, node->push_key);
                node->polling.timer = Clock::now();
                node->Stop();
            } else {
                ERROR("[FATAL] main->TryPush Duplicate Key {} ", node->push_key);
            }
        }
    }

    template <>
    void KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Stop() {
        this->push_end = true;
    }

    template <>
    void KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Reset() {
        this->task_hub->Reset();
        if (this->sub_task_hub) {
            this->sub_task_hub->Reset();
        }
        this->check_hub->Reset();
        this->push_end = false;
        this->once_and_pop_out_end = false;
    }

    template <>
    bool KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Check(KMCCCheckSource m_source) {
        LOG("KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Check {}", this->push_key);

        if (push_type == PushType::cutin) {
            return this->check_hub->Check(m_source);
        } else if (sub_task_hub && push_type == PushType::keyword) {
            return this->check_hub->Check(m_source) && sub_task_hub->Check();
        }

        ERROR(
            "----------unknown error---------- "
            "KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Check {}",
            this->push_key);

        return false;
    }

    template <>
    bool KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::PushTask() {
        if (push_type == PushType::cutin) {
            return manager->main->TryPush(this->push_key, this);
        } else if (sub_task_hub && push_type == PushType::keyword) {
            return sub_task_hub->PushTask();
        }

        ERROR(
            "----------unknown error---------- "
            "KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::PushTask {}",
            this->push_key);

        return false;
    }

    // -1 once flagがあるもので処理済みのものは以降処理しないもの
    // -99 基本はポーリング中を表す
    // 正の値 : 0はプッシュされたタスクではないので関係なし
    //          1はプッシュしたものが処理済み
    //          2はポーリング時間を過ぎたのでタスク削除
    //          3はポーリング時間の設定がないのでタスク削除
    template <>
    int KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Polling() {
        if (this->push_end) {
            if (push_type == PushType::cutin) {
                //
                bool contains = this->manager->main->Contains(push_key);

                if (this->once && contains == false) {
                    this->once_and_pop_out_end = true;
                    return -1;
                } else if (this->once == false && contains == false) {
                    this->Reset();
                    return 1;
                } else {
                    LOG("[{}]", push_key);
                    if (this->polling.stay_time > 0.0f) {
                        time_point<Clock> now = Clock::now();
                        float st_t = this->polling.stay_time;
                        milliseconds diff = duration_cast<milliseconds>(now - this->polling.timer);
                        long long dur = diff.count();
                        if (dur >= st_t * KMCCT::TIME_SCALE_MS) {
                            LOG("[{}] REMOVE dur : {}, stay_time_ms : {}, stay_time(base) : {}", push_key, dur,
                                st_t * KMCCT::TIME_SCALE_MS, st_t);
                            this->manager->main->TryPopOut(push_key);

                            if (!this->once) {
                                this->Reset();
                                return 2;
                            } else {
                                this->once_and_pop_out_end = true;
                                return -1;
                            }
                        }
                    } else {
                        this->manager->main->TryPopOut(push_key);
                        if (!this->once) {
                            this->Reset();
                            return 3;
                        } else {
                            this->once_and_pop_out_end = true;
                            return -1;
                        }
                    }
                }
            } else if (sub_task_hub && push_type == PushType::keyword) {
                if (this->once) {
                    this->once_and_pop_out_end = true;
                    return -1;
                } else if (this->once == false && this->polling.stay_time > 0.0f) {
                    LOG("[{}]", push_key);
                    time_point<Clock> now = Clock::now();
                    float st_t = this->polling.stay_time;
                    milliseconds diff = duration_cast<milliseconds>(now - this->polling.timer);
                    long long dur = diff.count();
                    if (dur >= st_t * KMCCT::TIME_SCALE_MS) {
                        LOG("[{}] dur : {}, stay_time_ms : {}, stay_time(base) : {}", push_key, dur,
                            st_t * KMCCT::TIME_SCALE_MS, st_t);

                        if (!this->once) {
                            this->Reset();
                            return 2;
                        } else {
                            this->once_and_pop_out_end = true;
                            return -1;
                        }
                    }
                } else {
                    this->Reset();
                    return 1;
                }
            } else {
                ERROR(
                    "----------unknown error---------- "
                    "KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Polling {}",
                    this->push_key);
            }

        } else {
            return 0;
        }

        return -99;
    }

    template <>
    void KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Update(const KMCKDElement &elem) {
        std::string end_name = std::to_string(elem.kd_path.level) + "." + elem.end_name;
        double n = 3.0;
        if (end_name == KMCCLevelTags::F_PRIORITY) {
            this->priority = elem.ivalue;
#pragma region[04][cycle]
        } else if (end_name == KMCCLevelTags::S_ONCE) {
            this->once = elem.ivalue == 1;
#pragma endregion

#pragma region[05][force_ct]
        } else if (end_name == KMCCLevelTags::S_FORCE_CUTIN) {
            this->force_cutin = elem.ivalue == 1;
        } else if (end_name == KMCCLevelTags::S_CUTIN_NAME) {
            this->force_cutin_name = elem.value;
#pragma endregion

#pragma region[030][flag]
        } else if (end_name == KMCCLevelTags::S_OP_NOT_CUTIN) {
            this->not_cutin = elem.ivalue == 1;
#pragma endregion

#pragma region[06][force_exp]
        } else if (end_name == KMCCLevelTags::S_EXP_ID) {
            this->force_exp_name = elem.value;
        } else if (end_name == KMCCLevelTags::S_FORCE_EXP) {
            this->force_exp_timing = elem.ivalue;
        } else if (end_name == KMCCLevelTags::S_EXPRESSION_TIME) {
            this->force_expression_time = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::S_EXP_COOL_TIME) {
            this->force_expression_cool_time = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::S_STOP_PERCENTAGE) {
            this->stop_percentage = round_n(elem.dvalue, n);
#pragma endregion

#pragma region[10][type_add]
        } else if (end_name == KMCCLevelTags::F_ADD_VALUE) {
            this->task_hub->add_value = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_COEF_1) {
            this->task_hub->coef_1 = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_COEF_2) {
            this->task_hub->coef_2 = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_COEF_RELATION) {
            this->task_hub->SetCoefR(elem.value);
        } else if (end_name == KMCCLevelTags::F_COOL_TIME) {
            this->task_hub->cool_time = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_UPPER_VALUE) {
            this->task_hub->upper_value = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_LOWER_VALUE) {
            this->task_hub->lower_value = round_n(elem.dvalue, n);

#pragma endregion

#pragma region[11][type_time]
        } else if (end_name == KMCCLevelTags::F_END_TIME) {
            this->task_hub->end_time = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_START_TIME) {
            this->task_hub->start_time = round_n(elem.dvalue, n);
#pragma endregion

#pragma region[12][type_amount]
        } else if (end_name == KMCCLevelTags::F_STACK_LIMIT) {
            this->task_hub->stack_limit = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_TARGET_VALUE) {
            this->task_hub->target_amount = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_ABANDON_AMOUNT) {
            this->task_hub->abandon_amount = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_STACK_COOL_TIME) {
            this->task_hub->cool_time = round_n(elem.dvalue, n);
#pragma endregion

        } else if (end_name == KMCCLevelTags::S_CALC_ADD_VALUE) {
            for (int i = 0; i < this->node_relations.size(); i++) {
                auto *relation = &this->node_relations.at(i).relations;
                if (elem.kd_path.escape_p == relation->key_name) {
                    relation->add_value = round_n(elem.dvalue, n);
                    break;
                }
            }
        } else if (end_name == KMCCLevelTags::S_CALC_SUBTRACT_VALUE) {
            for (int i = 0; i < this->node_relations.size(); i++) {
                auto *relation = &this->node_relations.at(i).relations;
                if (elem.kd_path.escape_p == relation->key_name) {
                    relation->subtract_value = round_n(elem.dvalue, n);
                    break;
                }
            }
        } else if (end_name == KMCCLevelTags::S_CALC_DIV_VALUE) {
            for (int i = 0; i < this->node_relations.size(); i++) {
                auto *relation = &this->node_relations.at(i).relations;
                if (elem.kd_path.escape_p == relation->key_name) {
                    relation->div_value = round_n(elem.dvalue, n);
                    break;
                }
            }
        } else if (end_name == KMCCLevelTags::S_CALC_MULT_VALUE) {
            for (int i = 0; i < this->node_relations.size(); i++) {
                auto *relation = &this->node_relations.at(i).relations;
                if (elem.kd_path.escape_p == relation->key_name) {
                    relation->mult_value = round_n(elem.dvalue, n);
                    break;
                }
            }
        } else if (end_name == KMCCLevelTags::F_STAY_TIME) {
            this->polling.stay_time = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_TIME) {
            this->cutin_setting.time = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_ANIM_TIME) {
            this->cutin_setting.anim_time = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_VOLUME) {
            this->cutin_setting.volume = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_OAR_TIME) {
            this->cutin_setting.oar_time = round_n(elem.dvalue, n);
        } else if (end_name == KMCCLevelTags::F_OVERRIDE_CI_SETTING) {
            this->cutin_setting.override_setting = elem.ivalue == 1;
        }
    }

    void KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>::Tuning(KMCCutinValues &target) {
        if (!force_cutin && force_cutin_name != "") {
            if (!KMCCT::KMCStateManager::GetSingleton()->GetWhetherThereNoState()) {
                WARN(
                    "[Cutin] This is a cut-in name with no forced interrupt"
                    "setting. It is currently uninterruptible.");
            } else {
                target.aaaakmctype = force_cutin_name;
            }
        } else if (force_cutin_name != "") {
            target.aaaakmctype = force_cutin_name;
        }

        if (force_exp_timing == 1) {
            target.aaaakmcExptype = force_exp_name;
            target.overri_fc_exp = target.aaaakmcExptype != force_cutin_name && force_exp_name != "";

            target.aaaakmcexp = force_expression_time;
            target.overri_exp_time = true;
        } else {
            target.aaaakmcExptype = force_cutin_name;
        }

        if (cutin_setting.override_setting) {
            // result.overri = cutin_setting;
            target.aaaakmcAnimtime = cutin_setting.anim_time;
            target.aaaakmcoar = cutin_setting.oar_time;
            target.aaaakmctime = cutin_setting.time;
            target.aaaakmcvolum = cutin_setting.volume;
            target.aaaakmcexp = cutin_setting.exp_time;
            target.overri_oar_time = true;
            target.overri_exp_time = true;
            if (force_exp_timing == 1) {
                target.aaaakmcexp = force_expression_time;
            }
        }
    }

    template <>
    void KMCCustomCondManager<KMCCustomCondMain>::Instruct() {
        for (auto &node : nodes) {
            if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                return;
            }
            std::string push_key = node->push_key;

            if (node->once_and_pop_out_end) {
                LOG("[{}] ONCE POP OUT END LOOP", push_key);
                continue;
            }

            if (node->Polling() < 0) {
                continue;
            }

            if (node->Check(source)) {
                // bool completed = node->task_hub->Completed();
                // if (!completed) {
                LOG("[{}]", node->project_name);
                node->task_hub->ToMove();
                //}

                main->checked_nodes.emplace(node->post_commit_push_key, node.get());
            }
        }
    }

    template <>
    void KMCCustomCondManager<KMCCustomCondMain>::Update(const KMCKDElement &elem) {
        std::string end_name = std::to_string(elem.kd_path.level) + "." + elem.end_name;

        if (end_name == KMCCLevelTags::T_PRIORITY) {
            this->priority = elem.ivalue;

#pragma region[011][body_slot]
        } else if (end_name == KMCCLevelTags::F_IS_MATCH) {
            this->source.body_slot.match = elem.ivalue;
        } else if (end_name == KMCCLevelTags::F_HEAD) {
            this->source.body_slot.Set(KMCCCJsonTags::HEAD, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_HAIR) {
            this->source.body_slot.Set(KMCCCJsonTags::HAIR, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_BODY) {
            this->source.body_slot.Set(KMCCCJsonTags::BODY, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_HANDS) {
            this->source.body_slot.Set(KMCCCJsonTags::HANDS, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_FOREARMS) {
            this->source.body_slot.Set(KMCCCJsonTags::FOREARMS, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_AMULET) {
            this->source.body_slot.Set(KMCCCJsonTags::AMULET, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_RING) {
            this->source.body_slot.Set(KMCCCJsonTags::RING, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_FEET) {
            this->source.body_slot.Set(KMCCCJsonTags::FEET, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_CALVES) {
            this->source.body_slot.Set(KMCCCJsonTags::CALVES, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_SHIELD) {
            this->source.body_slot.Set(KMCCCJsonTags::SHIELD, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_TAIL) {
            this->source.body_slot.Set(KMCCCJsonTags::TAIL, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_LONGHAIR) {
            this->source.body_slot.Set(KMCCCJsonTags::LONGHAIR, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_CIRCLET) {
            this->source.body_slot.Set(KMCCCJsonTags::CIRCLET, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_EARS) {
            this->source.body_slot.Set(KMCCCJsonTags::EARS, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_FACE_COVERING_OVER_THE_MOUTH) {
            this->source.body_slot.Set(KMCCCJsonTags::FACE_COVERING_OVER_THE_MOUTH, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_NECK_COVERING) {
            this->source.body_slot.Set(KMCCCJsonTags::NECK_COVERING, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_CLOAK) {
            this->source.body_slot.Set(KMCCCJsonTags::CLOAK, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_BACKPACK) {
            this->source.body_slot.Set(KMCCCJsonTags::BACKPACK, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_MAGIC_FX) {
            this->source.body_slot.Set(KMCCCJsonTags::MAGIC_FX, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_SKIRT) {
            this->source.body_slot.Set(KMCCCJsonTags::SKIRT, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_DECAPITATEHEAD) {
            this->source.body_slot.Set(KMCCCJsonTags::DECAPITATEHEAD, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_DECAPITATE) {
            this->source.body_slot.Set(KMCCCJsonTags::DECAPITATE, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_GENITALS_OR_UNDERWEAR) {
            this->source.body_slot.Set(KMCCCJsonTags::GENITALS_OR_UNDERWEAR, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_PANTS) {
            this->source.body_slot.Set(KMCCCJsonTags::PANTS, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_STOCKINGS) {
            this->source.body_slot.Set(KMCCCJsonTags::STOCKINGS, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_FACE_COVERING_USUALLY_UPPER_FACE) {
            this->source.body_slot.Set(KMCCCJsonTags::FACE_COVERING_USUALLY_UPPER_FACE, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_TORSO) {
            this->source.body_slot.Set(KMCCCJsonTags::TORSO, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_SHOULDERS) {
            this->source.body_slot.Set(KMCCCJsonTags::SHOULDERS, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_SHIRT_OR_LEFT_ARM) {
            this->source.body_slot.Set(KMCCCJsonTags::SHIRT_OR_LEFT_ARM, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_LOOSE_SHIRT_OR_RIGHT_ARM) {
            this->source.body_slot.Set(KMCCCJsonTags::LOOSE_SHIRT_OR_RIGHT_ARM, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_MISCELLANEOUS) {
            this->source.body_slot.Set(KMCCCJsonTags::MISCELLANEOUS, elem.ivalue);
        } else if (end_name == KMCCLevelTags::F_FX01) {
            this->source.body_slot.Set(KMCCCJsonTags::FX01, elem.ivalue);
#pragma endregion
        }
    }

    KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *KMCCustomCondMain::Try2Get() {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_main_pushed_pop_mtx_);
            LOG("[Try2Get] Count {}", pushed_task.size());

            if (pushed_task.size() > 0) {
                return pushed_task.begin()->second;
            }

            return nullptr;
        }
    }

    bool KMCCustomCondMain::TryPopOut(std::string pop_key) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_main_pushed_pop_mtx_);

            LOG("[TryPopOut] Count {}", pushed_task.size());

            if (pushed_task.contains(pop_key)) {
                auto num = pushed_task.erase(pop_key);
                LOG("[TryPopOut] erase {}", num);
                // auto node = task.mapped();
                // if (!node->once) {
                //     node->Reset();
                // } else {
                //     node->once_and_pop_out_end = true;
                // }
                // pushed_task.erase(pop_key);
                return true;
            }

            return false;
        }
    }

    bool KMCCustomCondMain::TryPush(std::string push_key,
                                    KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_main_pushed_pop_mtx_);

            if (this->pushed_task.contains(push_key)) {
                return false;
            }

            this->pushed_task.insert(std::make_pair(push_key, node));

            LOG("[TryPush] Count {}, Project {}, ", pushed_task.size(), node->project_name);

            return true;
        }
    }

    bool KMCCustomCondMain::TryPushExp(std::string push_key,
                                       KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        pushed_exp_task.insert(std::make_pair(push_key, node));

        LOG("[TryPushExp] Count {}, Project {}", pushed_exp_task.size(), node->project_name);

        return true;
    }

    bool KMCCustomCondMain::TryReleaseExp() {
        pushed_exp_task.clear();
        LOG("[TryReleaseExp]");

        return true;
    }

    void KMCCustomCondMain::PlayFcExp() {
        LOG("[PlayFcExp] Count {}", pushed_exp_task.size());
        for (auto &[key, node] : pushed_exp_task) {
            int ret = KMCCT::KMCExpression::GetSingleton()->ForceExp(
                node->force_exp_name, node->force_expression_cool_time, node->force_expression_time);
            LOG("[PlayFcExp] ExpID {} Count {}, Project {}, ret {}", node->force_exp_name, pushed_exp_task.size(),
                node->project_name, ret);

            if (ret != 1) {
                LOG("[PlayFcExp] FC EXP =====> Project {}", pushed_exp_task.size(), node->project_name);
                break;
            }
        }

        pushed_exp_task.clear();
    }

    bool KMCCustomCondMain::Contains(std::string find_key) {
        {
            std::lock_guard<std::mutex> lock(aaaakmc_main_pushed_pop_mtx_);
            LOG("[Contains] Count {}, Project {}, ", pushed_task.size(), find_key);
            if (this->pushed_task.size() > 0 && this->pushed_task.contains(find_key)) {
                return true;
            } else {
                return false;
            }
        }
    }

    void KMCCutinCondition::SetupJsonNodes(std::string jsonFileName) {
        ptree pt;
        try {
            read_json(COMMON_PATH + jsonFileName, pt);
            LOG("JsonFileName = {}", jsonFileName);
            for (auto &elements : pt) {
                std::string cond_main_key = elements.first;
                auto cond_main_value = elements.second;
                now_json_node = cond_main_key;
                int level = 1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (auto nested = cond_main_value.get_child_optional(""); nested && !nested.get().empty()) {
                    // if (boost::optional<int> disable = nested.get().get_optional<int>(KMCCCJsonTags::DISABLE)) {
                    //     if (disable.get() == 1) {
                    //         LOG("Level {} key {} disable", level, now_json_node + "." + KMCCCJsonTags::DISABLE);

                    //        continue;
                    //    }
                    //} else {
                    //    LOG("Level {} key {} not found disable flag. enable", level,
                    //        now_json_node + "." + KMCCCJsonTags::DISABLE);
                    //}

                    custom_cond.managers.emplace_back(std::make_unique<KMCCustomCondManager<KMCCustomCondMain>>());

                    KMCCustomCondManager<KMCCustomCondMain> *manager = custom_cond.managers.back().get();
                    manager->cond_custom_pro_name = cond_main_key;
                    manager->main = &custom_cond;

                    SetupJsonNodesLevel1(nested.get(), level, manager);

                    // validate

                    if (Validate_manager(manager) == false) {
                        // validate managerがerrorの場合はそのmanager自体にdisableフラグを立てる
                        manager->disable = true;
                    }

                    for (auto &node : manager->nodes) {
                        if (Validate_node(node.get()) == false || manager->disable) {
                            // validate がerrorの場合はそのnode自体にdisableフラグを立てる
                            node.get()->disable = true;
                            ERROR("Level {} key {} Validate check error(worker node). force disable", level,
                                  now_json_node + "..." + node.get()->cond_custom_node_name);
                        }
                    }

                    if (manager->disable) {
                        ERROR("Level {} key {} Validate check error. force disable", level, now_json_node);
                    } else {
                    }
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, cond_main_key);
                }
            }
        } catch (std::exception ex) {
            ERROR("ERROR LOADING {}", ex.what());
        }
    }

    void KMCCutinCondition::SetupJsonNodesLevel1(ptree pt, int level,
                                                 KMCCustomCondManager<KMCCustomCondMain> *manager) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = manager->cond_custom_pro_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::OPTION) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesOption(nested.get(), level, manager, nullptr);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::MAIN) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesMain(nested.get(), level, manager);

                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::CONDITION) {
                    // condition配下をループする関数をコールする。
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesLevel2(nested.get(), level, manager);

                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesMain(boost::property_tree::ptree pt, int level,
                                               KMCCustomCondManager<KMCCustomCondMain> *manager) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = manager->cond_custom_pro_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            for (auto &child : pt) {
                now_json_node = bef_now_json_node;
                std::string escape = now_json_node;
                std::replace(escape.begin(), escape.end(), '.', '@');
                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;
                escape = escape + "@" + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::CATEGORY) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesCategory(nested.get(), level, manager);

                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::BODY_SLOT) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesBodySlot(nested.get(), level, manager);

                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }

                } else if (k1 == KMCCCJsonTags::KEYWORD) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesKeyword(nested.get(), level, manager);

                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::CROSS_HAIR_REF_NAME) {
                    // header
                    path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
                    // mcm
                    path_mapping[project_name].emplace_back(KMCKEPath(
                        key_detail.Build<std::string>(level, escape, "GetName>", true, '@'), k1, '@', level, true));
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        for (auto it = nested.get().begin(); it != nested.get().end(); ++it) {
                            const std::string text = it->second.data();
                            manager->source.cross_hair_ref_name.emplace_back(text);
                            LOG(" cross_hair_ref_name = {}", text);
                        }
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::FORMULAS) {
                    // header
                    path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
                    // mcm
                    path_mapping[project_name].emplace_back(
                        KMCKEPath(key_detail.Build<std::string>(level, now_json_node, ""), k1, '/', level, true, true));
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        for (auto it = nested.get().begin(); it != nested.get().end(); ++it) {
                            const std::string text = it->second.data();
                            KMCFormula formula;
                            formula.cond = text;
                            manager->source.formula.emplace_back(formula);

                            // if (auto nested_form = it->second.get_child_optional("");
                            //     nested_form && !nested_form.get().empty()) {
                            //     SetupJsonNodesFormula(nested_form.get(), level, manager);
                            // } else {
                            //     ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                            // }
                        }
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }
    void KMCCutinCondition::SetupJsonNodesCategory(boost::property_tree::ptree pt, int level,
                                                   KMCCustomCondManager<KMCCustomCondMain> *manager) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = manager->cond_custom_pro_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            for (auto &child : pt) {
                now_json_node = bef_now_json_node;
                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::MAIN_CATEGORY) {
                    std::string re = NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                    std::transform(re.begin(), re.end(), re.begin(), ::tolower);
                    manager->source.main_category = re;
                } else if (k1 == KMCCCJsonTags::SUB_CATEGORY_1) {
                    std::string re = NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                    std::transform(re.begin(), re.end(), re.begin(), ::tolower);
                    manager->source.sub1_category = re;
                } else if (k1 == KMCCCJsonTags::SUB_CATEGORY_2) {
                    std::string re = NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                    std::transform(re.begin(), re.end(), re.begin(), ::tolower);
                    manager->source.sub2_category = re;
                } else if (k1 == KMCCCJsonTags::SUB_CATEGORY_3) {
                    std::string re = NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                    std::transform(re.begin(), re.end(), re.begin(), ::tolower);
                    manager->source.sub3_category = re;
                } else if (k1 == KMCCCJsonTags::SUB_CATEGORY_4) {
                    std::string re = NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                    std::transform(re.begin(), re.end(), re.begin(), ::tolower);
                    manager->source.sub4_category = re;
                } else if (k1 == KMCCCJsonTags::SUB_CATEGORY_5) {
                    std::string re = NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                    std::transform(re.begin(), re.end(), re.begin(), ::tolower);
                    manager->source.sub5_category = re;
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }
    void KMCCutinCondition::SetupJsonNodesBodySlot(boost::property_tree::ptree pt, int level,
                                                   KMCCustomCondManager<KMCCustomCondMain> *manager) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = manager->cond_custom_pro_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            for (auto &child : pt) {
                now_json_node = bef_now_json_node;
                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::IS_MATCH) {
                    manager->source.body_slot.match =
                        NodeTreesGetValue<int>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::HEAD) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::HAIR) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::BODY) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::HANDS) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::FOREARMS) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::AMULET) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::RING) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::FEET) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::CALVES) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::SHIELD) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::TAIL) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::LONGHAIR) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::CIRCLET) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::EARS) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::FACE_COVERING_OVER_THE_MOUTH) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::NECK_COVERING) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::CLOAK) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::BACKPACK) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::MAGIC_FX) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::SKIRT) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::DECAPITATEHEAD) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::DECAPITATE) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::GENITALS_OR_UNDERWEAR) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::PANTS) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::STOCKINGS) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::FACE_COVERING_USUALLY_UPPER_FACE) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::TORSO) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::SHOULDERS) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::SHIRT_OR_LEFT_ARM) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::LOOSE_SHIRT_OR_RIGHT_ARM) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::MISCELLANEOUS) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else if (k1 == KMCCCJsonTags::FX01) {
                    manager->source.body_slot.Set(
                        k1, NodeTreesGetValue<int>(child.second, level, now_json_node, project_name));
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesKeyword(boost::property_tree::ptree pt, int level,
                                                  KMCCustomCondManager<KMCCustomCondMain> *manager) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = manager->cond_custom_pro_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            for (auto &child : pt) {
                now_json_node = bef_now_json_node;
                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::KEYWORD_FORMID) {
                    manager->source.keyword_formid =
                        NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::KEYWORD_PLUGIN_NAME) {
                    manager->source.keyword_plugin_name =
                        NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::TEMP_KEYWORD_NAME) {
                    manager->source.temp_keyword_name =
                        NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesFormula(boost::property_tree::ptree pt, int level,
                                                  KMCCustomCondManager<KMCCustomCondMain> *manager) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = manager->cond_custom_pro_name;
            // header
            // path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            KMCFormula formula;
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;
                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::FORMULAS_NOT_EQUAL) {
                    formula.not_equal = NodeTreesGetValue<int>(child.second, level) == 1;
                } else if (k1 == KMCCCJsonTags::FORMULAS_COND) {
                    formula.cond = NodeTreesGetValue<std::string>(child.second, level);
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }

            manager->source.formula.emplace_back(formula);
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesLevel2(boost::property_tree::ptree pt, int level,
                                                 KMCCustomCondManager<KMCCustomCondMain> *manager) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = manager->cond_custom_pro_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                auto second = child.second;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);
                if (auto nested = second.get_child_optional(""); nested && !nested.get().empty()) {
                    // condition detail配下をループする関数をコールする。
                    // if (boost::optional<int> disable = nested.get().get_optional<int>(KMCCCJsonTags::DISABLE)) {
                    //    if (disable == 1) {
                    //        LOG("Level {} key {} disable", level, now_json_node + "." + KMCCCJsonTags::DISABLE);
                    //        continue;
                    //    }
                    //} else {
                    //    LOG("Level {} key {} not found disable flag. enable", level,
                    //        now_json_node + "." + KMCCCJsonTags::DISABLE);
                    //}

                    manager->nodes.emplace_back(
                        std::make_unique<KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>>());
                    KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node =
                        manager->nodes.back().get();
                    node->manager = manager;
                    // チェックの処理方針を決定する
                    if (GetCheckTaskDetail(manager->source, &node->check_hub) == false) {
                        auto source = manager->source;
                        ERROR(
                            "Level {} key {} Check processing cannot be performed. Incorrect combination. "
                            "main_category {} sub1 {} sub2 {} sub3 {} sub4 {} sub5 {}",
                            level, now_json_node, source.main_category, source.sub1_category, source.sub2_category,
                            source.sub3_category, source.sub4_category, source.sub5_category);
                        manager->disable = true;
                        throw std::exception();
                    }

                    if (!node->check_hub->Init()) {
                        auto source = manager->source;
                        ERROR(
                            "Level {} key {} Check process cannot continue. Initialization failure. "
                            "main_category {} sub1 {} sub2 {} sub3 {} sub4 {} sub5 {}",
                            level, now_json_node, source.main_category, source.sub1_category, source.sub2_category,
                            source.sub3_category, source.sub4_category, source.sub5_category);
                        manager->disable = true;
                        throw std::exception();
                    }
                    node->cond_custom_node_name = k1;
                    node->project_name = manager->cond_custom_pro_name + "." + node->cond_custom_node_name;
                    node->push_key = std::to_string(manager->priority) + "." + node->project_name;

                    SetupJsonNodesLevel3(nested.get(), level, manager, node);

                    if (node->cutin_cond_type_sub == CutinCondSubType::keyword) {
                        node->push_type = PushType::keyword;
                    } else {
                        node->push_type = PushType::cutin;
                    }

                    if (node->sub_task_hub) {
                        node->sub_task_hub->Init();
                    }
                    if (!node->task_hub) {
                        // dummyの場合はまずカットインなしか判定する
                        if (node->not_cutin && node->force_exp_timing == 2) {
                            // dummyの計算処理方針を設定する
                            node->cutin_cond_type = CutinCondType::dummy;

                            if (GetWorkDetail(node->cutin_cond_type, &node->task_hub) == false) {
                                ERROR(
                                    "Level {} key {} dummy work not found. wt?"
                                    "cond_type {} ",
                                    level, now_json_node, static_cast<int>(node->cutin_cond_type));
                                node->disable = true;
                                throw std::exception();
                            }
                        } else {
                            // 定義なしの場合はエラー
                            ERROR(
                                "Level {} key {} For no task node, define not_cutin as 1 and force_exp_timing as 2"
                                "cond_type {} ",
                                level, now_json_node, static_cast<int>(node->cutin_cond_type));
                            node->disable = true;
                            throw std::exception();
                        }
                    }
                    if (!node->disable) {
                        if (!node->task_hub->Init(node->check_hub.get(), &manager->source)) {
                            ERROR(
                                "Level {} key {} Node initialization failed. "
                                "cond_type {} ",
                                level, now_json_node, static_cast<int>(node->cutin_cond_type));
                            node->disable = true;
                            throw std::exception();
                        }
                    }
                    node->post_commit_push_key = std::to_string(node->priority) + "." + node->project_name;

                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesLevel3(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::OPTION) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesOption(nested.get(), level, manager, node);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::OPERATION) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesOperation(nested.get(), level, manager, node);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::TYPE_ADD || k1 == KMCCCJsonTags::TYPE_TIME ||
                           k1 == KMCCCJsonTags::TYPE_AMOUNT) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        if (k1 == KMCCCJsonTags::TYPE_ADD) {
                            node->cutin_cond_type = CutinCondType::add;
                        } else if (k1 == KMCCCJsonTags::TYPE_TIME) {
                            node->cutin_cond_type = CutinCondType::time;
                        } else if (k1 == KMCCCJsonTags::TYPE_AMOUNT) {
                            node->cutin_cond_type = CutinCondType::amount;
                        }

                        // チェック後の計算処理方針を決定する
                        if (GetWorkDetail(node->cutin_cond_type, &node->task_hub) == false) {
                            ERROR(
                                "Level {} key {} Work processing cannot be performed. Incorrect combination. "
                                "cond_type {} ",
                                level, now_json_node, static_cast<int>(node->cutin_cond_type));
                            node->disable = true;
                            throw std::exception();
                        }

                        SetupJsonNodesTask(nested.get(), level, &node->task_hub, project_name);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::NODE_RELATIONS) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesRelations(nested.get(), level, manager, node);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }

                } else if (k1 == KMCCCJsonTags::POLLING) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesPolling(nested.get(), level, manager, node);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::CUTIN_SETTING) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesCutinSetting(nested.get(), level, manager, node);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesOperation(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::OPERATION_DETAIL) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesOPDetail(nested.get(), level, manager, node);

                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::CYCLE) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesCycle(nested.get(), level, manager, node);

                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::FORCE_CT) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesForceCT(nested.get(), level, manager, node);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::FORCE_EXPRESSION) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        SetupJsonNodesForceEXP(nested.get(), level, manager, node);
                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else if (k1 == KMCCCJsonTags::PUSH_TEMP_KEYWORDS) {
                    if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                        if (k1 == KMCCCJsonTags::PUSH_TEMP_KEYWORDS) {
                            node->cutin_cond_type_sub = CutinCondSubType::keyword;
                        }

                        // チェック後のsub taskを決定する
                        if (GetCheckSubTaskDetail(node->cutin_cond_type_sub, &node->sub_task_hub) == false) {
                            ERROR(
                                "Level {} key {} Work(sub) processing cannot be performed. Incorrect combination. "
                                "cond_type {} ",
                                level, now_json_node, static_cast<int>(node->cutin_cond_type_sub));
                            node->disable = true;
                            throw std::exception();
                        }

                        SetupJsonNodesKeyword(nested.get(), level, &node->sub_task_hub, project_name);

                    } else {
                        ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                    }
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesOPDetail(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::OP_NOT_CUTIN) {
                    node->not_cutin = NodeTreesGetValue<int>(child.second, level, now_json_node, project_name) == 1;
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesCycle(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::ONCE) {
                    node->once = NodeTreesGetValue<int>(child.second, level, now_json_node, project_name) == 1;
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }
    void KMCCutinCondition::SetupJsonNodesForceCT(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::FORCE_CUTIN) {
                    node->force_cutin = NodeTreesGetValue<int>(child.second, level, now_json_node, project_name) == 1;
                } else if (k1 == KMCCCJsonTags::CUTIN_NAME) {
                    node->force_cutin_name =
                        NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesForceEXP(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::EXP_ID) {
                    node->force_exp_name =
                        NodeTreesGetValue<std::string>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::EXPRESSION_TIME) {
                    node->force_expression_time =
                        NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::FORCE_EXP) {
                    node->force_exp_timing = NodeTreesGetValue<int>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::EXP_COOL_TIME) {
                    node->force_expression_cool_time =
                        NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::STOP_PERCENTAGE) {
                    node->stop_percentage = NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesTask(boost::property_tree::ptree pt, int level,
                                               std::unique_ptr<KMCCustomCondTaskHub> *task, std::string proj_name) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            // header
            path_mapping[proj_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::ADD_VALUE) {
                    (*task)->add_value = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::COEF_1) {
                    (*task)->coef_1 = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::COEF_2) {
                    (*task)->coef_2 = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::COEF_RELATION) {
                    std::string re = NodeTreesGetValue<std::string>(child.second, level, now_json_node, proj_name);

                    std::transform(re.begin(), re.end(), re.begin(), ::tolower);
                    (*task)->coef_relation = re;
                } else if (k1 == KMCCCJsonTags::COOL_TIME) {
                    (*task)->cool_time = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::UPPER_VALUE) {
                    (*task)->upper_value = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::LOWER_VALUE) {
                    (*task)->lower_value = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::END_TIME) {
                    (*task)->end_time = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::START_TIME) {
                    (*task)->start_time = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::STACK_LIMIT) {
                    (*task)->stack_limit = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::TARGET_AMOUNT) {
                    (*task)->target_amount = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::ABANDON_AMOUNT) {
                    (*task)->abandon_amount = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::STACK_COOL_TIME) {
                    (*task)->cool_time = NodeTreesGetValue<float>(child.second, level, now_json_node, proj_name);
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesKeyword(boost::property_tree::ptree pt, int level,
                                                  std::unique_ptr<KMCCustomCondSubTaskHub> *sub_task,
                                                  std::string proj_name) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            // header
            path_mapping[proj_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::PUSH_TEMP_KEYWORD_NAME) {
                    (*sub_task)->sub_task_source.keyword_name =
                        NodeTreesGetValue<std::string>(child.second, level, now_json_node, proj_name);
                } else if (k1 == KMCCCJsonTags::PUSH_KEYWORD_CATEGORY) {
                    (*sub_task)->sub_task_source.category =
                        NodeTreesGetValue<std::string>(child.second, level, now_json_node, proj_name);
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesRelations(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));
            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                std::string escape = now_json_node;
                std::replace(escape.begin(), escape.end(), '.', '@');
                now_json_node = now_json_node + "." + k1;
                escape = escape + "@" + k1;
                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);
                STNodeRelations subt;
                std::string nbef_now_json_node = now_json_node;
                if (auto nested = child.second.get_child_optional(""); nested && !nested.get().empty()) {
                    // header

                    path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(escape), k1, '@', level));

                    for (auto &elements : nested.get()) {
                        int nlevel = level + 1;
                        now_json_node = nbef_now_json_node;

                        std::string k2 = elements.first;
                        std::string escape2 = escape + "@" + k2;
                        now_json_node = now_json_node + "." + k2;

                        if (k2 == KMCCCJsonTags::CALC_MATH) {
                            std::string nnbef_now_json_node = now_json_node;

                            if (auto nested_calc_detail = elements.second.get_child_optional("");
                                nested_calc_detail && !nested_calc_detail.get().empty()) {
                                for (auto &calc_detail : nested_calc_detail.get()) {
                                    int ncdlevel = level + 1;
                                    now_json_node = nnbef_now_json_node;

                                    std::string k3 = calc_detail.first;
                                    std::string escape3 = escape2 + "@" + k3;
                                    now_json_node = now_json_node + "." + k3;

                                    if (k3 == KMCCCJsonTags::CALC_ADD_VALUE) {
                                        subt.add_value = NodeTreesGetValue<float>(calc_detail.second, ncdlevel, escape3,
                                                                                  project_name, true, '@', k1);
                                        subt.calc = true;
                                    } else if (k3 == KMCCCJsonTags::CALC_SUBTRACT_VALUE) {
                                        subt.subtract_value = NodeTreesGetValue<float>(
                                            calc_detail.second, ncdlevel, escape3, project_name, true, '@', k1);
                                        subt.calc = true;
                                    } else if (k3 == KMCCCJsonTags::CALC_DIV_VALUE) {
                                        subt.div_value = NodeTreesGetValue<float>(calc_detail.second, ncdlevel, escape3,
                                                                                  project_name, true, '@', k1);
                                        subt.calc = true;
                                    } else if (k3 == KMCCCJsonTags::CALC_MULT_VALUE) {
                                        subt.mult_value = NodeTreesGetValue<float>(
                                            calc_detail.second, ncdlevel, escape3, project_name, true, '@', k1);
                                        subt.calc = true;
                                    } else {
                                        ERROR("Level {} key {} It's not in the format.", ncdlevel, now_json_node);
                                    }
                                }
                            } else {
                                ERROR("Level {} key {} It's not in the format.", nlevel, now_json_node);
                            }
                        } else if (k2 == KMCCCJsonTags::TARGET_NODE) {
                            std::string tnode_name = NodeTreesGetValue<std::string>(elements.second, nlevel, escape2,
                                                                                    project_name, true, '@', k1);
                            subt.target_name = k1 + "." + tnode_name;
                            subt.escape_name = k1 + "@" + tnode_name;
                            subt.key_name = k1;
                        } else if (k2 == KMCCCJsonTags::CALC_TIMING) {
                            subt.timing =
                                NodeTreesGetValue<int>(elements.second, nlevel, escape2, project_name, true, '@', k1);
                        } else {
                            ERROR("Level {} key {} It's not in the format.", nlevel, now_json_node);
                        }
                    }
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }

                node->relations.emplace_back(subt);
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesPolling(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::STAY_TIME) {
                    node->polling.stay_time =
                        NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } /* else if (k1 == KMCCCJsonTags::PUT_ON_HOLD) {
                     node->polling.put_on_hold = NodeTreesGetValue<float>(child.second, level, false);
                 }*/
                else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesCutinSetting(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = node->project_name;
            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::TIME) {
                    node->cutin_setting.time =
                        NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::ANIM_TIME) {
                    node->cutin_setting.anim_time =
                        NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::VOLUME) {
                    node->cutin_setting.volume =
                        NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::OAR_TIME) {
                    node->cutin_setting.oar_time =
                        NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::EXP_TIME) {
                    node->cutin_setting.exp_time =
                        NodeTreesGetValue<float>(child.second, level, now_json_node, project_name);
                } else if (k1 == KMCCCJsonTags::OVERRIDE_CI_SETTING) {
                    node->cutin_setting.override_setting =
                        NodeTreesGetValue<int>(child.second, level, now_json_node, project_name) == 1;
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    void KMCCutinCondition::SetupJsonNodesOption(
        boost::property_tree::ptree pt, int level, KMCCustomCondManager<KMCCustomCondMain> *manager,
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string bef_now_json_node = now_json_node;
        level = level + 1;
        try {
            std::string project_name = "";
            if (node) {
                project_name = node->project_name;
            } else {
                project_name = manager->cond_custom_pro_name;
            }

            // header
            path_mapping[project_name].emplace_back(KMCKEPath(key_detail.Build(now_json_node), level));

            for (auto &child : pt) {
                now_json_node = bef_now_json_node;

                std::string k1 = child.first;
                now_json_node = now_json_node + "." + k1;

                LOG("Level {} key {} <-----[IN PROCESS]", level, now_json_node);

                if (k1 == KMCCCJsonTags::DISABLE) {
                    if (node) {
                        node->disable = NodeTreesGetValue<int>(child.second, level, now_json_node, project_name) == 1;
                    } else {
                        manager->disable =
                            NodeTreesGetValue<int>(child.second, level, now_json_node, project_name) == 1;
                    }

                } else if (k1 == KMCCCJsonTags::PRIORITY) {
                    if (node) {
                        node->priority = NodeTreesGetValue<int>(child.second, level, now_json_node, project_name);
                    } else {
                        manager->priority = NodeTreesGetValue<int>(child.second, level, now_json_node, project_name);
                    }
                } else {
                    ERROR("Level {} key {} It's not in the format.", level, now_json_node);
                }
            }
        } catch (std::exception ex) {
            ERROR("Level {} key {} --------------LoadError.-------------- wt{}", level, now_json_node, ex.what());
        }

        now_json_node = bef_now_json_node;
    }

    template <typename T1>
    T1 KMCCutinCondition::NodeTreesGetValue(boost::property_tree::ptree pt, int level, std::string path,
                                            std::string name, bool escape, char escp_c, std::string escp_p) {
        if (boost::optional<T1> value = pt.get_value_optional<T1>()) {
            T1 checked_value = value.get();

            path_mapping[name].emplace_back(
                KMCKEPath(key_detail.Build(level, path, checked_value, escape, escp_c), escp_p, escp_c, level));

            LOG("Level {} Key {} target value {} type {}.", level, now_json_node, checked_value, typeid(T1).name());
            return checked_value;
            // boost::any any = checked_value;
            // bool check_result = false;

            // if (any.type() == typeid(std::string)) {
            //     std::string t = boost::any_cast<std::string>(any);
            //     check_result = v.validator(validate_target_key, t, must);

            //    LOG("Level {} Key {} Validate target value {}.", level, now_json_node, t);
            //} else if(any.type() == typeid(int)) {
            //    int t = boost::any_cast<int>(any);
            //    check_result = v.validator(validate_target_key, std::to_string(t), must);
            //    LOG("Level {} Key {} Validate target value {}.", level, now_json_node, t);
            //} else if (any.type() == typeid(float)) {
            //    float t = boost::any_cast<float>(any);
            //    check_result = v.validator(validate_target_key, std::to_string(t), must);
            //    LOG("Level {} Key {} Validate target value {}.", level, now_json_node, t);
            //} else if (any.type() == typeid(double)) {
            //    double t = boost::any_cast<double>(any);
            //    check_result = v.validator(validate_target_key, std::to_string(t), must);
            //    LOG("Level {} Key {} Validate target value {}.", level, now_json_node, t);
            //} else if (any.type() == typeid(bool)) {
            //    bool t = boost::any_cast<bool>(any);
            //    if (t) {
            //        check_result = v.validator(validate_target_key, "1", must);
            //    } else {
            //        check_result = v.validator(validate_target_key, "0", must);
            //    }
            //    LOG("Level {} Key {} Validate target value {}.", level, now_json_node, t);
            //}

            // if (check_result) {
            //     return checked_value;
            // } else {
            //     if (must) {
            //         ERROR("Level {} Key {} Required, please enter a value.", level, now_json_node);
            //         throw std::exception();
            //     } else {
            //         LOG("Level {} Key {} validate check error. return default value.", level, now_json_node);
            //         return default_value;
            //     }
            // }
        } else {
            ERROR("Level {} Key {} It's not in the format.", level, now_json_node);
            throw std::exception();
        }
    }

    template <typename T1>
    T1 KMCCutinCondition::NodeTreesGetValue(boost::property_tree::ptree pt, int level) {
        if (boost::optional<T1> value = pt.get_value_optional<T1>()) {
            T1 checked_value = value.get();

            LOG("Level {} Key {} target value {} type {}.", level, now_json_node, checked_value, typeid(T1).name());
            return checked_value;
        } else {
            ERROR("Level {} Key {} It's not in the format.", level, now_json_node);
            throw std::exception();
        }
    }

    bool KMCCutinCondition::Validate_manager(KMCCustomCondManager<KMCCustomCondMain> *manager) {
        KMCCCheckSource source = manager->source;
        std::string validate_value_category =
            source.main_category + "/" + source.sub1_category + "/" + source.sub2_category + "/" +
            source.sub3_category + "/" + source.sub4_category + "/" + source.sub5_category + "/" +
            source.keyword_formid + "/" + source.keyword_plugin_name + "/" + source.temp_keyword_name;
        std::string message;
        if (!v.validator(KMCCCJsonTags::MAIN_CATEGORY, validate_value_category, true, message)) {
            // main categoryとの組み合わせがおかしい
            ERROR(" --------------Validate Error.-------------- wt : [MAIN_CATEGORY] {}", message);
            return false;
        }

        return true;
    }

    bool KMCCutinCondition::Validate_node(KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>> *node) {
        std::string message;
        if (node->force_exp_timing > 0) {
            //[06][force_exp]
            std::string validate_fexp = std::to_string(node->force_exp_timing) + "/" + node->force_exp_name + "/" +
                                        std::to_string(node->force_expression_cool_time) + "/" +
                                        std::to_string(node->force_expression_time) + "/" +
                                        std::to_string(node->stop_percentage);
            if (!v.validator(KMCCCJsonTags::FORCE_EXPRESSION, validate_fexp, true, message)) {
                // FORCE_EXPRESSIONの組み合わせがおかしい
                ERROR(" --------------Validate Error.-------------- wt : [FORCE_EXPRESSION] {}", message);
                return false;
            }

            if (message != "") {
                WARN(" --------------Validate WARN.-------------- wt : [FORCE_EXPRESSION] {}", message);
            }
        }

        if (node->cutin_cond_type == CutinCondType::add || node->cutin_cond_type == CutinCondType::time|| node->cutin_cond_type == CutinCondType::amount/* ||
            node->cutin_cond_type == CutinCondType::add_keyword ||
            node->cutin_cond_type == CutinCondType::remove_keyword*/) {
            // Taskノードのチェック
            if (node->cutin_cond_type == CutinCondType::add) {
                std::string validate_value_add =
                    std::to_string(node->task_hub->coef_1) + "/" + std::to_string(node->task_hub->coef_2) + "/" +
                    node->task_hub->coef_relation + "/" + std::to_string(node->task_hub->cool_time) + "/" +
                    std::to_string(node->task_hub->lower_value) + "/" + std::to_string(node->task_hub->upper_value);
                if (!v.validator(KMCCCJsonTags::TYPE_ADD, validate_value_add, true, message)) {
                    // type addの組み合わせがおかしい
                    ERROR(" --------------Validate Error.-------------- wt : [TYPE_ADD] {}", message);
                    return false;
                }
            } else if (node->cutin_cond_type == CutinCondType::time) {
                std::string validate_value_time =
                    std::to_string(node->task_hub->end_time) + "/" + std::to_string(node->task_hub->start_time);
                if (!v.validator(KMCCCJsonTags::TYPE_TIME, validate_value_time, true, message)) {
                    // type addの組み合わせがおかしい
                    ERROR(" --------------Validate Error.-------------- wt : [TYPE_TIME] {}", message);
                    return false;
                }
            } else if (node->cutin_cond_type == CutinCondType::amount) {
                std::string validate_value_amount = std::to_string(node->task_hub->abandon_amount) + "/" +
                                                    std::to_string(node->task_hub->target_amount) + "/" +
                                                    std::to_string(node->task_hub->stack_limit) + "/" +
                                                    std::to_string(node->task_hub->cool_time);
                if (!v.validator(KMCCCJsonTags::TYPE_AMOUNT, validate_value_amount, true, message)) {
                    // type addの組み合わせがおかしい
                    ERROR(" --------------Validate Error.-------------- wt : [TYPE_AMOUNT] {}", message);
                    return false;
                }
            } else {
                ERROR(" --------------Validate Error.-------------- wt : The type of type_? is unknown");
                return false;
            }

        } else if (node->cutin_cond_type == CutinCondType::dummy) {
            WARN("[Validate] This Node IsDummy {}", node->project_name);
        } else {
            // 製作者側用
            ERROR(" --------------Validate Error.-------------- wt : Unknown Error");
            return false;
        }

        if (node->cutin_cond_type_sub != CutinCondSubType::none) {
            if (node->cutin_cond_type_sub == CutinCondSubType::keyword) {
                if (node->sub_task_hub) {
                    auto ssource = node->sub_task_hub->sub_task_source;
                    std::string validate_ky = ssource.keyword_name + "/" + ssource.category;
                    if (!v.validator(KMCCCJsonTags::PUSH_TEMP_KEYWORDS, validate_ky, true, message)) {
                        // type add keywordの組み合わせがおかしい
                        ERROR(" --------------Validate Error.-------------- wt : [TYPE_TEMP_KEYWORD] {}", message);
                        return false;
                    }
                } else {
                    ERROR(" --------------Validate Error.-------------- wt : unknown error");
                    return false;
                }
            } else {
                ERROR(" --------------Validate Error.-------------- wt : The type of type_? is unknown");
                return false;
            }
        }

        for (auto &sub_node : node->relations) {
            if (sub_node.target_name != "") {
                // NODE_RELATIONSノードがある場合
                std::string validate_value_subtract = std::to_string(sub_node.timing) + "/" + "";
                if (!v.validator(KMCCCJsonTags::NODE_RELATIONS, validate_value_subtract, true, message)) {
                    // subtractのtimingが1 or 0ではない
                    ERROR(" --------------Validate Error.-------------- wt : [SUBTRACT] {}", message);
                    return false;
                }
            }
        }

        if (node->polling.stay_time != 0.0f /*|| node->polling.put_on_hold != 0.0f*/) {
            // pollingノードがある場合
            std::string validate_value_polling =
                std::to_string(node->polling.stay_time) + "/" + "" /*std::to_string(node->polling.put_on_hold)*/;
            if (!v.validator(KMCCCJsonTags::POLLING, validate_value_polling, true, message)) {
                // subtractのtimingが1 or 0ではない
                ERROR(" --------------Validate Error.-------------- wt : [POLLING] {}", message);
                return false;
            }
        }

        auto cutin_setting = node->cutin_setting;
        if (cutin_setting.time != 0.0f || cutin_setting.anim_time != 0.0f || cutin_setting.volume != 0.0f) {
            // cutin_settingノードがある場合
            std::string validate_value_cutin_setting =
                std::to_string(cutin_setting.time) + "/" + std::to_string(cutin_setting.anim_time) + "/" +
                std::to_string(cutin_setting.volume) + "/" + std::to_string(cutin_setting.oar_time) + "/" +
                std::to_string(cutin_setting.exp_time);
            if (!v.validator(KMCCCJsonTags::CUTIN_SETTING, validate_value_cutin_setting, true, message)) {
                // subtractのtimingが1 or 0ではない
                ERROR(" --------------Validate Error.-------------- wt : [CUTIN_SETTING] {}", message);
                return false;
            }
        }

        return true;
    }
}