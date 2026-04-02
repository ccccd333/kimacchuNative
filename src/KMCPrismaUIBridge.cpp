#include "KMCPrismaUIBridge.h"

#include "KMCDisplayWordAndTexture.h"
#include "KMCUtility.h"

static void OnReceiveKMCDefineCutin(const char* data) { SKSE::log::info("KMCDefineCutin Result ==> {}", data); }

static void OnReceiveKMCPlayPlayerCutin(const char* data) { SKSE::log::info("KMCPlayPlayerCutin Result ==> {}", data); }

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
        });

        SKSE::log::info("PrismaUI API initialized successfully");
    }

    void KMCPrismaUIBridge::Focus() { 
        prisma_ui->Focus(cutin_view); 
    }

    void KMCPrismaUIBridge::UnFocus() { 
        prisma_ui->Unfocus(cutin_view);
    }

    void KMCPrismaUIBridge::AddPath(const json& j) {
        std::string script = "KMCDefineCutin(" + j.dump() + ")";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCDefineCutin);
    }

    void KMCPrismaUIBridge::KMCPlayPlayerCutin(int group) {
        std::string script = "KMCPlayPlayerCutin(" + std::to_string(group) + ")";
        prisma_ui->Invoke(cutin_view, script.c_str(), OnReceiveKMCPlayPlayerCutin);
    }
}