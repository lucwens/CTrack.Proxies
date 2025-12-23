#pragma once

//==============================================================================
// ProxyKeywords.h - Backward Compatibility Header
//==============================================================================
// This header provides backward compatibility aliases for legacy code.
// New code should use the namespaced constants from:
//   - ProxyMessages.h (ProxyMsg::, ProxyParam::, ProxyState::, etc.)
//   - EngineMessages.h (EngineMsg::, EngineParam::, EventType::)
//==============================================================================

#include "ProxyMessages.h"

#include <map>
#include <vector>

//==============================================================================
// Legacy Aliases - Prefer namespaced versions for new code
//==============================================================================
// These aliases maintain backward compatibility with existing code.
// When writing new code, prefer using ProxyMsg::, ProxyParam::, etc.
//==============================================================================

//------------------------------------------------------------------------------
// Lifecycle Commands
//------------------------------------------------------------------------------
// Prefer: ProxyMsg::Quit
inline constexpr char const *TAG_COMMAND_QUIT = ProxyMsg::Quit;

// Prefer: ProxyParam::Result
inline constexpr char const *ATTRIB_RESULT = ProxyParam::Result;

// Prefer: ProxyParam::ResultFeedback
inline constexpr char const *ATTRIB_RESULT_FEEDBACK = ProxyParam::ResultFeedback;

//------------------------------------------------------------------------------
// Handshake
//------------------------------------------------------------------------------
// Prefer: ProxyMsg::Handshake
inline constexpr char const *TAG_HANDSHAKE = ProxyMsg::Handshake;

// Prefer: ProxyParam::Challenge
inline constexpr char const *ATTRIB_CHALLENGE = ProxyParam::Challenge;

//------------------------------------------------------------------------------
// Error and Warning
//------------------------------------------------------------------------------
// Prefer: ProxyMsg::Error
inline constexpr char const *TAG_ERROR = ProxyMsg::Error;

// Prefer: ProxyMsg::Warning
inline constexpr char const *TAG_WARNING = ProxyMsg::Warning;

//------------------------------------------------------------------------------
// Data Stream Attributes
//------------------------------------------------------------------------------
// Prefer: ProxyParam::Data3D
inline constexpr char const *ATTRIB_DATA_3D = ProxyParam::Data3D;

// Prefer: ProxyParam::Data3DParents
inline constexpr char const *ATTRIB_DATA_3D_PARENTS = ProxyParam::Data3DParents;

// Prefer: ProxyParam::Data6DOF
inline constexpr char const *ATTRIB_DATA_6DOF = ProxyParam::Data6DOF;

// Prefer: ProxyParam::DataProbes
inline constexpr char const *ATTRIB_DATA_PROBES = ProxyParam::DataProbes;

// Prefer: ProxyParam::DataChannels
inline constexpr char const *ATTRIB_DATA_CHANNELS = ProxyParam::DataChannels;

//------------------------------------------------------------------------------
// Hardware Detection
//------------------------------------------------------------------------------
// Prefer: ProxyMsg::HardwareDetect
inline constexpr char const *TAG_COMMAND_HARDWAREDETECT = ProxyMsg::HardwareDetect;

// Prefer: ProxyParam::Present
inline constexpr char const *ATTRIB_HARDWAREDETECT_PRESENT = ProxyParam::Present;

// Prefer: ProxyParam::Feedback
inline constexpr char const *ATTRIB_HARDWAREDETECT_FEEDBACK = ProxyParam::Feedback;

// Prefer: ProxyParam::NumTrackers
inline constexpr char const *ATTRIB_HARDWAREDETECT_NUM_TRACKERS = ProxyParam::NumTrackers;

// Prefer: ProxyParam::Names
inline constexpr char const *ATTRIB_HARDWAREDETECT_NAMES = ProxyParam::Names;

// Prefer: ProxyParam::SerialNumbers
inline constexpr char const *ATTRIB_HARDWAREDETECT_SERIALS = ProxyParam::SerialNumbers;

// Prefer: ProxyParam::ExeVersion
inline constexpr char const *ATTRIB_HARDWAREDETECT_EXECUTINGAPIVERSION = ProxyParam::ExeVersion;

// Prefer: ProxyParam::ApiVersion
inline constexpr char const *ATTRIB_HARDWAREDETECT_APIVERSION = ProxyParam::ApiVersion;

// Prefer: ProxyParam::ProbingSupported
inline constexpr char const *ATTRIB_HARDWAREDETECT_PROBING_SUPPORTED = ProxyParam::ProbingSupported;

