#include "KMCStorageUtilTracker.h"

#include <MinHook.h>
#include <Windows.h>

#include "KMCConfig.h"
#include "RE/V/VirtualMachine.h"

namespace KMCCT {

    //std::unordered_map<std::string, std::map<uint64_t, VMObjectHandleInfo>> StorageUtilTracker::key_to_handles;
    std::list<StorageObservedValue> StorageUtilTracker::watch_targets;
    //std::set<std::string> StorageUtilTracker::watched_keys;
    //std::set<uint64_t> StorageUtilTracker::watch_strage_util_key_ids;
    //std::mutex StorageUtilTracker::key_handle_map_mutex;

    void StorageUtilTracker::Init() {
        if (InstallStorageUtilHooks()) {
            auto stu = *KMCCT::KMCConfig::GetSingleton()->getIDetectionStorageUtil();
            for (int i = 0; i < stu.size(); i++) {
                std::string k = stu[i].first;
                std::string v = stu[i].second;

                auto spvalue = KMCSplit(v, ',');
                const auto& key = spvalue.at(0);

                StorageObservedValue target{};
                //-------------------------debug 用の一時的な設定---------------
                // 本来はCondition.jsonの内容から読み取ること
                auto form_id = RE::TESDataHandler::GetSingleton()->LookupForm(std::stoll("20", NULL, 16), "Skyrim.esm");
                if (form_id) {

                    auto handle = BuildHandleFromStackPointer(form_id);

                    //auto strage_util_id = StorageUtilCalcID(form_id);
                    //target.strage_util_key_id = strage_util_id;
                    target.strage_key_name = key;
                    target.compare_value.SetValue("int", "1");
                    target.vm_object = handle;
                    
                    //key_to_handles.emplace(key, std::map<uint64_t, VMObjectHandleInfo>{});
                    watch_targets.push_back(target);
                    //watched_keys.insert(key);
                    //watch_strage_util_key_ids.insert(strage_util_id);
                    //-------------------------debug 用の一時的な設定---------------
                } else {
                    SKSE::log::info("StorageUtil RE::TESDataHandler::GetSingleton()->LookupForm not found. ");
                }
            }
        }
    }

    void StorageUtilTracker::FetchAllValues(std::list<StorageObservedValue>& result) {
        //std::unordered_map<std::string, std::map<uint64_t, VMObjectHandleInfo>> calc_base_map{};
        //for (auto k : watched_keys) {
        //    auto vm_handle_map = CopyKeyHandleMap(k);
        //    if (!vm_handle_map.empty()) {
        //        calc_base_map.emplace(k, vm_handle_map);
        //    }
        //}

        for (auto k : watch_targets) {
            //if (calc_base_map.contains(k.strage_key_name)) {
            StorageObservedValue monitor{.strage_key_name = k.strage_key_name, .vm_object = k.vm_object, .has_value = false};
                //const auto& handle_map = calc_base_map.at(k.strage_key_name);

                //if (handle_map.contains(k.strage_util_key_id)) {
                //    auto& handle = handle_map.at(k.strage_util_key_id);
                    
                    switch (k.compare_value.value_type) {
                        case KMCValueType::KM_INT: {
                            int result_int = 0;
                            if (GetIntValue(k.strage_key_name, k.vm_object, result_int)) {
                                monitor.compare_value.SetIntValue(result_int);
                                monitor.has_value = true;
                                result.push_back(monitor);
                            }
                            break;
                        }
                        case KMCValueType::KM_FLOAT: {
                            float result_float = 0;
                            if (GetFloatValue(k.strage_key_name, k.vm_object, result_float)) {
                                monitor.compare_value.SetFloatValue(result_float);
                                monitor.has_value = true;
                                result.push_back(monitor);
                            }
                            break;
                        }
                        case KMCValueType::KM_STRING: {
                            std::string result_string = "";
                            if (GetStringValue(k.strage_key_name, k.vm_object, result_string)) {
                                monitor.compare_value.SetStringValue(result_string);
                                monitor.has_value = true;
                                result.push_back(monitor);
                            }
                            break;
                        }
                    }
                    
               // }
            
            //}
        }
    }

