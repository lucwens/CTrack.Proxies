#include "StressTest.h"
#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/Logging.h"
#include "../Libraries/XML/ProxyKeywords.h"

#include <filesystem>
#include <fmt/core.h>
#include <iomanip>
#include <sstream>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

StressTest::StressTest(DriverVicon *driver, std::shared_ptr<CTrack::MessageResponder> responder)
    : m_pDriver(driver), m_pResponder(responder), m_RandomEngine(std::random_device{}()), m_ActionDistribution(0, 6) // 7 possible actions
{
}

StressTest::~StressTest()
{
    Stop();
    CloseLogFile();
}

void StressTest::Start()
{
    if (m_State == TestState::Running)
    {
        PrintWarning("Stress test is already running");
        return;
    }

    m_bStopRequested   = false;
    m_IterationCount   = 0;
    m_ErrorCount       = 0;
    m_SuccessCount     = 0;
    m_bCurrentlyTracking = false;
    m_StartTime        = std::chrono::steady_clock::now();

    InitLogFile();
    LogInfo("=== STRESS TEST STARTED ===");
    LogInfo(fmt::format("Purpose: Test Vicon communication stability"));
    LogInfo(fmt::format("Actions: Hardware Detect, Config Detect, Start Tracking, Stop Tracking, Wait periods"));

    m_State      = TestState::Running;
    m_TestThread = std::thread(&StressTest::TestLoop, this);

    PrintInfo("Stress test STARTED - Press 'y' to stop");
}

void StressTest::Stop()
{
    if (m_State != TestState::Running)
    {
        return;
    }

    m_bStopRequested = true;
    m_State          = TestState::Stopping;
    PrintInfo("Stopping stress test (shutdown will be performed as final action)...");
    LogInfo("Stop requested by user - waiting for test loop to complete with shutdown");

    if (m_TestThread.joinable())
    {
        m_TestThread.join();
    }

    // Note: TestLoop now handles the final shutdown internally

    auto   endTime     = std::chrono::steady_clock::now();
    auto   duration    = std::chrono::duration_cast<std::chrono::seconds>(endTime - m_StartTime);
    int    hours       = static_cast<int>(duration.count() / 3600);
    int    minutes     = static_cast<int>((duration.count() % 3600) / 60);
    int    seconds     = static_cast<int>(duration.count() % 60);

    LogInfo("=== STRESS TEST COMPLETED ===");
    LogInfo(fmt::format("Total duration: {}h {}m {}s", hours, minutes, seconds));
    LogInfo(fmt::format("Total iterations: {}", m_IterationCount.load()));
    LogInfo(fmt::format("Successful actions: {}", m_SuccessCount.load()));
    LogInfo(fmt::format("Errors encountered: {}", m_ErrorCount.load()));

    CloseLogFile();
    m_State = TestState::Idle;

    PrintInfo("Stress test STOPPED");
    PrintInfo(fmt::format("Results: {} iterations, {} successes, {} errors", m_IterationCount.load(), m_SuccessCount.load(), m_ErrorCount.load()));
}

bool StressTest::IsRunning() const
{
    return m_State == TestState::Running;
}

StressTest::TestState StressTest::GetState() const
{
    return m_State;
}

std::string StressTest::GetStatusString() const
{
    switch (m_State)
    {
        case TestState::Idle:
            return "Idle";
        case TestState::Running:
            return fmt::format("Running - Iteration {}, Errors {}", m_IterationCount.load(), m_ErrorCount.load());
        case TestState::Stopping:
            return "Stopping...";
        case TestState::Error:
            return fmt::format("Error - stopped after {} iterations with {} errors", m_IterationCount.load(), m_ErrorCount.load());
        default:
            return "Unknown";
    }
}

int StressTest::GetIterationCount() const
{
    return m_IterationCount;
}

int StressTest::GetErrorCount() const
{
    return m_ErrorCount;
}

