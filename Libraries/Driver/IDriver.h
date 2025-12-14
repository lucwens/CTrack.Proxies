#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace CTrack
{

/// @brief Abstract interface for all device drivers
/// @details Enables device-agnostic stress testing and profiling across all proxy types.
///          All drivers (Vicon, Leica.LMF, NDI, Template, etc.) should implement this interface.
class IDriver
{
  public:
    virtual ~IDriver() = default;

    //-------------------------------------------------------------------------
    // Device Information
    //-------------------------------------------------------------------------

    /// @brief Get the device name for logging purposes
    /// @return Device name (e.g., "Vicon", "Leica.LMF", "NDI.Optotrak")
    virtual std::string GetDeviceName() const = 0;

    //-------------------------------------------------------------------------
    // Connection Management
    //-------------------------------------------------------------------------

    /// @brief Establish connection to the device
    /// @return true if connection successful
    virtual bool Connect() = 0;

    /// @brief Disconnect from the device
    virtual void Disconnect() = 0;

    /// @brief Check if currently connected to device
    /// @return true if connected
    virtual bool IsConnected() const = 0;

    //-------------------------------------------------------------------------
    // Detection
    //-------------------------------------------------------------------------

    /// @brief Detect available hardware
    /// @param[out] feedback Description of detected hardware or error message
    /// @return true if hardware detected successfully
    virtual bool HardwareDetect(std::string& feedback) = 0;

    /// @brief Detect configuration (markers, subjects, etc.)
    /// @param[out] feedback Description of detected configuration or error message
    /// @return true if configuration detected successfully
    virtual bool ConfigDetect(std::string& feedback) = 0;

    //-------------------------------------------------------------------------
    // Tracking Operations
    //-------------------------------------------------------------------------

    /// @brief Initialize tracking at specified frequency
    /// @param frequencyHz Desired tracking frequency in Hz
    /// @return true if initialization successful
    virtual bool Initialize(double frequencyHz) = 0;

    /// @brief Process one frame of data
    /// @return true if frame processed successfully, false if no data or error
    virtual bool Run() = 0;

    /// @brief Check if currently tracking/running
    /// @return true if tracking is active
    virtual bool IsRunning() const = 0;

    /// @brief Get the latest measurement values
    /// @param[out] values Vector to receive measurement data
    /// @return true if values available
    virtual bool GetValues(std::vector<double>& values) = 0;

    /// @brief Stop tracking and shutdown
    /// @return true if shutdown successful
    virtual bool Shutdown() = 0;

    //-------------------------------------------------------------------------
    // Status and Diagnostics
    //-------------------------------------------------------------------------

    /// @brief Get the last error message
    /// @return Error description or empty string if no error
    virtual std::string GetLastError() const = 0;

    /// @brief Get current frame number
    /// @return Frame number (0 if not tracking)
    virtual uint32_t GetFrameNumber() const = 0;

    /// @brief Get current frames per second
    /// @return FPS value (0.0 if not tracking)
    virtual double GetCurrentFPS() const = 0;

    //-------------------------------------------------------------------------
    // Optional: Device-Specific Capabilities (with defaults)
    //-------------------------------------------------------------------------

    /// @brief Check if device supports a specific capability
    /// @param capability Capability name (e.g., "6DOF", "Markers", "LaserPointer")
    /// @return true if capability supported
    virtual bool HasCapability(const std::string& capability) const
    {
        (void)capability; // Suppress unused parameter warning
        return false;     // Default: no special capabilities
    }

    /// @brief Get recommended polling interval in milliseconds
    /// @return Polling interval (default: 20ms for 50Hz)
    /// @details Used by StressTest to adapt frame processing rate per device
    virtual int GetRecommendedPollingIntervalMs() const
    {
        return 20; // Default 50Hz
    }

    /// @brief Get device-specific information for stress test logging
    /// @return Additional info string (e.g., SDK version, device model)
    virtual std::string GetDeviceInfo() const
    {
        return ""; // Default: no additional info
    }
};

} // namespace CTrack
