#pragma once
#include <KMCCutinCond/KMCCategory.h>

#include "KMCCCJsonTags.h"
#include "KMCUtility.h"

namespace KMCCT {

    class KMCValidator {
    public:
        bool validator(std::string tag, std::string v_value, bool must, std::string &wt) {
            if (tag == KMCCCJsonTags::MAIN_CATEGORY) {
                return validate_main_category(v_value, must, wt);
            } else if (tag == KMCCCJsonTags::TYPE_ADD) {
                return validate_type_add(v_value, must, wt);
            } else if (tag == KMCCCJsonTags::TYPE_TIME) {
                return validate_type_time(v_value, must, wt);
            } else if (tag == KMCCCJsonTags::PUSH_TEMP_KEYWORDS) {
                return validate_push_temp_keywords(v_value, must, wt);
            } else if (tag == KMCCCJsonTags::TYPE_AMOUNT) {
                return validate_type_amount(v_value, must, wt);
            } else if (tag == KMCCCJsonTags::NODE_RELATIONS) {
                return validate_node_relations(v_value, must, wt);
            } else if (tag == KMCCCJsonTags::POLLING) {
                return validate_polling(v_value, must, wt);
            } else if (tag == KMCCCJsonTags::CUTIN_SETTING) {
                return validate_cutin_setting(v_value, must, wt);
            } else if (tag == KMCCCJsonTags::FORCE_EXPRESSION) {
                return validate_force_expression(v_value, must, wt);
            }

            // if (tag == KMCCCJsonTags::PRIORITY) {
            //     return validate_priority(v_value, must);
            // } else if (tag == KMCCCJsonTags::MAIN_CATEGORY) {
            //     return validate_main_category(v_value, must, wt);
            // } else if (tag == KMCCCJsonTags::SUB_CATEGORY_1) {
            //     return validate_sub_category_1(v_value, must);
            // } else if (tag == KMCCCJsonTags::SUB_CATEGORY_2) {
            //     return validate_sub_category_2(v_value, must);
            // } else if (tag == KMCCCJsonTags::SUB_CATEGORY_3) {
            //     return validate_sub_category_3(v_value, must);
            // } else if (tag == KMCCCJsonTags::SUB_CATEGORY_4) {
            //     return validate_sub_category_4(v_value, must);
            // } else if (tag == KMCCCJsonTags::SUB_CATEGORY_5) {
            //     return validate_sub_category_5(v_value, must);
            // } else if (tag == KMCCCJsonTags::KEYWORD_FORMID) {
            //     return validate_keyword_formid(v_value, must);
            // } else if (tag == KMCCCJsonTags::KEYWORD_PLUGIN_NAME) {
            //     return validate_keyword_plugin_name(v_value, must);
            // } else if (tag == KMCCCJsonTags::TYPE_ADD) {
            //     return validate_type_add(v_value, must);
            // } else if (tag == KMCCCJsonTags::TYPE_TIME) {
            //     return validate_type_time(v_value, must);
            // } else if (tag == KMCCCJsonTags::COEF_1) {
            //     return validate_coef_1(v_value, must);
            // } else if (tag == KMCCCJsonTags::COEF_2) {
            //     return validate_coef_2(v_value, must);
            // } else if (tag == KMCCCJsonTags::COEF_RELATION) {
            //     return validate_coef_relation(v_value, must);
            // } else if (tag == KMCCCJsonTags::COOL_TIME) {
            //     return validate_cool_time(v_value, must);
            // } else if (tag == KMCCCJsonTags::UPPER_VALUE) {
            //     return validate_upper_value(v_value, must);
            // } else if (tag == KMCCCJsonTags::LOWER_VALUE) {
            //     return validate_lower_value(v_value, must);
            // } else if (tag == KMCCCJsonTags::SUBTRACT_TIMING) {
            //     return validate_subtract_timing(v_value, must);
            // } else if (tag == KMCCCJsonTags::STAY_TIME) {
            //     return validate_polling(v_value, must);
            // } else if (tag == KMCCCJsonTags::PUT_ON_HOLD) {
            //     return validate_put_on_hold(v_value, must);
            // } else if (tag == KMCCCJsonTags::TIME) {
            //     return validate_time(v_value, must);
            // } else if (tag == KMCCCJsonTags::ANIM_TIME) {
            //     return validate_anim_time(v_value, must);
            // } else if (tag == KMCCCJsonTags::VOLUME) {
            //     return validate_volume(v_value, must);
            // } else if (tag == KMCCCJsonTags::CUTIN_TIME) {
            //     return validate_cutin_time(v_value, must);
            // } else if (tag == KMCCCJsonTags::START_TIME) {
            //     return validate_start_time(v_value, must);
            // }

            LOG("no validate [{}]", tag);
            return true;
        }

