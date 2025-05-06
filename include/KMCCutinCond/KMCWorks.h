#pragma once
#include "IWWFunctions.h"
#include "KMCConfig.h"
#include "KMCChecks.h"

namespace KMCCT {

    using Clock = std::chrono::steady_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::time_point;
    using namespace std::literals::chrono_literals;

    enum class CutinCondType { add, time, amount, none };
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

        virtual void Add(float c_add_value) {}
        virtual void Subtract(float subtract_value) {}
        virtual void Div(float div_value) {}
        virtual void Mult(float mult_value) {}
        virtual bool IsEpsilon() { return true; }
        virtual float CompPercent() { return 0.0; }
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
        float add_time_v = 1.0f;

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
        KMCCCheckSource *csource;
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
                wrk_value_add += add_value * r;
            } else if (coef_relation == CCTCoefR::COEF_1) {
                wrk_value_add += add_value * coef_1;
            } else if (coef_relation == CCTCoefR::COEF_2) {
                wrk_value_add += add_value * coef_2;
            }

            LOG("[ADD.ToMove] av {}", std::to_string(wrk_value_add));
        }

        void Add(float c_add_value) override {
            wrk_value_add += c_add_value;
            LOG("[ADD.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_add), std::to_string(c_add_value),
                std::to_string(commit_wrk_value_add));
        }

        void Subtract(float subtract_value) override {
            if (subtract_value > 0.0f && wrk_value_add - subtract_value >= 0.0f) {
                wrk_value_add -= subtract_value;
                LOG("[ADD.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
                    std::to_string(subtract_value), std::to_string(commit_wrk_value_add));

            } else if (subtract_value > 0.0f) {
                wrk_value_add = 0.0f;
                LOG("[ADD.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
                    std::to_string(subtract_value), std::to_string(commit_wrk_value_add));
            }
        }

        void Div(float div_value) override {
            if (div_value != 0.0f) {
                wrk_value_add /= div_value;
                LOG("[ADD.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_add), std::to_string(div_value),
                    std::to_string(commit_wrk_value_add));
            } else {
                LOG("[ADD.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
                    std::to_string(wrk_value_add), std::to_string(div_value), std::to_string(commit_wrk_value_add));
            }
        }

        void Mult(float mult_value) override {
            wrk_value_add *= mult_value;
            LOG("[ADD.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_add), std::to_string(mult_value),
                std::to_string(commit_wrk_value_add));
        }

        bool IsEpsilon() override { return fabsf(wrk_value_add - commit_wrk_value_add) < FLT_EPSILON; }

        float CompPercent() override { 
            if (upper_value == 0.0f) return 100.0f;

            return (commit_wrk_value_add / upper_value) * 100.0f; 
        }

        void Reset() override {
            commit_wrk_value_add = lower_value;
            wrk_value_add = lower_value;
            cool_time_now = false;
            LOG("[ADD.Reset] av {} cv {}", std::to_string(wrk_value_add), std::to_string(commit_wrk_value_add));
        }
        void Commit() override {
            commit_wrk_value_add = wrk_value_add;
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
        void ToMove() override {
            wrk_value_time += add_time_v;
            LOG("[TIME.ToMove] av {}", std::to_string(wrk_value_time));
        }

        void Add(float c_add_value) override {
            wrk_value_time += c_add_value;
            LOG("[TIME.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_time), std::to_string(c_add_value),
                std::to_string(commit_wrk_value_time));
        }

        void Subtract(float subtract_value) override {
            if (subtract_value > 0.0f && wrk_value_time - subtract_value >= 0.0f) {
                wrk_value_time -= subtract_value;
                LOG("[TIME.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_time),
                    std::to_string(subtract_value), std::to_string(commit_wrk_value_time));
            } else if (subtract_value > 0.0f) {
                wrk_value_time = 0.0f;
                LOG("[TIME.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_time),
                    std::to_string(subtract_value), std::to_string(commit_wrk_value_time));
            }
        }

        void Div(float div_value) override {
            if (div_value != 0.0f) {
                wrk_value_time /= div_value;
                LOG("[TIME.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_time), std::to_string(div_value),
                    std::to_string(commit_wrk_value_time));
            } else {
                LOG("[TIME.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
                    std::to_string(wrk_value_time), std::to_string(div_value), std::to_string(commit_wrk_value_time));
            }
        }

        void Mult(float mult_value) override {
            wrk_value_time *= mult_value;
            LOG("[TIME.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_time), std::to_string(mult_value),
                std::to_string(commit_wrk_value_time));
        }

        bool IsEpsilon() override { return fabsf(wrk_value_time - commit_wrk_value_time) < FLT_EPSILON; }

        float CompPercent() override {
            if (end_time == 0.0f) return 100.0f;

            return (commit_wrk_value_time / end_time) * 100.0f;
        }

        void Reset() override {
            commit_wrk_value_time = start_time;
            wrk_value_time = start_time;
            LOG("[TIME.Reset] av {} cv {}", std::to_string(wrk_value_time), std::to_string(commit_wrk_value_time));
        }
        void Commit() override {
            commit_wrk_value_time = wrk_value_time;
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

        void ToMove() override;

        void Add(float c_add_value) override {
            wrk_value_add += c_add_value;
            LOG("[Amount.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_add), std::to_string(c_add_value),
                std::to_string(commit_wrk_value_add));
        }

        void Subtract(float subtract_value) override {
            if (subtract_value > 0.0f && wrk_value_add - subtract_value >= 0.0f) {
                wrk_value_add -= subtract_value;
                LOG("[Amount.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
                    std::to_string(subtract_value), std::to_string(commit_wrk_value_add));

            } else if (subtract_value > 0.0f) {
                wrk_value_add = 0.0f;
                LOG("[Amount.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
                    std::to_string(subtract_value), std::to_string(commit_wrk_value_add));
            }
        }

        void Div(float div_value) override {
            if (div_value != 0.0f) {
                wrk_value_add /= div_value;
                LOG("[Amount.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_add), std::to_string(div_value),
                    std::to_string(commit_wrk_value_add));
            } else {
                LOG("[Amount.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
                    std::to_string(wrk_value_add), std::to_string(div_value), std::to_string(commit_wrk_value_add));
            }
        }

        void Mult(float mult_value) override {
            wrk_value_add *= mult_value;
            LOG("[Amount.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_add), std::to_string(mult_value),
                std::to_string(commit_wrk_value_add));
        }

        bool IsEpsilon() override { return fabsf(wrk_value_add - commit_wrk_value_add) < FLT_EPSILON; }

        float CompPercent() override {
            if (target_amount == 0.0f) return 100.0f;

            return (commit_wrk_value_add / target_amount) * 100.0f;
        }

        void Reset() override {
            commit_wrk_value_add = 0.0f;
            wrk_value_add = 0.0f;
            cool_time_now = false;
            LOG("[Amount.Reset] av {} cv {}", std::to_string(wrk_value_add), std::to_string(commit_wrk_value_add));
        }
        void Commit() override {
            commit_wrk_value_add = wrk_value_add;
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

    // class KMCCCAddKeyword : public KMCCustomCondTaskHub {
    // public:
    //     std::unique_ptr<KMCCustomCondTaskHub> GetDetail(CutinCondType cct) override {
    //         if (cct == CutinCondType::add_keyword) {
    //             return std::make_unique<KMCCCAddKeyword>();
    //         }
    //         return nullptr;
    //     }

    //    void ToMove() override {
    //        if (cct_sub == CutinCondType::add) {

    //            if (cool_time > 0.0f) {
    //                if (cool_time_now) {
    //                    time_point<Clock> now = Clock::now();
    //                    milliseconds diff = duration_cast<milliseconds>(now - timer);
    //                    long long dur = diff.count();
    //                    if (dur >= cool_time * KMCCT::TIME_SCALE_MS) {
    //                        cool_time_now = false;
    //                    } else {
    //                        LOG("[ADD.ToMove] cool time av {}, dur(ms) {}, cool time(ms) {}",
    //                            std::to_string(wrk_value_add), dur, cool_time * KMCCT::TIME_SCALE_MS);
    //                        return;
    //                    }
    //                }

    //                if (!cool_time_now) {
    //                    timer = Clock::now();
    //                    cool_time_now = true;
    //                }
    //            }

    //            if (coef_relation == "between") {
    //                std::uniform_real_distribution<float> get_rand_uni_real(coef_1, coef_2);
    //                float r = get_rand_uni_real(mt);
    //                wrk_value_add += add_value * r;
    //            } else if (coef_relation == "coef_1") {
    //                wrk_value_add += add_value * coef_1;
    //            } else if (coef_relation == "coef_2") {
    //                wrk_value_add += add_value * coef_2;
    //            }

    //            LOG("[AddKeyword.ADD.ToMove] av {}", std::to_string(wrk_value_add));
    //        } else {
    //            // time
    //            wrk_value_time += add_time_v;
    //            LOG("[AddKeyword.TIME.ToMove] av {}", std::to_string(wrk_value_time));
    //        }
    //    }

    //    void Add(float c_add_value) override {
    //        if (cct_sub == CutinCondType::add) {
    //            wrk_value_add += c_add_value;
    //            LOG("[AddKeyword.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_add),
    //            std::to_string(c_add_value),
    //                std::to_string(commit_wrk_value_add));
    //        } else {
    //            wrk_value_time += c_add_value;
    //            LOG("[AddKeyword.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_time),
    //            std::to_string(c_add_value),
    //                std::to_string(commit_wrk_value_time));
    //        }
    //    }

    //    void Subtract(float subtract_value) override {
    //        if (cct_sub == CutinCondType::add) {
    //            if (subtract_value > 0.0f && wrk_value_add - subtract_value >= 0.0f) {
    //                wrk_value_add -= subtract_value;
    //                LOG("[AddKeyword.ADD.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
    //                    std::to_string(subtract_value), std::to_string(commit_wrk_value_add));
    //            } else if (subtract_value > 0.0f) {
    //                wrk_value_add = 0.0f;
    //                LOG("[AddKeyword.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
    //                    std::to_string(subtract_value), std::to_string(commit_wrk_value_add));
    //            }
    //        } else {
    //            if (subtract_value > 0.0f && wrk_value_time - subtract_value >= 0.0f) {
    //                wrk_value_time -= subtract_value;
    //                LOG("[AddKeyword.TIME.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_time),
    //                    std::to_string(subtract_value), std::to_string(commit_wrk_value_time));
    //            } else if (subtract_value > 0.0f) {
    //                wrk_value_time = 0.0f;
    //                LOG("[AddKeyword.TIME.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_time),
    //                    std::to_string(subtract_value), std::to_string(commit_wrk_value_time));
    //            }
    //        }
    //    }

    //    void Div(float div_value) override {
    //        if (cct_sub == CutinCondType::add) {
    //            if (div_value != 0.0f) {
    //                wrk_value_add /= div_value;
    //                LOG("[AddKeyword.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_add),
    //                std::to_string(div_value),
    //                    std::to_string(commit_wrk_value_add));
    //            } else {
    //                LOG("[AddKeyword.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
    //                    std::to_string(wrk_value_add), std::to_string(div_value),
    //                    std::to_string(commit_wrk_value_add));
    //            }
    //        } else {
    //            if (div_value != 0.0f) {
    //                wrk_value_time /= div_value;
    //                LOG("[AddKeyword.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_time),
    //                std::to_string(div_value),
    //                    std::to_string(commit_wrk_value_time));
    //            } else {
    //                LOG("[AddKeyword.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
    //                    std::to_string(wrk_value_time), std::to_string(div_value),
    //                    std::to_string(commit_wrk_value_time));
    //            }
    //        }
    //    }

    //    void Mult(float mult_value) override {
    //        if (cct_sub == CutinCondType::add) {
    //            wrk_value_add *= mult_value;
    //            LOG("[AddKeyword.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_add),
    //            std::to_string(mult_value),
    //                std::to_string(commit_wrk_value_add));
    //        } else {
    //            wrk_value_time *= mult_value;
    //            LOG("[AddKeyword.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_time),
    //            std::to_string(mult_value),
    //                std::to_string(commit_wrk_value_time));
    //        }
    //    }

    //    bool IsEpsilon() override {
    //        if (cct_sub == CutinCondType::add) {
    //            return fabsf(wrk_value_add - commit_wrk_value_add) < FLT_EPSILON;
    //        } else {
    //            return fabsf(wrk_value_time - commit_wrk_value_time) < FLT_EPSILON;
    //        }
    //    }

    //    void Reset() override {
    //        if (cct_sub == CutinCondType::add) {
    //            commit_wrk_value_add = cutin_lower_value;
    //            wrk_value_add = cutin_lower_value;
    //            cool_time_now = false;
    //            LOG("[AddKeyword.ADD.Reset] av {} cv {}", std::to_string(wrk_value_add),
    //            std::to_string(commit_wrk_value_add));
    //        } else {
    //            commit_wrk_value_time = start_time;
    //            wrk_value_time = start_time;
    //            LOG("[AddKeyword.TIME.Reset] av {} cv {}", std::to_string(wrk_value_time),
    //            std::to_string(commit_wrk_value_time));
    //        }
    //    }
    //    void Commit() override {
    //        if (cct_sub == CutinCondType::add) {
    //            commit_wrk_value_add = wrk_value_add;
    //            LOG("[AddKeyword.ADD.Commit] cv {}, av {}", std::to_string(commit_wrk_value_add),
    //            std::to_string(wrk_value_add));
    //        } else {
    //            commit_wrk_value_time = wrk_value_time;
    //            LOG("[AddKeyword.TIME.Commit] cv {}, av {}", std::to_string(commit_wrk_value_time),
    //                std::to_string(wrk_value_time));
    //        }
    //    }

    //    bool Completed() override {
    //        if (cct_sub == CutinCondType::add) {
    //            if (commit_wrk_value_add >= upper_value) {
    //                LOG("[AddKeyword.ADD.Completed] cv {}, av {}", std::to_string(commit_wrk_value_add),
    //                    std::to_string(wrk_value_add));
    //                return true;
    //            }

    //            return false;
    //        } else {
    //            if (commit_wrk_value_time >= end_time) {
    //                LOG("[AddKeyword.TIME.Completed] cv {}, av {}", std::to_string(commit_wrk_value_time),
    //                    std::to_string(end_time));
    //                return true;
    //            }

    //            return false;
    //        }

    //        return false;
    //    }
    //};

    // class KMCCCRemoveKeyword : public KMCCustomCondTaskHub {
    // public:
    //     std::unique_ptr<KMCCustomCondTaskHub> GetDetail(CutinCondType cct) override {
    //         if (cct == CutinCondType::remove_keyword) {
    //             return std::make_unique<KMCCCRemoveKeyword>();
    //         }
    //         return nullptr;
    //     }
    //     void ToMove() override {
    //         if (cct_sub == CutinCondType::add) {
    //             if (coef_relation == "between") {
    //                 std::uniform_real_distribution<float> get_rand_uni_real(coef_1, coef_2);
    //                 float r = get_rand_uni_real(mt);
    //                 wrk_value_add += add_value * r;
    //             } else if (coef_relation == "coef_1") {
    //                 wrk_value_add += add_value * coef_1;
    //             } else if (coef_relation == "coef_2") {
    //                 wrk_value_add += add_value * coef_2;
    //             }

    //            LOG("[RemoveKeyword.ADD.ToMove] av {}", std::to_string(wrk_value_add));
    //        } else {
    //            // time
    //            wrk_value_time += add_time_v;
    //            LOG("[RemoveKeyword.TIME.ToMove] av {}", std::to_string(wrk_value_time));
    //        }
    //    }
    //    void Add(float c_add_value) override {
    //        if (cct_sub == CutinCondType::add) {
    //            wrk_value_add += c_add_value;
    //            LOG("[RemoveKeyword.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_add),
    //            std::to_string(c_add_value),
    //                std::to_string(commit_wrk_value_add));
    //        } else {
    //            wrk_value_time += c_add_value;
    //            LOG("[RemoveKeyword.Add] av {}, addv {}, rv {}", std::to_string(wrk_value_time),
    //            std::to_string(c_add_value),
    //                std::to_string(commit_wrk_value_time));
    //        }
    //    }

    //    void Subtract(float subtract_value) override {
    //        if (cct_sub == CutinCondType::add) {
    //            if (subtract_value > 0.0f && wrk_value_add - subtract_value >= 0.0f) {
    //                wrk_value_add -= subtract_value;
    //                LOG("[RemoveKeyword.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
    //                    std::to_string(subtract_value), std::to_string(commit_wrk_value_add));
    //            } else if (subtract_value > 0.0f) {
    //                wrk_value_add = 0.0f;
    //                LOG("[RemoveKeyword.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_add),
    //                    std::to_string(subtract_value), std::to_string(commit_wrk_value_add));
    //            }
    //        } else {
    //            if (subtract_value > 0.0f && wrk_value_time - subtract_value >= 0.0f) {
    //                wrk_value_time -= subtract_value;
    //                LOG("[RemoveKeyword.Subtract] av {}, subtractv {}, rv {}", std::to_string(wrk_value_time),
    //                    std::to_string(subtract_value), std::to_string(commit_wrk_value_time));
    //            } else if (subtract_value > 0.0f) {
    //                wrk_value_time = 0.0f;
    //                LOG("[RemoveKeyword.Subtract.reset] av {}, subtractv {}, rv {}", std::to_string(wrk_value_time),
    //                    std::to_string(subtract_value), std::to_string(commit_wrk_value_time));
    //            }
    //        }
    //    }

    //    void Div(float div_value) override {
    //        if (cct_sub == CutinCondType::add) {
    //            if (div_value != 0.0f) {
    //                wrk_value_add /= div_value;
    //                LOG("[RemoveKeyword.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_add),
    //                    std::to_string(div_value), std::to_string(commit_wrk_value_add));
    //            } else {
    //                LOG("[RemoveKeyword.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
    //                    std::to_string(wrk_value_add), std::to_string(div_value),
    //                    std::to_string(commit_wrk_value_add));
    //            }
    //        } else {
    //            if (div_value != 0.0f) {
    //                wrk_value_time /= div_value;
    //                LOG("[RemoveKeyword.Div] av {}, divv {}, rv {}", std::to_string(wrk_value_time),
    //                    std::to_string(div_value), std::to_string(commit_wrk_value_time));
    //            } else {
    //                LOG("[RemoveKeyword.Div] skip div(Reason: For division by zero) av {}, divv {}, rv {}",
    //                    std::to_string(wrk_value_time), std::to_string(div_value),
    //                    std::to_string(commit_wrk_value_time));
    //            }
    //        }
    //    }

    //    void Mult(float mult_value) override {
    //        if (cct_sub == CutinCondType::add) {
    //            wrk_value_add *= mult_value;
    //            LOG("[RemoveKeyword.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_add),
    //                std::to_string(mult_value), std::to_string(commit_wrk_value_add));
    //        } else {
    //            wrk_value_time *= mult_value;
    //            LOG("[RemoveKeyword.Mult] av {}, multv {}, rv {}", std::to_string(wrk_value_time),
    //                std::to_string(mult_value), std::to_string(commit_wrk_value_time));
    //        }
    //    }

    //    bool IsEpsilon() override {
    //        if (cct_sub == CutinCondType::add) {
    //            return fabsf(wrk_value_add - commit_wrk_value_add) < FLT_EPSILON;
    //        } else {
    //            return fabsf(wrk_value_time - commit_wrk_value_time) < FLT_EPSILON;
    //        }
    //    }

    //    void Reset() override {
    //        if (cct_sub == CutinCondType::add) {
    //            commit_wrk_value_add = cutin_lower_value;
    //            wrk_value_add = cutin_lower_value;
    //            LOG("[RemoveKeyword.ADD.Reset] av {} cv {}", std::to_string(wrk_value_add),
    //                std::to_string(commit_wrk_value_add));
    //        } else {
    //            commit_wrk_value_time = start_time;
    //            wrk_value_time = start_time;
    //            LOG("[RemoveKeyword.TIME.Reset] av {} cv {}", std::to_string(wrk_value_time),
    //                std::to_string(commit_wrk_value_time));
    //        }
    //    }
    //    void Commit() override {
    //        if (cct_sub == CutinCondType::add) {
    //            commit_wrk_value_add = wrk_value_add;
    //            LOG("[RemoveKeyword.ADD.Commit] cv {}, av {}", std::to_string(commit_wrk_value_add),
    //                std::to_string(wrk_value_add));
    //        } else {
    //            commit_wrk_value_time = wrk_value_time;
    //            LOG("[RemoveKeyword.TIME.Commit] cv {}, av {}", std::to_string(commit_wrk_value_time),
    //                std::to_string(wrk_value_time));
    //        }
    //    }

    //    bool Completed() override {
    //        if (cct_sub == CutinCondType::add) {
    //            if (commit_wrk_value_add >= upper_value) {
    //                LOG("[RemoveKeyword.ADD.Completed] cv {}, av {}", std::to_string(commit_wrk_value_add),
    //                    std::to_string(wrk_value_add));
    //                return true;
    //            }

    //            return false;
    //        } else {
    //            if (commit_wrk_value_time >= end_time) {
    //                LOG("[RemoveKeyword.TIME.Completed] cv {}, av {}", std::to_string(commit_wrk_value_time),
    //                    std::to_string(end_time));
    //                return true;
    //            }

    //            return false;
    //        }

    //        return false;
    //    }
    //};

    // class KMCCustomCondTaskHub : public KMCCCAdd, public KMCCCTime {
    // public:
    //     KMCCustomCondTaskHub() {}

    // public:
    // };

    bool GetWorkDetail(CutinCondType cct, std::unique_ptr<KMCCustomCondTaskHub>* hub);
}