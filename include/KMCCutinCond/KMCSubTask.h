#pragma once
#include "KMCConfig.h"

namespace KMCCT {
    enum class CutinCondSubType { keyword, none };

    enum class AdRm { add, remove, both };

    class KMCCSubTaskSource {
    public:
        std::string category = "";
        std::string keyword_name = "";
        bool disable_keyword_check = false;

        //std::vector<std::string> keywords;

        // has
        std::set<std::string> ad;
        std::set<std::string> rm;
        AdRm adrm;

    public:
        bool keyword_adrm_build() {
            auto sp = KMCSplit(category, ',');
            auto keysp = KMCSplit(keyword_name, ',');
            if (keysp.size() != sp.size()) {
                return false;
            }

            bool flag = false;
            bool nflag = false;
            for (int i = 0; i < sp.size(); i++) {
                if (sp.at(i) == "add") {
                    ad.emplace(keysp.at(i));
                    flag = true;
                } else if (sp.at(i) == "remove") {
                    rm.emplace(keysp.at(i));
                    nflag = true;
                }
            }

            if (flag && nflag) {
                adrm = AdRm::both;
            } else if (flag) {
                adrm = AdRm::add;
            } else {
                adrm = AdRm::remove;
            }

            return true;
        }
    };

    class KMCCustomCondSubTaskHub {
    public:
        KMCCustomCondSubTaskHub() {}
        virtual ~KMCCustomCondSubTaskHub() {}

        virtual std::unique_ptr<KMCCustomCondSubTaskHub> GetDetail(CutinCondSubType cct) { return nullptr; }
        virtual void Init() {}
        virtual void Reset() {}
        virtual bool Check() { return false; }
        virtual bool PushTask() { return false; }
    public:
        KMCCSubTaskSource sub_task_source;
    };

    class KMCCCKeyword : public KMCCustomCondSubTaskHub {
    public:
        std::unique_ptr<KMCCustomCondSubTaskHub> GetDetail(CutinCondSubType cct) override {
            if (cct == CutinCondSubType::keyword) {
                return std::make_unique<KMCCCKeyword>();
            }
            return nullptr;
        }

        void Init() override {}

        void Reset() override {}

        bool Check() override;

        bool PushTask() override;
    };

    bool GetCheckSubTaskDetail(CutinCondSubType source, std::unique_ptr<KMCCustomCondSubTaskHub> *hub);
}