// Prefer: ProxyParam::TrackingSupported
inline constexpr char const *ATTRIB_HARDWAREDETECT_TRACKING_SUPPORTED = ProxyParam::TrackingSupported;

// Prefer: ProxyParam::Serial
inline constexpr char const *ATTRIB_HARDWAREDETECT_SERIAL = ProxyParam::Serial;

// Prefer: ProxyParam::Type
inline constexpr char const *ATTRIB_HARDWAREDETECT_TYPE = ProxyParam::Type;

// Prefer: ProxyParam::IpAddress
inline constexpr char const *ATTRIB_HARDWAREDETECT_IPADDRESS = ProxyParam::IpAddress;

// Prefer: ProxyParam::IpAddresses
inline constexpr char const *ATTRIB_HARDWAREDETECT_IPADDRESSES = ProxyParam::IpAddresses;

// Prefer: ProxyParam::Ports
inline constexpr char const *ATTRIB_HARDWAREDETECT_IPPORTS = ProxyParam::Ports;

// Prefer: ProxyParam::Comments
inline constexpr char const *ATTRIB_HARDWAREDETECT_COMMENTS = ProxyParam::Comments;

// Prefer: ProxyParam::Pos4x4
inline constexpr char const *ATTRIB_HARDWAREDETECT_POS4x4 = ProxyParam::Pos4x4;

//------------------------------------------------------------------------------
// Connection Parameters
//------------------------------------------------------------------------------
// Prefer: ProxyParam::ProxyExe
inline constexpr char const *ATTRIB_PROXY_EXE = ProxyParam::ProxyExe;

// Prefer: ProxyParam::TcpPort
inline constexpr char const *ATTRIB_TCP_PORT = ProxyParam::TcpPort;

// Prefer: ProxyParam::TcpIpServer
inline constexpr char const *ATTRIB_TCP_IPSERVER = ProxyParam::TcpIpServer;

//------------------------------------------------------------------------------
// Configuration Detection
//------------------------------------------------------------------------------
// Prefer: ProxyMsg::ConfigDetect
inline constexpr char const *TAG_COMMAND_CONFIGDETECT = ProxyMsg::ConfigDetect;

// Prefer: ProxyMsg::ConfigMarker
inline constexpr char const *TAG_CONFIG_MARKER = ProxyMsg::ConfigMarker;

// Prefer: ProxyMsg::Config6DOF
inline constexpr char const *TAG_CONFIG_6DOF = ProxyMsg::Config6DOF;

// Prefer: ProxyMsg::ConfigProbe
inline constexpr char const *TAG_CONFIG_PROBE = ProxyMsg::ConfigProbe;

// Prefer: ProxyParam::ConfigName
inline constexpr char const *ATTRIB_CONFIG_NAME = ProxyParam::ConfigName;

// Prefer: ProxyParam::OrientConvention
inline constexpr char const *ATTRIB_CONFIG_ORIENT_CONVENTION = ProxyParam::OrientConvention;

// Prefer: ProxyParam::Residu
inline constexpr char const *ATTRIB_CONFIG_RESIDU = ProxyParam::Residu;

// Prefer: ProxyParam::Buttons
inline constexpr char const *ATTRIB_CONFIG_BUTTONS = ProxyParam::Buttons;

// Prefer: ProxyParam::TipDiameter
inline constexpr char const *ATTRIB_CONFIG_TIP_DIAMETER = ProxyParam::TipDiameter;

// Prefer: ProxyParam::ProbePresent
inline constexpr char const *ATTRIB_PROBE_PRESENT = ProxyParam::ProbePresent;

// Prefer: ProxyParam::NumMarkers
inline constexpr char const *ATTRIB_NUM_MARKERS = ProxyParam::NumMarkers;

// Prefer: ProxyParam::Models
inline constexpr char const *ATTRIB_MODELS = ProxyParam::Models;

// Prefer: ProxyParam::Serials
inline constexpr char const *ATTRIB_SERIALS = ProxyParam::Serials;

// Prefer: ProxyParam::ModelTypes
inline constexpr char const *ATTRIB_MODEL_TYPES = ProxyParam::ModelTypes;

// Prefer: ProxyParam::MarkerCodes
inline constexpr char const *ATTRIB_MARKER_CODES = ProxyParam::MarkerCodes;

// Prefer: ProxyParam::SyncEnable
inline constexpr char const *ATTRIB_SYNC_ENABLE = ProxyParam::SyncEnable;