    private:
        bool validate_priority(std::string v_value, bool must) {
            int c = stoi(v_value);
            if (c >= 0) {
                return true;
            }

            return false;
        }

#pragma region L1
        bool validate_main_category(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            if (!(sp.at(0) == KMCCCMainCategory::PLAYER || sp.at(0) == KMCCCMainCategory::LOCATION ||
                  sp.at(0) == KMCCCMainCategory::NOTHING || sp.at(0) == KMCCCMainCategory::TEMP_KEYWORD ||
                  sp.at(0) == KMCCCMainCategory::CROSS_HAIR || sp.at(0) == KMCCCMainCategory::FORMULA)) {
                wt = "Something is wrong with what you have designated as the main category. [now] " + sp.at(0);
                return false;
            }

            if (!(sp.at(0) == KMCCCMainCategory::NOTHING) && !(sp.at(0) == KMCCCMainCategory::TEMP_KEYWORD) &&
                !(sp.at(0) == KMCCCMainCategory::CROSS_HAIR) && !(sp.at(0) == KMCCCMainCategory::FORMULA)) {
                if (!(sp.at(1) == KMCCCSubCategory::MOVE || sp.at(1) == KMCCCSubCategory::COMBAT ||
                      sp.at(1) == KMCCCSubCategory::KEYWORD || sp.at(1) == KMCCCSubCategory::RUNNING ||
                      sp.at(1) == KMCCCSubCategory::IDLE || sp.at(1) == KMCCCSubCategory::KILL ||
                      sp.at(1) == KMCCCSubCategory::BODY_SLOT || sp.at(1) == KMCCCSubCategory::MAGIC_EFFECT_KEYWORD)) {
                    wt = "Something is wrong with what you have designated as the sub category 1. [now] " + sp.at(1);
                    return false;
                }
            }

            if (sp.at(1) == KMCCCSubCategory::KEYWORD || sp.at(1) == KMCCCSubCategory::MAGIC_EFFECT_KEYWORD) {
                auto sp_k = KMCSplit(sp.at(2), ',');
                bool ok = true;
                for (auto v : sp_k) {
                    if (!(v == "has" || v == "nhas")) {
                        ok = false;
                    }
                }

                if (!ok) {
                    wt = "If subcategory 1 is a KEYWORD, subcategory 2 should be specified with has or nhas [now] " +
                         sp.at(2);
                    return false;
                }

                // keywordéwíËÇ»ÇÃÇ…formidÇ‚ÉvÉâÉOÉCÉìñºÇ™Ç»Ç¢èÍçá
                if (sp.at(6) == "" || sp.at(7) == "") {
                    wt = "No formid or plugin name, even though it is a KEYWORD designation. [now] " + sp.at(6) +
                         " [now] " + sp.at(7);
                    return false;
                }
            }

            if (sp.at(0) == KMCCCMainCategory::TEMP_KEYWORD) {
                auto sp_k = KMCSplit(sp.at(1), ',');
                bool ok = true;
                for (auto v : sp_k) {
                    if (!(v == "has" || v == "nhas")) {
                        ok = false;
                    }
                }

                if (!ok) {
                    wt = "If maincategory 1 is a TEMP_KEYWORD, subcategory 1 should be specified with has or nhas "
                         "[now] " +
                         sp.at(1);
                    return false;
                }

                if (sp.at(8) == "") {
                    wt = "No formid or plugin name, even though it is a TEMP_KEYWORD designation. [now] " + sp.at(8);
                    return false;
                }
            }

            return true;
        }
#pragma endregion

#pragma region L3

