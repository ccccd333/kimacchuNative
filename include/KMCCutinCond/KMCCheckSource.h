#pragma once
#include "KMCUtility.h"
#include "KMCCCJsonTags.h"
#include "IWWFunctions.h"
#include <boost/algorithm/string/trim.hpp>

namespace KMCCT {

    enum class HasNHan {
        has,
        nhas,
        both
    };

    enum class AndOr {
        isAnd,
        isOr,
        none
    };

    class KMCCSBodySlot {
    public:
        KMCCSBodySlot() {}
        using Slot = RE::BIPED_MODEL::BipedObjectSlot;

        Slot GetSlot(std::string name) {
            if (name == KMCCCJsonTags::HEAD) {
                return Slot::kHead;
            } else if (name == KMCCCJsonTags::HAIR) {
                return Slot::kHair;
            } else if (name == KMCCCJsonTags::BODY) {
                return Slot::kBody;
            } else if (name == KMCCCJsonTags::HANDS) {
                return Slot::kHands;
            } else if (name == KMCCCJsonTags::FOREARMS) {
                return Slot::kForearms;
            } else if (name == KMCCCJsonTags::AMULET) {
                return Slot::kAmulet;
            } else if (name == KMCCCJsonTags::RING) {
                return Slot::kRing;
            } else if (name == KMCCCJsonTags::FEET) {
                return Slot::kFeet;
            } else if (name == KMCCCJsonTags::CALVES) {
                return Slot::kCalves;
            } else if (name == KMCCCJsonTags::SHIELD) {
                return Slot::kShield;
            } else if (name == KMCCCJsonTags::TAIL) {
                return Slot::kTail;
            } else if (name == KMCCCJsonTags::LONGHAIR) {
                return Slot::kLongHair;
            } else if (name == KMCCCJsonTags::CIRCLET) {
                return Slot::kCirclet;
            } else if (name == KMCCCJsonTags::EARS) {
                return Slot::kEars;
            } else if (name == KMCCCJsonTags::FACE_COVERING_OVER_THE_MOUTH) {
                return Slot::kModMouth;
            } else if (name == KMCCCJsonTags::NECK_COVERING) {
                return Slot::kModNeck;
            } else if (name == KMCCCJsonTags::CLOAK) {
                return Slot::kModChestPrimary;
            } else if (name == KMCCCJsonTags::BACKPACK) {
                return Slot::kModBack;
            } else if (name == KMCCCJsonTags::MAGIC_FX) {
                return Slot::kModMisc1;
            } else if (name == KMCCCJsonTags::SKIRT) {
                return Slot::kModPelvisPrimary;
            } else if (name == KMCCCJsonTags::DECAPITATEHEAD) {
                return Slot::kDecapitateHead;
            } else if (name == KMCCCJsonTags::DECAPITATE) {
                return Slot::kDecapitate;
            } else if (name == KMCCCJsonTags::GENITALS_OR_UNDERWEAR) {
                return Slot::kModPelvisSecondary;
            } else if (name == KMCCCJsonTags::PANTS) {
                return Slot::kModLegRight;
            } else if (name == KMCCCJsonTags::STOCKINGS) {
                return Slot::kModLegLeft;
            } else if (name == KMCCCJsonTags::FACE_COVERING_USUALLY_UPPER_FACE) {
                return Slot::kModFaceJewelry;
            } else if (name == KMCCCJsonTags::TORSO) {
                return Slot::kModChestSecondary;
            } else if (name == KMCCCJsonTags::SHOULDERS) {
                return Slot::kModShoulder;
            } else if (name == KMCCCJsonTags::SHIRT_OR_LEFT_ARM) {
                return Slot::kModArmLeft;
            } else if (name == KMCCCJsonTags::LOOSE_SHIRT_OR_RIGHT_ARM) {
                return Slot::kModArmRight;
            } else if (name == KMCCCJsonTags::MISCELLANEOUS) {
                return Slot::kModMisc2;
            } else if (name == KMCCCJsonTags::FX01) {
                return Slot::kFX01;
            }

            return Slot::kNone;
        }

