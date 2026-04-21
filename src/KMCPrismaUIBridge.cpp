#include "KMCPrismaUIBridge.h"

#include "KMCDisplayWordAndTexture.h"
#include "KMCCutin.h"
#include "KMCUtility.h"

static void OnReceiveKMCDefineCutin(const char* data) { SKSE::log::info("KMCDefineCutin Result ==> {}", data); }

static void OnReceiveKMCBatchPreloadGroups(const char* data) { SKSE::log::info("KMCBatchPreloadGroups Result ==> {}", data); }

static void OnReceiveKMCPlayPlayerCutin(const char* data) { SKSE::log::info("KMCPlayPlayerCutin Result ==> {}", data); }

static void OnReceiveKMCPlayFollowerCutin(const char* data) { SKSE::log::info("KMCPlayFollowerCutin Result ==> {}", data); }

static void OnReceiveKMCShowStopIcon(const char* data) { SKSE::log::info("KMCShowStopIcon Result ==> {}", data); }

static void OnReceiveKMCHideStopIcon(const char* data) { SKSE::log::info("KMCHideStopIcon Result ==> {}", data); }

static void OnReceiveKMCStopAndHideCutinAndIcon(const char* data) {
    SKSE::log::info("KMCStopAndHideCutinAndIcon Result ==> {}", data);
}

SINGLETONBODY(KMCCT::KMCPrismaUIBridge)
namespace KMCCT {

    PRISMA_UI_API::IVPrismaUI1* prisma_ui;
    PrismaView cutin_view;

    void KMCPrismaUIBridge::Init() {
        prisma_ui = static_cast<PRISMA_UI_API::IVPrismaUI1*>(
            PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1));

        if (!prisma_ui) {
            ERROR("Failed to initialize PrismaUI API");
            return;
        }

        cutin_view = prisma_ui->CreateView("KMCCutinPlugin/index.html", [](PrismaView view) -> void {
            SKSE::log::info("View DOM is ready {}", view);

            KMCDisplayWordAndTexture::GetSingleton()->Init();

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

        SKSE::log::info("PrismaUI API initialized successfully");
    }

    void KMCPrismaUIBridge::Focus() { 
        prisma_ui->Focus(cutin_view); 
    }

    void KMCPrismaUIBridge::UnFocus() { 
        prisma_ui->Unfocus(cutin_view);
    }

    void KMCPrismaUIBridge::KMCDefineCutin(const json& j) {
        std::string script = "KMCDefineCutin(" + j.dump() + ")";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCDefineCutin);
    }

    void KMCPrismaUIBridge::KMCBatchPreloadGroups(int id, int next_group, int f_id, int f_next_group) {
        nlohmann::json group_data_map;
        group_data_map[std::to_string(id)] = next_group;
        group_data_map[std::to_string(f_id)] = f_next_group;

        std::string script = "KMCBatchPreloadGroups(" + group_data_map.dump() + ")";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCBatchPreloadGroups);
    }

    void KMCPrismaUIBridge::KMCPlayPlayerCutin(int group, int next_group) {
        json group_data_map = {{"group", group}, {"next_group", next_group}};

        std::string script = "KMCPlayPlayerCutin(" + group_data_map.dump() + ")";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCPlayPlayerCutin);
    }

    void KMCPrismaUIBridge::KMCPlayFollowerCutin(int id, int group, int next_group) {
        json group_data_map = {{"id", id}, {"group", group}, {"next_group", next_group}};

        std::string script = "KMCPlayFollowerCutin(" + group_data_map.dump() + ")";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCPlayFollowerCutin);
    }

    void KMCPrismaUIBridge::KMCShowStopIcon() {
        std::string script = "KMCShowStopIcon()";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCShowStopIcon);
    }

    void KMCPrismaUIBridge::KMCHideStopIcon() {
        std::string script = "KMCHideStopIcon()";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCHideStopIcon);
    }

    void KMCPrismaUIBridge::KMCStopAndHideCutinAndIcon() {
        std::string script = "KMCStopAndHideCutinAndIcon()";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCStopAndHideCutinAndIcon);
    }
}