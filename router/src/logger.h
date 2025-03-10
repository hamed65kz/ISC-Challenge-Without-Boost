#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

class Logger {
public:
    static void init(const std::string& name, const std::string& filename) {
        try {
            // Create sinks
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                filename, 1024 * 1024 * 5, 3
            );

            // Create combined logger
            std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
            auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
            
            // Set pattern: [YYYY-MM-DD HH:MM:SS] [logger] [level] message
            logger->set_pattern("[%Y-%m-%d %T.%e] [%n] [%^%l%$] %v");
            logger->set_level(spdlog::level::info);
            
            spdlog::register_logger(logger);
            spdlog::flush_every(std::chrono::seconds(3));
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

    static std::shared_ptr<spdlog::logger> get(const std::string& name) {
        return spdlog::get(name);
    }
};