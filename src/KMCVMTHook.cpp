//#include "KMCVMTHook.h"
//#include "IWWFunctions.h"
//
//namespace KMCCT {
//
//    namespace vm_hook {
//
//        void KMCVMTHook::install(RE::BSScript::IVirtualMachine* a_vm) {
//            bind_native_function =
//                (bind_native_function_t)HookMethod((LPVOID)a_vm, (PVOID)bind_native_function_hook, 24 * sizeof(void*));
//        }
//
//        bool KMCVMTHook::bind_native_function_hook([[maybe_unused]] RE::BSScript::IVirtualMachine* self,
//                                                RE::BSScript::IFunction* a_fn) {
//            bool result;
//
//            auto addr = (char*)&(*a_fn) + 0x50;
//            const auto callback = *reinterpret_cast<std::uintptr_t*>(addr);
//            LOG("KMCVMTHook::bind_native_function_hook object : {}, name : {} callback : {}", a_fn->GetObjectTypeName().c_str(), a_fn->GetName().c_str(), callback);
//            result = bind_native_function(self, a_fn);
//
//            return result;
//        }
//
//        KMCVMTHook::bind_native_function_t KMCVMTHook::bind_native_function = nullptr;
//    }
//}