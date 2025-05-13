#pragma once

#include <chrono>
#include <vector>
#include <boost/json.hpp>

namespace CTrack
{

    const std::string LogExtension  = "ndjson";
    const std::string DumpExtension = "dmp";

    enum class LogSeverity
    {
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
        LOG_FATAL
    };

    class CLogging
    {
      public:
        CLogging()                                   = default;
        ~CLogging()                                  = default;
        CLogging(const CLogging &)                   = delete;
        CLogging        &operator=(const CLogging &) = delete;
        static CLogging &GetInstance()
        {
            static CLogging instance;
            return instance;
        }

      private:
        std::string m_LogFileName;
        std::string m_DumpFileName;
        std::string m_LogMode;
        bool        m_UseJson{false};
        bool        m_UseDump{false};
    };

        void
        InitLogging(const std::string mode);

    boost::json::object CreateJSON(LogSeverity severity, const std::string Message);
    std::string         GetJsonLogString(boost::json::object &iMessageJSON, bool pretty = true);
    void                Log(LogSeverity severity, const std::string &iMessage);
    void                Log(boost::json::object &iMessageJSON);

    void LogError(int iErrorCode = 0, const char *iSourceFile = NULL, int iSourceFileLine = 0, const char *iMessage = NULL);
    void LogError(const char *iMessage);
    void LogError(const std::string iMessage);

    std::string SeverityToString(LogSeverity severity);
    double      GetDurationInSeconds(); // this is how long the program is running
    std::string GetTimeStampString(int NumDecimals = 3, char TimeSeparator = ':', bool IncludeDuration = true);
    std::string GetLogFileName(const std::string Extension, const std::string mode = "UI");

    //--------------------------------------------------------------------------------------------------------------------------------------
    /*
     * CPrintLogRecord for all profiling related exceptions.
     */
    //--------------------------------------------------------------------------------------------------------------------------------------
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
} // namespace CTrack
