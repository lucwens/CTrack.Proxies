#pragma once

#include <map>
#include <vector>

constexpr char const *TAG_COMMAND_QUIT                          = "QUIT";
constexpr char const *ATTRIB_RESULT                             = "result";
constexpr char const *ATTRIB_RESULT_FEEDBACK                    = "feedback";

//------------------------------------------------------------------------------------------------------------------
/*
HANDSHAKE
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_HANDSHAKE                             = "HANDSHAKE";
constexpr char const *ATTRIB_CHALLENGE                          = "challenge";

//------------------------------------------------------------------------------------------------------------------
/*
ERROR and WARNING
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_ERROR                                 = "ERROR";
constexpr char const *TAG_WARNING                               = "WARNING";

//------------------------------------------------------------------------------------------------------------------
/*
LIST OF DATA
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *ATTRIB_DATA_3D                            = "data_3d";
constexpr char const *ATTRIB_DATA_3D_PARENTS                    = "data_3d_parents";
constexpr char const *ATTRIB_DATA_6DOF                          = "data_6dof";
constexpr char const *ATTRIB_DATA_PROBES                        = "data_probes";
constexpr char const *ATTRIB_DATA_CHANNELS                      = "data_channels";
//
//------------------------------------------------------------------------------------------------------------------
/*
HARDWARE DETECTION
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_COMMAND_HARDWAREDETECT                = "HARDWARE_DETECT";
constexpr char const *ATTRIB_HARDWAREDETECT_PRESENT             = "present";
constexpr char const *ATTRIB_HARDWAREDETECT_FEEDBACK            = "feedback";
constexpr char const *ATTRIB_HARDWAREDETECT_NUM_TRACKERS        = "num_trackers";
constexpr char const *ATTRIB_HARDWAREDETECT_NAMES               = "names";
constexpr char const *ATTRIB_HARDWAREDETECT_SERIALS             = "serial_numbers";
constexpr char const *ATTRIB_HARDWAREDETECT_EXECUTINGAPIVERSION = "exe_version";
constexpr char const *ATTRIB_HARDWAREDETECT_APIVERSION          = "api_version";
constexpr char const *ATTRIB_HARDWAREDETECT_PROBING_SUPPORTED   = "probing_supported";
constexpr char const *ATTRIB_HARDWAREDETECT_TRACKING_SUPPORTED  = "tracking_supported";
constexpr char const *ATTRIB_HARDWAREDETECT_SERIAL              = "serial";
constexpr char const *ATTRIB_HARDWAREDETECT_TYPE                = "type";
constexpr char const *ATTRIB_HARDWAREDETECT_IPADDRESS           = "ipaddress";
constexpr char const *ATTRIB_HARDWAREDETECT_IPADDRESSES         = "ipaddresses";
constexpr char const *ATTRIB_HARDWAREDETECT_IPPORTS             = "ports";
constexpr char const *ATTRIB_HARDWAREDETECT_COMMENTS            = "comments";
constexpr char const *ATTRIB_HARDWAREDETECT_POS4x4              = "pos4x4";

//------------------------------------------------------------------------------------------------------------------
/*
XML params
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *ATTRIB_PROXY_EXE                          = "proxy_exe";
constexpr char const *ATTRIB_TCP_PORT                           = "tcp_port";
constexpr char const *ATTRIB_TCP_IPSERVER                       = "tcp_ipserver";

//------------------------------------------------------------------------------------------------------------------
/*
CONFIGURATION DETECTION
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_COMMAND_CONFIGDETECT                  = "CONFIG_DETECT";
constexpr char const *TAG_CONFIG_MARKER                         = "MARKER";
constexpr char const *TAG_CONFIG_6DOF                           = "OBJECT6DOF";
constexpr char const *TAG_CONFIG_PROBE                          = "PROBE";

constexpr char const *ATTRIB_CONFIG_NAME                        = "name";
constexpr char const *ATTRIB_CONFIG_ORIENT_CONVENTION           = "orient_convention";
constexpr char const *ATTRIB_CONFIG_RESIDU                      = "residu";
constexpr char const *ATTRIB_CONFIG_BUTTONS                     = "buttons";
constexpr char const *ATTRIB_CONFIG_TIP_DIAMETER                = "tip_diameter";
constexpr char const *ATTRIB_PROBE_PRESENT                      = "probe_present";
constexpr char const *ATTRIB_NUM_MARKERS                        = "num_markers";
constexpr char const *ATTRIB_MODELS                             = "models";
constexpr char const *ATTRIB_SERIALS                            = "serials";
constexpr char const *ATTRIB_MODEL_TYPES                        = "model_types";
constexpr char const *ATTRIB_MARKER_CODES                       = "marker_codes";
constexpr char const *ATTRIB_SYNC_ENABLE                        = "sync_enable";
constexpr char const *ATTRIB_INCLUDE_PROBE                      = "include_probe";
constexpr char const *ATTRIB_INCLUDE_IJK                        = "include_IJK";
constexpr char const *ATTRIB_INCLUDE_Z_ANGLE                    = "include_z_angle";
constexpr char const *ATTRIB_INCLUDE_UNCODED_MARKERS            = "include_uncoded_markers";
constexpr char const *ATTRIB_6DOF                               = "6dof";
constexpr char const *ATTRIB_CONFIG_3DMARKERS                   = "markers";
constexpr char const *ATTRIB_PROBES                             = "probes";
constexpr char const *ATTRIB_PROBE_NUMBUTTONS                   = "numbuttons";
constexpr char const *ATTRIB_PROBE_TIPDIAMETER                  = "tipdiameter";

//------------------------------------------------------------------------------------------------------------------
/*
START / STOP
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_COMMAND_CHECKINIT                     = "CHECK_INIT";
constexpr char const *ATTRIB_CHECKINIT_MEASFREQ                 = "meas_freq";
constexpr char const *TAG_COMMAND_SHUTDOWN                      = "SHUTDOWN";
constexpr char const *ATTRIB_CHECKINIT_3DNAMES                  = "_3d_names";
constexpr char const *ATTRIB_CHECKINIT_3DINDICES                = "_3d_indices";
constexpr char const *ATTRIB_CHECKINIT_CHANNELNAMES             = "channel_names";
constexpr char const *ATTRIB_CHECKINIT_CHANNELTYPES             = "channel_types";

constexpr int ChannelTypeNormal                                 = 0;
constexpr int ChannelTypeButton                                 = 1;
constexpr int ChannelTypeTime                                   = 2;

//------------------------------------------------------------------------------------------------------------------
/*
STATES
*/
//------------------------------------------------------------------------------------------------------------------

