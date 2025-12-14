#pragma once

//
// ProfilingControl.h
//
// Runtime control for Tracy profiling. Even when TRACY_ENABLE is defined,
// profiling can be disabled at runtime via the profiling setting in the
// proxy configuration file.
//
// Usage:
//   1. Call CTrack::SetProfilingEnabled(true/false) at startup based on settings
//   2. Use CTRACK_ZONE_* macros instead of Tracy's Zone* macros
//   3. Use CTRACK_FRAME_MARK instead of FrameMark
//   4. Use CTRACK_PLOT instead of TracyPlot
//   5. Use CTRACK_MESSAGE_* instead of TracyMessage*
//

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

#include <atomic>

namespace CTrack
{

// Global profiling enabled flag - thread-safe
inline std::atomic<bool> g_ProfilingEnabled{false};

// Set profiling enabled/disabled at runtime
inline void SetProfilingEnabled(bool enabled)
{
    g_ProfilingEnabled.store(enabled, std::memory_order_release);
}

// Check if profiling is enabled
inline bool IsProfilingEnabled()
{
    return g_ProfilingEnabled.load(std::memory_order_acquire);
}

} // namespace CTrack

//
// Wrapper macros that check the profiling flag before using Tracy
//
// These macros mirror Tracy's zone macros but pass the profiling enabled flag
// to the ScopedZone constructor's 'active' parameter.
//

#ifdef TRACY_ENABLE

// Zone macros - use Tracy's internal structure with our profiling flag
// The 'active' parameter controls whether the zone actually captures data

#define CTRACK_ZONE_SCOPED() \
    static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,TracyLine) { nullptr, TracyFunction, TracyFile, (uint32_t)TracyLine, 0 }; \
    tracy::ScopedZone TracyConcat(___tracy_scoped_zone,TracyLine)( &TracyConcat(__tracy_source_location,TracyLine), CTrack::IsProfilingEnabled() );

#define CTRACK_ZONE_SCOPED_N(name) \
    static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,TracyLine) { name, TracyFunction, TracyFile, (uint32_t)TracyLine, 0 }; \
    tracy::ScopedZone TracyConcat(___tracy_scoped_zone,TracyLine)( &TracyConcat(__tracy_source_location,TracyLine), CTrack::IsProfilingEnabled() );

#define CTRACK_ZONE_SCOPED_C(color) \
    static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,TracyLine) { nullptr, TracyFunction, TracyFile, (uint32_t)TracyLine, color }; \
    tracy::ScopedZone TracyConcat(___tracy_scoped_zone,TracyLine)( &TracyConcat(__tracy_source_location,TracyLine), CTrack::IsProfilingEnabled() );

#define CTRACK_ZONE_SCOPED_NC(name, color) \
    static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location,TracyLine) { name, TracyFunction, TracyFile, (uint32_t)TracyLine, color }; \
    tracy::ScopedZone TracyConcat(___tracy_scoped_zone,TracyLine)( &TracyConcat(__tracy_source_location,TracyLine), CTrack::IsProfilingEnabled() );

// Frame mark - only if profiling enabled
#define CTRACK_FRAME_MARK() \
    do { if (CTrack::IsProfilingEnabled()) { FrameMark; } } while(0)

#define CTRACK_FRAME_MARK_NAMED(name) \
    do { if (CTrack::IsProfilingEnabled()) { FrameMarkNamed(name); } } while(0)

// Plot macros
#define CTRACK_PLOT(name, val) \
    do { if (CTrack::IsProfilingEnabled()) { TracyPlot(name, val); } } while(0)

// Message macros
#define CTRACK_MESSAGE(txt, size) \
    do { if (CTrack::IsProfilingEnabled()) { TracyMessage(txt, size); } } while(0)

#define CTRACK_MESSAGE_L(txt) \
    do { if (CTrack::IsProfilingEnabled()) { TracyMessageL(txt); } } while(0)

#define CTRACK_MESSAGE_C(txt, size, color) \
    do { if (CTrack::IsProfilingEnabled()) { TracyMessageC(txt, size, color); } } while(0)

#define CTRACK_MESSAGE_LC(txt, color) \
    do { if (CTrack::IsProfilingEnabled()) { TracyMessageLC(txt, color); } } while(0)

#else // TRACY_ENABLE not defined

// No-op macros when Tracy is not enabled
#define CTRACK_ZONE_SCOPED()
#define CTRACK_ZONE_SCOPED_N(name)
#define CTRACK_ZONE_SCOPED_C(color)
#define CTRACK_ZONE_SCOPED_NC(name, color)
#define CTRACK_FRAME_MARK()
#define CTRACK_FRAME_MARK_NAMED(name)
#define CTRACK_PLOT(name, val)
#define CTRACK_MESSAGE(txt, size)
#define CTRACK_MESSAGE_L(txt)
#define CTRACK_MESSAGE_C(txt, size, color)
#define CTRACK_MESSAGE_LC(txt, color)

#endif // TRACY_ENABLE
