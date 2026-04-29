#include "KMCCutinCond/KMCCheckSource.h"
#include "KMCCutinCondition.h"
#include "KMCStorageUtilTracker.h"

namespace KMCCT {

    KMCFormula::KMCCompType KMCFormula::GetType(std::string v, std::vector<std::string> &r) {
        r = KMCSplit(v, ',');
        std::string s = r.at(0);
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "glob") {
            return KMCCompType::glob;
        } else if (s == "armo") {
            return KMCCompType::armo;
        } else if (s == "storage_util") {
            return KMCCompType::storage_util;
        } else if (s == "stuv") {
            return KMCCompType::storage_target_val;
        }

        return KMCCompType::value;
    }

    bool KMCFormula::GetComp1Global(std::vector<std::string> &v, std::string a) {
        if (v.size() != 3) {
            KMC_ERROR("ERROR The global value should have three elements. {}", a);
            return false;
        }

        std::string gv1 = v.at(1);
        std::string gv2 = v.at(2);
        boost::algorithm::trim(gv1);
        boost::algorithm::trim(gv2);
        glob_1 = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(std::stoll(gv1, NULL, 16), gv2);
        if (glob_1 == nullptr) {
            KMC_ERROR("ERROR The formula definition is incorrect {}", a);
            return false;
        }

        comp1 = [this]() {
            if (this->glob_1) {
                return this->glob_1->value;
            }
            return 0.0f;
        };

        return true;
    }

    bool KMCFormula::GetComp2Global(std::vector<std::string> &v, std::string a) {
        if (v.size() != 3) {
            KMC_ERROR("ERROR The global value should have three elements. {}", a);
            return false;
        }

        std::string gv1 = v.at(1);
        std::string gv2 = v.at(2);
        boost::algorithm::trim(gv1);
        boost::algorithm::trim(gv2);
        glob_2 = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(
            std::stoll(gv1, NULL, 16), gv2);
        if (glob_2 == nullptr) {
            KMC_ERROR("ERROR The formula definition is incorrect {}", a);
            return false;
        }

        comp2 = [this]() {
            if (this->glob_2) {
                return this->glob_2->value;
            }
            return 0.0f;
        };

        return true;
    }

    bool KMCFormula::GetComp1Armo(std::vector<std::string> &v, std::string a) {
        if (v.size() != 3) {
            KMC_ERROR("ERROR The armo value should have three elements. {}", a);
            return false;
        }

        std::string av1 = v.at(1);
        std::string av2 = v.at(2);
        boost::algorithm::trim(av1);
        boost::algorithm::trim(av2);
        p_armo_1 = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectARMO>(std::stoll(av1, NULL, 16), av2);
        if (p_armo_1 == nullptr) {
            KMC_ERROR("ERROR The formula definition is incorrect {}", a);
            return false;
        }

        armo_1 = p_armo_1->GetFormID();

        comp1 = [this]() {
            if (this->p_armo_1) {
                return std::bit_cast<float>(armo_1);
            }
            return 0.0f;
        };

        return true;
    }
    bool KMCFormula::GetComp2Armo(std::vector<std::string> &v, std::string a) {
        if (v.size() != 3) {
            KMC_ERROR("ERROR The armo value should have three elements. {}", a);
            return false;
        }

        std::string av1 = v.at(1);
        std::string av2 = v.at(2);
        boost::algorithm::trim(av1);
        boost::algorithm::trim(av2);
        p_armo_2 = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectARMO>(
            std::stoll(av1, NULL, 16), av2);
        if (p_armo_2 == nullptr) {
            KMC_ERROR("ERROR The formula definition is incorrect {}", a);
            return false;
        }

        armo_2 = p_armo_2->GetFormID();

        comp2 = [this]() {
            if (this->p_armo_2) {
                return std::bit_cast<float>(armo_2);
            }
            return 0.0f;
        };

        return true;
    }

    
    bool KMCFormula::GetComp1StrageUtil(std::vector<std::string> &v, std::string a) {
        if (v.size() != 6) {
            KMC_ERROR("ERROR The strage_util value should have six elements. {}", a);
            return false;
        }

        std::string av1 = v.at(1);
        std::string av2 = v.at(2);
        std::string av3 = v.at(3);
        std::string av4 = v.at(4);
        std::string av5 = v.at(5);
        boost::algorithm::trim(av1);
        boost::algorithm::trim(av2);
        boost::algorithm::trim(av3);
        boost::algorithm::trim(av4);
        boost::algorithm::trim(av5);

        long long form_r_id = 0;
        bool conv = false;
        try {
            form_r_id = std::stoll(av1, nullptr, 16);
            conv = true;
        } catch (const std::invalid_argument) {
            KMC_LOG("Invalid hex string: {}", av1);
        } catch (const std::out_of_range) {
            KMC_ERROR("Value out of range: {}", av1);
            return false;
        }

        comp1_v_sov.sov_is_null = true;

        if (conv) {
            RE::TESForm *form_id = nullptr;
            form_id = RE::TESDataHandler::GetSingleton()->LookupForm(form_r_id, av2);
            if (form_id) {
                comp1_v_sov.vm_object = StorageUtilTracker::BuildHandleFromStackPointer(form_id);
                comp1_v_sov.sov_is_null = false;
            } else {
                KMC_ERROR("[KMCFormula]StorageUtil RE::TESDataHandler::GetSingleton()->LookupForm not found. ");
                return false;
            }
        }

        comp1_v_sov.access_key = av3;

        if (!comp1_v_sov.default_value.SetValue(av5, av4)) {
            KMC_ERROR("[KMCFormula]StorageUtil The combination of type and default_value is incorrect. If it's 1, it should be set to int. ");
            return false;
        }

        return true;
    }
    bool KMCFormula::GetComp2StrageUtil(std::vector<std::string> &v, std::string a) {
        if (v.size() != 6) {
            KMC_ERROR("ERROR The strage_util value should have six elements. {}", a);
            return false;
        }

        std::string av1 = v.at(1);
        std::string av2 = v.at(2);
        std::string av3 = v.at(3);
        std::string av4 = v.at(4);
        std::string av5 = v.at(5);
        boost::algorithm::trim(av1);
        boost::algorithm::trim(av2);
        boost::algorithm::trim(av3);
        boost::algorithm::trim(av4);
        boost::algorithm::trim(av5);

        long long form_r_id = 0;
        bool conv = false;
        try {
            form_r_id = std::stoll(av1, nullptr, 16);
            conv = true;
        } catch (const std::invalid_argument) {
            KMC_LOG("Invalid hex string: {}", av1);
        } catch (const std::out_of_range) {
            KMC_ERROR("Value out of range: {}", av1);
            return false;
        }

        comp2_v_sov.sov_is_null = true;

        if (conv) {
            RE::TESForm *form_id = nullptr;
            form_id = RE::TESDataHandler::GetSingleton()->LookupForm(form_r_id, av2);
            if (form_id) {
                comp2_v_sov.vm_object = StorageUtilTracker::BuildHandleFromStackPointer(form_id);
                comp2_v_sov.sov_is_null = false;
            } else {
                KMC_ERROR("[KMCFormula]StorageUtil RE::TESDataHandler::GetSingleton()->LookupForm not found. ");
                return false;
            }
        }

        comp2_v_sov.access_key = av3;

        if (!comp2_v_sov.default_value.SetValue(av5, av4)) {
            KMC_ERROR(
                "[KMCFormula]StorageUtil The combination of type and default_value is incorrect. If it's 1, it should "
                "be set to int. ");
            return false;
        }
        return true;
    }

    bool KMCFormula::GetComp1StorageTargetVal(std::vector<std::string> &v, std::string a) {
        if (v.size() != 3) {
            KMC_ERROR("ERROR The stuv value should have three elements. {}", a);
            return false;
        }

        std::string av1 = v.at(1);
        std::string av2 = v.at(2);
        boost::algorithm::trim(av1);
        boost::algorithm::trim(av2);

        if (!comp1_stuv.SetValue(av2, av1)) {
            KMC_ERROR(
                "[KMCFormula]StorageUtil (stuv) The combination of type and default_value is incorrect. If it's 1, it should "
                "be set to int. ");
            return false;
        }

        return true;
    }
    bool KMCFormula::GetComp2StorageTargetVal(std::vector<std::string> &v, std::string a) {
        if (v.size() != 3) {
            KMC_ERROR("ERROR The stuv value should have three elements. {}", a);
            return false;
        }


        std::string av1 = v.at(1);
        std::string av2 = v.at(2);
        boost::algorithm::trim(av1);
        boost::algorithm::trim(av2);

        if (!comp2_stuv.SetValue(av2, av1)) {
            KMC_ERROR(
                "[KMCFormula]StorageUtil (stuv) The combination of type and default_value is incorrect. If it's 1, it "
                "should "
                "be set to int. ");
            return false;
        }

        return true;
    }

    bool KMCFormula::GetComp1Value(std::vector<std::string> &v, std::string a) {
        if (v.size() != 1) {
            KMC_ERROR("ERROR The value should have one elements. {}", a);
            return false;
        }

        try {
            std::string v1 = v.at(0);
            boost::algorithm::trim(v1);
            comp_v_1 = std::stof(v1);
            comp1 = [this]() {
                return this->comp_v_1;
            };
        } catch (std::exception &e) {
            KMC_ERROR("ERROR ===> {}, wt = {}", a, e.what());
            return false;
        }

        return true;
    }
    bool KMCFormula::GetComp2Value(std::vector<std::string> &v, std::string a) {
        if (v.size() != 1) {
            KMC_ERROR("ERROR The value should have one elements. {}", a);
            return false;
        }

        try {
            std::string v1 = v.at(0);
            boost::algorithm::trim(v1);
            comp_v_2 = std::stof(v1);
            comp2 = [this]() { return this->comp_v_2; };
        } catch (std::exception &e) {
            KMC_ERROR("ERROR ===> {}, wt = {}", a, e.what());
            return false;
        }

        return true;
    }

    bool KMCFormula::Build() {
        auto sp = KMCSplit(cond, ')');

        if (sp.size() < 2) return false;

        auto c1 = KMCSplit(sp.at(0), '(');
        if (c1.size() != 2) return false;
        std::string cv1s = c1.at(1);
        boost::algorithm::trim(cv1s);

        auto cs = KMCSplit(sp.at(1), '(');
        if (cs.size() != 2) return false;
        std::string cv2s = cs.at(1);
        boost::algorithm::trim(cv2s);
        std::string sign = cs.at(0);
        boost::algorithm::trim(sign);

        isign = StringToKMCInequalitySign(sign);
        if (KMCInequalitySign::unk == isign) return false;

        std::vector<std::string> cv1b;
        comp_type1 = GetType(cv1s, cv1b);
        if (comp_type1 == KMCFormula::KMCCompType::glob) {
            if (!GetComp1Global(cv1b, cv1s)) return false;
            UpdateLookupMode(KMCLookupMode::Lambda);
            KMC_LOG("[KMCFormula] Apply gv 1 {}", cv1s);
        } else if (comp_type1 == KMCFormula::KMCCompType::value) {
            if (!GetComp1Value(cv1b, cv1s)) return false;
            UpdateLookupMode(KMCLookupMode::Lambda);
            KMC_LOG("[KMCFormula] Apply v 1 {}", cv1s);
        } else if (comp_type1 == KMCFormula::KMCCompType::armo) {
            if (!GetComp1Armo(cv1b, cv1s)) return false;
            UpdateLookupMode(KMCLookupMode::Lambda);
            KMC_LOG("[KMCFormula] Apply v 1 {}", cv1s);
        } else if (comp_type1 == KMCFormula::KMCCompType::storage_util) {
            if (!GetComp1StrageUtil(cv1b, cv1s)) return false;
            UpdateLookupMode(KMCLookupMode::remote);
            remote_type = KMCRemoteType::strage_util;
            KMC_LOG("[KMCFormula] Apply stuv 1 {}", cv1s);
        } else if (comp_type1 == KMCFormula::KMCCompType::storage_target_val) {
            if (!GetComp1StorageTargetVal(cv1b, cv1s)) return false;
            UpdateLookupMode(KMCLookupMode::remote);
            remote_type = KMCRemoteType::strage_util;
            KMC_LOG("[KMCFormula] Apply stuv 1 {}", cv1s);
        } else {
            KMC_ERROR("[KMCFormula] v 1 {}", cv1s);
            return false;
        }

        std::vector<std::string> cv2b;
        comp_type2 = GetType(cv2s, cv2b);
        if (comp_type2 == KMCFormula::KMCCompType::glob) {
            if (!GetComp2Global(cv2b, cv2s)) return false;
            UpdateLookupMode(KMCLookupMode::Lambda);
            KMC_LOG("[KMCFormula] Apply gv 2 {}", cv2s);
        } else if (comp_type2 == KMCFormula::KMCCompType::value) {
            if (!GetComp2Value(cv2b, cv2s)) return false;
            UpdateLookupMode(KMCLookupMode::Lambda);
            KMC_LOG("[KMCFormula] Apply v 2 {}", cv2s);
        } else if (comp_type2 == KMCFormula::KMCCompType::armo) {
            if (!GetComp2Armo(cv2b, cv2s)) return false;
            UpdateLookupMode(KMCLookupMode::Lambda);
            KMC_LOG("[KMCFormula] Apply v 2 {}", cv2s);
        } else if (comp_type2 == KMCFormula::KMCCompType::storage_util) {
            if (!GetComp2StrageUtil(cv2b, cv2s)) return false;
            UpdateLookupMode(KMCLookupMode::remote);
            remote_type = KMCRemoteType::strage_util;
            KMC_LOG("[KMCFormula] Apply stuv 2 {}", cv2s);
        } else if (comp_type2 == KMCFormula::KMCCompType::storage_target_val) {
            if (!GetComp2StorageTargetVal(cv2b, cv2s)) return false;
            UpdateLookupMode(KMCLookupMode::remote);
            remote_type = KMCRemoteType::strage_util;
            KMC_LOG("[KMCFormula] Apply stuv 2 {}", cv2s);
        } else {
            KMC_ERROR("[KMCFormula] v 2 {}", cv2s);
            return false;
        }

        if (sp.size() > 2) {
            std::string andor = sp.at(2);
            boost::algorithm::trim(andor);
            transform(andor.begin(), andor.end(), andor.begin(), ::tolower);
            if (andor == "or") {
                and_or = AndOr::isOr;
            } else {
                and_or = AndOr::isAnd;
            }
        } else {
            and_or = AndOr::isAnd;
        }

        return true;
    }

    bool KMCFormula::EndProc() {
        if (cache_type == KMCCacheType::armo) {
            auto armo_fuc = [this](KMCCompType comp_type, RE::FormID armo, int idx_armo, float comp_v) {
                if (comp_type == KMCCompType::armo || comp_type == KMCCompType::value) {
                    if (comp_type == KMCCompType::armo) {
                        this->c_form_id = armo;
                        this->c_index = idx_armo;
                    } else if (comp_type == KMCCompType::value) {
                        if (comp_v > 0.0f) {
                            this->c_is_worn = true;
                        } else {
                            this->c_is_worn = false;
                        }
                    }
                } else {
                    KMC_ERROR("The right and left sides of the formula armo must be armo and value");
                }
            };

            armo_fuc(comp_type1, armo_1, idx_armo_1, comp_v_1);
            armo_fuc(comp_type2, armo_2, idx_armo_2, comp_v_2);
        } else if (remote_type == KMCRemoteType::strage_util) {
            bool detect_stu = false;
            MultiTypeValue c1;
            MultiTypeValue c2;
            if (comp_type1 == KMCCompType::storage_util) {
                detect_stu = true;            
                c1 = comp1_v_sov.default_value;
            } else if (comp_type1 == KMCCompType::storage_target_val) {
                c1 = comp1_stuv;
            } else {
                KMC_ERROR("ERROR: StorageUtil can only be compared with 'stuv' tokens.");
                return false;
            }

            if (comp_type2 == KMCCompType::storage_util) {
                detect_stu = true;  
                c2 = comp2_v_sov.default_value;
            } else if (comp_type2 == KMCCompType::storage_target_val) {
                c2 = comp2_stuv;
            } else {
                KMC_ERROR("ERROR: StorageUtil can only be compared with 'stuv' tokens.");
                return false;
            }

            if (!c1.Comp(c2)) {
                KMC_ERROR("ERROR: [StorageUtil] Incompatible Types: Left side is {} but right side is {}.",
                          (int)c1.value_type, (int)c2.value_type);
                return false;
            }

            if (!detect_stu) {
                KMC_ERROR(
                    "ERROR: [StorageUtil] Invalid comparison. You are comparing 'stuv' with 'stuv', but one side must "
                    "be a 'storage_util' source.");
                return false;
            }
        }


        return true;
    }

    bool KMCCCheckSource::BuildFormula(KMCCCheckSource &source) {
        int andor_c = 0;
        
        for (int formi = 0; formi < source.formula.size(); formi++) {
            auto fm = &source.formula.at(formi);
            if (!fm->Build()) {
                KMC_ERROR("The formula definition is incorrect. Please review it.");
                return false;
            }

            fm->Cache(cache_index, KMCCT::KMCCutinCondition::GetSingleton()->GetMain()->cache_container);

            if (!fm->EndProc()) {
                KMC_ERROR("The formula definition is incorrect. Please review it.");
                return false;
            }

            source.cond_formula[andor_c].emplace_back(fm);
            if (fm->and_or == AndOr::isOr) {
                KMC_LOG("[FROMULA OR] {} EntryNo ==> {}", fm->cond, andor_c);
                ++andor_c;
            } else {
                KMC_LOG("[FROMULA AND] {} EntryNo ==> {}", fm->cond, andor_c);
            }
        }

        return true;
    }
}