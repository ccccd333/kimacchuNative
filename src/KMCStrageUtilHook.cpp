#include "KMCStrageUtilHook.h"

#include <MinHook.h>
#include <Windows.h>

#include "KMCUtility.h"

#include "KMCConfig.h"

namespace KMCCT {

    void KMCStrageUtilHook::InstallStorageUtilHooks() {
        auto base = (uintptr_t)GetModuleHandleA("PapyrusUtil.dll");

        if (!base) {
            SKSE::log::error("PapyrusUtil.dll not found");
            return;
        }

        // SET(Int, Float, String ... )

        // Ghidra Image Base: 0x180000000
        // FUN_18009c090
        // FUN_18009c240
        // FUN_18009c400
        uintptr_t addr_set_int = base + 0x9C090;
        uintptr_t addr_set_float = base + 0x9C240;
        uintptr_t addr_set_string = base + 0x9C400;

        if (MH_Initialize() != MH_OK) {
            SKSE::log::info("MH_Initialize failed");
            return;
        }

        // JMP r/m64
        // SKSE間の絶対ジャンプ
        MH_CreateHook((LPVOID)addr_set_int, &HookSetInt, (LPVOID*)&_original_set_int);
        MH_CreateHook((LPVOID)addr_set_float, &HookSetFloat, (LPVOID*)&_original_set_float);
        MH_CreateHook((LPVOID)addr_set_string, &HookSetString, (LPVOID*)&_original_set_string);

        MH_EnableHook((LPVOID)addr_set_int);
        MH_EnableHook((LPVOID)addr_set_float);
        MH_EnableHook((LPVOID)addr_set_string);

        // GET(Int, Float, String ... ) Func Pointer

        uintptr_t addr_get_int = base + 0x9C6D0;
        uintptr_t addr_get_float = base + 0x9C7B0;
        uintptr_t addr_get_string = base + 0x9C890;

		_get_int = (get_int_t)(addr_get_int);
        _get_float = (get_float_t)(addr_get_float);
        _get_string = (get_string_t)(addr_get_string);

        // GET(Int, Float, String ... )
        // JMP r/m64
        // SKSE間の絶対ジャンプ
        MH_CreateHook((LPVOID)addr_get_int, &HookGetInt, (LPVOID*)&_original_get_int);
        MH_CreateHook((LPVOID)addr_get_float, &HookGetFloat, (LPVOID*)&_original_get_float);
        MH_CreateHook((LPVOID)addr_get_string, &HookGetString, (LPVOID*)&_original_get_string);

        MH_EnableHook((LPVOID)addr_get_int);
        MH_EnableHook((LPVOID)addr_get_float);
        MH_EnableHook((LPVOID)addr_get_string);

        // フックできた場合にトランポリン先を設定
        _get_int = _original_get_int;
        _get_float = _original_get_float;
        _get_string = _original_get_string;

        SKSE::log::info("StorageUtil hooks installed");
    }

	int KMCStrageUtilHook::GetIntValue(void* StaticFunctionTag, void* VMHandle, const char* key, int default_val) {
        if (!_get_int) return default_val;
        return _get_int(StaticFunctionTag, VMHandle, key, default_val);
    }

    float KMCStrageUtilHook::GetFloatValue(void* StaticFunctionTag, void* VMHandle, const char* key,
                                              float default_val) {
        if (!_get_float) return default_val;
        return _get_float(StaticFunctionTag, VMHandle, key, default_val);
    }

    const char* KMCStrageUtilHook::GetStringValue(void* ret, void* StaticFunctionTag, void* VMHandle, const char* key,
                                                     const char* default_val) {
        if (!_get_string) return default_val;
        return _get_string(ret, StaticFunctionTag, VMHandle, key, default_val);
    }

