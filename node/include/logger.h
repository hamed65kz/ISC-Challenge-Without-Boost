#ifndef LOGGER_H
#define LOGGER_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

/**
 * @brief A wrapper class for using spdlog for logging.
 *
 * This class provides a mechanism to log messages to both the console and to a rotating log file.
 * It supports different log levels (trace, debug, info, warn, error, critical) and allows configuration
 * of the log file path, maximum file size, and the number of rotated files.
 */
class Logger {
public:
    /**
     * @brief Initializes the logger with the specified file path and sizes.
     *
     * This static method initializes the console and file loggers with the given parameters.
     * The console logger outputs colored log messages to the standard output,
     * while the file logger writes log messages to a specified file with rotation.
     *
     * @param file_path The path to the log file. Default is "logs/app.log".
     * @param max_file_size The maximum size of the log file before it gets rotated. Default is 5MB.
     * @param max_files The maximum number of rotated log files to keep. Default is 3.
     */
    static void Initialize(const std::string& file_path = "logs/app.log",
                          size_t max_file_size = 1024 * 1024 * 5,  // 5MB
                          size_t max_files = 3) {
        try {
            // Create console sink with color, we use mt version for being thread safe
            //The suffix _mt in their names indicates that they are thread-safe, meaning they can handle concurrent logging calls from multiple threads without corrupting the log output.
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("%^[%T] : %v%$");
            
            // Create rotating file sink, we use mt version for being thread safe
            //The suffix _mt in their names indicates that they are thread-safe, meaning they can handle concurrent logging calls from multiple threads without corrupting the log output.
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                file_path, max_file_size, max_files
            );
            file_sink->set_pattern("[%Y-%m-%d %T.%e] [%l] : %v");

            // Create and register loggers
            auto console_logger_ = std::make_shared<spdlog::logger>("console", console_sink);
            auto file_logger_ = std::make_shared<spdlog::logger>("file", file_sink);

            // Set default log levels
            console_logger_->set_level(spdlog::level::trace);
            file_logger_->set_level(spdlog::level::trace);

            spdlog::register_logger(console_logger_);
            spdlog::register_logger(file_logger_);

            // Flush immediately for critical errors
            spdlog::flush_on(spdlog::level::err);
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
        }
    }

    /**
     * @brief Gets the console logger.
     *
     * @return A shared pointer to the console logger.
     */
    static std::shared_ptr<spdlog::logger> Console() { return spdlog::get("console"); }

    /**
     * @brief Gets the file logger.
     *
     * @return A shared pointer to the file logger.
     */
    static std::shared_ptr<spdlog::logger> File() { return spdlog::get("file"); }
};

// Macros for ease of use
/**
 * @brief Logging macros for console logging.
 */
#define LOG_TRACE(...)   SPDLOG_LOGGER_TRACE(Logger::Console(), __VA_ARGS__)
#define LOG_DEBUG(...)   SPDLOG_LOGGER_DEBUG(Logger::Console(), __VA_ARGS__)
#define LOG_INFO(...)    SPDLOG_LOGGER_INFO(Logger::Console(), __VA_ARGS__)
#define LOG_WARN(...)    SPDLOG_LOGGER_WARN(Logger::Console(), __VA_ARGS__)
#define LOG_ERROR(...)   SPDLOG_LOGGER_ERROR(Logger::Console(), __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(Logger::Console(), __VA_ARGS__)

/**
 * @brief Logging macros for file logging.
 */
#define FLOG_TRACE(...)   SPDLOG_LOGGER_TRACE(Logger::File(), __VA_ARGS__)
#define FLOG_DEBUG(...)   SPDLOG_LOGGER_DEBUG(Logger::File(), __VA_ARGS__)
#define FLOG_INFO(...)    SPDLOG_LOGGER_INFO(Logger::File(), __VA_ARGS__)
#define FLOG_WARN(...)    SPDLOG_LOGGER_WARN(Logger::File(), __VA_ARGS__)
#define FLOG_ERROR(...)   SPDLOG_LOGGER_ERROR(Logger::File(), __VA_ARGS__)
#define FLOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(Logger::File(), __VA_ARGS__)
                        

#endif