        bool validate_type_add(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            float coef_1 = stof(sp.at(0));
            float coef_2 = stof(sp.at(1));
            if (coef_1 < 0.0 || coef_2 < 0.0) {
                // 0ñ¢ñûÇÃåWêîÇÕNG
                wt = "coef_1 and coef_2 are negative numbers. Please make them positive numbers. [now] " + sp.at(0) +
                     " [now] " + sp.at(1);
                return false;
            }

            std::string relation = sp.at(2);
            if (!(relation == "between" || relation == "coef_1" || relation == "coef_2")) {
                // åWêîÇÃä÷åWê´Ç™Ç®Ç©ÇµÇ¢èÍçáÇÕNG
                wt = "coef_relation must be specified as between or coef_1 or coef_2 [now] " + sp.at(2);
                return false;
            }

            float cool_time = stof(sp.at(3));
            if (cool_time < 0.0f) {
                // ïâêîÇÃéûä‘ÇÕéÛÇØïtÇØÇ»Ç¢
                wt = "cool_time should be a positive number. [now] " + sp.at(3);
                return false;
            }

            float lower_value = stof(sp.at(4));
            float upper_value = stof(sp.at(5));
            if (lower_value < 0.0f || upper_value < 0.0f) {
                // ïâêîÇÃéûä‘ÇÕéÛÇØïtÇØÇ»Ç¢
                wt =
                    "lower_value and upper_value should be a positive number. [now] " + sp.at(4) + " [now] " + sp.at(5);
                return false;
            }

            return true;
        }

        bool validate_type_time(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            float cutin_time = stof(sp.at(0));
            float start_time = stof(sp.at(1));
            if (cutin_time < 0.0 || start_time < 0.0) {
                // 0ñ¢ñûÇÕNG
                wt = "cutin_time, start_time should be a positive number. [now] " + sp.at(0) + " [now] " + sp.at(1);
                return false;
            }

            return true;
        }

        bool validate_type_amount(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            float abandon_amount = stof(sp.at(0));
            float target_amount = stof(sp.at(1));
            float stack_limit = stof(sp.at(2));
            float cool_time = stof(sp.at(3));
            if (abandon_amount < 0.0 || target_amount < 0.0 || stack_limit < 0.0 || cool_time < 0.0) {
                // 0ñ¢ñûÇÕNG
                wt = "Cannot define a negative value in the definition in type_amount";
                return false;
            }

            return true;
        }

        bool validate_push_temp_keywords(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            std::string keyword = sp.at(0);
            std::string category = sp.at(1);

            auto ksp = KMCSplit(keyword, ',');
            auto kpnsp = KMCSplit(category, ',');
            if (ksp.size() != kpnsp.size()) {
                wt = "The number of definitions of keyword_name and category in type_temp_keywords "
                     "should be the same. [now] " +
                     sp.at(0) + " [now] " + sp.at(1);
                return false;
            }

            return true;
        }

        // bool validate_type_remove_keyword(std::string v_value, bool must, std::string &wt) {
        //     auto sp = KMCSplit(v_value, '/');
        //     std::string keyword = sp.at(0);
        //     std::string keyword_plugin_name = sp.at(1);

        //    auto ksp = KMCSplit(keyword, ',');
        //    auto kpnsp = KMCSplit(keyword_plugin_name, ',');

        //    if (ksp.size() != kpnsp.size()) {
        //        wt = "The number of definitions of remove_keyword_formid and remove_keyword_plugin_name in "
        //             "type_remove_keyword "
        //             "should be the same. [now] " +
        //             sp.at(0) + " [now] " + sp.at(1);
        //        return false;
        //    }

        //    return true;
        //}

        bool validate_node_relations(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            int subtract_timing = stoi(sp.at(0));

            if (!(subtract_timing == 0 || subtract_timing == 1)) {
                wt = "subtract_timing must be 0 or 1. [now] " + sp.at(0);
                return false;
            }

            return true;
        }

        bool validate_polling(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            float stay_time = stof(sp.at(0));
            // float put_on_hold = stof(sp.at(1));
            if (stay_time < 0.0f /*|| put_on_hold < 0.0f*/) {
                wt = "stay_time of the polling node must be a positive number greater than or equal to "
                     "0. [now] " +
                     sp.at(0);
                return false;
            }

            return true;
        }

        bool validate_cutin_setting(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            float time = stof(sp.at(0));
            float anim_time = stof(sp.at(1));
            float volume = stof(sp.at(2));
            float oar_time = stof(sp.at(3));
            float exp_time = stof(sp.at(4));
            if (time < 0.0f || anim_time < 0.0f) {
                wt = "time and anim_time of the polling node must be a positive number greater than or equal to "
                     "0. [now] " +
                     sp.at(0) + " [now] " + sp.at(1);
                return false;
            }

            if (volume < 0.0f /* || volume > 3.0f*/) {
                wt = "Volume should be set greater than or equal to 0. [now] " + sp.at(2);
                return false;
            }

            if (oar_time < 0.0f || exp_time < 0.0f) {
                wt = "oar_time, exp_time of the polling node must be a positive number greater than or equal to 0. "
                     "[now]  " +
                     sp.at(2);
                return false;
            }

            return true;
        }

