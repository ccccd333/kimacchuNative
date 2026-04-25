#include "KMCCutinCond/KMCCheckSource.h"
#include "KMCCutinCondition.h"

namespace KMCCT {

    KMCFormula::KMCCompType KMCFormula::GetType(std::string v, std::vector<std::string> &r) {
        r = KMCSplit(v, ',');
        std::string s = r.at(0);
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "glob") {
            return KMCCompType::glob;
        } else if (s == "armo") {
            return KMCCompType::armo;
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
        glob_1 = (RE::TESGlobal *)RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(gv1, NULL, 16), gv2);
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
        glob_2 = (RE::TESGlobal *)RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(gv1, NULL, 16), gv2);
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
        p_armo_1 = (RE::TESObjectARMO *)RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(av1, NULL, 16), av2);
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
        p_armo_2 =
            (RE::TESObjectARMO *)RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(av1, NULL, 16), av2);
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
            KMC_LOG("[KMCFormula] Apply gv 1 {}", cv1s);
        } else if (comp_type1 == KMCFormula::KMCCompType::value) {
            if (!GetComp1Value(cv1b, cv1s)) return false;
            KMC_LOG("[KMCFormula] Apply v 1 {}", cv1s);
        } else if (comp_type1 == KMCFormula::KMCCompType::armo) {
            if (!GetComp1Armo(cv1b, cv1s)) return false;
            KMC_LOG("[KMCFormula] Apply v 1 {}", cv1s);
        } else {
            KMC_ERROR("[KMCFormula] v 1 {}", cv1s);
            return false;
        }

        std::vector<std::string> cv2b;
        comp_type2 = GetType(cv2s, cv2b);
        if (comp_type2 == KMCFormula::KMCCompType::glob) {
            if (!GetComp2Global(cv2b, cv2s)) return false;
            KMC_LOG("[KMCFormula] Apply gv 2 {}", cv2s);
        } else if (comp_type2 == KMCFormula::KMCCompType::value) {
            if (!GetComp2Value(cv2b, cv2s)) return false;
            KMC_LOG("[KMCFormula] Apply v 2 {}", cv2s);
        } else if (comp_type2 == KMCFormula::KMCCompType::armo) {
            if (!GetComp2Armo(cv2b, cv2s)) return false;
            KMC_LOG("[KMCFormula] Apply v 2 {}", cv2s);
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

    void KMCFormula::EndProc() {
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
        }

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

            fm->EndProc();

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