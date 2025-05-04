#pragma once
namespace KMCCT {
    class KMCCCJsonTags {
    public:
        static constexpr const char* DISABLE = "disable";
        static constexpr const char* PRIORITY = "priority";
        static constexpr const char* DESCRIPTION = "description";

        static constexpr const char* OPTION = "[000][option]";

        static constexpr const char* MAIN = "[01][main]";
        static constexpr const char* CATEGORY = "[010][category]";
        static constexpr const char* MAIN_CATEGORY = "main_category";
        static constexpr const char* SUB_CATEGORY_1 = "sub_category_1";
        static constexpr const char* SUB_CATEGORY_2 = "sub_category_2";
        static constexpr const char* SUB_CATEGORY_3 = "sub_category_3";
        static constexpr const char* SUB_CATEGORY_4 = "sub_category_4";
        static constexpr const char* SUB_CATEGORY_5 = "sub_category_5";
        static constexpr const char* KEYWORD = "[012][keyword]";
        static constexpr const char* KEYWORD_FORMID = "keyword_formid";
        static constexpr const char* KEYWORD_PLUGIN_NAME = "keyword_plugin_name";
        static constexpr const char* TEMP_KEYWORD_NAME = "temp_keyword_name";
        static constexpr const char* CROSS_HAIR_REF_NAME = "[011][cross_hair_ref_name]";

        static constexpr const char* CONDITION = "[02][condition]";

        static constexpr const char* OPERATION = "[03][operation]";
        static constexpr const char* CYCLE = "[04][cycle]";
        static constexpr const char* ONCE = "once";
        static constexpr const char* FORCE_CT = "[05][force_ct]";
        static constexpr const char* FORCE_CUTIN = "force_cutin";
        static constexpr const char* CUTIN_NAME = "cutin_name";

        static constexpr const char* TYPE_ADD = "[10][type_add]";
        static constexpr const char* ADD_VALUE = "add_value";
        static constexpr const char* COEF_1 = "coef_1";
        static constexpr const char* COEF_2 = "coef_2";
        static constexpr const char* COEF_RELATION = "coef_relation";
        static constexpr const char* COOL_TIME = "cool_time";
        static constexpr const char* UPPER_VALUE = "upper_value";
        static constexpr const char* LOWER_VALUE = "lower_value";

        static constexpr const char* TYPE_TIME = "[11][type_time]";
        static constexpr const char* END_TIME = "end_time";
        static constexpr const char* START_TIME = "start_time";

        // type type_amount
        static constexpr const char* TYPE_AMOUNT = "[12][type_amount]";
        static constexpr const char* STACK_LIMIT = "stack_limit";
        static constexpr const char* TARGET_AMOUNT = "target_amount";
        static constexpr const char* ABANDON_AMOUNT = "abandon_amount";
        static constexpr const char* STACK_COOL_TIME = "stack_cool_time";

        static constexpr const char* TYPE_TEMP_KEYWORDS = "[13][type_temp_keywords]";
        static constexpr const char* TYPE_TEMP_KEYWORD_NAME = "keyword_name";
        static constexpr const char* TYPE_KEYWORD_CATEGORY = "category";

        static constexpr const char* NODE_RELATIONS = "[30][node_relations]";
        //static constexpr const char* SUBTRACT_VALUE = "subtract_value";
        static constexpr const char* CALC_MATH = "[31][calc_math]";
        
        static constexpr const char* CALC_ADD_VALUE = "add_value";
        static constexpr const char* CALC_SUBTRACT_VALUE = "subtract_value";
        static constexpr const char* CALC_DIV_VALUE = "div_value";
        static constexpr const char* CALC_MULT_VALUE = "mult_value";

        static constexpr const char* TARGET_NODE = "target_node";
        static constexpr const char* CALC_TIMING = "calc_timing";

        static constexpr const char* POLLING = "[40][polling]";
        static constexpr const char* STAY_TIME = "stay_time";

