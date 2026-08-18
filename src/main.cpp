#include "AnimSyncTogether/AnimationProbe.h"
#include "AnimSyncTogether/Version.h"

#include <spdlog/sinks/basic_file_sink.h>

namespace logger = SKSE::log;

namespace
{
    void SetupLog()
    {
        auto path = logger::log_directory();
        if (!path) {
            SKSE::stl::report_and_fail("Unable to resolve SKSE log directory");
        }

        *path /= "AnimSyncTogether.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
        log->set_level(spdlog::level::trace);
        log->flush_on(spdlog::level::trace);
        spdlog::set_default_logger(std::move(log));
    }

    void OnSKSEMessage(SKSE::MessagingInterface::Message* message)
    {
        if (!message) {
            return;
        }

        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            logger::info("Data loaded; installing animation probe");
            AnimSyncTogether::AnimationProbe::GetSingleton()->Install();
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    SetupLog();

    logger::info(
        "AnimSync Together v{} loading",
        AnimSyncTogether::Version::STRING);

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(OnSKSEMessage)) {
        logger::critical("Failed to register SKSE messaging listener");
        return false;
    }

    logger::info("AnimSync Together initialized");
    return true;
}
