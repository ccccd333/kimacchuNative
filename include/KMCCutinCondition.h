#pragma once
#include "KMCUtility.h"
#include "KMCCutinCond/KMCWorks.h"
#include "KMCCutinCond/KMCChecks.h"
#include "KMCCutinCond/KMCCheckSource.h"
#include "KMCCutinCond/KMCValidator.h"
#include "KMCCutinCond/KMCSubTask.h"
#include "KMCCutinCond/KMCCKeyDetail.h"
#include "KMCCutin.h"

namespace KMCCT {

    const std::string CUT_IN_CONDITION_FILE_NAME = "Condition.json";

    enum class ManageType {
        keyword,
        other
    };

    enum class PushType { cutin, keyword, none };

    template <typename TMane>
    class KMCCustomCondWorkerNode {
    public:
        KMCCustomCondWorkerNode() {}

        void Stop();
        void Reset();
        bool Check(KMCCCheckSource m_source);
        bool PushTask();
        int Polling();
        void Update(const KMCKDElement &elem);

        struct RelationsData {
        public:
            RelationsData(KMCCustomCondWorkerNode<TMane>* n, STNodeRelations rel) { 
                rnode = n;
                relations = rel;
            }
            KMCCustomCondWorkerNode<TMane>* rnode;
            STNodeRelations relations;
        };

        // 自ノードを他ノードからアクセスするポインタ
        KMCCustomCondWorkerNode* GetThisNode() { return this; }

        void Tuning(KMCCutinValues &target);

    public:
        TMane* manager;
        CutinCondType cutin_cond_type = CutinCondType::none;
        CutinCondSubType cutin_cond_type_sub = CutinCondSubType::none;

        // push type
        PushType push_type = PushType::none;

        std::vector<STNodeRelations> relations;
        STPolling polling = STPolling();
        STCutinSetting cutin_setting = STCutinSetting();
        std::unique_ptr<KMCCustomCondTaskHub> task_hub;
        std::unique_ptr<KMCCustomCondSubTaskHub> sub_task_hub;
        std::unique_ptr<KMCCustomCondCheckHub> check_hub;

        bool check_ok = false;

        // Operation

        // op detail
        bool not_cutin = false;
        
        // force_ct
        bool force_cutin = false;
        std::string force_cutin_name = "";

        // force_exp
        std::string force_exp_name = "";
        float force_expression_time = 1.0f;
        float force_expression_cool_time = 1.0f;
        int force_exp_timing = -1;
        float stop_percentage = 0.0f;

        std::vector<RelationsData> node_relations;
        // タスク完了時にmanagerへpushする際の優先度(post commit時使用)
        int priority = 0;
        // 状態が続く限り一度しか完了報告出さないフラグ
        bool once = false;
        bool push_end = false;
        bool once_and_pop_out_end = false;
        bool disable = false;

        std::string cond_custom_node_name = "";
        std::string project_name = "";
        std::string push_key = "";
        std::string post_commit_push_key = "";
    };

    template <typename TMain>
    class KMCCustomCondManager {
    public:
        // nodeのマネージャ。
        KMCCustomCondManager() {}
        void Instruct();
        void Update(const KMCKDElement& elem);
        //bool Push(KMCCustomCondWorkerNode<KMCCustomCondManager>* node);

        //struct PushedCompTask {
        //public:
        //    PushedCompTask() {}
        //    PushedCompTask(KMCCustomCondWorkerNode<KMCCustomCondManager>* n, time_point<Clock> t) { 
        //        node = n;
        //        timer = t;
        //    }
        //    KMCCustomCondWorkerNode<KMCCustomCondManager>* node;
        //    time_point<Clock> timer;
        //};

    public:
        TMain* main;

        ManageType m_type = ManageType::other;

        std::vector<std::unique_ptr<KMCCustomCondWorkerNode<KMCCustomCondManager>>> nodes;
        // 資材管理
        KMCCCheckSource source;

        // タスク完了時にmainへpushする際の優先度
        // 他より優先度が低ければpolling
        int priority = 0;

        // pushされたノード
        //std::vector<std::pair<std::string, PushedCompTask>> comp_tasks;

        // コンディションのカスタムプロジェクト名
        std::string cond_custom_pro_name = "";

        bool disable = false;
       
    };