        static constexpr const char* CUTIN_SETTING = "[50][cutin_setting]";
        static constexpr const char* TIME = "time";
        static constexpr const char* ANIM_TIME = "anim_time";
        static constexpr const char* VOLUME = "volume";
        static constexpr const char* OAR_TIME = "oar_time";


        //static constexpr const char* TYPE_REMOVE_KEYWORD = "type_remove_keyword";
        static constexpr const char* OVERRIDE_CI_SETTING = "override_ci_setting";
    };

    class KMCCLevelTags {
    public:
        static constexpr const char* T_DISABLE = "3.disable";
        static constexpr const char* T_PRIORITY = "3.priority";

        static constexpr const char* T_MAIN_CATEGORY = "3.main_category";
        static constexpr const char* T_SUB_CATEGORY_1 = "3.sub_category_1";
        static constexpr const char* T_SUB_CATEGORY_2 = "3.sub_category_2";
        static constexpr const char* T_SUB_CATEGORY_3 = "3.sub_category_3";
        static constexpr const char* T_SUB_CATEGORY_4 = "3.sub_category_4";
        static constexpr const char* T_SUB_CATEGORY_5 = "3.sub_category_5";
        static constexpr const char* T_KEYWORD_FORMID = "3.keyword_formid";
        static constexpr const char* T_KEYWORD_PLUGIN_NAME = "3.keyword_plugin_name";
        static constexpr const char* T_TEMP_KEYWORD_NAME = "3.temp_keyword_name";
        static constexpr const char* T_CROSS_HAIR_REF_NAME = "3.[011][cross_hair_ref_name]";

        static constexpr const char* F_DISABLE = "5.disable";
        static constexpr const char* F_PRIORITY = "5.priority";

        static constexpr const char* S_ONCE = "6.once";
        static constexpr const char* S_FORCE_CUTIN = "6.force_cutin";
        static constexpr const char* S_CUTIN_NAME = "6.cutin_name";

        // type add
        static constexpr const char* F_ADD_VALUE = "5.add_value";
        static constexpr const char* F_COEF_1 = "5.coef_1";
        static constexpr const char* F_COEF_2 = "5.coef_2";
        static constexpr const char* F_COEF_RELATION = "5.coef_relation";
        static constexpr const char* F_COOL_TIME = "5.cool_time";
        static constexpr const char* F_UPPER_VALUE = "5.upper_value";
        static constexpr const char* F_LOWER_VALUE = "5.lower_value";

        // type time
        static constexpr const char* F_END_TIME = "5.end_time";
        static constexpr const char* F_START_TIME = "5.start_time";

        // type keyword
        static constexpr const char* F_TYPE_TEMP_KEYWORD_NAME = "5.keyword_name";
        static constexpr const char* F_TYPE_KEYWORD_CATEGORY = "5.category";

        // type type_amount
        static constexpr const char* F_STACK_LIMIT = "5.stack_limit";
        static constexpr const char* F_TARGET_VALUE = "5.target_amount";
        static constexpr const char* F_ABANDON_AMOUNT = "5.abandon_amount";
        static constexpr const char* F_STACK_COOL_TIME = "5.stack_cool_time";

        static constexpr const char* S_CALC_ADD_VALUE = "6.add_value";
        static constexpr const char* S_CALC_SUBTRACT_VALUE = "6.subtract_value";
        static constexpr const char* S_CALC_DIV_VALUE = "6.div_value";
        static constexpr const char* S_CALC_MULT_VALUE = "6.mult_value";

        static constexpr const char* S_TARGET_NODE = "6.target_node";
        static constexpr const char* S_CALC_TIMING = "6.calc_timing";

        static constexpr const char* F_STAY_TIME = "5.stay_time";

        static constexpr const char* F_TIME = "5.time";
        static constexpr const char* F_ANIM_TIME = "5.anim_time";
        static constexpr const char* F_VOLUME = "5.volume";
        static constexpr const char* F_OAR_TIME = "5.oar_time";
        static constexpr const char* F_OVERRIDE_CI_SETTING = "5.override_ci_setting";
    };

}