enum ERunState
{
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR
};

// constexpr char const *TAG_STATE_ERROR      = "ERROR";
constexpr char const *ATTRIB_STATE         = "state";
constexpr char const *ATTRIB_MESSAGE       = "message";
constexpr char const *STATE_STRING_IDLE    = "IDLE";
constexpr char const *STATE_STRING_RUNNING = "RUNNING";
constexpr char const *STATE_STRING_ERROR   = "ERROR";

//------------------------------------------------------------------------------------------------------------------
/*
EVENTS
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_EVENT            = "EVENT";
constexpr char const *ATTRIB_EVENT_TYPE    = "type";
constexpr char const *ATTRIB_EVENT_MESSAGE = "message";

//------------------------------------------------------------------------------------------------------------------
/*
BUTTON VALUES
*/
//------------------------------------------------------------------------------------------------------------------

/*
we have 8 bytes, so 64 bits in total, we need 2 bits to store the above states, so we can have 64/2=32 buttons
*/

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

constexpr double BUTTON_NONE                          = 0.0;
constexpr double BUTTON_TRIGGER                       = 1.0;
constexpr double BUTTON_VALIDATE                      = 2.0;
constexpr double BUTTON_VALIDATE_LONG                 = 3.0;
constexpr double BUTTON_DECLINE                       = 4.0;
constexpr double BUTTON_DECLINE_LONG                  = 5.0;
constexpr double BUTTON_ARROW_UP                      = 6.0;
constexpr double BUTTON_ARROW_DOWN                    = 7.0;

//------------------------------------------------------------------------------------------------------------------
/*
OTHER COMMANDS
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_COMMAND_COMPENSATESTART     = "COMPENSATE_START";
constexpr char const *TAG_COMMAND_PROBECALIBRATESTART = "PROBE_CALIBRATE_START";
constexpr char const *ATTRIB_SIM_FILEPATH             = "filepath";

//==============================================================================
// Engine TCP Message IDs (JSON messages via TCPGRAM_CODE_MESSAGE)
//==============================================================================
// These message IDs are used with the JSON-based TCPGRAM_CODE_MESSAGE protocol.
// For CNode-derived objects, serialize to XML and embed in JSON params as:
//   { "id": "engine.command", "params": { "nodeType": "...", "xml": "..." } }
//==============================================================================

// Command/Node messages (replaces TCPGRAM_CODE_COMMAND)
constexpr char const *MSG_ENGINE_COMMAND     = "engine.command";     // CNode command with embedded XML
constexpr char const *MSG_ENGINE_CONFIG      = "engine.config";      // Configuration deployment

// State messages (replaces TCPGRAM_CODE_STATUS)
constexpr char const *MSG_ENGINE_STATE       = "engine.state";       // State change notification

// Log/feedback messages (replaces TCPGRAM_CODE_STRING, TCPGRAM_CODE_EVENT)
constexpr char const *MSG_ENGINE_LOG         = "engine.log";         // Log messages with level
constexpr char const *MSG_ENGINE_EVENT       = "engine.event";       // Event notifications

// Control messages (replaces TCPGRAM_CODE_INTERRUPT)
constexpr char const *MSG_ENGINE_INTERRUPT   = "engine.interrupt";   // Interrupt current operation

// Error messages (replaces TCPGRAM_CODE_ERROR)
constexpr char const *MSG_ENGINE_ERROR       = "engine.error";       // Error with file/line/message

// License messages (new JSON format - replaces CLicense node)
constexpr char const *MSG_ENGINE_LICENSE     = "engine.license";     // License status and data

// JSON param keys for engine messages
constexpr char const *PARAM_NODE_TYPE        = "nodeType";           // CNode class name
constexpr char const *PARAM_XML              = "xml";                // Serialized XML content
constexpr char const *PARAM_STATE_NAME       = "stateName";          // State class name
constexpr char const *PARAM_STATE_DATA       = "stateData";          // Optional state-specific data
constexpr char const *PARAM_LOG_LEVEL        = "level";              // debug, info, warning, error
constexpr char const *PARAM_LOG_MESSAGE      = "message";            // Log/error message text
constexpr char const *PARAM_ERROR_FILE       = "file";               // Source file for error
constexpr char const *PARAM_ERROR_LINE       = "line";               // Line number for error
constexpr char const *PARAM_EVENT_TYPE       = "eventType";          // Event type identifier
constexpr char const *PARAM_EVENT_DATA       = "data";               // Event-specific data
constexpr char const *PARAM_REASON           = "reason";             // Interrupt reason

//------------------------------------------------------------------------------------------------------------------
/*
COMMANDLINE PARAMETERS
*/
//------------------------------------------------------------------------------------------------------------------

constexpr const char *SERIAL                          = "serial";
constexpr const char *SHOWCONSOLE                     = "showconsole";
constexpr const char *TCPPORT                         = "tcpport";
constexpr const char *PROFILING                       = "profiling";