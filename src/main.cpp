#include "AnimSyncTogether/AnimationProbe.h"
#include "AnimSyncTogether/Version.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <spdlog/sinks/basic_file_sink.h>

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
        spdlog::set_default_logger(std::move(log));
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);
    }

    void OnSKSEMessage(SKSE::MessagingInterface::Message* message)
    {
        if (!message) {
            return;
        }

        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            SKSE::log::info("Data loaded; installing animation probe");
            AnimSyncTogether::AnimationProbe::GetSingleton()->Install();
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SetupLog();

    SKSE::Init(skse);

    SKSE::log::info(
        "AnimSync Together v{} loading",
        AnimSyncTogether::Version::STRING);

    const auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(OnSKSEMessage)) {
        SKSE::log::critical("Failed to register SKSE messaging listener");
        return false;
    }

    SKSE::log::info("AnimSync Together initialized");
    return true;
}
