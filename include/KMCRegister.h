#pragma once

namespace KMCCT {
    class KMCRegister {
        SINGLETONHEADER(KMCRegister)
    public:
        ~KMCRegister() {}
        
    };

    void OnMessageReceived(SKSE::MessagingInterface::Message* a_msg);
    bool PapyrusRegister(RE::BSScript::IVirtualMachine * vm);
};