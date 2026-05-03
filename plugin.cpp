#include "KMCRegister.h"

//#include "src/hooks/hooks.h"

//#include <SkyrimScripting/Plugin.h>

namespace KMCCT {


    SKSEPluginLoad(const SKSE::LoadInterface *skse) {
        SKSE::Init(skse);

        auto logsFolder = SKSE::log::log_directory();
        if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
        auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
        auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
        auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
        spdlog::set_default_logger(std::move(loggerPtr));
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);

        

        SKSE::GetMessagingInterface()->RegisterListener(KMCCT::OnMessageReceived);
        SKSE::GetPapyrusInterface()->Register(KMCCT::PapyrusRegister);

	    //if (!KMCCT::Install()) {
     //       ERROR("Failed to install necessary hooks.");
     //   }

        //const auto serialization = SKSE::GetSerializationInterface();
        //RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<EventType>(
        //    new CallbackEventSink<EventType>(callback));
        
        //On<RE::TESTopicInfoEvent>([](const RE::TESTopicInfoEvent* event) {
        //    ConsoleLog("topic event");
        //});




        return true;
    }

    //// If there are certain events that you want to listen to, it's easy as well:
    //EventHandlers {
    //    On<RE::TESActivateEvent>([](const RE::TESActivateEvent* event) {
    //        auto activated = event->objectActivated->GetBaseObject()->GetName();
    //        auto activator = event->actionRef->GetBaseObject()->GetName();
    //        // This will print things to the game console like:
    //        // Hod activated Mill
    //        // Hilde activated Door
    //        ConsoleLog("{} activated {}", activator, activated);
    //    });
    //}
}