    bool StorageUtilTracker::InstallStorageUtilHooks() {
        auto base = (uintptr_t)GetModuleHandleA("PapyrusUtil.dll");

        if (!base) {
            SKSE::log::error("PapyrusUtil.dll not found");
            return false;
        }

        // SET(Int, Float, String ... )

        // Ghidra Image Base: 0x180000000
        // FUN_18009c090
        // FUN_18009c240
        // FUN_18009c400
        //uintptr_t addr_set_int = base + 0x9C090;
        //uintptr_t addr_set_float = base + 0x9C240;
        //uintptr_t addr_set_string = base + 0x9C400;

        if (MH_Initialize() != MH_OK) {
            SKSE::log::info("MH_Initialize failed");
            return false;
        }

        //// JMP r/m64
        //// SKSE間の絶対ジャンプ
        //MH_CreateHook((LPVOID)addr_set_int, &HookSetInt, (LPVOID*)&_original_set_int);
        //MH_CreateHook((LPVOID)addr_set_float, &HookSetFloat, (LPVOID*)&_original_set_float);
        //MH_CreateHook((LPVOID)addr_set_string, &HookSetString, (LPVOID*)&_original_set_string);

        //MH_EnableHook((LPVOID)addr_set_int);
        //MH_EnableHook((LPVOID)addr_set_float);
        //MH_EnableHook((LPVOID)addr_set_string);

        // GET(Int, Float, String ... ) Func Pointer

        uintptr_t addr_get_int = base + 0x9C6D0;
        uintptr_t addr_get_float = base + 0x9C7B0;
        uintptr_t addr_get_string = base + 0x9C890;

        _get_int = (get_int_t)(addr_get_int);
        _get_float = (get_float_t)(addr_get_float);
        _get_string = (get_string_t)(addr_get_string);

        uintptr_t addr_has_int = base + 0x9D2F0;
        uintptr_t addr_has_float = base + 0x9D3C0;
        uintptr_t addr_has_string = base + 0x9D490;

        _has_int = (has_value_t)addr_has_int;
        _has_float = (has_value_t)addr_has_float;
        _has_string = (has_value_t)addr_has_string;

        //// GET(Int, Float, String ... )
        //// JMP r/m64
        //// SKSE間の絶対ジャンプ
        //MH_CreateHook((LPVOID)addr_get_int, &HookGetInt, (LPVOID*)&_original_get_int);
        //MH_CreateHook((LPVOID)addr_get_float, &HookGetFloat, (LPVOID*)&_original_get_float);
        //MH_CreateHook((LPVOID)addr_get_string, &HookGetString, (LPVOID*)&_original_get_string);

        //MH_EnableHook((LPVOID)addr_get_int);
        //MH_EnableHook((LPVOID)addr_get_float);
        //MH_EnableHook((LPVOID)addr_get_string);

        //// フックできた場合にトランポリン先を設定
        //_get_int = _original_get_int;
        //_get_float = _original_get_float;
        //_get_string = _original_get_string;

        SKSE::log::info("StorageUtil hooks installed");

        return true;
    }

    bool StorageUtilTracker::GetIntValue(std::string key, VMObjectHandleInfo vm_handle_info,
                                        int& result) {
        if (!_get_int || !_has_int) return false;

        bool is_available = true;
        void* form = ResolveForm(vm_handle_info, is_available);
        if (!is_available) {
            return false;
        }
        auto key_cstr = key.c_str();

        if (_has_int(nullptr, form, key_cstr)) {
            result = _get_int(nullptr, form, key.c_str(), 0);
            return true;
        }
        return false;
    }

    bool StorageUtilTracker::GetFloatValue(std::string key, VMObjectHandleInfo vm_handle_info,
                                          float& result) {
        if (!_get_float) return false;

        bool is_available = true;
        void* form = ResolveForm(vm_handle_info, is_available);
        if (!is_available) {
            return false;
        }
        auto key_cstr = key.c_str();
        if (_has_float(nullptr, form, key_cstr)) {
            result = _get_float(nullptr, form, key_cstr, 0.0);

            return true;
        }
        return false;
    }

    bool StorageUtilTracker::GetStringValue(std::string key, VMObjectHandleInfo vm_handle_info, std::string& result) {
        if (!_get_string) return false;
        bool is_available = true;
        std::string default_val = "<null>";
        void* ret = nullptr; 

        void* form = ResolveForm(vm_handle_info, is_available);
        if (!is_available) {
            return false;
        }
        auto key_cstr = key.c_str();
        if (_has_string(nullptr, form, key_cstr)) {
            result = _get_string(ret, nullptr, form, key_cstr, default_val.c_str());

            return true;
        }
        return false;
    }

    //int StorageUtilTracker::HookGetInt(void* StaticFunctionTag, void* stack_id, const char* key, int default_value) {
    //    CaptureHandleForKey(key, stack_id);

    //    return _original_get_int(StaticFunctionTag, stack_id, key, default_value);
    //}
    //float StorageUtilTracker::HookGetFloat(void* StaticFunctionTag, void* stack_id, const char* key,
    //                                      float default_value) {
    //    CaptureHandleForKey(key, stack_id);

    //    return _original_get_float(StaticFunctionTag, stack_id, key, default_value);
    //}
    //const char* StorageUtilTracker::HookGetString(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
    //                                             const char* default_value) {
    //    CaptureHandleForKey(key, stack_id);

    //    return _original_get_string(ret, StaticFunctionTag, stack_id, key, default_value);
    //}

