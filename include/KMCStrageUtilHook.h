#pragma once
namespace KMCCT {

    class KMCStrageUtilHook {
    public:
        static void InstallStorageUtilHooks();

		static int GetIntValue(void* StaticFunctionTag, void* VMHandle, const char* key, int default_val);

        static float GetFloatValue(void* StaticFunctionTag, void* VMHandle, const char* key, float default_val);

        static const char* GetStringValue(void* ret, void* StaticFunctionTag, void* VMHandle, const char* key,
                                          const char* default_val);

    private:

        using set_number_t = int (*)(void* StaticFunctionTag, void* stack_id, const char* key, int value);
        using set_float_t = float (*)(void* StaticFunctionTag, void* stack_id, const char* key, float value);
        using set_string_t = void* (*)(void* ret, void* StaticFunctionTag, void* stack_id, const char* key, const char* value);

		using get_int_t = int (*)(void*, void*, const char*, int);
        using get_float_t = float (*)(void*, void*, const char*, float);
        using get_string_t = const char* (*)(void* ret, void*, void*, const char*, const char*);


        static inline set_number_t _original_set_int = nullptr;
        static inline set_float_t _original_set_float = nullptr;
        static inline set_string_t _original_set_string = nullptr;

		static inline get_int_t _get_int = nullptr;
        static inline get_float_t _get_float = nullptr;
        static inline get_string_t _get_string = nullptr;

        static inline get_int_t _original_get_int = nullptr;
        static inline get_float_t _original_get_float = nullptr;
        static inline get_string_t _original_get_string = nullptr;

        static int HookGetInt(void* StaticFunctionTag, void* stack_id, const char* key, int default_value);
        static float HookGetFloat(void* StaticFunctionTag, void* stack_id, const char* key, float default_value);
        static const char* HookGetString(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
                                         const char* default_value);

        static int HookSetInt(void* StaticFunctionTag, void* stack_id, const char* key, int value);
        static float HookSetFloat(void* StaticFunctionTag, void* stack_id, const char* key, float value);
        static void* HookSetString(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
                                   const char* value);


    };

}