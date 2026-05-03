#include "KMCPrismaUIBridge.h"

#include "KMCDisplayWordAndTexture.h"
#include "KMCCutin.h"
#include "KMCUtility.h"
#include "KMCProfile.h"
#include "KMCExpression.h"

//static void OnReceiveKMCDefineCutin(const char* data) { SKSE::log::info("KMCDefineCutin Result ==> {}", data); }
//
//static void OnReceiveKMCBatchPreloadGroups(const char* data) { SKSE::log::info("KMCBatchPreloadGroups Result ==> {}", data); }
//
//static void OnReceiveKMCPlayPlayerCutin(const char* data) { SKSE::log::info("KMCPlayPlayerCutin Result ==> {}", data); }
//
//static void OnReceiveKMCPlayFollowerCutin(const char* data) { SKSE::log::info("KMCPlayFollowerCutin Result ==> {}", data); }
//
//static void OnReceiveKMCShowStopIcon(const char* data) { SKSE::log::info("KMCShowStopIcon Result ==> {}", data); }
//
//static void OnReceiveKMCHideStopIcon(const char* data) { SKSE::log::info("KMCHideStopIcon Result ==> {}", data); }
//
//static void OnReceiveKMCStopAndHideCutinAndIcon(const char* data) {
//    SKSE::log::info("KMCStopAndHideCutinAndIcon Result ==> {}", data);
//}

SINGLETONBODY(KMCCT::KMCPrismaUIBridge)
namespace KMCCT {

    PRISMA_UI_API::IVPrismaUI2* prisma_ui;
    PrismaView cutin_view;

    void KMCPrismaUIBridge::Init() {
        prisma_ui = static_cast<PRISMA_UI_API::IVPrismaUI2*>(
            PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V2));

        if (!prisma_ui) {
            SKSE::log::error("Failed to initialize PrismaUI API");
            return;
        }

        cutin_view = prisma_ui->CreateView("KMCCutinPlugin/index.html", [](PrismaView view) -> void {
            SKSE::log::info("View DOM is ready {}", view);
            KMCCT::KMCProfile::GetSingleton()->Init();
            KMCDisplayWordAndTexture::GetSingleton()->Init();
            KMCCT::KMCExpression::GetSingleton()->Init();
            
            // KMCPrismaUIBridge::GetSingleton()->KMCPlayPlayerCutin(1);

            // prisma_ui->Focus(cutin_view);
        });

        prisma_ui->RegisterJSListener(cutin_view, "KMCOnCutinFinished", [](const char* data) -> void {
            SKSE::log::info("Cutin finished for display : {}", data);
            KMCCutin::GetSingleton()->SetCutinFinished(true);
        });

        prisma_ui->RegisterJSListener(cutin_view, "KMCOnCutinStartReady", [](const char* data) -> void {
            SKSE::log::info("Cutin start for display : {}", data);
            KMCCutin::GetSingleton()->SetCutinStartReady(true);
        });

        prisma_ui->RegisterJSListener(cutin_view, "KMCOnCutinUnavailable", [](const char* data) -> void {
            SKSE::log::info("Cutin unavailable for display : {}", data);
            KMCCutin::GetSingleton()->SetCutinUnavailable(true);
        });

        prisma_ui->RegisterJSListener(cutin_view, "OnCacheLoaded", [](const char* data) -> void {
            SKSE::log::info("Cutin assets loaded for display ID: {}", data);
            KMCCutin::GetSingleton()->SetFollowerCacheDataLoaded(true);
        });

        prisma_ui->RegisterConsoleCallback(cutin_view, [](PrismaView view, PRISMA_UI_API::ConsoleMessageLevel level, const char* message) {
            switch (level) {
                    case PRISMA_UI_API::ConsoleMessageLevel::Log:
                    SKSE::log::info("[JS] {}", message);
                    break;
                    case PRISMA_UI_API::ConsoleMessageLevel::Warning:
                    SKSE::log::warn("[JS] {}", message);
                    break;
                    case PRISMA_UI_API::ConsoleMessageLevel::Error:
                    SKSE::log::error("[JS] {}", message);
                    break;
                    case PRISMA_UI_API::ConsoleMessageLevel::Debug:
                    SKSE::log::debug("[JS] {}", message);
                    break;
                    case PRISMA_UI_API::ConsoleMessageLevel::Info:
                    SKSE::log::info("[JS] {}", message);
                    break;
            }
        });