        bool validate_force_expression(std::string v_value, bool must, std::string &wt) {
            auto sp = KMCSplit(v_value, '/');
            auto force_exp_timing = stoi(sp.at(0));

            auto force_exp_name = sp.at(1);


            auto force_expression_cool_time = stof(sp.at(2));
            auto force_expression_time = stof(sp.at(3));
            auto stop_percentage = stof(sp.at(4));

            if (force_expression_cool_time < 0.0f || force_expression_time < 0.0f) {
                wt = "Do not set negative numbers for expression_time and exp_cool_time.";
                return false;
            }

            if (stop_percentage < 0.0f || stop_percentage > 100.0f) {
                wt = "stop_percentage should be set with a value between 0 and 100";
                return false;
            }

            if (force_exp_timing == 1 || force_exp_timing == 2) {
                if (force_exp_name == "") {
                    wt = "Force_exp_timing is set to 1 or 2, but force_exp_name is set to an empty string.";
                }
            }

            return true;
        }
#pragma endregion
        //
        //
        // #pragma region L1
        //
        //
        //        bool validate_sub_category_1(std::string v_value, bool must) {
        //            if (v_value == "move") {
        //                return true;
        //            } else if (v_value == "combat") {
        //                return true;
        //            } else if (v_value == "talk") {
        //                return true;
        //            } else if (v_value == "keyword") {
        //                return true;
        //            }
        //
        //            return false;
        //        }
        //        bool validate_sub_category_2(std::string v_value, bool must) {
        //            auto sp = KMCSplit(v_value, ',');
        //            bool ok = true;
        //            for (auto v : sp) {
        //                if (v == "has") {
        //                } else if (v == "nhas") {
        //                } else {
        //                    ok = false;
        //                }
        //            }
        //            return ok;
        //        }
        //        bool validate_sub_category_3(std::string v_value, bool must) {
        //            bool ok = true;
        //            return ok;
        //        }
        //        bool validate_sub_category_4(std::string v_value, bool must) {
        //            bool ok = true;
        //            return ok;
        //        }
        //        bool validate_sub_category_5(std::string v_value, bool must) {
        //            bool ok = true;
        //            return ok;
        //        }
        //        bool validate_keyword_formid(std::string v_value, bool must) {
        //            if (v_value == "" && must) {
        //                return false;
        //            }
        //            return true;
        //        }
        //        bool validate_keyword_plugin_name(std::string v_value, bool must) {
        //            if (v_value == "" && must) {
        //                return false;
        //            }
        //            return true;
        //        }
        //
        // #pragma endregion
        //
        // #pragma region L2
        // #pragma endregion
        //
        // #pragma region L3
        //
        //
        //        bool validate_add_value(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            } else if (must && c == 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //
        //        bool validate_coef_1(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //
        //        bool validate_coef_2(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //
        //        bool validate_coef_relation(std::string v_value, bool must) {
        //            if (v_value == "between" || v_value == "coef_1" || v_value == "coef_2") {
        //                return true;
        //            }
        //
        //            return false;
        //        }
        //
        //        bool validate_cool_time(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //
        //        bool validate_upper_value(std::string v_value, bool must) {
        //
        //            //float c = stof(v_value);
        //            //if (c < 0.0f) {
        //            //    return false;
        //            //}
        //
        //            return true;
        //        }
        //
        //        bool validate_cutin_lower_value(std::string v_value, bool must) {
        //            // float c = stof(v_value);
        //            // if (c < 0.0f) {
        //            //     return false;
        //            // }
        //
        //            return true;
        //        }
        //
        //        bool validate_polling(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //        bool validate_put_on_hold(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //
        //        bool validate_time(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //        bool validate_anim_time(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //        bool validate_volume(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f || c > 3.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //        bool validate_cutin_time(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        //        bool validate_start_time(std::string v_value, bool must) {
        //            float c = stof(v_value);
        //            if (c < 0.0f) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        // #pragma endregion
        //
        // #pragma region L4
        //        bool validate_subtract_timing(std::string v_value, bool must) {
        //            int c = stoi(v_value);
        //            if (c != 0 || c != 1) {
        //                return false;
        //            }
        //
        //            return true;
        //        }
        // #pragma endregion
    };
}