        void Set(std::string name, int value) {
            int v = value > 1 ? 0 : value;

            if (check_value.contains(name)) {
                check_value.at(name) = v;
            } else {
                check_value.emplace(name, v);                
            }
        }

        void Init() {
            slots.clear();

            for (auto &[uk, uv] : check_value) {
                slots.emplace_back(static_cast<std::uint32_t>(GetSlot(uk)));
            }
        }

        std::vector<std::uint32_t> slots;
        std::map<std::string, int> check_value; 
        int match = 0;
    };

    class KMCFormula {
    public:
        KMCFormula(){};

        enum class KMCCompType {
            value,
            glob,
            none
        };

        KMCCompType GetType(std::string v, std::vector<std::string> &r);
        bool GetComp1Global(std::vector<std::string> &v, std::string a);
        bool GetComp2Global(std::vector<std::string> &v, std::string a);
        bool GetComp1Value(std::vector<std::string> &v, std::string a);
        bool GetComp2Value(std::vector<std::string> &v, std::string a);
        bool Build();

        bool not_equal = false;
        std::string cond = "";

        RE::TESGlobal *glob_1 = nullptr;
        RE::TESGlobal *glob_2 = nullptr;
        float comp_v_1 = 0.0;
        float comp_v_2 = 0.0;

        std::function<float(void)> comp1;
        std::function<float(void)> comp2;

        KMCInequalitySign isign = KMCInequalitySign::equal;
        AndOr and_or = AndOr::none;
    };

    class KMCCCheckSource {
    public:
        std::string main_category;
        std::string sub1_category;
        std::string sub2_category;
        std::string sub3_category;
        std::string sub4_category;
        std::string sub5_category;
        std::string keyword_formid;
        std::string keyword_plugin_name;
        std::string temp_keyword_name;
        std::vector<std::string> cross_hair_ref_name;
        

        KMCCSBodySlot body_slot;

        std::vector<RE::TESForm*> forms;
        std::vector<RE::BGSKeyword*> keywords;
        
        // has
        std::vector<RE::BGSKeyword *> has;
        std::vector<RE::BGSKeyword *> nhas;
        HasNHan hsnhs;

        // thas
        std::set<std::string> thas;
        std::set<std::string> tnhas;
        HasNHan thsnhs;

        // formula
        std::vector<KMCFormula> formula;
        std::map<int, std::vector<KMCFormula*>> cond_formula;
        
    public:
        void body_slot_build() { 
            body_slot.Init();
        }

        bool keyword_has_nhas_build(std::string ctgry) {
            auto sp = KMCSplit(ctgry, ',');
            if (keywords.size() != sp.size()) {
                return false;
            }

            bool flag = false;
            bool nflag = false;
            for (int i = 0; i < sp.size(); i++) {
                if (sp.at(i) == "has") {
                    has.emplace_back(keywords.at(i));
                    flag = true;
                } else if (sp.at(i) == "nhas") {
                    nhas.emplace_back(keywords.at(i));
                    nflag = true;
                }
            }

            if (flag && nflag) {
                hsnhs = HasNHan::both;
            } else if (flag) {
                hsnhs = HasNHan::has;
            } else {
                hsnhs = HasNHan::nhas;
            }

            return true;
        }

        bool temp_keyword_has_nhas_build(std::string ctgry) {
            auto sp = KMCSplit(ctgry, ',');
            auto tkeysp = KMCSplit(temp_keyword_name, ',');
            if (tkeysp.size() != sp.size()) {
                return false;
            }
            bool flag = false;
            bool nflag = false;
            for (int i = 0; i < sp.size(); i++) {
                if (sp.at(i) == "has") {
                    thas.emplace(tkeysp.at(i));
                    flag = true;
                } else if (sp.at(i) == "nhas") {
                    tnhas.emplace(tkeysp.at(i));
                    nflag = true;
                }
            }

            if (flag && nflag) {
                thsnhs = HasNHan::both;
            } else if (flag) {
                thsnhs = HasNHan::has;
            } else {
                thsnhs = HasNHan::nhas;
            }

            return true;
        }

        bool EvaluateFormula() {
            for (auto &[key, value] : cond_formula) {

                bool t = false;
                bool f = false;
                for (auto &formv : value) {
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
    };
}