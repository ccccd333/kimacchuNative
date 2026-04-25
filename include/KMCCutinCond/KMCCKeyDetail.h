#pragma once
#include "KMCCutinCond/KMCCJsonValueInterval.h"

namespace KMCCT {
    struct KMCKEPath {
    public:
        KMCKEPath() {}
        KMCKEPath(std::string p, int le) { 
            path = p; 
            level = le;
        }
        KMCKEPath(std::string p, std::string esc_p, char esc_p_char, int le) { 
            path = p;
            escape_p = esc_p;
            escape_char = esc_p_char;
            escape = true;
            level = le;
        }
        KMCKEPath(std::string p, std::string esc_p, char esc_p_char, int le, bool is_pu) {
            path = p;
            escape_p = esc_p;
            escape_char = esc_p_char;
            escape = true;
            level = le;
            is_push = is_pu;
        }
        KMCKEPath(std::string p, std::string esc_p, char esc_p_char, int le, bool is_pu_chi, bool is_pu) {
            path = p;
            escape_p = esc_p;
            escape_char = esc_p_char;
            escape = true;
            level = le;
            is_push_child = is_pu_chi;
            is_push = is_pu;
        }
    public:
        std::string path = "";
        std::string escape_p = "";
        char escape_char = '/';
        bool escape = false;
        int level = 0;
        bool is_push = false;
        bool is_push_child = false;
    };

    struct KMCKDElement {
    public:
        KMCKDElement() {}
        KMCKDElement(std::string pa, std::string na, std::string ty, std::string ranmin, std::string ranmax,
                     std::string inte, std::string ov) {
            path = pa;
            name = na;
            type = ty;
            range_min = ranmin;
            range_max = ranmax;
            interval = inte;
            origin_value = ov;
        }
    public:
        std::string path = "";
        std::string name = "";
        std::string type = "";

        std::string range_min = "";
        std::string range_max = "";
        std::string interval = "";
        std::string origin_value = "";

        int ivalue = 0;
        long lvalue = 0;
        std::string value = "";
        double dvalue = 0.0;

        std::string ac_key = "";
        int index = -1;
        KMCKEPath kd_path;
        std::string end_name = "";
    };

    struct KMCKDOption {
    public:
        KMCKDOption() {}
        KMCKDOption(int oid, int oi) { 
            id = oid;
            index = oi;
        }
    public:
        int id = -1;
        int index = -1;
    };

    class KMCCKeyDetail {
    public:
        const int ELEM_MAX = 6;
        const int RANG_MAX = 2;

    public:
        KMCCJsonValueInterval interval;

    public:
        void Setup() { interval.Setup(); }

        template <typename T1>
        std::string Build(int level, std::string path, T1 value, bool escape = false, char escp = '@') {
            std::string result = "";
            boost::any any = value;
            std::vector<std::string> sp;
            if (escape) {
                sp = KMCSplit(path, escp);
            } else {
                sp = KMCSplit(path, '.');
            }
            std::string name = sp.back();
            std::string inte = interval.GetInterval(level, name);

            if (any.type() == typeid(int)) {
                int t = boost::any_cast<int>(any);
                result = path + "/" + name + "/i" + "/" + inte + "/" + std::to_string(t);
            } else if (any.type() == typeid(std::string)) {
                std::string t = boost::any_cast<std::string>(any);
                if (t.empty()) {
                    t = " ";
                }
                result = path + "/" + name + "/s" + "/" + inte + "/" + t;
            } else if (any.type() == typeid(float)) {
                float t = boost::any_cast<float>(any);
                result = path + "/" + name + "/f" + "/" + inte + "/" + std::to_string(t);
            } else if (any.type() == typeid(double)) {
                double t = boost::any_cast<double>(any);
                result = path + "/" + name + "/l" + "/" + inte + "/" + std::to_string(t);
            } else {
                KMC_ERROR("KMCCKeyDetail Build unkown type");
            }

            KMC_LOG("KMCCKeyDetail result : [{}]", result);

            return result;
        }

        std::string Build(std::string path) { 

            auto sp = KMCSplit(path, '.');
            std::string name = sp.back();
            std::string result = path + "/" + name + "/h" + "/" + "N-N/N" + "/" + " ";
            return result;
        }

        std::string Build(std::string path, std::string escape) {
            auto sp = KMCSplit(path, '.');
            std::string name = sp.back();
            std::string result = path + "/" + name + "/h" + "/" + "N-N/N" + "/" + " ";
            return result;
        }
    };
}