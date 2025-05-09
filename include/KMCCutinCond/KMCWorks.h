#pragma once
#include "IWWFunctions.h"
#include "KMCChecks.h"
#include "KMCConfig.h"

namespace KMCCT {

    using Clock = std::chrono::steady_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::time_point;
    using namespace std::literals::chrono_literals;

    enum class CutinCondType { add, time, amount, dummy, none };
    class CCTCoefR {
    public:
        static constexpr const char* BETWEEN = "between";
        static constexpr const char* COEF_1 = "coef_1";
        static constexpr const char* COEF_2 = "coef_2";
    };

    class KMCCustomCondTaskHub {
    public:
        KMCCustomCondTaskHub() {}
        virtual ~KMCCustomCondTaskHub() {}

        virtual std::unique_ptr<KMCCustomCondTaskHub> GetDetail(CutinCondType cct) { return nullptr; }

        virtual bool Init(KMCCustomCondCheckHub* check, KMCCCheckSource* source) { return false; }
        virtual bool PreProcessing() { return false; }
        // カウントを進める。Addなら係数ごとにカウントアップ。Timeなら1秒加算など
        virtual void ToMove() {}

        virtual void Reset() {}
        virtual void Commit() {}
        virtual bool Completed() { return false; }

        virtual void Add(float c_add_value, float correction) {}
        virtual void Subtract(float subtract_value, float correction) {}
        virtual void Div(float div_value, float correction) {}
        virtual void Mult(float mult_value, float correction) {}
        virtual bool IsEpsilon() { return true; }
        virtual float CompPercent() { return 0.0; }

        virtual void Correction(float c) {}
        void SetCoefR(std::string cr) {
            if (cr == CCTCoefR::BETWEEN) {
                coef_relation = CCTCoefR::BETWEEN;
            } else if (cr == CCTCoefR::COEF_1) {
                coef_relation = CCTCoefR::COEF_1;
            } else if (cr == CCTCoefR::COEF_2) {
                coef_relation = CCTCoefR::COEF_2;
            }
            ERROR("CCTCoefR::Check NG {}", cr);
            coef_relation = CCTCoefR::BETWEEN;
        }

    public:
        // add
        float add_value = 1.0f;
        float coef_1 = 0.1f;
        float coef_2 = 1.0f;
        std::string coef_relation = CCTCoefR::BETWEEN;
        float cool_time = 0.0f;
        float upper_value = 30.0f;
        float lower_value = 0.0f;
        // time
        float end_time = 30.0f;
        float start_time = 0.0f;
        float add_time_v = F_CUT_IN_COND_WHILE_WAIT_TIME;

        time_point<Clock> timer;
        bool cool_time_now = false;

        // add
        float wrk_value_add = 0.0f;
        float commit_wrk_value_add = 0.0f;

        // time
        float wrk_value_time = 0.0f;
        float commit_wrk_value_time = 0.0f;

        // amount
        float stack_limit = 100.0f;
        float target_amount = 100.0f;
        float abandon_amount = 10.0f;
        float amount = 0.0f;

        //  rand
        std::mt19937 mt;

        // check task
        KMCCustomCondCheckHub* check_hub;
        KMCCCheckSource* csource;
    };

    class KMCCCAdd : public KMCCustomCondTaskHub {
    public:
        std::unique_ptr<KMCCustomCondTaskHub> GetDetail(CutinCondType cct) override {
            if (cct == CutinCondType::add) {
                return std::make_unique<KMCCCAdd>();
            }
            return nullptr;
        }

        bool Init(KMCCustomCondCheckHub* check, KMCCCheckSource* source) override { return true; }
        bool PreProcessing() override { return true; }
        void Correction(float c) override {
            wrk_value_add = wrk_value_add + wrk_value_add * c;
            LOG("[KMCCCAdd.Correction] av {}", wrk_value_add);
        }