    //int StorageUtilTracker::HookSetInt(void* StaticFunctionTag, void* stack_id, const char* key, int value) {
    //    CaptureHandleForKey(key, stack_id);

    //    return _original_set_int(StaticFunctionTag, stack_id, key, value);
    //}

    //float StorageUtilTracker::HookSetFloat(void* StaticFunctionTag, void* stack_id, const char* key, float value) {
    //    CaptureHandleForKey(key, stack_id);

    //    return _original_set_float(StaticFunctionTag, stack_id, key, value);
    //}

    //void* StorageUtilTracker::HookSetString(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
    //                                       const char* value) {
    //    CaptureHandleForKey(key, stack_id);

    //    return _original_set_string(ret, StaticFunctionTag, stack_id, key, value);
    //}

    VMObjectHandleInfo StorageUtilTracker::BuildHandleFromStackPointer(RE::TESForm* tes_form) {
        // auto start = std::chrono::high_resolution_clock::now();

        VMObjectHandleInfo handle{.handle = 0, .form_type = RE::FormType::None, .strage_util_key_id = 0, .form_id = 0};
        if (tes_form) {
            RE::VMHandle resolved_handle = 0;
            RE::FormType form_type;

            auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
            auto policy = vm->GetObjectHandlePolicy();
            if (!policy) {
                SKSE::log::info("[BuildHandle] invalid input stack_id={} policy={}", (void*)tes_form, (void*)policy);
                return handle;
            }

            //auto form = reinterpret_cast<RE::TESForm*>(stack_id);

            // Formで来るのでSEHいらない
            //__try {
            form_type = tes_form->GetFormType();
            resolved_handle = policy->GetHandleForObject(form_type, tes_form);
            //} __except (EXCEPTION_EXECUTE_HANDLER) {
            //    SKSE::log::error("[BuildHandle] EXCEPTION reading form. stack id = {}", stack_id);
            //    return handle;
            //}

            if (!policy->IsHandleObjectAvailable(resolved_handle)) {
                SKSE::log::info("[BuildHandle] handle invalid. resolved_handle = {}", resolved_handle);
            }
            handle.handle = resolved_handle;
            handle.form_type = form_type;
            handle.form_id = tes_form->GetFormID();
        }

        // auto end = std::chrono::high_resolution_clock::now();
        // auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        // SKSE::log::info("[BuildHandle] time={} ns", ns);

        return handle;
    }

    //void StorageUtilTracker::CaptureHandleForKey(const char* key, void* stack_id) {
    //    if (key) {
    //        // メインスレッド側を全体保護する。
    //        std::lock_guard<std::mutex> guard(key_handle_map_mutex);

    //        auto it = key_to_handles.find(key);
    //        if (it != key_to_handles.end()) {
    //            // strage utilは内部で32bit + 32bit(form)の計算をしてマップしてる
    //            // npcなどは一時的なTESForm*しか作らない消えたら参照ごと消えるので
    //            // VMHandleを持つ
    //            // 有効なVMHandleからTESForm*を作り直しDLL側からStrageUtilのGetを扱う
    //            // policy->GetHandleForObject平均4000nsなので同じものはスキップする
    //            auto strage_util_key_id = StorageUtilCalcID(stack_id);
    //            if (it->second.contains(strage_util_key_id)) return;

    //            // 監視対象でないなら捨てる
    //            if (!watch_strage_util_key_ids.contains(strage_util_key_id)) return;
    //            auto handle_info = BuildHandleFromStackPointer(stack_id);
    //            handle_info.strage_util_key_id = strage_util_key_id;
    //            it->second[handle_info.strage_util_key_id] = handle_info;
    //        }
    //    }
    //}

    void* StorageUtilTracker::ResolveForm(VMObjectHandleInfo handle, bool& is_available) {
        is_available = true;
        auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        auto policy = vm->GetObjectHandlePolicy();

        if (!policy) {
            SKSE::log::info("[ResolveForm] invalid input policy={}", (void*)policy);
            return nullptr;
        }

        if (!policy->IsHandleObjectAvailable(handle.handle)) {
            SKSE::log::info("[ResolveForm] handle invalid. handle = {}", handle.handle);
            is_available = false;
            return nullptr;
        }

        return policy->GetObjectForHandle(handle.form_type, handle.handle);
    }

    //std::map<uint64_t, VMObjectHandleInfo> StorageUtilTracker::CopyKeyHandleMap(const std::string& key) {
    //    std::map<uint64_t, VMObjectHandleInfo> local_map;
    //    std::lock_guard<std::mutex> guard(key_handle_map_mutex);

    //    auto it = key_to_handles.find(key);
    //    if (it != key_to_handles.end()) {
    //        local_map = it->second;
    //    }

    //    return local_map;
    //}
}