        SKSE::log::info("PrismaUI API initialized successfully");
    }

    void KMCPrismaUIBridge::Focus() { 
        if (prisma_ui->IsValid(cutin_view)) {
            prisma_ui->Focus(cutin_view);
        } else {
            SKSE::log::warn("Focus failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::UnFocus() {
        if (prisma_ui->IsValid(cutin_view)) {
            prisma_ui->Unfocus(cutin_view);
        } else {
            SKSE::log::warn("UnFocus failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCDefineCutin(const json& j) {
        if (prisma_ui->IsValid(cutin_view)) {
            std::string script = "KMCDefineCutin(" + j.dump() + ")";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCDefineCutin failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCPreloadGroup(int id, int group) {
        if (prisma_ui->IsValid(cutin_view)) {
            json group_data_map = {{"id", id}, {"group", group}};

            std::string script = "KMCPreloadGroup(" + group_data_map.dump() + ")";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCBatchPreloadGroups failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCBatchPreloadGroups(int id, int next_group, int f_id, int f_next_group) {
        if (prisma_ui->IsValid(cutin_view)) {
            nlohmann::json group_data_map;
            group_data_map[std::to_string(id)] = next_group;
            group_data_map[std::to_string(f_id)] = f_next_group;

            std::string script = "KMCBatchPreloadGroups(" + group_data_map.dump() + ")";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCBatchPreloadGroups failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCPlayPlayerCutin(int group, int next_group, std::string actor_name) {
        if (prisma_ui->IsValid(cutin_view)) {
            json group_data_map = {{"group", group}, {"next_group", next_group}, {"actor_name", actor_name}};

            std::string script = "KMCPlayPlayerCutin(" + group_data_map.dump() + ")";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCPlayPlayerCutin failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCPlayFollowerCutin(int id, int group, int next_group, std::string actor_name) {
        if (prisma_ui->IsValid(cutin_view)) {
            json group_data_map = {
                {"id", id}, {"group", group}, {"next_group", next_group}, {"actor_name", actor_name}};

            std::string script = "KMCPlayFollowerCutin(" + group_data_map.dump() + ")";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCPlayFollowerCutin failed: cutin_view is invalid. ID: {}", id);
        }
    }

    void KMCPrismaUIBridge::KMCShowStopIcon() {
        if (prisma_ui->IsValid(cutin_view)) {
            std::string script = "KMCShowStopIcon()";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCShowStopIcon failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCHideStopIcon() {
        if (prisma_ui->IsValid(cutin_view)) {
            std::string script = "KMCHideStopIcon()";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCHideStopIcon failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCStopAndHideCutinAndIcon() {
        if (prisma_ui->IsValid(cutin_view)) {
            std::string script = "KMCStopAndHideCutinAndIcon()";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCStopAndHideCutinAndIcon failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCSetupProfile(const json& j) {
        if (prisma_ui->IsValid(cutin_view)) {
            std::string script = "KMCSetupProfile(" + j.dump() + ")";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCSetupProfile failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCShowProfile() {
        if (prisma_ui->IsValid(cutin_view)) {
            std::string script = "KMCShowProfile()";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCShowProfile failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCHideProfile() {
        if (prisma_ui->IsValid(cutin_view)) {
            std::string script = "KMCHideProfile()";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCHideProfile failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCUpdateProfileText(const json& j) {
        if (prisma_ui->IsValid(cutin_view)) {
            std::string script = "KMCUpdateProfileText(" + j.dump() + ")";
            prisma_ui->Invoke(cutin_view, script.c_str());
        } else {
            SKSE::log::warn("KMCUpdateProfileText failed: cutin_view is invalid.");
        }
    }

    void KMCPrismaUIBridge::KMCPrismaViewDestroy() {
        if (prisma_ui->IsValid(cutin_view)) {
            prisma_ui->Destroy(cutin_view);
            SKSE::log::info("View destroyed and memory freed");
        } else {
            SKSE::log::warn("KMCPrismaViewDestroy: View was already invalid or prisma_ui is null.");
        }
    }

    KMCPrismaUIBridge::~KMCPrismaUIBridge() { 
        // share pointerでviewをprismaUI側が持っててviewを解放している
        // なのでここでやる必要がない
        //KMCPrismaViewDestroy();
    }
}