    class KMCCustomCondMain {
    public:
        KMCCustomCondMain() {}
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* Try2Get();
        bool TryPopOut(std::string pop_key);
        bool TryPush(std::string push_key, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        bool Contains(std::string find_key);
        
        bool TryPushExp(std::string push_key, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        bool TryReleaseExp();

        void PlayFcExp();

        void PreCommit();
        void Commit();
        void Post();
    public:
        std::vector<std::unique_ptr<KMCCustomCondManager<KMCCustomCondMain>>> managers;
        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>*> pushed_task;
        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>*> pushed_exp_task;

        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>*> checked_nodes;
        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>*> pre_commit_nodes;
        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>*> pre_completed_nodes;
        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>*> completed_nodes;

        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>*> post_nodes;        
    };

    class KMCCutinCondition {
        SINGLETONHEADER(KMCCutinCondition)
    public:
        ~KMCCutinCondition(){};
        void Setup();
        void Init();
        int ToMove();
        KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* GetTaskResult();
        void Completed(KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        bool Contains(std::string project_name);
        void SetMCMUpdate(KMCKDElement elem);
        
        void ResetAll();

        std::vector<std::string> GetCutinCondition();
        std::vector<std::string> GetCutinConditionNode(std::string root_name);
        int GetPathNumber(std::string root_name, std::string work_name);
        void SetIDContainer(std::string root_name, std::string work_name, int index, int option);
        std::vector<std::string> GetMArray(std::string root_name, int index);
        std::vector<std::string> GetNArray(std::string root_name, std::string work_name, int index);

        std::vector<std::string> SearchID(std::string root_name, std::string work_name, int option);

        int SetResultMCM(std::string root_name, std::string work_name, int option, std::string result_value);
        bool IsUpdateMCM();
        std::string SaveKMCMCM();
    private:
        void Update();
        void PreProcess();
        void StartEvaluation();
        void Commit();

        void SetupJsonNodes(std::string jsonFileName);
        void SetupJsonNodesLevel1(boost::property_tree::ptree pt, int level,
                                  KMCCustomCondManager<KMCCustomCondMain>* manager);

        void SetupJsonNodesMain(boost::property_tree::ptree pt, int level,
                                  KMCCustomCondManager<KMCCustomCondMain>* manager);
        void SetupJsonNodesCategory(boost::property_tree::ptree pt, int level,
                                    KMCCustomCondManager<KMCCustomCondMain>* manager);
        void SetupJsonNodesKeyword(boost::property_tree::ptree pt, int level,
                                    KMCCustomCondManager<KMCCustomCondMain>* manager);
        void SetupJsonNodesLevel2(boost::property_tree::ptree pt, int level,
                                  KMCCustomCondManager<KMCCustomCondMain>* manager);
        void SetupJsonNodesLevel3(boost::property_tree::ptree pt, int level,
                                  KMCCustomCondManager<KMCCustomCondMain>* manager,
                                  KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        void SetupJsonNodesOperation(boost::property_tree::ptree pt, int level,
                                  KMCCustomCondManager<KMCCustomCondMain>* manager,
                                  KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        void SetupJsonNodesOPDetail(boost::property_tree::ptree pt, int level,
                                 KMCCustomCondManager<KMCCustomCondMain>* manager,
                                 KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        void SetupJsonNodesCycle(boost::property_tree::ptree pt, int level,
                                     KMCCustomCondManager<KMCCustomCondMain>* manager,
                                     KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        void SetupJsonNodesForceCT(boost::property_tree::ptree pt, int level,
                                 KMCCustomCondManager<KMCCustomCondMain>* manager,
                                 KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        void SetupJsonNodesForceEXP(boost::property_tree::ptree pt, int level,
                                   KMCCustomCondManager<KMCCustomCondMain>* manager,
                                   KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        void SetupJsonNodesTask(boost::property_tree::ptree pt, int level, std::unique_ptr<KMCCustomCondTaskHub>* task,
                                std::string proj_name);
        void SetupJsonNodesKeyword(boost::property_tree::ptree pt, int level,
                                   std::unique_ptr<KMCCustomCondSubTaskHub>* sub_task,
                                std::string proj_name);

        void SetupJsonNodesRelations(boost::property_tree::ptree pt, int level,
                                  KMCCustomCondManager<KMCCustomCondMain>* manager,
                                  KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        void SetupJsonNodesPolling(boost::property_tree::ptree pt, int level,
                                    KMCCustomCondManager<KMCCustomCondMain>* manager,
                                    KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);
        void SetupJsonNodesCutinSetting(boost::property_tree::ptree pt, int level,
                                   KMCCustomCondManager<KMCCustomCondMain>* manager,
                                   KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);

        void SetupJsonNodesOption(boost::property_tree::ptree pt, int level,
                                  KMCCustomCondManager<KMCCustomCondMain>* manager,
                                  KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);

        template <typename T1>
        T1 NodeTreesGetValue(boost::property_tree::ptree pt, int level, std::string path, std::string name,
                             bool escape = false, char escp_c = '@', std::string escp_p = "");

        bool Validate_manager(KMCCustomCondManager<KMCCustomCondMain>* manager);
        bool Validate_node(KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>* node);

    private:
        KMCCustomCondMain custom_cond;
        std::map<std::string, KMCCustomCondWorkerNode<KMCCustomCondManager<KMCCustomCondMain>>*> work_nodes;
        std::map<std::string, KMCCustomCondManager<KMCCustomCondMain>*> work_managers;
        std::map<std::string, std::vector<std::string>> name_mapping;
        std::map<std::string, std::vector<KMCKEPath>> path_mapping;
        std::vector<int> temp_container;
        KMCCKeyDetail key_detail;
        KMCValidator v;
        std::string now_json_node = "";
        //KMCCCJsonDefaultValues default_value;

        // -----MCM
        std::map<std::string, std::vector<KMCKDElement>> element;
        std::map<std::string, std::vector<KMCKDOption>> ids;
        std::vector<std::string> marray;

        std::vector<std::string> narray;

        std::vector<std::string> oparray;

        std::map<std::string, KMCKDElement> result_array;

        // MCM update
        std::vector<KMCKDElement> mcm_update;
    };
}