    int KMCStrageUtilHook::HookGetInt(void* StaticFunctionTag, void* stack_id, const char* key, int default_value) {
        uint64_t full_id = 0;
        uint32_t low = 0;
        if (stack_id) {
            low = *(uint32_t*)((uintptr_t)stack_id + 0x14);
            auto high = *(uint8_t*)((uintptr_t)stack_id + 0x1A);

            full_id = ((uint64_t)high << 32) | low;
        }

        SKSE::log::info("[SetInt] full_id = {} form={} key={} value={}", full_id, low, key ? key : "null",
                        (int)default_value);

        return _original_get_int(StaticFunctionTag, stack_id, key, default_value);
    }
    float KMCStrageUtilHook::HookGetFloat(void* StaticFunctionTag, void* stack_id, const char* key,
                                          float default_value) {
        uint64_t full_id = 0;
        uint32_t low = 0;
        if (stack_id) {
            low = *(uint32_t*)((uintptr_t)stack_id + 0x14);
            auto high = *(uint8_t*)((uintptr_t)stack_id + 0x1A);

            full_id = ((uint64_t)high << 32) | low;
        }

        SKSE::log::info("[SetInt] full_id = {} form={} key={} value={}", full_id, low, key ? key : "null",
                        (int)default_value);

        return _original_get_float(StaticFunctionTag, stack_id, key, default_value);
    }
    const char* KMCStrageUtilHook::HookGetString(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
                                           const char* default_value) {
        uint64_t full_id = 0;
        uint32_t low = 0;
        if (stack_id) {
            low = *(uint32_t*)((uintptr_t)stack_id + 0x14);
            auto high = *(uint8_t*)((uintptr_t)stack_id + 0x1A);

            full_id = ((uint64_t)high << 32) | low;
        }

        SKSE::log::info("[SetInt] full_id = {} form={} key={} value={}", full_id, low, key ? key : "null",
                        default_value ? default_value : "null");

        return _original_get_string(ret, StaticFunctionTag, stack_id, key, default_value);
    }

    int KMCStrageUtilHook::HookSetInt(void* StaticFunctionTag, void* stack_id, const char* key, int value) {

        uint64_t full_id = 0; 
        uint32_t low = 0;
        if (stack_id) {
            low = *(uint32_t*)((uintptr_t)stack_id + 0x14);
            auto high = *(uint8_t*)((uintptr_t)stack_id + 0x1A);

            full_id = ((uint64_t)high << 32) | low;
        }

        SKSE::log::info("[SetInt] full_id = {} form={} key={} value={}", full_id, low, key ? key : "null",
                        (int)value);

        return _original_set_int(StaticFunctionTag, stack_id, key, value);
    }

    float KMCStrageUtilHook::HookSetFloat(void* StaticFunctionTag, void* stack_id, const char* key, float value) {
        float f = value;

        uint64_t full_id = 0;
        uint32_t low = 0;
        if (stack_id) {
            low = *(uint32_t*)((uintptr_t)stack_id + 0x14);
            auto high = *(uint8_t*)((uintptr_t)stack_id + 0x1A);

            full_id = ((uint64_t)high << 32) | low;
        }

        SKSE::log::info("[SetFloat] full_id = {} form={} key={} value={}", full_id, low, key ? key : "null", f);

        return _original_set_float(StaticFunctionTag, stack_id, key, value);
    }

    void* KMCStrageUtilHook::HookSetString(void* ret, void* StaticFunctionTag, void* stack_id, const char* key,
                                           const char* value) {
        uint64_t full_id = 0;
        uint32_t low = 0;
        if (stack_id) {
            low = *(uint32_t*)((uintptr_t)stack_id + 0x14);
            auto high = *(uint8_t*)((uintptr_t)stack_id + 0x1A);

            full_id = ((uint64_t)high << 32) | low;
        }

        SKSE::log::info("[SetString] full_id = {} form={} key={} value={}", full_id, low, key ? key : "null",
                        value ? value : "null");

        return _original_set_string(ret, StaticFunctionTag, stack_id, key, value);
    }
}