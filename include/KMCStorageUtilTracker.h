#pragma once
#include "KMCUtility.h"
namespace KMCCT {

    struct VMObjectHandleInfo {
        RE::VMHandle handle;
        RE::FormType form_type;
        uint64_t strage_util_key_id;
        RE::FormID form_id;
    };

    struct StorageObservedValue {
        std::string strage_key_name;
        MultiTypeValue compare_value;
        VMObjectHandleInfo vm_object;
        bool has_value = false;
    };

    class StorageUtilTracker {
    public:
        static void Init();

        static void FetchAllValues(std::list<StorageObservedValue>& result);

        static bool GetIntValue(std::string strage_key_name, VMObjectHandleInfo vm_handle_info, int& result);

        static bool GetFloatValue(std::string strage_key_name, VMObjectHandleInfo vm_handle_info, float& result);

        static bool GetStringValue(std::string strage_key_name, VMObjectHandleInfo vm_handle_info, std::string& result);

        static VMObjectHandleInfo BuildHandleFromStackPointer(RE::TESForm* tes_form);

    private:
        //using set_number_t = int (*)(void* StaticFunctionTag, void* stack_id, const char* key, int value);
        //using set_float_t = float (*)(void* StaticFunctionTag, void* stack_id, const char* key, float value);
        //using set_string_t = void* (*)(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
        //                               const char* value);

        using get_int_t = int (*)(void*, void*, const char*, int);
        using get_float_t = float (*)(void*, void*, const char*, float);
        using get_string_t = const char* (*)(void* ret, void*, void*, const char*, const char*);

        using has_value_t = bool (*)(void* ,void* , const char*);

        //static inline set_number_t _original_set_int = nullptr;
        //static inline set_float_t _original_set_float = nullptr;
        //static inline set_string_t _original_set_string = nullptr;

        static inline get_int_t _get_int = nullptr;
        static inline get_float_t _get_float = nullptr;
        static inline get_string_t _get_string = nullptr;

        static inline has_value_t _has_int = nullptr;
        static inline has_value_t _has_float = nullptr;
        static inline has_value_t _has_string = nullptr;

        //static inline get_int_t _original_get_int = nullptr;
        //static inline get_float_t _original_get_float = nullptr;
        //static inline get_string_t _original_get_string = nullptr;

        static bool InstallStorageUtilHooks();

        //static int HookGetInt(void* StaticFunctionTag, void* stack_id, const char* key, int default_value);
        //static float HookGetFloat(void* StaticFunctionTag, void* stack_id, const char* key, float default_value);
        //static const char* HookGetString(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
        //                                 const char* default_value);

        //static int HookSetInt(void* StaticFunctionTag, void* stack_id, const char* key, int value);
        //static float HookSetFloat(void* StaticFunctionTag, void* stack_id, const char* key, float value);
        //static void* HookSetString(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
        //                           const char* value);
        
        //static void CaptureHandleForKey(const char* key, void* stack_id);
        static void* ResolveForm(VMObjectHandleInfo handle, bool &is_available);
        //static std::map<uint64_t, VMObjectHandleInfo> CopyKeyHandleMap(const std::string& key);

        //static std::unordered_map<std::string, std::map<uint64_t, VMObjectHandleInfo>> key_to_handles;
        static std::list<StorageObservedValue> watch_targets;
        //static std::set<std::string> watched_keys;
        //static std::set<uint64_t> watch_strage_util_key_ids;
        //static std::mutex key_handle_map_mutex;
    };

}