void StressTest::TestLoop()
{
    LogInfo("Test loop started");

    // Phase 1: Mandatory Hardware Detection
    LogInfo("=== PHASE 1: Hardware Detection ===");
    m_IterationCount++;
    LogInfo(fmt::format("--- Iteration {} ---", m_IterationCount.load()));
    LogInfo("Executing mandatory Hardware Detection");

    if (!DoHardwareDetect())
    {
        m_ErrorCount++;
        LogError("Mandatory Hardware Detection failed - aborting stress test");
        m_State = TestState::Error;
        return;
    }
    m_SuccessCount++;
    LogInfo("Hardware Detection completed successfully");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (m_bStopRequested)
    {
        LogInfo("Stop requested after hardware detection");
        DoShutdown();
        return;
    }

    // Phase 2: Mandatory Configuration Detection
    LogInfo("=== PHASE 2: Configuration Detection ===");
    m_IterationCount++;
    LogInfo(fmt::format("--- Iteration {} ---", m_IterationCount.load()));
    LogInfo("Executing mandatory Configuration Detection");

    if (!DoConfigDetect())
    {
        m_ErrorCount++;
        LogError("Mandatory Configuration Detection failed - aborting stress test");
        m_State = TestState::Error;
        DoShutdown();
        return;
    }
    m_SuccessCount++;
    LogInfo("Configuration Detection completed successfully");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (m_bStopRequested)
    {
        LogInfo("Stop requested after configuration detection");
        DoShutdown();
        return;
    }

    // Phase 3: Random Actions
    LogInfo("=== PHASE 3: Random Actions ===");
    LogInfo("Starting random action sequence - press 'y' to stop");

    while (!m_bStopRequested)
    {
        m_IterationCount++;
        TestAction action = SelectRandomAction();

        LogInfo(fmt::format("--- Iteration {} ---", m_IterationCount.load()));
        LogInfo(fmt::format("Selected action: {}", ActionToString(action)));

        bool success = ExecuteAction(action);

        if (!success)
        {
            m_ErrorCount++;
            LogError(fmt::format("Action failed! Total errors: {}", m_ErrorCount.load()));

            // On error, we stop the test
            LogError("Stopping stress test due to error");
            m_State = TestState::Error;
            break;
        }
        else
        {
            m_SuccessCount++;
        }

        // Small delay between actions to avoid overwhelming the system
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Phase 4: Final Shutdown
    LogInfo("=== PHASE 4: Final Shutdown ===");
    DoShutdown();

    LogInfo("Test loop ended");
}

StressTest::TestAction StressTest::SelectRandomAction()
{
    // Smart action selection based on current state
    // If currently tracking, prefer operations that make sense while tracking
    // If not tracking, prefer operations that make sense when idle

    int actionIndex = m_ActionDistribution(m_RandomEngine);

    // Map random number to action, with state-aware logic
    if (m_bCurrentlyTracking)
    {
        // When tracking: prefer wait operations or stop
        switch (actionIndex)
        {
            case 0:
            case 1:
                return TestAction::WaitShort;
            case 2:
                return TestAction::WaitMedium;
            case 3:
                return TestAction::WaitLong;
            case 4:
            case 5:
            case 6:
                return TestAction::StopTracking;
            default:
                return TestAction::WaitShort;
        }
    }
    else
    {
        // When not tracking: can do hardware detect, config detect, or start tracking
        switch (actionIndex)
        {
            case 0:
            case 1:
                return TestAction::HardwareDetect;
            case 2:
            case 3:
                return TestAction::ConfigDetect;
            case 4:
            case 5:
            case 6:
                return TestAction::StartTracking;
            default:
                return TestAction::HardwareDetect;
        }
    }
}

bool StressTest::ExecuteAction(TestAction action)
{
    bool        success = false;
    std::string details;

    auto startTime = std::chrono::steady_clock::now();

    try
    {
        switch (action)
        {
            case TestAction::HardwareDetect:
                success = DoHardwareDetect();
                break;
            case TestAction::ConfigDetect:
                success = DoConfigDetect();
                break;
            case TestAction::StartTracking:
                success = DoStartTracking();
                break;
            case TestAction::StopTracking:
                success = DoStopTracking();
                break;
            case TestAction::WaitShort:
                DoWait(m_WaitShortDistribution.a(), m_WaitShortDistribution.b());
                success = true;
                break;
            case TestAction::WaitMedium:
                DoWait(m_WaitMediumDistribution.a(), m_WaitMediumDistribution.b());
                success = true;
                break;
            case TestAction::WaitLong:
                DoWait(m_WaitLongDistribution.a(), m_WaitLongDistribution.b());
                success = true;
                break;
        }
    }
    catch (const std::exception &e)
    {
        success = false;
        details = fmt::format("Exception: {}", e.what());
        LogError(details);
        LOG_ERROR_MSG(details);
    }
    catch (...)
    {
        success = false;
        details = "Unknown exception occurred";
        LogError(details);
        LOG_ERROR_MSG(details);
    }

    auto endTime      = std::chrono::steady_clock::now();
    auto durationMs   = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    details           = fmt::format("Duration: {}ms", durationMs);

    LogAction(action, success, details);
    return success;
}

std::string StressTest::ActionToString(TestAction action) const
{
    switch (action)
    {
        case TestAction::HardwareDetect:
            return "Hardware Detect";
        case TestAction::ConfigDetect:
            return "Config Detect";
        case TestAction::StartTracking:
            return "Start Tracking";
        case TestAction::StopTracking:
            return "Stop Tracking";
        case TestAction::WaitShort:
            return "Wait Short (5-30s)";
        case TestAction::WaitMedium:
            return "Wait Medium (30-120s)";
        case TestAction::WaitLong:
            return "Wait Long (2-5min)";
        default:
            return "Unknown";
    }
}

bool StressTest::DoHardwareDetect()
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("StressTest::HardwareDetect", 0x44FF44);  // Green
#endif
    LogInfo("Executing Hardware Detect...");
    PrintInfo("STRESS TEST: Hardware Detect");

    CTrack::Message message(TAG_COMMAND_HARDWAREDETECT);
    CTrack::Reply   reply = m_pDriver->HardwareDetect(message);

    if (reply)
    {
        bool present = reply->GetParams().value(ATTRIB_HARDWAREDETECT_PRESENT, false);
        std::string feedback = reply->GetParams().value(ATTRIB_HARDWAREDETECT_FEEDBACK, std::string("No feedback"));

        LogInfo(fmt::format("Hardware present: {}", present ? "Yes" : "No"));
        LogInfo(fmt::format("Feedback: {}", feedback));

        if (!present)
        {
            LogWarning("Hardware not detected - this may indicate a connection issue");
            return false;
        }
        return true;
    }

    LogError("Hardware detect returned null reply");
    return false;
}

