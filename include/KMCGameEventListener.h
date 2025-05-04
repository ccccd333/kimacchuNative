#pragma once
#include "KMCUtility.h"



namespace KMCCT {

    class KMCGameEventListener {
        SINGLETONHEADER(KMCGameEventListener)
    public:
        ~KMCGameEventListener();

        void SetCallBack(std::function<void(const RE::TESContainerChangedEvent*)> callback);
        void SetCallBack(std::function<void(const RE::TESHitEvent*)> callback);
        void SetCallBack(std::function<void(const RE::TESFastTravelEndEvent*)> callback);
        void SetCallBack(std::function<void(const RE::MenuOpenCloseEvent*)> callback);
        void SetCallBack(std::function<void(const RE::TESDeathEvent*)> callback);

        void Init();
    private:

    };
}