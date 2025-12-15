# Tracy Profiling Color Codes for CTrack Proxies

This document describes the standard Tracy profiling zone color scheme used across all CTrack device proxies for consistent performance analysis.

## Prerequisites

Tracy profiling is enabled in both **Debug|x64** and **Release|x64** build configurations with:
- `TRACY_ENABLE` - Enables Tracy instrumentation
- `TRACY_ON_DEMAND` - Only captures when Tracy profiler is connected (minimal overhead when not profiling)

Use the `profiling` setting in your proxy's JSON configuration to control runtime behavior.

## Standard Zone Color Coding

### Driver Functions (IDriver Implementation)

All device drivers should use these standard colors for common operations:

| Function | Zone Name Pattern | Color | Hex Code | Description |
|----------|-------------------|-------|----------|-------------|
| `Connect()` | DeviceName::Connect | ![#4488FF](https://placehold.co/15x15/4488FF/4488FF.png) Blue | `0x4488FF` | Establishes connection to device/server |
| `Disconnect()` | DeviceName::Disconnect | ![#FF4444](https://placehold.co/15x15/FF4444/FF4444.png) Red | `0xFF4444` | Disconnects from device/server |
| `HardwareDetect()` | DeviceName::HardwareDetect | ![#44FF44](https://placehold.co/15x15/44FF44/44FF44.png) Green | `0x44FF44` | Detects connected hardware |
| `ConfigDetect()` | DeviceName::ConfigDetect | ![#FFAA00](https://placehold.co/15x15/FFAA00/FFAA00.png) Orange | `0xFFAA00` | Detects device configuration |
| `Initialize()` / `CheckInitialize()` | DeviceName::Initialize | ![#00FFFF](https://placehold.co/15x15/00FFFF/00FFFF.png) Cyan | `0x00FFFF` | Initializes tracking/measurement |
| `Run()` | DeviceName::Run | ![#FF00FF](https://placehold.co/15x15/FF00FF/FF00FF.png) Magenta | `0xFF00FF` | Processes incoming frame/measurement data |
| `GetValues()` | DeviceName::GetValues | ![#8888FF](https://placehold.co/15x15/8888FF/8888FF.png) Light Blue | `0x8888FF` | Retrieves current measurement values |
| `Shutdown()` / `ShutDown()` | DeviceName::Shutdown | ![#FF8800](https://placehold.co/15x15/FF8800/FF8800.png) Dark Orange | `0xFF8800` | Stops tracking and shuts down |

**Example usage in driver code:**
```cpp
bool YourDriver::HardwareDetect(std::string& feedback)
{
    CTRACK_ZONE_SCOPED_NC("YourDevice::HardwareDetect", 0x44FF44);  // Green
    // ... implementation ...
}
```

Replace `YourDevice` with your actual device name (e.g., "Vicon", "Leica.LMF", "NDI.Optotrak").

### StressTest Functions

Stress test zones use consistent colors across all proxies:

| Function | Zone Name | Color | Hex Code | Description |
|----------|-----------|-------|----------|-------------|
| `DoHardwareDetect()` | StressTest::HardwareDetect | ![#44FF44](https://placehold.co/15x15/44FF44/44FF44.png) Green | `0x44FF44` | Stress test hardware detection action |
| `DoConfigDetect()` | StressTest::ConfigDetect | ![#FFAA00](https://placehold.co/15x15/FFAA00/FFAA00.png) Orange | `0xFFAA00` | Stress test configuration detection action |
| `DoStartTracking()` | StressTest::StartTracking | ![#00FFFF](https://placehold.co/15x15/00FFFF/00FFFF.png) Cyan | `0x00FFFF` | Stress test start tracking action |
| `DoStopTracking()` | StressTest::StopTracking | ![#FF4444](https://placehold.co/15x15/FF4444/FF4444.png) Red | `0xFF4444` | Stress test stop tracking action |
| `DoShutdown()` | StressTest::Shutdown | ![#FF8800](https://placehold.co/15x15/FF8800/FF8800.png) Dark Orange | `0xFF8800` | Stress test final shutdown action |
| `DoWait()` | StressTest::Wait | ![#888888](https://placehold.co/15x15/888888/888888.png) Gray | `0x888888` | Stress test wait period |

## Message Color Coding

Tracy messages are sent from logging functions to provide real-time visibility into operations. Use these standard colors for consistency:

| Log Level | Color | Hex Code | Usage |
|-----------|-------|----------|-------|
| Info | ![#44FF44](https://placehold.co/15x15/44FF44/44FF44.png) Green | `0x44FF44` | General information messages |
| Warning | ![#FFFF00](https://placehold.co/15x15/FFFF00/FFFF00.png) Yellow | `0xFFFF00` | Warning conditions |
| Error | ![#FF4444](https://placehold.co/15x15/FF4444/FF4444.png) Red | `0xFF4444` | Error conditions |
| Action Success | ![#44FF44](https://placehold.co/15x15/44FF44/44FF44.png) Green | `0x44FF44` | Successful action completion |
| Action Failure | ![#FF4444](https://placehold.co/15x15/FF4444/FF4444.png) Red | `0xFF4444` | Failed action completion |

**Example usage:**
```cpp
// Info message
CTRACK_MESSAGE_C(message.c_str(), message.size(), 0x44FF44);

// Warning message
CTRACK_MESSAGE_C(warning.c_str(), warning.size(), 0xFFFF00);

// Error message
CTRACK_MESSAGE_C(error.c_str(), error.size(), 0xFF4444);
```

## Complete Color Reference

| Color Name | Sample | Hex Code | Usage |
|------------|--------|----------|-------|
| Blue | ![#4488FF](https://placehold.co/60x20/4488FF/4488FF.png) | `#4488FF` | Connection operations |
| Light Blue | ![#8888FF](https://placehold.co/60x20/8888FF/8888FF.png) | `#8888FF` | Data retrieval |
| Cyan | ![#00FFFF](https://placehold.co/60x20/00FFFF/00FFFF.png) | `#00FFFF` | Initialization/Start |
| Green | ![#44FF44](https://placehold.co/60x20/44FF44/44FF44.png) | `#44FF44` | Detection/Success/Info |
| Orange | ![#FFAA00](https://placehold.co/60x20/FFAA00/FFAA00.png) | `#FFAA00` | Configuration |
| Dark Orange | ![#FF8800](https://placehold.co/60x20/FF8800/FF8800.png) | `#FF8800` | Shutdown |
| Red | ![#FF4444](https://placehold.co/60x20/FF4444/FF4444.png) | `#FF4444` | Disconnect/Stop/Error |
| Magenta | ![#FF00FF](https://placehold.co/60x20/FF00FF/FF00FF.png) | `#FF00FF` | Runtime processing |
| Yellow | ![#FFFF00](https://placehold.co/60x20/FFFF00/FFFF00.png) | `#FFFF00` | Warnings |
| Gray | ![#888888](https://placehold.co/60x20/888888/888888.png) | `#888888` | Waiting/Idle |

## Usage Instructions

### For Any CTrack Proxy:

1. Build your proxy using **Debug|x64** or **Release|x64** configuration
2. Ensure `profiling` is set to `true` in your settings JSON file
3. Run your proxy executable
4. Connect the Tracy profiler application to capture data
5. Zones will appear in the timeline view with their assigned colors
6. Messages will appear in the messages panel with color coding

### Stress Test Profiling

When running the stress test (press 'z' to start, 'y' to stop):

1. **Phase 1**: Hardware Detection ![#44FF44](https://placehold.co/10x10/44FF44/44FF44.png)
2. **Phase 2**: Configuration Detection ![#FFAA00](https://placehold.co/10x10/FFAA00/FFAA00.png)
3. **Phase 3**: Random Actions (various colors based on action)
4. **Phase 4**: Final Shutdown ![#FF8800](https://placehold.co/10x10/FF8800/FF8800.png)

All phases emit Tracy messages for detailed logging visibility.

## Benefits of Consistent Color Coding

- **Cross-Proxy Comparison**: Compare performance across different device types
- **Quick Identification**: Instantly recognize function types by color
- **Debugging**: Spot anomalies in execution patterns
- **Documentation**: Color-coded timeline screenshots are self-documenting
- **Onboarding**: New developers quickly understand profiling data

## Implementation Checklist

When adding Tracy profiling to a new proxy:

- [ ] Use `CTRACK_ZONE_SCOPED_NC("DeviceName::FunctionName", color)` in all IDriver methods
- [ ] Follow the standard color scheme from the table above
- [ ] Replace "DeviceName" with your actual device name
- [ ] Use `CTRACK_MESSAGE_C()` for log messages with appropriate colors
- [ ] Add `CTRACK_PLOT()` for metrics like FPS
- [ ] Add `CTRACK_FRAME_MARK()` in main loop
- [ ] Test with Tracy profiler connected to verify colors appear correctly

## Reference Implementations

See these proxies for complete examples:
- **Vicon**: `Proxies/Vicon/DriverVicon.cpp` - Reference implementation
- **Template**: `Proxies/Template/Driver.cpp` - Minimal example
- **Leica.LMF**: `Proxies/Leica.LMF/LeicaDriver.cpp` - Managed C++/CLI example

For detailed implementation instructions, see:
- **Main Guide**: `Proxies/Doc/Stress and profiling.md`
- **Profiling Control**: `Proxies/Libraries/Utility/ProfilingControl.h`