        void ToMove() override {
            if (cool_time > 0.0f) {
                if (cool_time_now) {
                    time_point<Clock> now = Clock::now();
                    milliseconds diff = duration_cast<milliseconds>(now - timer);
                    long long dur = diff.count();
                    if (dur >= cool_time * KMCCT::TIME_SCALE_MS) {
                        cool_time_now = false;
                    } else {
                        LOG("[ADD.ToMove] cool time av {}, dur(ms) {}, cool time(ms) {}", std::to_string(wrk_value_add),
                            dur, cool_time * KMCCT::TIME_SCALE_MS);
                        return;
                    }
                }

                if (!cool_time_now) {
                    timer = Clock::now();
                    cool_time_now = true;
                }
            }

            if (coef_relation == CCTCoefR::BETWEEN) {
                std::uniform_real_distribution<float> get_rand_uni_real(coef_1, coef_2);
                float r = get_rand_uni_real(mt);
                wrk_value_add = add_value * r;
            } else if (coef_relation == CCTCoefR::COEF_1) {
                wrk_value_add = add_value * coef_1;
            } else if (coef_relation == CCTCoefR::COEF_2) {
                wrk_value_add = add_value * coef_2;
            }

            LOG("[ADD.ToMove] av {}", std::to_string(wrk_value_add));
        }

        void Add(float c_add_value, float correction) override {
            float cor_addv = c_add_value + c_add_value * correction;
            wrk_value_add += cor_addv;
            LOG("[ADD.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_add), std::to_string(cor_addv),
                std::to_string(commit_wrk_value_add));
        }

        void Subtract(float subtract_value, float correction) override {
            float cor_subv = subtract_value + subtract_value * correction;
            if (cor_subv > 0.0f && (commit_wrk_value_add + wrk_value_add) - cor_subv >= 0.0f) {
                wrk_value_add -= cor_subv;
                LOG("[ADD.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
                    std::to_string(cor_subv), std::to_string(commit_wrk_value_add));

            } else if (cor_subv > 0.0f) {
                wrk_value_add = 0.0f;
                LOG("[ADD.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
                    std::to_string(cor_subv), std::to_string(commit_wrk_value_add));
            }
        }

        void Div(float div_value, float correction) override {
            float cor_divv = div_value + div_value * correction;
            if (cor_divv != 0.0f) {
                wrk_value_add /= cor_divv;
                LOG("[ADD.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_add), std::to_string(cor_divv),
                    std::to_string(commit_wrk_value_add));
            } else {
                LOG("[ADD.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
                    std::to_string(wrk_value_add), std::to_string(cor_divv), std::to_string(commit_wrk_value_add));
            }
        }

        void Mult(float mult_value, float correction) override {
            float cor_multv = mult_value + mult_value * correction;
            wrk_value_add *= cor_multv;
            LOG("[ADD.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_add), std::to_string(cor_multv),
                std::to_string(commit_wrk_value_add));
        }

        bool IsEpsilon() override {
            if (wrk_value_add < FLT_EPSILON) {
                wrk_value_add = 0.0f;
                return true;
            }

            return false;
        }

        float CompPercent() override {
            if (upper_value == 0.0f) return 0.0f;

            return (commit_wrk_value_add / upper_value) * 100.0f;
        }

        void Reset() override {
            commit_wrk_value_add = lower_value;
            wrk_value_add = 0.0f;
            cool_time_now = false;
            LOG("[ADD.Reset] av {} cv {}", std::to_string(wrk_value_add), std::to_string(commit_wrk_value_add));
        }
        void Commit() override {
            commit_wrk_value_add += wrk_value_add;
            wrk_value_add = 0.0f;
            LOG("[ADD.Commit] cv {}, av {}", std::to_string(commit_wrk_value_add), std::to_string(wrk_value_add));
        }

        bool Completed() override {
            if (commit_wrk_value_add >= upper_value) {
                LOG("[ADD.Completed] cv {}, av {}", std::to_string(commit_wrk_value_add),
                    std::to_string(wrk_value_add));
                return true;
            }

            return false;
        }
    };

    class KMCCCTime : public KMCCustomCondTaskHub {
    public:
        std::unique_ptr<KMCCustomCondTaskHub> GetDetail(CutinCondType cct) override {
            if (cct == CutinCondType::time) {
                return std::make_unique<KMCCCTime>();
            }
            return nullptr;
        }

