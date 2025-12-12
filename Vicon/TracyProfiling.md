# Tracy Profiling - Vicon Proxy

This document describes the Tracy profiling zones and messages implemented in the Vicon proxy for performance analysis.

## Prerequisites

Tracy profiling is only active in the **Profiler** build configuration, which defines:
- `TRACY_ENABLE` - Enables Tracy instrumentation
- `TRACY_ON_DEMAND` - Only captures when Tracy profiler is connected

## Zone Color Coding

### DriverVicon.cpp

| Function | Zone Name | Color | Hex Code | Description |
|----------|-----------|-------|----------|-------------|
| `Connect()` | Vicon::Connect | ![#4488FF](https://placehold.co/15x15/4488FF/4488FF.png) Blue | `0x4488FF` | Establishes connection to Vicon DataStream server |
| `Disconnect()` | Vicon::Disconnect | ![#FF4444](https://placehold.co/15x15/FF4444/FF4444.png) Red | `0xFF4444` | Disconnects from Vicon DataStream server |
| `HardwareDetect()` | Vicon::HardwareDetect | ![#44FF44](https://placehold.co/15x15/44FF44/44FF44.png) Green | `0x44FF44` | Detects connected Vicon cameras |
| `ConfigDetect()` | Vicon::ConfigDetect | ![#FFAA00](https://placehold.co/15x15/FFAA00/FFAA00.png) Orange | `0xFFAA00` | Detects markers and 6DOF subjects |
| `CheckInitialize()` | Vicon::CheckInitialize | ![#00FFFF](https://placehold.co/15x15/00FFFF/00FFFF.png) Cyan | `0x00FFFF` | Initializes tracking at specified frequency |
| `Run()` | Vicon::Run | ![#FF00FF](https://placehold.co/15x15/FF00FF/FF00FF.png) Magenta | `0xFF00FF` | Processes incoming frame data |
| `GetValues()` | Vicon::GetValues | ![#8888FF](https://placehold.co/15x15/8888FF/8888FF.png) Light Blue | `0x8888FF` | Retrieves current tracking values |
| `ShutDown()` | Vicon::ShutDown | ![#FF8800](https://placehold.co/15x15/FF8800/FF8800.png) Dark Orange | `0xFF8800` | Stops tracking and disconnects |

### StressTest.cpp

| Function | Zone Name | Color | Hex Code | Description |
|----------|-----------|-------|----------|-------------|
| `DoHardwareDetect()` | StressTest::HardwareDetect | ![#44FF44](https://placehold.co/15x15/44FF44/44FF44.png) Green | `0x44FF44` | Stress test hardware detection action |
| `DoConfigDetect()` | StressTest::ConfigDetect | ![#FFAA00](https://placehold.co/15x15/FFAA00/FFAA00.png) Orange | `0xFFAA00` | Stress test configuration detection action |
| `DoStartTracking()` | StressTest::StartTracking | ![#00FFFF](https://placehold.co/15x15/00FFFF/00FFFF.png) Cyan | `0x00FFFF` | Stress test start tracking action |
| `DoStopTracking()` | StressTest::StopTracking | ![#FF4444](https://placehold.co/15x15/FF4444/FF4444.png) Red | `0xFF4444` | Stress test stop tracking action |
| `DoShutdown()` | StressTest::Shutdown | ![#FF8800](https://placehold.co/15x15/FF8800/FF8800.png) Dark Orange | `0xFF8800` | Stress test final shutdown action |
| `DoWait()` | StressTest::Wait | ![#888888](https://placehold.co/15x15/888888/888888.png) Gray | `0x888888` | Stress test wait period |

## Message Color Coding

Tracy messages are sent from the StressTest logging functions to provide real-time visibility into test progress.

| Log Level | Color | Hex Code | Usage |
|-----------|-------|----------|-------|
| Info | ![#44FF44](https://placehold.co/15x15/44FF44/44FF44.png) Green | `0x44FF44` | General information messages |
| Warning | ![#FFFF00](https://placehold.co/15x15/FFFF00/FFFF00.png) Yellow | `0xFFFF00` | Warning conditions |
| Error | ![#FF4444](https://placehold.co/15x15/FF4444/FF4444.png) Red | `0xFF4444` | Error conditions |
| Action Success | ![#44FF44](https://placehold.co/15x15/44FF44/44FF44.png) Green | `0x44FF44` | Successful action completion |
| Action Failure | ![#FF4444](https://placehold.co/15x15/FF4444/FF4444.png) Red | `0xFF4444` | Failed action completion |

## Color Reference

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

## Usage

1. Build the Vicon proxy using the **Profiler|x64** configuration
2. Run the Vicon proxy executable
3. Connect the Tracy profiler application to capture data
4. Zones will appear in the timeline view with their assigned colors
5. Messages will appear in the messages panel with color coding

## Stress Test Sequence

When running the stress test (press 'z' to start, 'y' to stop):

1. **Phase 1**: Hardware Detection ![#44FF44](https://placehold.co/10x10/44FF44/44FF44.png)
2. **Phase 2**: Configuration Detection ![#FFAA00](https://placehold.co/10x10/FFAA00/FFAA00.png)
3. **Phase 3**: Random Actions (various colors based on action)
4. **Phase 4**: Final Shutdown ![#FF8800](https://placehold.co/10x10/FF8800/FF8800.png)

All phases emit Tracy messages for detailed logging visibility.
