#pragma once

#include <string>
#include <string_view>
#include <mutex>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <optional>
#include <system_error>
#include <stdexcept>
#include <cstdint> // For std::uint_least32_t

// Boost.JSON headers
#include <boost/json.hpp>

// --- Conditional Source Location (Polyfill for C++17) ---
// Check for C++20 std::source_location support (though we'll use polyfill for C++17 target)
#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
#include <source_location>
// If C++20 is available, one could alias std::source_location.
// However, to ensure C++17 compatibility as per LogTesting.cpp, we'll use the polyfill.
#endif


namespace CTrack
{
    void        InitLogging(const std::string mode);
    std::string GetTimeStampString(int NumDecimals = 3, char TimeSeparator = ':', bool IncludeDuration = true);
    // Fallback for C++17 or when std::source_location is not available
    // (Adapted from LogTesting.cpp)
    struct source_location_polyfill
    {
      private:
        const char         *file_name_     = "unknown";
        const char         *function_name_ = "unknown";
        std::uint_least32_t line_          = 0;

      public:
        constexpr source_location_polyfill() noexcept = default;
        constexpr source_location_polyfill(const char *file, const char *func, std::uint_least32_t line) : file_name_(file), function_name_(func), line_(line)
        {
        }

        [[nodiscard]] constexpr std::uint_least32_t line() const noexcept { return line_; }
        [[nodiscard]] constexpr const char         *file_name() const noexcept { return file_name_; }
        [[nodiscard]] constexpr const char         *function_name() const noexcept { return function_name_; }
    };
    using source_location_t = CTrack::source_location_polyfill;

// Macro to get the current source location using the polyfill
// __func__ is standard C++11. MSVC also supports __FUNCTION__.
#define MAKE_SOURCE_LOCATION() CTrack::source_location_polyfill(__FILE__, __func__, static_cast<std::uint_least32_t>(__LINE__))

    // Original constants from Logging.h
    const std::string LogExtension  = "ndjson"; // Newline Delimited JSON
    const std::string DumpExtension = "dmp";

    // Enum for log levels (consistent with both original Logging.h and LogTesting.cpp)
    enum class LogSeverity
    {
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
        LOG_FATAL, // From original Logging.h
        LOG_DEBUG
    };

    // Helper to convert LogSeverity to string (declaration)
    std::string_view SeverityToString(LogSeverity level);

    class CLogging
    {
      public:
        // Singleton access
        static CLogging &getInstance();

        // Deleted copy/move constructors and assignment operators
        CLogging(const CLogging &)            = delete;
        CLogging &operator=(const CLogging &) = delete;
        CLogging(CLogging &&)                 = delete;
        CLogging &operator=(CLogging &&)      = delete;

        // --- Public logging methods ---
        void log(LogSeverity level, std::string_view message, const source_location_t &location);
        void log(LogSeverity level, std::string_view message, int errorCode, const source_location_t &location);
        void log(LogSeverity level, std::string_view message, const std::error_code &ec, const source_location_t &location);
        void log(LogSeverity level, const std::exception &ex, const source_location_t &location);

        // --- Convenience methods ---
        void info(std::string_view message, const source_location_t &loc);
        void warning(std::string_view message, const source_location_t &loc);
        void warning(std::string_view message, int errorCode, const source_location_t &loc);
        void warning(std::string_view message, const std::error_code &ec, const source_location_t &loc);
        void warning(const std::exception &ex, const source_location_t &loc);
        void error(std::string_view message, const source_location_t &loc);
        void error(const std::exception &ex, const source_location_t &loc);
        void error(std::string_view message, int errorCode, const source_location_t &loc);
        void error(std::string_view message, const std::error_code &ec, const source_location_t &loc);
        void debug(std::string_view message, const source_location_t &loc);
        void fatal(std::string_view message, const source_location_t &loc); // For LOG_FATAL

        // --- Configuration methods ---
        void        enableConsoleOutput(bool enable);
        void        enableFileOutput(bool enable, const std::string &filepath = ""); // Empty path for default name
        void        setMinLogLevel(LogSeverity level);
        LogSeverity getMinLogLevel() const;

