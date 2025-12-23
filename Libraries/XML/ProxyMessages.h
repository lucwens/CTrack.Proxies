#pragma once

//==============================================================================
// Proxy TCP/XML Message IDs
//==============================================================================
// These message IDs are used for XML-based TCP communication between
// CTrack Engine and device proxy executables.
//
// Message format: JSON with "id" field matching these constants
//   { "id": "<message_id>", "params": { ... } }
//==============================================================================

namespace ProxyMsg
{
    //--------------------------------------------------------------------------
    // Lifecycle Commands
    //--------------------------------------------------------------------------
    // Quit the proxy application
    constexpr char const *Quit = "QUIT";

    // Handshake/authentication
    // Params: { "challenge": "..." } / Response: { "response": "..." }
    constexpr char const *Handshake = "HANDSHAKE";

    //--------------------------------------------------------------------------
    // Hardware Detection
    //--------------------------------------------------------------------------
    // Detect connected hardware devices
    // Response params: { "present": bool, "num_trackers": N, "names": [...], ... }
    constexpr char const *HardwareDetect = "HARDWARE_DETECT";

    //--------------------------------------------------------------------------
    // Configuration Detection
    //--------------------------------------------------------------------------
    // Detect device configuration (markers, 6DOF objects, probes)
    // Response contains MARKER, OBJECT6DOF, PROBE child elements
    constexpr char const *ConfigDetect = "CONFIG_DETECT";

    // Configuration element types
    constexpr char const *ConfigMarker = "MARKER";
    constexpr char const *Config6DOF   = "OBJECT6DOF";
    constexpr char const *ConfigProbe  = "PROBE";

    //--------------------------------------------------------------------------
    // Measurement Control
    //--------------------------------------------------------------------------
    // Initialize and start measurement
    // Params: { "meas_freq": N, "_3d_names": [...], ... }
    constexpr char const *CheckInit = "CHECK_INIT";

    // Stop measurement and shutdown
    constexpr char const *Shutdown = "SHUTDOWN";

    //--------------------------------------------------------------------------
    // Calibration Commands
    //--------------------------------------------------------------------------
    // Start tracker compensation
    constexpr char const *CompensateStart = "COMPENSATE_START";

    // Start probe calibration
    constexpr char const *ProbeCalibrateStart = "PROBE_CALIBRATE_START";

    //--------------------------------------------------------------------------
    // Notifications
    //--------------------------------------------------------------------------
    // Error notification
    // Params: { "message": "..." }
    constexpr char const *Error = "ERROR";

    // Warning notification
    // Params: { "message": "..." }
    constexpr char const *Warning = "WARNING";

    // Event notification
    // Params: { "type": "...", "message": "..." }
    constexpr char const *Event = "EVENT";

} // namespace ProxyMsg

//==============================================================================
// Proxy Parameter Keys
//==============================================================================
// Used for consistent access to JSON/XML parameters in proxy messages
//==============================================================================

namespace ProxyParam
{
    //--------------------------------------------------------------------------
    // General Parameters
    //--------------------------------------------------------------------------
    constexpr char const *Result         = "result";
    constexpr char const *ResultFeedback = "feedback";
    constexpr char const *Message        = "message";
    constexpr char const *State          = "state";

    //--------------------------------------------------------------------------
    // Handshake Parameters
    //--------------------------------------------------------------------------
    constexpr char const *Challenge = "challenge";

    //--------------------------------------------------------------------------
    // Hardware Detection Parameters
    //--------------------------------------------------------------------------
    constexpr char const *Present            = "present";
    constexpr char const *Feedback           = "feedback";
    constexpr char const *NumTrackers        = "num_trackers";
    constexpr char const *Names              = "names";
    constexpr char const *SerialNumbers      = "serial_numbers";
    constexpr char const *ExeVersion         = "exe_version";
    constexpr char const *ApiVersion         = "api_version";
    constexpr char const *ProbingSupported   = "probing_supported";
    constexpr char const *TrackingSupported  = "tracking_supported";
    constexpr char const *Serial             = "serial";
    constexpr char const *Type               = "type";
    constexpr char const *IpAddress          = "ipaddress";
    constexpr char const *IpAddresses        = "ipaddresses";
    constexpr char const *Ports              = "ports";
    constexpr char const *Comments           = "comments";
    constexpr char const *Pos4x4             = "pos4x4";