        bool Init(KMCCustomCondCheckHub* check, KMCCCheckSource* source) override { return true; }
        bool PreProcessing() override { return true; }
        void Correction(float c) override {
            wrk_value_time = wrk_value_time + wrk_value_time * c;
            LOG("[KMCCCTime.Correction] av {}", wrk_value_time);
        }
        void ToMove() override {
            wrk_value_time = add_time_v;
            LOG("[TIME.ToMove] av {}", std::to_string(wrk_value_time));
        }

        void Add(float c_add_value, float correction) override {
            float cor_addv = c_add_value + c_add_value * correction;
            wrk_value_time += cor_addv;
            LOG("[TIME.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_time), std::to_string(cor_addv),
                std::to_string(commit_wrk_value_time));
        }

        void Subtract(float subtract_value, float correction) override {
            float cor_subv = subtract_value + subtract_value * correction;
            if (cor_subv > 0.0f && (commit_wrk_value_time + wrk_value_time) - cor_subv >= 0.0f) {
                wrk_value_time -= cor_subv;
                LOG("[TIME.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_time),
                    std::to_string(cor_subv), std::to_string(commit_wrk_value_time));
            } else if (cor_subv > 0.0f) {
                wrk_value_time = 0.0f;
                LOG("[TIME.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_time),
                    std::to_string(cor_subv), std::to_string(commit_wrk_value_time));
            }
        }

        void Div(float div_value, float correction) override {
            float cor_divv = div_value + div_value * correction;
            if (cor_divv != 0.0f) {
                wrk_value_time /= cor_divv;
                LOG("[TIME.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_time), std::to_string(cor_divv),
                    std::to_string(commit_wrk_value_time));
            } else {
                LOG("[TIME.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
                    std::to_string(wrk_value_time), std::to_string(cor_divv), std::to_string(commit_wrk_value_time));
            }
        }

        void Mult(float mult_value, float correction) override {
            float cor_multv = mult_value + mult_value * correction;
            wrk_value_time *= cor_multv;
            LOG("[TIME.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_time), std::to_string(cor_multv),
                std::to_string(commit_wrk_value_time));
        }

        bool IsEpsilon() override {
            if (wrk_value_time < FLT_EPSILON) {
                wrk_value_time = 0.0f;
                return true;
            }

            return false;
        }

        float CompPercent() override {
            if (end_time == 0.0f) return 0.0f;

            return (commit_wrk_value_time / end_time) * 100.0f;
        }

        void Reset() override {
            commit_wrk_value_time = start_time;
            wrk_value_time = 0.0f;
            LOG("[TIME.Reset] av {} cv {}", std::to_string(wrk_value_time), std::to_string(commit_wrk_value_time));
        }
        void Commit() override {
            commit_wrk_value_time += wrk_value_time;
            wrk_value_time = 0.0f;
            LOG("[TIME.Commit] cv {}, av {}", std::to_string(commit_wrk_value_time), std::to_string(wrk_value_time));
        }

        bool Completed() override {
            if (commit_wrk_value_time >= end_time) {
                LOG("[TIME.Completed] cv {}, av {}", std::to_string(commit_wrk_value_time), std::to_string(end_time));
                return true;
            }

            return false;
        }
    };

    class KMCCCAmount : public KMCCustomCondTaskHub {
    public:
        std::mutex mtx;

    public:
        std::unique_ptr<KMCCustomCondTaskHub> GetDetail(CutinCondType cct) override {
            if (cct == CutinCondType::amount) {
                return std::make_unique<KMCCCAmount>();
            }
            return nullptr;
        }

        void AddItemEvent(const RE::TESContainerChangedEvent* event);

        bool Init(KMCCustomCondCheckHub* check, KMCCCheckSource* source) override;

        bool PreProcessing() override { return true; }

        void Correction(float c) override { LOG("[KMCCCAmount.Correction]"); }

        void ToMove() override;

