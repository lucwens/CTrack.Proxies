

#include "../../../version.h"
#include "Logging.h"
#include "Print.h"

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <sys\timeb.h>
#include <time.h>
#include <windows.h>

#ifndef REL_DIR_LOG
#define REL_DIR_LOG "..\\Log\\"
#endif

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

    std::string GenerateLogFileName(const std::string mode)
    {
        const std::filesystem::path relLogPath = REL_DIR_LOG;
        std::filesystem::path       exePath    = std::filesystem::absolute(GetExecutablePath());
        std::string                 appName    = exePath.stem().string();
        std::filesystem::path       logDir     = exePath.parent_path() / relLogPath;
        std::filesystem::create_directories(logDir);
        std::string timeString  = GetTimeStampString(0, '_', false);
        std::string logFileName = fmt::format("{}{}_{}_{}.", logDir.string(), appName, timeString, mode);

        return logFileName;
    }

    std::string GetLogFileName(const std::string Extension, const std::string mode)
    {
        std::string ReturnString;
        if (g_LogFileBaseName.empty())
        {
            g_LogFileBaseName = GenerateLogFileName(mode);
        }
        return g_LogFileBaseName + Extension;
    }

    void InitLogging(const std::string mode)
    {
        std::string log_file      = GetLogFileName(LogExtension, mode);
        std::string VersionString = fmt::format("{} {} {}", GIT_TAG, BUILD_DATE, GIT_HASH);
        Log(LogSeverity::LOG_INFO, VersionString);
    }

    void LogError(int iErrorCode, const char *iSourceFile, int iSourceFileLine, const char *iMessage)
    {
        std::string TotalErrormessage = fmt::format("Error {} in {} at line {}:\n{}", iErrorCode, iSourceFile, iSourceFileLine, iMessage);
        Log(LogSeverity::LOG_ERROR, TotalErrormessage);
    }

    void LogError(const char *iMessage)
    {
        Log(LogSeverity::LOG_ERROR, std::string(iMessage));
    }

    void LogError(const std::string iMessage)
    {
        Log(LogSeverity::LOG_ERROR, iMessage);
    }

    std::string SeverityToString(LogSeverity s)
    {
        switch (s)
        {
            case LogSeverity::LOG_INFO:
                return "[INFO]";
            case LogSeverity::LOG_WARNING:
                return "[WARN]";
            case LogSeverity::LOG_ERROR:
                return "[ERROR]";
            case LogSeverity::LOG_DEBUG:
                return "[DEBUG]";
            case LogSeverity::LOG_FATAL:
                return "[FATAL]";
            default:
                return "[UNKNOWN]";
        }
    }

    boost::json::object CreateJSON(LogSeverity severity, const std::string Message)
    {
        // Create a JSON object with the current timestamp, severity, and message
        boost::json::object jsonObject;
        jsonObject["timestamp"] = GetTimeStampString(3, ':', true);
        jsonObject["severity"]  = SeverityToString(severity);
        jsonObject["message"]   = Message;
        return jsonObject;
    }

    std::string pretty_print_to_string(const boost::json::value &jv, std::string *indent = nullptr)
    {
        std::ostringstream os;

        std::string indent_;
        if (!indent)
            indent = &indent_;

        switch (jv.kind())
        {
            case boost::json::kind::object:
            {
                os << "{\n";
                indent->append(4, ' ');
                auto const &obj = jv.get_object();
                if (!obj.empty())
                {
                    auto it = obj.begin();
                    for (;;)
                    {
                        os << *indent << boost::json::serialize(it->key()) << " : ";
                        os << pretty_print_to_string(it->value(), indent);
                        if (++it == obj.end())
                            break;
                        os << ",\n";
                    }
                }
                os << "\n";
                indent->resize(indent->size() - 4);
                os << *indent << "}";
                break;
            }

            case boost::json::kind::array:
            {
                os << "[\n";
                indent->append(4, ' ');
                auto const &arr = jv.get_array();
                if (!arr.empty())
                {
                    auto it = arr.begin();
                    for (;;)
                    {
                        os << *indent;
                        os << pretty_print_to_string(*it, indent);
                        if (++it == arr.end())
                            break;
                        os << ",\n";
                    }
                }
                os << "\n";
                indent->resize(indent->size() - 4);
                os << *indent << "]";
                break;
            }

            case boost::json::kind::string:
                os << boost::json::serialize(jv.get_string());
                break;

            case boost::json::kind::uint64:
            case boost::json::kind::int64:
            case boost::json::kind::double_:
                os << jv;
                break;

            case boost::json::kind::bool_:
                os << (jv.get_bool() ? "true" : "false");
                break;

            case boost::json::kind::null:
                os << "null";
                break;
        }

        if (indent->empty())
            os << "\n";

        return os.str();
    }

    std::string GetJsonLogString(boost::json::object & iMessageJSON, bool pretty)
    {
        if (!pretty)
        {
            return boost::json::serialize(iMessageJSON);
        }
        else
        {
            return pretty_print_to_string(iMessageJSON);
        }
    }

    void Log(LogSeverity severity, const std::string &iMessage)
    {
        boost::json::object JSON = CreateJSON(severity, iMessage);
        Log(JSON);
    }

    void Log(boost::json::object & iMessageJSON)
    {
        std::unique_lock<std::mutex> lock(g_LogMutex);
        std::string                  logFileName = GetLogFileName(LogExtension);
        std::ofstream                logFile(logFileName, std::ios::app);
        if (!logFile.is_open())
            return;
        logFile << GetJsonLogString(iMessageJSON);
    }

    //---------------------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------------------
    CPrintLogRecord::CPrintLogRecord(const char *iStartMessage, const char *iStopMessage, const char *iIdentifier)
        : m_StartMessage(iStartMessage), m_StopMessage(iStopMessage), m_Identifier(iIdentifier)
    {
        m_Timer = std::chrono::high_resolution_clock::now();
        PrintInfo("{} : {}", m_Identifier, m_StartMessage);
    }

    CPrintLogRecord::~CPrintLogRecord()
    {
        auto                          finish  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - m_Timer;
        PrintInfo("{} : {} [{}]", m_Identifier, m_StopMessage, elapsed.count());
    }
}