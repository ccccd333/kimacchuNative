#pragma once
#include "KMCUtility.h"

namespace KMCCT {
    enum class HasNHan {
        has,
        nhas,
        both
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
    public:
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
    };
}