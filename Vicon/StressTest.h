#pragma once

#include "../Libraries/TCP/Message.h"
#include "../Libraries/TCP/MessageResponder.h"
#include "DriverVicon.h"

#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

// Forward declaration
class DriverVicon;

/**
 * @brief Stress test class for testing Vicon communication stability
 *
 * This class performs random sequences of actions (hardware detect, config detect,
 * start, stop) to test the robustness of the Vicon connection. All actions are
 * logged to a dedicated log file, and the test automatically stops on errors.
 *
 * When tracking is started, a random measurement duration (10-120 seconds) is selected.
 * Tracking will continue for at least this duration before stopping is allowed,
 * ensuring meaningful measurement periods for stability testing.
 */
class StressTest
{
  public:
    enum class TestAction
    {
        HardwareDetect,
        ConfigDetect,
        StartTracking,
        StopTracking,
        WaitShort,   // Wait 5-30 seconds
        WaitMedium,  // Wait 30-120 seconds
        WaitLong     // Wait 2-5 minutes
    };

    enum class TestState
    {
        Idle,
        Running,
        Stopping,
        Error
    };

    StressTest(DriverVicon *driver, std::shared_ptr<CTrack::MessageResponder> responder);
    ~StressTest();

    // Start/stop the stress test
    void Start();
    void Stop();
    bool IsRunning() const;

    // Get current state and statistics
    TestState   GetState() const;
    std::string GetStatusString() const;
    int         GetIterationCount() const;
    int         GetErrorCount() const;
    bool        IsTracking() const;  // Returns true if stress test is currently in a tracking session

  private:
    // Test execution
    void        TestLoop();
    TestAction  SelectRandomAction();
    bool        ExecuteAction(TestAction action);
    std::string ActionToString(TestAction action) const;

    // Action implementations
    bool DoHardwareDetect();
    bool DoConfigDetect();
    bool DoStartTracking();
    bool DoStopTracking();
    bool DoShutdown();
    void DoWait(int minSeconds, int maxSeconds);

    // Logging
    void InitLogFile();
    void CloseLogFile();
    void LogMessage(const std::string &level, const std::string &message);
    void LogInfo(const std::string &message);
    void LogWarning(const std::string &message);
    void LogError(const std::string &message);
    void LogAction(TestAction action, bool success, const std::string &details = "");

    // Timestamp helper
    std::string GetTimestamp() const;

    // Member variables
    DriverVicon                               *m_pDriver;
    std::shared_ptr<CTrack::MessageResponder>  m_pResponder;

    std::atomic<TestState> m_State{TestState::Idle};
    std::atomic<bool>      m_bStopRequested{false};
    std::atomic<int>       m_IterationCount{0};
    std::atomic<int>       m_ErrorCount{0};
    std::atomic<int>       m_SuccessCount{0};

    // Tracking state for stress test logic
    std::atomic<bool>                         m_bCurrentlyTracking{false};
    std::chrono::steady_clock::time_point     m_TrackingStartTime;
    int                                       m_RequiredMeasurementSeconds{0};

    // Thread management
    std::thread m_TestThread;

    // Logging
    std::ofstream m_LogFile;
    std::mutex    m_LogMutex;
    std::string   m_LogFilePath;

    // Random number generation
    std::mt19937                          m_RandomEngine;
    std::uniform_int_distribution<int>    m_ActionDistribution;
    std::uniform_int_distribution<int>    m_WaitShortDistribution{5, 30};
    std::uniform_int_distribution<int>    m_WaitMediumDistribution{30, 120};
    std::uniform_int_distribution<int>    m_WaitLongDistribution{120, 300};
    std::uniform_int_distribution<int>    m_MeasurementDurationDistribution{10, 120};  // 10 seconds to 2 minutes

    // Test start time
    std::chrono::steady_clock::time_point m_StartTime;
};
