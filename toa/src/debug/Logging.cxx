//
// Carson R - 1/23/2023
//

// Dependencies
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
// Local Includes
#include <app/App.hxx>
#include "debug/Logging.hxx"

namespace TitanOfAir::logging
{
    std::string consoleName;
    std::shared_ptr<spdlog::logger> console;

    void configureLoggers()
    {
        try
        {
            consoleName = App::LOG;
            console = spdlog::stdout_color_mt(consoleName);
            console->set_level(spdlog::level::trace);
            spdlog::register_logger(console);
        }
        catch (spdlog::spdlog_ex& ex)
        {
            console->warn("Log Init: {}", ex.what());
        }
    }
}