#include "KMCPapyrus.h"

SINGLETONBODY(KMCCT::KMCPapyrus)

namespace KMCCT {

    bool KMCPapyrus::KMCIsCapturedForm() {
        if (!get_first_cforms) return true;

        return false;
    }

    void KMCPapyrus::KMCCacheForms(std::vector<std::string> form_names) {
        if (get_first_cforms) {
            get_first_cforms = false;
            for (auto &value : form_names) {
                try {
                    auto sp = KMCSplit(value, ',');
                    auto form =
                        RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll(sp.at(0), NULL, 16), sp.at(1));
                    if (form == nullptr) {
                        LOG("KMCGetForms not found formid or plugin name {}", value);
                    }
                    form_list.push_back(form);
                } catch (std::exception &e) {
                    ERROR("KMCGetForms fatal {}", e.what());
                    form_list.push_back(nullptr);
                }
            }
        }
    }

    std::vector<RE::TESForm *> KMCPapyrus::KMCGetCachedForms() { return form_list; }

    bool KMCPapyrus::KMCIsCachedIdentity() {
        if (!get_init_first_ident) return true;
        return false;
    }

    void KMCPapyrus::KMCCreateIdentificationByDataAnalysis(std::vector<std::string> container, int start_index,
                                         std::vector<std::string> identitys, std::string identNameMismatch) {
        if (get_init_first_ident) {
            get_init_first_ident = false;
            for (int i = start_index; i < container.size(); i++) {
                auto container_value = container[i];
                bool match = false;
                for (auto &value : identitys) {
                    if (container_value.find(value) != std::string::npos) {
                        IdentityDatas.push_back(value);
                        match = true;
                        break;
                    }
                }

                if (!match) {
                    IdentityDatas.push_back(identNameMismatch);
                }
            }
        }
    }

    std::vector<std::string> KMCPapyrus::KMCGetCachedIdentificationByDataAnalysis() { 
        return IdentityDatas;
    }

    bool KMCPapyrus::KMCIsCachedModNames() {
        if (!get_init_first_mod_names) return true;
        return false;
    }
    void KMCPapyrus::KMCCacheModNames(std::vector<std::string> mod_names) {
        if (get_init_first_mod_names) {
            get_init_first_mod_names = false;
            for (auto &value : mod_names) {
                try {
                    int index = RE::TESDataHandler::GetSingleton()->GetLoadedModIndex(value).value_or(255);
                    ModNames.push_back(index);
                } catch (std::bad_optional_access &e) {
                    ERROR("KMCCacheModNames fatal {}", e.what());
                    ModNames.push_back(255);
                }
            }
        }
    }
    std::vector<int> KMCPapyrus::KMCGetCachedModNames() { return ModNames; }
}