        // Method to initialize logging with version info etc.
        // Can be called explicitly after getting the instance.
        void initialize(const std::string &mode = "APP", const std::string &appVersionInfo = "");

      private:
        // Private constructor and destructor for singleton
        CLogging();
        ~CLogging();

        // Core internal logging method
        void logInternal(LogSeverity              level,
                         const source_location_t &location, // Use the alias
                         std::string_view primaryMessage, std::optional<int> directErrorCode = std::nullopt,
                         std::optional<std::error_code> stdErrorCode = std::nullopt, std::optional<std::string> exceptionType = std::nullopt,
                         std::optional<std::vector<std::string>> stackTrace = std::nullopt);

        // Helper methods (can be static if they don't rely on instance members directly,
        // or non-static if they use instance-specific config like date formats if that were added)
        static std::string getCurrentTimestampISO8601();
        std::string        generateDefaultLogFileName(const std::string &mode);
        static std::string getApplicationName();
        static std::string getModuleFileName(); // Windows specific

        // Member variables
        std::mutex    m_logMutex;
        std::ofstream m_logFile;
        bool          m_consoleOutputEnabled;
        bool          m_fileOutputEnabled;
        LogSeverity   m_minLogLevel;
        std::string   m_logFileBaseName; // From original global g_LogFileBaseName concept
        std::string   m_currentLogFilePath;
    };

// --- Logging Macros (adapted from LogTesting.cpp) ---
#define LOG_INFO(message)                        CTrack::CLogging::getInstance().info(message, MAKE_SOURCE_LOCATION())
#define LOG_WARNING(message)                     CTrack::CLogging::getInstance().warning(message, MAKE_SOURCE_LOCATION())
#define LOG_WARNING_EX(exception)                CTrack::CLogging::getInstance().warning(exception, MAKE_SOURCE_LOCATION())
#define LOG_ERROR_MSG(message)                   CTrack::CLogging::getInstance().error(message, MAKE_SOURCE_LOCATION()) // Renamed from LOG_ERROR
#define LOG_ERROR_EX(exception)                  CTrack::CLogging::getInstance().error(exception, MAKE_SOURCE_LOCATION())
#define LOG_ERROR_CODE(message, code)            CTrack::CLogging::getInstance().error(message, code, MAKE_SOURCE_LOCATION())
#define LOG_ERROR_STDCODE(message, stdErrorCode) CTrack::CLogging::getInstance().error(message, stdErrorCode, MAKE_SOURCE_LOCATION())
#define LOG_DEBUG(message)                       CTrack::CLogging::getInstance().debug(message, MAKE_SOURCE_LOCATION())
#define LOG_FATAL(message)                       CTrack::CLogging::getInstance().fatal(message, MAKE_SOURCE_LOCATION())

    // --- Retaining CPrintLogRecord from original Logging.h ---
    // This class seems to use a separate PrintInfo mechanism, so it's kept as is.
    // If PrintInfo should use CLogging, CPrintLogRecord would need modification.
    class CPrintLogRecord
    {
      public:
        CPrintLogRecord(const char *iStartMessage, const char *iStopMessage, const char *iIdentifier);
        ~CPrintLogRecord();

      protected:
        std::string                                                 m_StartMessage;
        std::string                                                 m_StopMessage;
        std::string                                                 m_Identifier;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Timer;
    };

#define PRINTLOGRECORD(START, STOP, ID) CTrack::CPrintLogRecord ProfileRecord(START, STOP, ID);

    // --- Potentially Deprecated or Refactored Free Functions ---
    // The old free functions like InitLogging, Log, LogError might be deprecated
    // or refactored to use CLogging::getInstance().
    // For now, their declarations are removed from here as the class provides the main API.
    // If they need to be kept for backward compatibility, they'd be declared here and
    // implemented in Logging.cpp to call the singleton.

    // Example of how an old free function could be adapted (declaration):
    // void Log(LogSeverity severity, const std::string &iMessage); // Implemented in .cpp to call CLogging::getInstance()

} // namespace CTrack
