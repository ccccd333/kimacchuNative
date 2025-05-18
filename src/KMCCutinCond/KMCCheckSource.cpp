#include "KMCCutinCond/KMCCheckSource.h"
#include "IWWFunctions.h"

namespace KMCCT {

    KMCFormula::KMCCompType KMCFormula::GetType(std::string v, std::vector<std::string> &r) {
        r = KMCSplit(v, ',');
        std::string s = r.at(0);
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "glob") {
            return KMCCompType::glob;
        }

        return KMCCompType::value;
    }

    bool KMCFormula::GetComp1Global(std::vector<std::string> &v, std::string a) {
        if (v.size() != 3) {
            ERROR("ERROR The global value should have three elements. {}", a);
            return false;
        }

        std::string gv1 = v.at(1);
        std::string gv2 = v.at(2);
        boost::algorithm::trim(gv1);
        boost::algorithm::trim(gv2);
        glob_1 = (RE::TESGlobal *)RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(gv1, NULL, 16), gv2);
        if (glob_1 == nullptr) {
            ERROR("ERROR The formula definition is incorrect {}", a);
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
            ERROR("ERROR The global value should have three elements. {}", a);
            return false;
        }

        std::string gv1 = v.at(1);
        std::string gv2 = v.at(2);
        boost::algorithm::trim(gv1);
        boost::algorithm::trim(gv2);
        glob_2 = (RE::TESGlobal *)RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(gv1, NULL, 16), gv2);
        if (glob_2 == nullptr) {
            ERROR("ERROR The formula definition is incorrect {}", a);
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

    bool KMCFormula::GetComp1Value(std::vector<std::string> &v, std::string a) {
        if (v.size() != 1) {
            ERROR("ERROR The value should have one elements. {}", a);
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
            ERROR("ERROR ===> {}, wt = {}", a, e.what());
            return false;
        }

        return true;
    }
    bool KMCFormula::GetComp2Value(std::vector<std::string> &v, std::string a) {
        if (v.size() != 1) {
            ERROR("ERROR The value should have one elements. {}", a);
            return false;
        }

        try {
            std::string v1 = v.at(0);
            boost::algorithm::trim(v1);
            comp_v_2 = std::stof(v1);
            comp2 = [this]() { return this->comp_v_2; };
        } catch (std::exception &e) {
            ERROR("ERROR ===> {}, wt = {}", a, e.what());
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
        KMCFormula::KMCCompType v1t = GetType(cv1s, cv1b);
        if (v1t == KMCFormula::KMCCompType::glob) {
            if (!GetComp1Global(cv1b, cv1s)) return false;
            LOG("[KMCFormula] Apply gv 1 {}", cv1s);
        } else {
            if (!GetComp1Value(cv1b, cv1s)) return false;
            LOG("[KMCFormula] Apply v 1 {}", cv1s);
        }

        std::vector<std::string> cv2b;
        KMCFormula::KMCCompType v2t = GetType(cv2s, cv2b);
        if (v2t == KMCFormula::KMCCompType::glob) {
            if (!GetComp2Global(cv2b, cv2s)) return false;
            LOG("[KMCFormula] Apply gv 2 {}", cv2s);
        } else {
            if (!GetComp2Value(cv2b, cv2s)) return false;
            LOG("[KMCFormula] Apply v 2 {}", cv2s);
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
}