// Prefer: ProxyParam::IncludeProbe
inline constexpr char const *ATTRIB_INCLUDE_PROBE = ProxyParam::IncludeProbe;

// Prefer: ProxyParam::IncludeIJK
inline constexpr char const *ATTRIB_INCLUDE_IJK = ProxyParam::IncludeIJK;

// Prefer: ProxyParam::IncludeZAngle
inline constexpr char const *ATTRIB_INCLUDE_Z_ANGLE = ProxyParam::IncludeZAngle;

// Prefer: ProxyParam::IncludeUncodedMarkers
inline constexpr char const *ATTRIB_INCLUDE_UNCODED_MARKERS = ProxyParam::IncludeUncodedMarkers;

// Prefer: ProxyParam::SixDOF
inline constexpr char const *ATTRIB_6DOF = ProxyParam::SixDOF;

// Prefer: ProxyParam::Markers3D
inline constexpr char const *ATTRIB_CONFIG_3DMARKERS = ProxyParam::Markers3D;

// Prefer: ProxyParam::Probes
inline constexpr char const *ATTRIB_PROBES = ProxyParam::Probes;

// Prefer: ProxyParam::ProbeNumButtons
inline constexpr char const *ATTRIB_PROBE_NUMBUTTONS = ProxyParam::ProbeNumButtons;

// Prefer: ProxyParam::ProbeTipDiameter
inline constexpr char const *ATTRIB_PROBE_TIPDIAMETER = ProxyParam::ProbeTipDiameter;

//------------------------------------------------------------------------------
// Start / Stop Commands
//------------------------------------------------------------------------------
// Prefer: ProxyMsg::CheckInit
inline constexpr char const *TAG_COMMAND_CHECKINIT = ProxyMsg::CheckInit;

// Prefer: ProxyParam::MeasFreq
inline constexpr char const *ATTRIB_CHECKINIT_MEASFREQ = ProxyParam::MeasFreq;

// Prefer: ProxyMsg::Shutdown
inline constexpr char const *TAG_COMMAND_SHUTDOWN = ProxyMsg::Shutdown;

// Prefer: ProxyParam::Names3D
inline constexpr char const *ATTRIB_CHECKINIT_3DNAMES = ProxyParam::Names3D;

// Prefer: ProxyParam::Indices3D
inline constexpr char const *ATTRIB_CHECKINIT_3DINDICES = ProxyParam::Indices3D;

// Prefer: ProxyParam::ChannelNames
inline constexpr char const *ATTRIB_CHECKINIT_CHANNELNAMES = ProxyParam::ChannelNames;

// Prefer: ProxyParam::ChannelTypes
inline constexpr char const *ATTRIB_CHECKINIT_CHANNELTYPES = ProxyParam::ChannelTypes;

// Prefer: ChannelType::Normal
inline constexpr int ChannelTypeNormal = ChannelType::Normal;

// Prefer: ChannelType::Button
inline constexpr int ChannelTypeButton = ChannelType::Button;

// Prefer: ChannelType::Time
inline constexpr int ChannelTypeTime = ChannelType::Time;

//------------------------------------------------------------------------------
// States
//------------------------------------------------------------------------------
// ERunState enum kept for backward compatibility
enum ERunState
{
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR
};

// Prefer: ProxyParam::State
inline constexpr char const *ATTRIB_STATE = ProxyParam::State;

// Prefer: ProxyParam::Message
inline constexpr char const *ATTRIB_MESSAGE = ProxyParam::Message;

// Prefer: ProxyState::Idle
inline constexpr char const *STATE_STRING_IDLE = ProxyState::Idle;

// Prefer: ProxyState::Running
inline constexpr char const *STATE_STRING_RUNNING = ProxyState::Running;

// Prefer: ProxyState::Error
inline constexpr char const *STATE_STRING_ERROR = ProxyState::Error;

//------------------------------------------------------------------------------
// Events
//------------------------------------------------------------------------------
// Prefer: ProxyMsg::Event
inline constexpr char const *TAG_EVENT = ProxyMsg::Event;

// Prefer: ProxyParam::EventType
inline constexpr char const *ATTRIB_EVENT_TYPE = ProxyParam::EventType;

// Prefer: ProxyParam::EventMessage
inline constexpr char const *ATTRIB_EVENT_MESSAGE = ProxyParam::EventMessage;

