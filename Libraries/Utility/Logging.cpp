#include "Logging.h" // Should be first for precompiled headers if used
#include "../../../version.h"
#include "Print.h" // For PrintInfo used by CPrintLogRecord

// Standard library includes
#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <fmt/core.h>

// Windows-specific for GetModuleFileName
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#ifndef REL_DIR_LOG
#define REL_DIR_LOG "..\\Log\\" // Default relative log directory
#endif

// If version.h and Print.h are needed for CPrintLogRecord or other parts:
// #include "../../../version.h" // Path from original Logging.cpp
// #include "Print.h"          // For PrintInfo used by CPrintLogRecord

namespace CTrack
{

    std::string g_LogFileBaseName;
    std::mutex  g_LogMutex;
    auto        g_StartFromProgram = std::chrono::system_clock::now();

    double GetDurationInSeconds()
    {
        auto now      = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_StartFromProgram).count();
        return duration / 1000.0;
    }

    std::string GetTimeStampString(int NumDecimals, char TimeSeparator, bool IncludeDuration)
    {
        // Current time
        auto   now                 = std::chrono::system_clock::now();
        auto   now_time            = std::chrono::system_clock::to_time_t(now);
        auto   now_ms              = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        double duration_in_seconds = GetDurationInSeconds();

        // Local time breakdown
        std::tm local_tm;
#if defined(_MSC_VER)
        localtime_s(&local_tm, &now_time); // MSVC
#else
        localtime_r(&now_time, &local_tm); // POSIX
#endif

        std::string sep(1, TimeSeparator);

        std::string format;
        if (NumDecimals > 0)
        {
            format = fmt::format("{{:04d}}{{:02d}}{{:02d}}-{{:02d}}{}{{:02d}}{}{{:02d}}.{{:03d}}", sep, sep);
            if (IncludeDuration)
                format += fmt::format(" [{{:.{}f}}]", NumDecimals);

            if (IncludeDuration)
            {
                return fmt::format(format, local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec,
                                   static_cast<int>(now_ms.count()), duration_in_seconds);
            }
            else
            {
                return fmt::format(format, local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec,
                                   static_cast<int>(now_ms.count()));
            }
        }
        else
        {
            format = fmt::format("{{:04d}}{{:02d}}{{:02d}}-{{:02d}}{}{{:02d}}{}{{:02d}}", sep, sep);
            if (IncludeDuration)
                format += " [{:.3f}]";

            if (IncludeDuration)
            {
                return fmt::format(format, local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec,
                                   duration_in_seconds);
            }
            else
            {
                return fmt::format(format, local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
            }
        }
    }

    std::string GetExecutablePath()
    {
        wchar_t path[MAX_PATH];
        DWORD   length = GetModuleFileNameW(nullptr, path, MAX_PATH);
        if (length == 0 || length == MAX_PATH)
        {
            throw std::runtime_error("Failed to get executable path");
        }
        return std::filesystem::path(path).string();
    }

    std::filesystem::path getExecutableDirectory()
    {
        wchar_t path[MAX_PATH];
        DWORD   length = GetModuleFileNameW(nullptr, path, MAX_PATH);
        if (length == 0 || length == MAX_PATH)
        {
            throw std::runtime_error("Failed to get executable path");
        }
        return std::filesystem::path(path).parent_path();
    }

    std::string GenerateLogFileName(const std::string AppID, const std::string Extension, bool makeWildcard)
    {
        const std::filesystem::path relLogPath = REL_DIR_LOG;
        std::filesystem::path       exePath    = std::filesystem::absolute(GetExecutablePath());
        std::string                 appName    = exePath.stem().string();
        std::filesystem::path       logDir     = exePath.parent_path() / relLogPath;
        std::filesystem::create_directories(logDir);
        std::string timeString = GetTimeStampString(0, '_', false);
        if (makeWildcard)
            timeString = ".*";
        std::string logFileName = fmt::format("{}{}_{}_{}.{}", logDir.string(), appName, timeString, AppID, Extension);

        return logFileName;
    }

    std::string GetLogFileName(const std::string Extension, const std::string AppID)
    {
        if (g_LogFileBaseName.empty())
        {
            g_LogFileBaseName = GenerateLogFileName(AppID, LogExtension, false);
        }
        if (Extension != LogExtension)
        {
            std::filesystem::path original(g_LogFileBaseName);
            std::filesystem::path newPath = original.parent_path() / original.stem();
            newPath += ".";
            newPath += Extension;
            return newPath.string();
        }
        else
            return g_LogFileBaseName;
    }

    namespace fs = std::filesystem;

    void DeleteOldLogs(const std::string &FullPathMask, int days_old)
    {
        fs::path    fullPath  = FullPathMask;
        std::string dir       = fullPath.parent_path().string();
        std::string file_mask = fullPath.filename().string();
        std::regex  mask(file_mask, std::regex::icase | std::regex::ECMAScript);
        auto        now = std::chrono::system_clock::now();
        for (const auto &entry : fs::recursive_directory_iterator(dir))
        {
            if (fs::is_regular_file(entry))
            {
                const auto       &path     = entry.path();
                const std::string filename = path.filename().string();

                if (std::regex_match(filename, mask))
                {
                    auto ftime = fs::last_write_time(path);
                    auto sctp  = std::chrono::system_clock::now() - (fs::file_time_type::clock::now() - ftime);
                    auto age   = std::chrono::duration_cast<std::chrono::hours>(now - sctp).count() / 24;

                    if (age >= days_old)
                    {
                        try
                        {
                            fs::remove(path);
                        }
                        catch (const fs::filesystem_error &e)
                        {
                        }
                    }
                }
            }
        }
    }

    void InitLogging(const std::string AppID)
    {
        // remove old log files
        std::string LogFileWildCard = GenerateLogFileName(AppID, LogExtension, true);
        DeleteOldLogs(LogFileWildCard, 7);

        // create a new log file
        std::string LogFilePath = GetLogFileName(LogExtension, AppID);
        CTrack::CLogging::getInstance().enableFileOutput(true, LogFilePath);
        CTrack::CLogging::getInstance().enableConsoleOutput(false);
        CTrack::CLogging::getInstance().setMinLogLevel(LogSeverity::LOG_INFO);

        // register our version
        std::string VersionString = fmt::format("{} {} {}", GIT_TAG, BUILD_DATE, GIT_HASH);
        LOG_INFO(VersionString);
    }

    std::string_view SeverityToString(LogSeverity level)
    {
        switch (level)
        {
            case LogSeverity::LOG_INFO:
                return "INFO";
            case LogSeverity::LOG_WARNING:
                return "WARNING";
            case LogSeverity::LOG_ERROR:
                return "ERROR";
            case LogSeverity::LOG_FATAL:
                return "FATAL";
            case LogSeverity::LOG_DEBUG:
                return "DEBUG";
            default:
                return "UNKNOWN";
        }
    }

    // --- CLogging Class Member Function Definitions ---

    CLogging &CLogging::getInstance()
    {
        static CLogging instance; // Meyers' Singleton
        return instance;
    }

    CLogging::CLogging() : m_consoleOutputEnabled(true), m_fileOutputEnabled(false), m_minLogLevel(LogSeverity::LOG_DEBUG) // Default minimum log level
    {
        // Constructor: Initialize any default states.
        // File opening is handled by enableFileOutput.
        // std::cout << "Logger instance created." << std::endl; // For debugging the logger itself
    }

    CLogging::~CLogging()
    {
        // Destructor: Ensure resources are cleaned up.
        // Log a shutdown message IF this is the valid instance and logging is enabled.
        // The MAKE_SOURCE_LOCATION() macro will provide context for the destructor's log call.
        // Need to be careful about logging during static deinitialization order issues.
        // A simple check:
        // static bool main_instance_destroyed = false;
        // if (!main_instance_destroyed) {
        //    main_instance_destroyed = true; // Attempt to prevent re-entrant logging from other static destructors
        //    log(LogSeverity::LOG_INFO, "Logger shutting down.", MAKE_SOURCE_LOCATION());
        // }
        // The above is tricky. A safer approach for shutdown logging is often explicit.
        // For now, just ensure the file is closed.

        // if (m_consoleOutputEnabled && !main_instance_destroyed) { // Example of console shutdown message
        //    std::cout << getCurrentTimestampISO8601() << " [INFO] Logger shutting down." << std::endl;
        // }
    }

    bool CLogging::logFileOpen()
    {
        if (m_fileOutputEnabled)
        {
            if (!m_logFile.is_open())
            {
                std::filesystem::path filepathObj(m_currentLogFilePath);
                if (filepathObj.has_parent_path())
                {
                    try
                    {
                        if (!std::filesystem::exists(filepathObj.parent_path()))
                        {
                            std::filesystem::create_directories(filepathObj.parent_path());
                        }
                    }
                    catch (const std::filesystem::filesystem_error &fs_ex)
                    {
                        PrintError("LOGGER ERROR: Could not create log directory: {} - {}", filepathObj.parent_path().string(), fs_ex.what());
                        // Optionally, could try to log to current directory as a fallback
                    }
                }

                m_logFile.open(m_currentLogFilePath, std::ios_base::app); // Open in append mode
                if (!m_logFile.is_open())
                {
                    PrintError("LOGGER ERROR: Could not open log file: {}", m_currentLogFilePath);
                    m_fileOutputEnabled = false; // Disable if opening failed
                    return false;
                }
            }
        }
        return m_logFile.is_open();
    }

    void CLogging::logFileClose()
    {
        if (m_logFile.is_open())
        {
            m_logFile.flush();
            m_logFile.close();
        }
    }

    void CLogging::initialize(const std::string &mode, const std::string &appVersionInfo)
    {
        // This method can be used to set up initial logging state,
        // like enabling file output with a default name and logging version info.
        if (m_logFileBaseName.empty())
        { // Only generate if not already set (e.g. by direct enableFileOutput call)
            m_logFileBaseName = generateDefaultLogFileName(mode.empty() ? "APP" : mode);
        }
        // Default behavior: enable file output on initialize if not explicitly disabled.
        if (!m_fileOutputEnabled && !m_currentLogFilePath.empty())
        {                                                 // Check if a path was set by generateDefaultLogFileName
            enableFileOutput(true, m_currentLogFilePath); // This will open the file
        }
        else if (!m_fileOutputEnabled && m_logFileBaseName.empty())
        {
            // If still no base name, generate one and enable file output
            std::string defaultPath = generateDefaultLogFileName(mode.empty() ? "APP" : mode) + LogExtension;
            enableFileOutput(true, defaultPath);
        }

        if (!appVersionInfo.empty())
        {
            log(LogSeverity::LOG_INFO, appVersionInfo, MAKE_SOURCE_LOCATION());
        }
        else
        {
            // Example: Log a generic startup message if no version info provided
            log(LogSeverity::LOG_INFO, "Logger initialized.", MAKE_SOURCE_LOCATION());
        }
    }

    std::string CLogging::getCurrentTimestampISO8601()
    {
        const auto         nowChrono = std::chrono::system_clock::now();
        const auto         in_time_t = std::chrono::system_clock::to_time_t(nowChrono);
        std::ostringstream timestamp_ss;

        // Standard C++ way to get tm struct safely
        std::tm localTime{};
#if defined(_MSC_VER)
        localtime_s(&localTime, &in_time_t);
#else
        localtime_r(&in_time_t, &localTime); // POSIX
#endif

        timestamp_ss << std::put_time(&localTime, "%Y-%m-%dT%H:%M:%S");
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(nowChrono.time_since_epoch()) % 1000;
        timestamp_ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << "Z"; // ISO 8601 UTC indicator
        return timestamp_ss.str();
    }

    std::string CLogging::getModuleFileName()
    {
#if defined(_WIN32) || defined(_WIN64)
        char path[MAX_PATH] = {0};
        ::GetModuleFileNameA(NULL, path, MAX_PATH); // NULL gets current process
        return std::string(path);
#else
        // Basic fallback for non-Windows systems
        // More robust solutions would involve /proc/self/exe on Linux, etc.
        return "UnknownApp";
#endif
    }

    std::string CLogging::getApplicationName()
    {
        try
        {
            std::filesystem::path executablePath = getModuleFileName();
            return executablePath.stem().string(); // Filename without extension
        }
        catch (const std::exception &)
        {
            return "UnknownApp"; // Fallback
        }
    }

    std::string CLogging::generateDefaultLogFileName(const std::string &mode)
    {
        // This combines logic from original Logging.cpp's GenerateLogFileName and LogTesting.cpp
        const std::filesystem::path relLogPath  = REL_DIR_LOG; // e.g., "..\\Log\\"
        std::filesystem::path       appNamePath = getModuleFileName();
        std::string                 appName     = appNamePath.stem().string();
        std::filesystem::path       logDir;

        try
        {
            logDir = appNamePath.parent_path() / relLogPath;
            if (!std::filesystem::exists(logDir))
            {
                std::filesystem::create_directories(logDir);
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "LOGGER SETUP ERROR: Could not create log directory " << logDir << ": " << e.what() << std::endl;
            // Fallback to current directory if log directory creation fails
            logDir = std::filesystem::current_path();
        }

        // Simplified timestamp for filename (YYYYMMDD_HHMMSS)
        const auto         nowChrono = std::chrono::system_clock::now();
        const auto         in_time_t = std::chrono::system_clock::to_time_t(nowChrono);
        std::ostringstream timestamp_ss;
        std::tm            localTime{};
#if defined(_MSC_VER)
        localtime_s(&localTime, &in_time_t);
#else
        localtime_r(&in_time_t, &localTime);
#endif
        timestamp_ss << std::put_time(&localTime, "%Y%m%d_%H%M%S");
        std::string timeString             = timestamp_ss.str();

        // Format: LogDir / AppName_Timestamp_Mode. (extension will be added later)
        std::filesystem::path baseFilePath = logDir / (appName + "_" + timeString + "_" + mode);
        return baseFilePath.string();
    }

    void CLogging::logInternal(LogSeverity level, const source_location_t &location, std::string_view primaryMessage, std::optional<int> directErrorCode,
                               std::optional<std::error_code> stdErrorCode, std::optional<std::string> exceptionType,
                               std::optional<std::vector<std::string>> stackTrace)
    {
        if (level < m_minLogLevel && level != LogSeverity::LOG_ERROR && level != LogSeverity::LOG_FATAL)
        {
            // Allow ERROR and FATAL messages to bypass the minLogLevel filter if it's set higher.
            return;
        }

        std::scoped_lock lock(m_logMutex);
        std::string      timestampStr = getCurrentTimestampISO8601();

        // Console Output (Plain Text)
        if (m_consoleOutputEnabled)
        {
            std::ostringstream consoleMsg;
            consoleMsg << timestampStr << " [" << SeverityToString(level) << "] "
                       << "[" << std::filesystem::path(location.file_name()).filename().string() << ":" << location.line() << " (" << location.function_name()
                       << ")] " << primaryMessage;

            if (directErrorCode)
                consoleMsg << " (Error Code: " << *directErrorCode << ")";
            if (stdErrorCode)
                consoleMsg << " (System Error: " << stdErrorCode->value() << " - " << stdErrorCode->message() << ")";
            if (exceptionType)
                consoleMsg << " (Exception Type: " << *exceptionType << ")"; // primaryMessage is ex.what()

            if (level == LogSeverity::LOG_ERROR || level == LogSeverity::LOG_FATAL)
            {
                PrintError(consoleMsg.str());
            }
            else if (level == LogSeverity::LOG_WARNING)
            {
                PrintWarning(consoleMsg.str());
            }
            else
            {
                PrintInfo(consoleMsg.str());
            }
        }

        // File Output (JSON)
        if (m_fileOutputEnabled && logFileOpen())
        {
            boost::json::object jsonLogEntry;
            jsonLogEntry["timestamp"] = timestampStr;
            jsonLogEntry["level"]     = std::string(SeverityToString(level));
            jsonLogEntry["message"]   = std::string(primaryMessage); // Convert string_view to string for Boost.JSON

            boost::json::object sourceDetails;
            if (!stackTrace)
            {
                sourceDetails["file"]         = std::filesystem::path(location.file_name()).filename().string();
                sourceDetails["line"]         = location.line();
                sourceDetails["function"]     = location.function_name();
                jsonLogEntry["sourceDetails"] = sourceDetails;
            }
            else
            {
                boost::json::array stackTraceArray;
                for (const auto &trace : *stackTrace)
                {
                    stackTraceArray.push_back(boost::json::value(trace));
                }
                sourceDetails["stack_trace"] = stackTraceArray;
            }

            if (directErrorCode)
                sourceDetails["error_code"] = *directErrorCode;
            if (stdErrorCode)
            {
                sourceDetails["system_error_value"]    = stdErrorCode->value();
                sourceDetails["system_error_message"]  = stdErrorCode->message();
                sourceDetails["system_error_category"] = stdErrorCode->category().name();
            }
            if (exceptionType)
            {
                sourceDetails["exception_type"] = *exceptionType;
            }

            try
            {
                // Output NDJSON (each JSON object on a new line)
                m_logFile << boost::json::serialize(jsonLogEntry) << std::endl;
            }
            catch (const std::exception &e)
            {
                // Fallback to console if JSON serialization fails
                PrintError("INTERNAL LOGGER ERROR: Failed to serialize JSON for file: {}", e.what());
            }
            logFileClose();
        }
    }

    // --- Public logging method implementations ---
    void CLogging::log(LogSeverity level, std::string_view message, const source_location_t &location)
    {
        logInternal(level, location, message);
    }

    void CLogging::log(LogSeverity level, std::string_view message, int errorCode, const source_location_t &location)
    {
        logInternal(level, location, message, errorCode);
    }

    void CLogging::log(LogSeverity level, std::string_view message, const std::error_code &ec, const source_location_t &location)
    {
        logInternal(level, location, message, std::nullopt, ec);
    }

    void CLogging::log(LogSeverity level, const std::exception &ex, const source_location_t &location)
    {
        // For exceptions, ex.what() becomes the primary message.
        // typeid(ex).name() gives mangled name. Demangling is platform-specific and complex.
        logInternal(level, location, ex.what(), std::nullopt, std::nullopt, typeid(ex).name());
    }

    void CLogging::log(LogSeverity level, std::string_view message, const std::vector<std::string> &stackTrace, const source_location_t &location)
    {
        // For exceptions, ex.what() becomes the primary message.
        // typeid(ex).name() gives mangled name. Demangling is platform-specific and complex.
        logInternal(level, location, message, std::nullopt, std::nullopt, std::nullopt, stackTrace);
    }

    // --- Convenience method implementations ---
    void CLogging::info(std::string_view message, const source_location_t &loc)
    {
        log(LogSeverity::LOG_INFO, message, loc);
    }

    void CLogging::warning(std::string_view message, const source_location_t &loc)
    {
        log(LogSeverity::LOG_WARNING, message, loc);
    }
    void CLogging::warning(std::string_view message, int errorCode, const source_location_t &loc)
    {
        log(LogSeverity::LOG_WARNING, message, errorCode, loc);
    }
    void CLogging::warning(std::string_view message, const std::error_code &ec, const source_location_t &loc)
    {
        log(LogSeverity::LOG_WARNING, message, ec, loc);
    }
    void CLogging::warning(const std::exception &ex, const source_location_t &loc)
    {
        log(LogSeverity::LOG_WARNING, ex, loc);
    }
    void CLogging::error(std::string_view message, const source_location_t &loc)
    {
        log(LogSeverity::LOG_ERROR, message, loc);
    }
    void CLogging::error(const std::exception &ex, const source_location_t &loc)
    {
        log(LogSeverity::LOG_ERROR, ex, loc);
    }
    void CLogging::error(std::string_view message, int errorCode, const source_location_t &loc)
    {
        log(LogSeverity::LOG_ERROR, message, errorCode, loc);
    }
    void CLogging::error(std::string_view message, const std::error_code &ec, const source_location_t &loc)
    {
        log(LogSeverity::LOG_ERROR, message, ec, loc);
    }
    void CLogging::debug(std::string_view message, const source_location_t &loc)
    {
        log(LogSeverity::LOG_DEBUG, message, loc);
    }
    void CLogging::fatal(std::string_view message, const source_location_t &loc)
    {
        log(LogSeverity::LOG_FATAL, message, loc);
        if (m_logFile.is_open())
        {
            m_logFile.close();
        }
    }

    // --- Configuration method implementations ---
    void CLogging::enableConsoleOutput(bool enable)
    {
        std::scoped_lock lock(m_logMutex);
        m_consoleOutputEnabled = enable;
    }

    void CLogging::enableFileOutput(bool enable, const std::string &userFilepath)
    {
        std::scoped_lock lock(m_logMutex);
        m_fileOutputEnabled = enable;

        if (enable)
        {
            std::string actualFilepath = userFilepath;
            if (userFilepath.empty())
            {
                // If m_logFileBaseName isn't set yet (e.g. initialize not called, or called with no mode)
                // we need to generate a default name.
                if (m_logFileBaseName.empty())
                {
                    m_logFileBaseName = generateDefaultLogFileName("LOG"); // Default mode if none provided
                }
                actualFilepath = m_logFileBaseName + LogExtension;
            }

            m_currentLogFilePath = actualFilepath; // Store the path being used
        }
    }

    void CLogging::setMinLogLevel(LogSeverity level)
    {
        std::scoped_lock lock(m_logMutex);
        m_minLogLevel = level;
    }

    LogSeverity CLogging::getMinLogLevel() const
    {
        // Mutex not strictly needed for const access to an enum if writes are protected,
        // but for consistency with setMinLogLevel or if it could be read during construction.
        // std::scoped_lock lock(m_logMutex); // Uncomment if strict thread safety for this read is paramount
        return m_minLogLevel;
    }

    // --- CPrintLogRecord Member Function Definitions (from original Logging.cpp) ---
    // Assuming PrintInfo is defined in Print.h or similar
    // If PrintInfo is not available, these will cause linker errors or need to use CLogging.

    // Forward declaration or include for PrintInfo if it's from Print.h
    // For example: void PrintInfo(const char* format, ...);
    // Or, if PrintInfo is part of CTrack namespace:
    // namespace CTrack { void PrintInfo(const char* fmt, ...); }
    // For now, I'll assume it's available globally or in CTrack.
    // If Print.h is not included, these will need to be adapted or removed.
    // For example, to use the new logger:
    // CTrack::CLogging::getInstance().info(fmt::format("{} : {}", m_Identifier, m_StartMessage), MAKE_SOURCE_LOCATION());

    CPrintLogRecord::CPrintLogRecord(const char *iStartMessage, const char *iStopMessage, const char *iIdentifier)
        : m_StartMessage(iStartMessage), m_StopMessage(iStopMessage), m_Identifier(iIdentifier)
    {
        m_Timer = std::chrono::high_resolution_clock::now();
        // Original used PrintInfo. If PrintInfo is to be replaced by CLogging:
        LOG_INFO(fmt::format("{} : {}", m_Identifier, m_StartMessage));
        // If PrintInfo is a separate mechanism from "Print.h", it would be:
        // PrintInfo("{} : {}", m_Identifier, m_StartMessage); // Requires Print.h and its implementation
    }

    CPrintLogRecord::~CPrintLogRecord()
    {
        auto                          finish  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - m_Timer;
        // Original used PrintInfo. If PrintInfo is to be replaced by CLogging:
        LOG_INFO(fmt::format("{} : {} [{:.3f}s]", m_Identifier, m_StopMessage, elapsed.count()));
        // If PrintInfo is a separate mechanism from "Print.h", it would be:
        // PrintInfo("{} : {} [{:.3f}s]", m_Identifier, m_StopMessage, elapsed.count()); // Requires Print.h
    }

    // --- Implementation of any adapted old free functions (if kept for compatibility) ---
    // Example:
    // void Log(LogSeverity severity, const std::string &iMessage) {
    //     CLogging::getInstance().log(severity, iMessage, MAKE_SOURCE_LOCATION());
    // }
    // void InitLogging(const std::string mode) {
    //    std::string versionString = "App Version Placeholder"; // Get actual version
    //    CLogging::getInstance().initialize(mode, versionString);
    // }

} // namespace CTrack