bool StressTest::DoConfigDetect()
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("StressTest::ConfigDetect", 0xFFAA00);  // Orange
#endif
    LogInfo("Executing Config Detect...");
    PrintInfo("STRESS TEST: Config Detect");

    CTrack::Message message(TAG_COMMAND_CONFIGDETECT);
    CTrack::Reply   reply = m_pDriver->ConfigDetect(message);

    if (reply)
    {
        auto &params = reply->GetParams();

        // Log detected markers
        if (params.contains(ATTRIB_CONFIG_3DMARKERS))
        {
            auto markers = params[ATTRIB_CONFIG_3DMARKERS].get<std::vector<std::string>>();
            LogInfo(fmt::format("Detected {} 3D markers", markers.size()));
        }

        // Log detected 6DOF subjects
        if (params.contains(ATTRIB_6DOF))
        {
            int subjectCount = 0;
            for (auto &[key, value] : params[ATTRIB_6DOF].items())
            {
                subjectCount++;
                LogInfo(fmt::format("6DOF Subject: {}", key));
            }
            LogInfo(fmt::format("Total 6DOF subjects: {}", subjectCount));
        }

        return true;
    }

    LogError("Config detect returned null reply");
    return false;
}

bool StressTest::DoStartTracking()
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("StressTest::StartTracking", 0x00FFFF);  // Cyan
#endif
    if (m_bCurrentlyTracking)
    {
        LogWarning("Already tracking - skipping start");
        return true;
    }

    LogInfo("Executing Start Tracking...");
    PrintInfo("STRESS TEST: Start Tracking");

    CTrack::Message message(TAG_COMMAND_CHECKINIT);
    message.GetParams()[ATTRIB_CHECKINIT_MEASFREQ] = 50.0; // 50 Hz

    CTrack::Reply reply = m_pDriver->CheckInitialize(message);

    if (reply)
    {
        bool result = reply->GetParams().value(ATTRIB_RESULT, false);
        std::string feedback = reply->GetParams().value(ATTRIB_RESULT_FEEDBACK, std::string(""));

        if (result)
        {
            m_bCurrentlyTracking = true;
            LogInfo("Tracking started successfully at 50 Hz");
            return true;
        }
        else
        {
            LogError(fmt::format("Failed to start tracking: {}", feedback));
            return false;
        }
    }

    LogError("Start tracking returned null reply");
    return false;
}

