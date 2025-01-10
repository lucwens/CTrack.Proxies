#pragma once

#include <map>
#include <vector>

constexpr auto TAG_COMMAND_QUIT                          = "QUIT";
constexpr auto ATTRIB_RESULT                             = "result";
constexpr auto ATTRIB_RESULT_OK                          = "ok";

//------------------------------------------------------------------------------------------------------------------
/*
HANDSHAKE
*/
//------------------------------------------------------------------------------------------------------------------
constexpr auto TAG_HANDSHAKE                             = "HANDSHAKE";
constexpr auto ATTRIB_CHALLENGE                          = "challenge";

//------------------------------------------------------------------------------------------------------------------
/*
HARDWARE DETECTION
*/
//------------------------------------------------------------------------------------------------------------------
constexpr auto TAG_COMMAND_HARDWAREDETECT                = "HARDWARE_DETECT";
constexpr auto ATTRIB_HARDWAREDETECT_PRESENT             = "present";
constexpr auto ATTRIB_HARDWAREDETECT_FEEDBACK            = "feedback";
constexpr auto ATTRIB_HARDWAREDETECT_NUM_TRACKERS        = "num_trackers";
constexpr auto ATTRIB_HARDWAREDETECT_SERIALS             = "serial_numbers";
constexpr auto ATTRIB_HARDWAREDETECT_EXECUTINGAPIVERSION = "exe_version";
constexpr auto ATTRIB_HARDWAREDETECT_APIVERSION          = "api_version";
constexpr auto ATTRIB_HARDWAREDETECT_PROBING_SUPPORTED   = "probing_supported";
constexpr auto ATTRIB_HARDWAREDETECT_TRACKING_SUPPORTED  = "tracking_supported";
constexpr auto ATTRIB_HARDWAREDETECT_SERIAL              = "serial";
constexpr auto ATTRIB_HARDWAREDETECT_TYPE                = "type";
constexpr auto ATTRIB_HARDWAREDETECT_IPADDRESS           = "ipaddress";
constexpr auto ATTRIB_HARDWAREDETECT_IPADDRESSES         = "ipaddresses";

//------------------------------------------------------------------------------------------------------------------
/*
XML params
*/
//------------------------------------------------------------------------------------------------------------------
constexpr auto ATTRIB_PROXY_EXE                          = "proxy_exe";
constexpr auto ATTRIB_TCP_PORT                           = "tcp_port";
constexpr auto ATTRIB_TCP_IPSERVER                       = "tcp_ipserver";

//------------------------------------------------------------------------------------------------------------------
/*
CONFIGURATION DETECTION
*/
//------------------------------------------------------------------------------------------------------------------
constexpr auto TAG_COMMAND_CONFIGDETECT                  = "CONFIG_DETECT";
constexpr auto ATTRIB_PROBE_PRESENT                      = "probe_present";
constexpr auto ATTRIB_NUM_MARKERS                        = "num_markers";
constexpr auto ATTRIB_MARKER_NAMES                       = "marker_names";
constexpr auto ATTRIB_MODELS                             = "models";
constexpr auto ATTRIB_SERIALS                            = "serials";
constexpr auto ATTRIB_MODEL_TYPES                        = "model_types";
constexpr auto ATTRIB_MARKER_CODES                       = "marker_codes";
constexpr auto ATTRIB_SYNC_ENABLE                        = "sync_enable";
constexpr auto ATTRIB_INCLUDE_PROBE                      = "include_probe";
constexpr auto ATTRIB_INCLUDE_IJK                        = "include_IJK";
constexpr auto ATTRIB_INCLUDE_Z_ANGLE                    = "include_z_angle";
constexpr auto ATTRIB_INCLUDE_UNCODED_MARKERS            = "include_uncoded_markers";

//------------------------------------------------------------------------------------------------------------------
/*
START / STOP
*/
//------------------------------------------------------------------------------------------------------------------
constexpr auto TAG_COMMAND_CHECKINIT                     = "CHECK_INIT";
constexpr auto ATTRIB_CHECKINIT_MEASFREQ                 = "meas_freq";
constexpr auto TAG_COMMAND_SHUTDOWN                      = "SHUTDOWN";

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

constexpr auto TAG_STATE_ERROR                 = "ERROR";
constexpr auto ATTRIB_STATE                    = "state";
constexpr auto STATE_STRING_IDLE               = "IDLE";
constexpr auto STATE_STRING_RUNNING            = "RUNNING";
constexpr auto STATE_STRING_ERROR              = "ERROR";
constexpr auto ATTRIB_MESSAGE                  = "message";

//------------------------------------------------------------------------------------------------------------------
/*
BUTTON VALUES
*/
//------------------------------------------------------------------------------------------------------------------
constexpr double BUTTON_NONE                   = 0.0;
constexpr double BUTTON_TRIGGER                = 1.0;
constexpr double BUTTON_VALIDATE               = 2.0;
constexpr double BUTTON_VALIDATE_LONG          = 3.0;
constexpr double BUTTON_DECLINE                = 4.0;
constexpr double BUTTON_DECLINE_LONG           = 5.0;
constexpr double BUTTON_ARROW_UP               = 6.0;
constexpr double BUTTON_ARROW_DOWN             = 7.0;

//------------------------------------------------------------------------------------------------------------------
/*
OTHER COMMANDS
*/
//------------------------------------------------------------------------------------------------------------------
constexpr auto TAG_COMMAND_COMPENSATESTART     = "COMPENSATE_START";
constexpr auto TAG_COMMAND_PROBECALIBRATESTART = "PROBE_CALIBRATE_START";
constexpr auto ATTRIB_SIM_FILEPATH             = "filepath";

//------------------------------------------------------------------------------------------------------------------
/*
CDeviceIPAddress
*/
//------------------------------------------------------------------------------------------------------------------

constexpr auto ATTRIB_SDK_VERSION              = "sdk_version";
constexpr auto ATTRIB_ADDRESS                  = "address";
constexpr auto ATTRIB_ADDRESS_CAMERA           = "address_camera";
constexpr auto ATTRIB_NAME                     = "name";
constexpr auto ATTRIB_SERIAL                   = "serial";
constexpr auto ATTRIB_PRESENT                  = "present";
