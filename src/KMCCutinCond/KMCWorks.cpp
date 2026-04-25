#include <boost/any.hpp>

#include "KMCCutinCond/KMCWorks.h"
#include "KMCGameEventListener.h"

namespace KMCCT {
    KMCCCAdd km_add;
    KMCCCTime km_time;
    KMCCCAmount km_amount;
    KMCCCDummy dummy;
    //KMCCCAddKeyword km_add_keyword;
    //KMCCCRemoveKeyword km_remove_keyword;

    std::vector<KMCCustomCondTaskHub *> Tasks = {&km_add, &km_time, &km_amount, &dummy};

    bool GetWorkDetail(CutinCondType cct, std::unique_ptr<KMCCustomCondTaskHub> *hub) {
        for (auto t : Tasks) {
            *hub = t->GetDetail(cct);
            if (*hub) {
                return true;
            }
        }

        return false;
    }

    bool KMCCCAmount::Init(KMCCustomCondCheckHub *check, KMCCCheckSource *source) {
        check_hub = check;
        csource = source;
        KMCCT::KMCGameEventListener::GetSingleton()->SetCallBack(
            [this](const RE::TESContainerChangedEvent *event) { this->AddItemEvent(event); });
        return true;
    }

    void KMCCCAmount::AddItemEvent(const RE::TESContainerChangedEvent *event) {
        if (event && event->newContainer == 0x14 && event->oldContainer != 0x00) {
            RE::TESForm *root = RE::TESForm::LookupByID(event->baseObj);
            if (root) {
                auto get_amount = root->GetGoldValue();
                auto get_item_name = root->GetName();
                

                if (get_amount <= abandon_amount) {
                    return;
                }

                if (!csource || !check_hub || !check_hub->Check(*csource)) {
                    return;
                }

                KMC_LOG("KMCCCAmount::AddItemEvent item {} amount {} target amount {} abandon amount {}", get_item_name,
                    get_amount, target_amount, abandon_amount);

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (get_amount + amount >= stack_limit) {
                        amount = stack_limit;
                    } else {
                        amount += get_amount;
                    }
                }
            }
        }
    }

    void KMCCCAmount::ToMove() {
        if (cool_time > 0.0f) {
            if (cool_time_now) {
                time_point<Clock> now = Clock::now();
                milliseconds diff = duration_cast<milliseconds>(now - timer);
                long long dur = diff.count();
                if (dur >= cool_time * KMCCT::TIME_SCALE_MS) {
                    cool_time_now = false;
                } else {
                    KMC_LOG("[KMCCCAmount.ToMove] cool time av {}, dur(ms) {}, cool time(ms) {}",
                        std::to_string(wrk_value_add), dur, cool_time * KMCCT::TIME_SCALE_MS);
                    return;
                }
            }

            if (!cool_time_now) {
                timer = Clock::now();
                cool_time_now = true;
            }
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            wrk_value_add = amount;
            amount = 0.0f;
            KMC_LOG("[Amount.ToMove] av {}", std::to_string(wrk_value_add));
        }
    }
}