    //--------------------------------------------------------------------------
    // Connection Parameters
    //--------------------------------------------------------------------------
    constexpr char const *ProxyExe    = "proxy_exe";
    constexpr char const *TcpPort     = "tcp_port";
    constexpr char const *TcpIpServer = "tcp_ipserver";

    //--------------------------------------------------------------------------
    // Configuration Detection Parameters
    //--------------------------------------------------------------------------
    constexpr char const *ConfigName          = "name";
    constexpr char const *OrientConvention    = "orient_convention";
    constexpr char const *Residu              = "residu";
    constexpr char const *Buttons             = "buttons";
    constexpr char const *TipDiameter         = "tip_diameter";
    constexpr char const *ProbePresent        = "probe_present";
    constexpr char const *NumMarkers          = "num_markers";
    constexpr char const *Models              = "models";
    constexpr char const *Serials             = "serials";
    constexpr char const *ModelTypes          = "model_types";
    constexpr char const *MarkerCodes         = "marker_codes";
    constexpr char const *SyncEnable          = "sync_enable";
    constexpr char const *IncludeProbe        = "include_probe";
    constexpr char const *IncludeIJK          = "include_IJK";
    constexpr char const *IncludeZAngle       = "include_z_angle";
    constexpr char const *IncludeUncodedMarkers = "include_uncoded_markers";
    constexpr char const *SixDOF              = "6dof";
    constexpr char const *Markers3D           = "markers";
    constexpr char const *Probes              = "probes";
    constexpr char const *ProbeNumButtons     = "numbuttons";
    constexpr char const *ProbeTipDiameter    = "tipdiameter";

    //--------------------------------------------------------------------------
    // Check/Init Parameters
    //--------------------------------------------------------------------------
    constexpr char const *MeasFreq       = "meas_freq";
    constexpr char const *Names3D        = "_3d_names";
    constexpr char const *Indices3D      = "_3d_indices";
    constexpr char const *ChannelNames   = "channel_names";
    constexpr char const *ChannelTypes   = "channel_types";

    //--------------------------------------------------------------------------
    // Data Stream Parameters
    //--------------------------------------------------------------------------
    constexpr char const *Data3D        = "data_3d";
    constexpr char const *Data3DParents = "data_3d_parents";
    constexpr char const *Data6DOF      = "data_6dof";
    constexpr char const *DataProbes    = "data_probes";
    constexpr char const *DataChannels  = "data_channels";

    //--------------------------------------------------------------------------
    // Event Parameters
    //--------------------------------------------------------------------------
    constexpr char const *EventType    = "type";
    constexpr char const *EventMessage = "message";

    //--------------------------------------------------------------------------
    // Simulation Parameters
    //--------------------------------------------------------------------------
    constexpr char const *SimFilePath = "filepath";

} // namespace ProxyParam

//==============================================================================
// Proxy State Constants
//==============================================================================

namespace ProxyState
{
    constexpr char const *Idle    = "IDLE";
    constexpr char const *Running = "RUNNING";
    constexpr char const *Error   = "ERROR";

} // namespace ProxyState

//==============================================================================
// Channel Type Constants
//==============================================================================

namespace ChannelType
{
    constexpr int Normal = 0;
    constexpr int Button = 1;
    constexpr int Time   = 2;

} // namespace ChannelType

//==============================================================================
// Button Value Constants
//==============================================================================

namespace ButtonValue
{
    constexpr double None         = 0.0;
    constexpr double Trigger      = 1.0;
    constexpr double Validate     = 2.0;
    constexpr double ValidateLong = 3.0;
    constexpr double Decline      = 4.0;
    constexpr double DeclineLong  = 5.0;
    constexpr double ArrowUp      = 6.0;
    constexpr double ArrowDown    = 7.0;

} // namespace ButtonValue

//==============================================================================
// Command Line Parameters
//==============================================================================

namespace ProxyCmdLine
{
    constexpr char const *Serial      = "serial";
    constexpr char const *ShowConsole = "showconsole";
    constexpr char const *TcpPort     = "tcpport";
    constexpr char const *Profiling   = "profiling";

} // namespace ProxyCmdLine