bool StressTest::DoStopTracking()
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("StressTest::StopTracking", 0xFF4444);  // Red
#endif
    if (!m_bCurrentlyTracking)
    {
        LogWarning("Not tracking - skipping stop");
        return true;
    }

    LogInfo("Executing Stop Tracking...");
    PrintInfo("STRESS TEST: Stop Tracking");

    CTrack::Message message(TAG_COMMAND_SHUTDOWN);
    CTrack::Reply   reply = m_pDriver->ShutDown(message);

    if (reply)
    {
        bool result = reply->GetParams().value(ATTRIB_RESULT, false);

        if (result)
        {
            m_bCurrentlyTracking = false;
            LogInfo("Tracking stopped successfully");
            return true;
        }
        else
        {
            LogError("Failed to stop tracking");
            return false;
        }
    }

    LogError("Stop tracking returned null reply");
    return false;
}

bool StressTest::DoShutdown()
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("StressTest::Shutdown", 0xFF8800);  // Dark Orange
#endif
    LogInfo("Executing Final Shutdown...");
    PrintInfo("STRESS TEST: Final Shutdown");

    // First stop tracking if we're tracking
    if (m_bCurrentlyTracking)
    {
        LogInfo("Stopping tracking before shutdown...");
        DoStopTracking();
    }

    CTrack::Message message(TAG_COMMAND_SHUTDOWN);
    CTrack::Reply   reply = m_pDriver->ShutDown(message);

    if (reply)
    {
        bool result = reply->GetParams().value(ATTRIB_RESULT, false);

        if (result)
        {
            LogInfo("Shutdown completed successfully");
            return true;
        }
        else
        {
            LogWarning("Shutdown returned false - may already be shut down");
            return true; // Not critical if already shut down
        }
    }

    LogWarning("Shutdown returned null reply - may already be shut down");
    return true; // Not critical
}

void StressTest::DoWait(int minSeconds, int maxSeconds)
{
#ifdef TRACY_ENABLE
    ZoneScopedNC("StressTest::Wait", 0x888888);  // Gray
#endif
    std::uniform_int_distribution<int> waitDist(minSeconds, maxSeconds);
    int                                waitSeconds = waitDist(m_RandomEngine);

    LogInfo(fmt::format("Waiting for {} seconds...", waitSeconds));
    PrintInfo(fmt::format("STRESS TEST: Waiting {} seconds", waitSeconds));

    // Wait in small increments to allow for stop requests
    int elapsed = 0;
    while (elapsed < waitSeconds && !m_bStopRequested)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed++;

        // If we're tracking, periodically check that we're still getting data
        if (m_bCurrentlyTracking && elapsed % 10 == 0)
        {
            // Call Run() to check connection health
            if (!m_pDriver->Run())
            {
                LogWarning("Driver Run() returned false during wait - connection may have dropped");
            }
        }
    }

    LogInfo(fmt::format("Wait completed ({} seconds)", elapsed));
}

