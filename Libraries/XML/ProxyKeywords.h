#pragma once

#include <map>
#include <vector>

constexpr char const *TAG_COMMAND_QUIT                          = "QUIT";
constexpr char const *ATTRIB_RESULT                             = "result";
constexpr char const *ATTRIB_RESULT_OK                          = "ok";
constexpr char const *ATTRIB_RESULT_NOK                         = "NOT ok";

//------------------------------------------------------------------------------------------------------------------
/*
HANDSHAKE
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_HANDSHAKE                             = "HANDSHAKE";
constexpr char const *ATTRIB_CHALLENGE                          = "challenge";

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

//------------------------------------------------------------------------------------------------------------------
/*
START / STOP
*/
//------------------------------------------------------------------------------------------------------------------
constexpr char const *TAG_COMMAND_CHECKINIT                     = "CHECK_INIT";
constexpr char const *ATTRIB_CHECKINIT_MEASFREQ                 = "meas_freq";
constexpr char const *TAG_COMMAND_SHUTDOWN                      = "SHUTDOWN";

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

constexpr char const *TAG_STATE_ERROR                 = "ERROR";
constexpr char const *ATTRIB_STATE                    = "state";
constexpr char const *STATE_STRING_IDLE               = "IDLE";
constexpr char const *STATE_STRING_RUNNING            = "RUNNING";
constexpr char const *STATE_STRING_ERROR              = "ERROR";
constexpr char const *ATTRIB_MESSAGE                  = "message";

//------------------------------------------------------------------------------------------------------------------
/*
BUTTON VALUES
*/
//------------------------------------------------------------------------------------------------------------------
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