        void Add(float c_add_value, float correction) override {
            float cor_addv = c_add_value + c_add_value * correction;
            wrk_value_add += cor_addv;
            LOG("[Amount.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_add), std::to_string(cor_addv),
                std::to_string(commit_wrk_value_add));
        }

        void Subtract(float subtract_value, float correction) override {
            float cor_subv = subtract_value + subtract_value * correction;

            if (cor_subv > 0.0f && (commit_wrk_value_add + wrk_value_add) - cor_subv >= 0.0f) {
                wrk_value_add -= cor_subv;
                LOG("[Amount.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
                    std::to_string(cor_subv), std::to_string(commit_wrk_value_add));

            } else if (cor_subv > 0.0f) {
                wrk_value_add = 0.0f;
                LOG("[Amount.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
                    std::to_string(cor_subv), std::to_string(commit_wrk_value_add));
            }
        }

        void Div(float div_value, float correction) override {
            float cor_divv = div_value + div_value * correction;
            if (cor_divv != 0.0f) {
                wrk_value_add /= cor_divv;
                LOG("[Amount.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_add), std::to_string(cor_divv),
                    std::to_string(commit_wrk_value_add));
            } else {
                LOG("[Amount.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
                    std::to_string(wrk_value_add), std::to_string(cor_divv), std::to_string(commit_wrk_value_add));
            }
        }

        void Mult(float mult_value, float correction) override {
            float cor_multv = mult_value + mult_value * correction;
            wrk_value_add *= cor_multv;
            LOG("[Amount.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_add), std::to_string(cor_multv),
                std::to_string(commit_wrk_value_add));
        }

        bool IsEpsilon() override {
            if (wrk_value_add < FLT_EPSILON) {
                wrk_value_time = 0.0f;
                return true;
            }

            return false;
        }

        float CompPercent() override {
            if (target_amount == 0.0f) return 0.0f;

            return (commit_wrk_value_add / target_amount) * 100.0f;
        }

        void Reset() override {
            commit_wrk_value_add = 0.0f;
            wrk_value_add = 0.0f;
            cool_time_now = false;
            LOG("[Amount.Reset] av {} cv {}", std::to_string(wrk_value_add), std::to_string(commit_wrk_value_add));
        }
        void Commit() override {
            commit_wrk_value_add += wrk_value_add;
            wrk_value_add = 0.0f;
            LOG("[Amount.Commit] cv {}, av {}", std::to_string(commit_wrk_value_add), std::to_string(wrk_value_add));
        }

        bool Completed() override {
            if (commit_wrk_value_add >= target_amount) {
                LOG("[Amount.Completed] cv {}, av {}", std::to_string(commit_wrk_value_add),
                    std::to_string(wrk_value_add));
                return true;
            }

            return false;
        }
    };

    class KMCCCDummy : public KMCCustomCondTaskHub {
    public:
        std::unique_ptr<KMCCustomCondTaskHub> GetDetail(CutinCondType cct) override {
            if (cct == CutinCondType::dummy) {
                return std::make_unique<KMCCCDummy>();
            }
            return nullptr;
        }

        bool Init(KMCCustomCondCheckHub* check, KMCCCheckSource* source) override { return true; }
        bool PreProcessing() override { return true; }

        void Correction(float c) override { LOG("[Dummy.Correction]"); }

        void ToMove() override { LOG("[Dummy.ToMove]"); }

        void Add(float c_add_value, float correction) override { LOG("[Dummy.Add]"); }

        void Subtract(float subtract_value, float correction) override { LOG("[Dummy.Subtract]"); }

        void Div(float div_value, float correction) override { LOG("[Dummy.Div]"); }

        void Mult(float mult_value, float correction) override { LOG("[Dummy.Mult]"); }

        bool IsEpsilon() override { return false; }

        float CompPercent() override { return 0.0f; }

        void Reset() override { LOG("[Dummy.Reset]"); }
        void Commit() override { LOG("[Dummy.Commit]"); }

        bool Completed() override {
            LOG("[Dummy.Completed]");

            return false;
        }
    };

    bool GetWorkDetail(CutinCondType cct, std::unique_ptr<KMCCustomCondTaskHub>* hub);
}