void StressTest::InitLogFile()
{
    // Generate log file path
    std::filesystem::path exePath = std::filesystem::current_path();
    std::filesystem::path logDir  = exePath / ".." / "Log";

    try
    {
        std::filesystem::create_directories(logDir);
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        PrintError(fmt::format("Failed to create log directory: {}", e.what()));
        logDir = exePath;
    }

    // Generate timestamp for filename
    auto         now       = std::chrono::system_clock::now();
    auto         time_t    = std::chrono::system_clock::to_time_t(now);
    std::tm      localTime{};
    localtime_s(&localTime, &time_t);

    std::ostringstream timestamp;
    timestamp << std::put_time(&localTime, "%Y%m%d_%H%M%S");

    m_LogFilePath = (logDir / fmt::format("ViconStressTest_{}.log", timestamp.str())).string();

    std::lock_guard<std::mutex> lock(m_LogMutex);
    m_LogFile.open(m_LogFilePath, std::ios::out | std::ios::app);

    if (!m_LogFile.is_open())
    {
        PrintError(fmt::format("Failed to open stress test log file: {}", m_LogFilePath));
    }
    else
    {
        PrintInfo(fmt::format("Stress test log file: {}", m_LogFilePath));
    }
}

void StressTest::CloseLogFile()
{
    std::lock_guard<std::mutex> lock(m_LogMutex);
    if (m_LogFile.is_open())
    {
        m_LogFile.flush();
        m_LogFile.close();
    }
}

std::string StressTest::GetTimestamp() const
{
    auto                       now       = std::chrono::system_clock::now();
    auto                       time_t    = std::chrono::system_clock::to_time_t(now);
    auto                       ms        = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm                    localTime{};
    localtime_s(&localTime, &time_t);

    std::ostringstream timestamp;
    timestamp << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();
    return timestamp.str();
}

void StressTest::LogMessage(const std::string &level, const std::string &message)
{
    std::string                 logLine = fmt::format("[{}] [{}] {}", GetTimestamp(), level, message);
    std::lock_guard<std::mutex> lock(m_LogMutex);

    if (m_LogFile.is_open())
    {
        m_LogFile << logLine << std::endl;
        m_LogFile.flush(); // Ensure immediate write for debugging
    }
}

void StressTest::LogInfo(const std::string &message)
{
    LogMessage("INFO", message);
    LOG_INFO(fmt::format("STRESS_TEST: {}", message));
#ifdef TRACY_ENABLE
    TracyMessageC(message.c_str(), message.size(), 0x44FF44);  // Green for info
#endif
}

void StressTest::LogWarning(const std::string &message)
{
    LogMessage("WARNING", message);
    PrintWarning(fmt::format("STRESS_TEST: {}", message));
    LOG_WARNING(fmt::format("STRESS_TEST: {}", message));
#ifdef TRACY_ENABLE
    TracyMessageC(message.c_str(), message.size(), 0xFFFF00);  // Yellow for warning
#endif
}

void StressTest::LogError(const std::string &message)
{
    LogMessage("ERROR", message);
    PrintError(fmt::format("STRESS_TEST: {}", message));
    LOG_ERROR_MSG(fmt::format("STRESS_TEST: {}", message));
#ifdef TRACY_ENABLE
    TracyMessageC(message.c_str(), message.size(), 0xFF4444);  // Red for error
#endif
}

void StressTest::LogAction(TestAction action, bool success, const std::string &details)
{
    std::string status   = success ? "SUCCESS" : "FAILED";
    std::string logEntry = fmt::format("Action: {} | Status: {} | {}", ActionToString(action), status, details);
    LogMessage(success ? "INFO" : "ERROR", logEntry);
#ifdef TRACY_ENABLE
    if (success)
    {
        TracyMessageC(logEntry.c_str(), logEntry.size(), 0x44FF44);  // Green for success
    }
    else
    {
        TracyMessageC(logEntry.c_str(), logEntry.size(), 0xFF4444);  // Red for failure
    }
#endif
}