//------------------------------------------------------------------------------
// Button Values
//------------------------------------------------------------------------------
// T_ProbeButton union kept for backward compatibility
union T_ProbeButton
{
    T_ProbeButton() { Reset(); };
    void Reset() { memset(&Int64Value, 0, sizeof(double)); };
    void Set(int ButtonIndex, int Value)
    {
        unsigned long long Value64 = Value;
        Int64Value &= ~(3ull << (ButtonIndex * 2)); // first clear
        Int64Value |= (Value64 << (ButtonIndex * 2));
    };
    int Get(int ButtonIndex) { return ((Int64Value >> (ButtonIndex * 2)) & 3); };

    unsigned long long Int64Value;
    double             DoubleVal;
};

// Prefer: ButtonValue::None
inline constexpr double BUTTON_NONE = ButtonValue::None;

// Prefer: ButtonValue::Trigger
inline constexpr double BUTTON_TRIGGER = ButtonValue::Trigger;

// Prefer: ButtonValue::Validate
inline constexpr double BUTTON_VALIDATE = ButtonValue::Validate;

// Prefer: ButtonValue::ValidateLong
inline constexpr double BUTTON_VALIDATE_LONG = ButtonValue::ValidateLong;

// Prefer: ButtonValue::Decline
inline constexpr double BUTTON_DECLINE = ButtonValue::Decline;

// Prefer: ButtonValue::DeclineLong
inline constexpr double BUTTON_DECLINE_LONG = ButtonValue::DeclineLong;

// Prefer: ButtonValue::ArrowUp
inline constexpr double BUTTON_ARROW_UP = ButtonValue::ArrowUp;

// Prefer: ButtonValue::ArrowDown
inline constexpr double BUTTON_ARROW_DOWN = ButtonValue::ArrowDown;

//------------------------------------------------------------------------------
// Other Commands
//------------------------------------------------------------------------------
// Prefer: ProxyMsg::CompensateStart
inline constexpr char const *TAG_COMMAND_COMPENSATESTART = ProxyMsg::CompensateStart;

// Prefer: ProxyMsg::ProbeCalibrateStart
inline constexpr char const *TAG_COMMAND_PROBECALIBRATESTART = ProxyMsg::ProbeCalibrateStart;

// Prefer: ProxyParam::SimFilePath
inline constexpr char const *ATTRIB_SIM_FILEPATH = ProxyParam::SimFilePath;

//------------------------------------------------------------------------------
// Command Line Parameters
//------------------------------------------------------------------------------
// Prefer: ProxyCmdLine::Serial
inline constexpr char const *SERIAL = ProxyCmdLine::Serial;

// Prefer: ProxyCmdLine::ShowConsole
inline constexpr char const *SHOWCONSOLE = ProxyCmdLine::ShowConsole;

// Prefer: ProxyCmdLine::TcpPort
inline constexpr char const *TCPPORT = ProxyCmdLine::TcpPort;

// Prefer: ProxyCmdLine::Profiling
inline constexpr char const *PROFILING = ProxyCmdLine::Profiling;

//==============================================================================
// Engine Message Aliases (for Proxy code that needs engine messages)
//==============================================================================
// Legacy constants for engine messages. For namespaced versions, include
// EngineMessages.h directly (from Include/ directory).
//==============================================================================

// Legacy MSG_ENGINE_* constants
inline constexpr char const *MSG_ENGINE_COMMAND   = "engine.command";
inline constexpr char const *MSG_ENGINE_CONFIG    = "engine.config";
inline constexpr char const *MSG_ENGINE_STATE     = "engine.state";
inline constexpr char const *MSG_ENGINE_LOG       = "engine.log";
inline constexpr char const *MSG_ENGINE_EVENT     = "engine.event";
inline constexpr char const *MSG_ENGINE_INTERRUPT = "engine.interrupt";
inline constexpr char const *MSG_ENGINE_ERROR     = "engine.error";
inline constexpr char const *MSG_ENGINE_LICENSE   = "engine.license";

// Legacy PARAM_* constants
inline constexpr char const *PARAM_NODE_TYPE   = "nodeType";
inline constexpr char const *PARAM_XML         = "xml";
inline constexpr char const *PARAM_STATE_NAME  = "stateName";
inline constexpr char const *PARAM_STATE_DATA  = "stateData";
inline constexpr char const *PARAM_LOG_LEVEL   = "level";
inline constexpr char const *PARAM_LOG_MESSAGE = "message";
inline constexpr char const *PARAM_ERROR_FILE  = "file";
inline constexpr char const *PARAM_ERROR_LINE  = "line";
inline constexpr char const *PARAM_EVENT_TYPE  = "eventType";
inline constexpr char const *PARAM_EVENT_DATA  = "data";
inline constexpr char const *PARAM_REASON      = "reason";
