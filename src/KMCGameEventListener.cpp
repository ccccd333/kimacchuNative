#include "KMCGameEventListener.h"

#include <boost/any.hpp>

#include "KMCConfig.h"

SINGLETONBODY(KMCCT::KMCGameEventListener)

namespace KMCCT {

    std::vector<std::function<void(const RE::TESContainerChangedEvent*)>> callback_container_change_event;
    std::vector<std::function<void(const RE::TESHitEvent*)>> callback_hit_event;
    std::vector<std::function<void(const RE::TESFastTravelEndEvent*)>> callback_fast_travel_end_event;
    std::vector<std::function<void(const RE::MenuOpenCloseEvent*)>> callback_menu_open_close_event;
    std::vector<std::function<void(const RE::TESDeathEvent*)>> callback_death_event_event;

    void KMCGameEventListener::Init() {
        if (callback_container_change_event.size() > 0) {
            On<RE::TESContainerChangedEvent>([](const RE::TESContainerChangedEvent* event) {
                for (auto callback : callback_container_change_event) {
                    callback(event);
                }
            });
        }

        if (callback_hit_event.size() > 0) {
            On<RE::TESHitEvent>([](const RE::TESHitEvent* event) {
                for (auto callback : callback_hit_event) {
                    callback(event);
                }
            });
        }

        if (callback_fast_travel_end_event.size() > 0) {
            On<RE::TESFastTravelEndEvent>([](const RE::TESFastTravelEndEvent* event) {
                for (auto callback : callback_fast_travel_end_event) {
                    callback(event);
                }
            });
        }

        if (callback_menu_open_close_event.size() > 0) {
            OnUI<RE::MenuOpenCloseEvent>([](const RE::MenuOpenCloseEvent* event) {
                for (auto callback : callback_menu_open_close_event) {
                    callback(event);
                }
            });
        }

        if (callback_death_event_event.size() > 0) {
            On<RE::TESDeathEvent>([](const RE::TESDeathEvent* event) {
                for (auto callback : callback_death_event_event) {
                    callback(event);
                }
            });
        }
    }

    void KMCGameEventListener::SetCallBack(std::function<void(const RE::TESContainerChangedEvent*)> callback) {
        callback_container_change_event.emplace_back(callback);
    }
    void KMCGameEventListener::SetCallBack(std::function<void(const RE::TESHitEvent*)> callback) {
        callback_hit_event.emplace_back(callback);
    }
    void KMCGameEventListener::SetCallBack(std::function<void(const RE::TESFastTravelEndEvent*)> callback) {
        callback_fast_travel_end_event.emplace_back(callback);
    }
    void KMCGameEventListener::SetCallBack(std::function<void(const RE::MenuOpenCloseEvent*)> callback) {
        callback_menu_open_close_event.emplace_back(callback);
    }
    void KMCGameEventListener::SetCallBack(std::function<void(const RE::TESDeathEvent*)> callback) {
        callback_death_event_event.emplace_back(callback);
    }
}