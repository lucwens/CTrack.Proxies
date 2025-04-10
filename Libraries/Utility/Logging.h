#pragma once

#include <chrono>
#include <vector>
#include <boost/json.hpp>

const std::string LogExtension  = "ndjson";
const std::string DumpExtension = "dmp";

enum class LogSeverity
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
};

void InitLogging();

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
std::string GetLogFileName(const std::string Extension);

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

#define PRINTLOGRECORD(START, STOP, ID) CPrintLogRecord ProfileRecord(START, STOP, ID);
