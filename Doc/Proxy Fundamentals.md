# Proxy Communication Fundamentals

This document describes the fundamental operation of the CTrack proxy system, covering connection establishment, authentication, telegram exchange, and data streaming.

## Table of Contents

1. [Overview](#overview)
2. [Connection Establishment](#connection-establishment)
3. [Authentication (Handshake)](#authentication-handshake)
4. [Telegram Format](#telegram-format)
5. [Message Exchange Protocol](#message-exchange-protocol)
6. [Response Handling](#response-handling)
7. [Data Streaming](#data-streaming)
8. [Connection Lifecycle](#connection-lifecycle)
9. [Error Handling](#error-handling)

---

## Overview

The CTrack proxy system enables communication between the main application and external measurement devices through standalone proxy executables. Each proxy implements a standardized TCP-based protocol that handles:

- Bidirectional command/response messaging
- Real-time measurement data streaming
- Cryptographic authentication
- Connection management and recovery

**Key Components:**
- `Proxies/Libraries/TCP/` - Core TCP communication layer
- `Proxies/Libraries/XML/ProxyKeywords.h` - Protocol message identifiers
- `CTrack_Data/ProxyDevice.h` - Main application proxy interface
- `CTrack_Data/ProxyHandshake.h` - Authentication implementation

---

## Connection Establishment

### Architecture

```
┌─────────────┐         TCP/IP          ┌─────────────┐
│   CTrack    │◄───────────────────────►│    Proxy    │
│ (Server)    │      Port 40000+        │  (Client)   │
└─────────────┘                         └─────────────┘
```

CTrack operates as the TCP server, listening for incoming connections from proxy executables.

### Server Mode (CTrack)

1. **Socket Creation**: IPv4 TCP socket (`AF_INET`, `SOCK_STREAM`, `IPPROTO_TCP`)
2. **Binding**: Binds to `INADDR_ANY` on the configured port (default: 40000)
3. **Listening**: Sets listen backlog to 1 connection
4. **Accept Loop**: Non-blocking `accept()` waits for client connections
5. **Callback**: `OnConnectFunction` triggered when connection established

**Port Range**: 40000 - 49999 (`TCP_PORT_START` to `TCP_PORT_END`)

### Client Mode (Proxy)

1. **Startup**: Proxy launched with command-line parameters:
   ```
   proxy.exe --tcpport=40001 --serial=ABC123
   ```
2. **Host Resolution**: `ResolveIP4_Address()` converts hostname to IP
3. **Connection**: `connect()` with retry on `WSAEWOULDBLOCK`
4. **Callback**: `OnConnectFunction` triggered on successful connection

### Connection Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `tcpport` | 40000 | TCP port number |
| `serial` | - | Device serial number |
| `showconsole` | false | Show console window |
| `profiling` | false | Enable Tracy profiling |

---

## Authentication (Handshake)

The proxy system implements RSA-2048 challenge-response authentication to verify proxy authenticity.

### Handshake Sequence

```
┌─────────────┐                              ┌─────────────┐
│   CTrack    │                              │    Proxy    │
└──────┬──────┘                              └──────┬──────┘
       │                                            │
       │  1. HANDSHAKE {challenge: base64_random}   │
       │───────────────────────────────────────────►│
       │                                            │
       │                                   2. Decode challenge
       │                                   3. Sign with RSA private key
       │                                            │
       │  4. HANDSHAKE {challenge: base64_signature}│
       │◄───────────────────────────────────────────│
       │                                            │
  5. Verify signature                               │
  6. Connection authenticated                       │
       │                                            │
```

### Cryptographic Details

- **Algorithm**: RSA-2048 with SHA-256
- **Signature Scheme**: EMSA-PKCS1-v1_5(SHA-256)
- **Library**: Botan
- **Keys**: Embedded in `ProxyHandshake.h`

### Message Format

**Challenge Request (CTrack → Proxy):**
```json
{
  "id": "HANDSHAKE",
  "params": {
    "challenge": "base64_encoded_random_bytes"
  }
}
```

**Challenge Response (Proxy → CTrack):**
```json
{
  "id": "HANDSHAKE",
  "params": {
    "challenge": "base64_encoded_signature"
  }
}
```

### Implementation Reference

```cpp
// ProxyHandshake.h - Proxy side
CTrack::Reply ProxyHandShake(const CTrack::Message &message)
{
    // 1. Extract and decode challenge
    std::string challengeStr = message.GetParams()[ATTRIB_CHALLENGE];
    std::vector<uint8_t> challenge = DecodeBase64(challengeStr);

    // 2. Sign the challenge
    std::vector<uint8_t> signature = SignData(challenge, privateKey);

    // 3. Return signed response
    return std::make_unique<CTrack::Message>(
        TAG_HANDSHAKE,
        {{ATTRIB_CHALLENGE, EncodeBase64(signature)}}
    );
}
```

---

## Telegram Format

All communication uses a binary telegram structure with a 5-byte header.

### Header Structure

```
┌────────────────────────────────────────────────────────┐
│  TMessageHeader (5 bytes, packed)                      │
├────────────────────────┬───────────────────────────────┤
│  m_Size (uint32_t)     │  m_Code (uint8_t)             │
│  4 bytes               │  1 byte                       │
│  Total telegram size   │  Message type code            │
└────────────────────────┴───────────────────────────────┘
```

### Telegram Codes

| Code | Constant | Description |
|------|----------|-------------|
| 0 | `TCPGRAM_CODE_DOUBLES` | Array of doubles (measurement data) |
| 1 | `TCPGRAM_CODE_COMMAND` | XML command |
| 2 | `TCPGRAM_CODE_STATUS` | XML status |
| 3 | `TCPGRAM_CODE_CONFIGURATION` | XML configuration |
| 4 | `TCPGRAM_CODE_STRING` | Text string |
| 5 | `TCPGRAM_CODE_EVENT` | Event notification |
| 6 | `TCPGRAM_CODE_INTERRUPT` | Interrupt signal |
| 7 | `TCPGRAM_CODE_ERROR` | Error message |
| 8 | `TCPGRAM_CODE_MESSAGE` | JSON-based message |

### JSON Message Format (Code 8)

Modern protocol uses JSON-serialized messages:

```json
{
  "id": "MESSAGE_IDENTIFIER",
  "params": {
    "key1": "value1",
    "key2": 123,
    "key3": true
  }
}
```

**Serialization**: Uses `nlohmann::json` library

---

## Message Exchange Protocol

### Standard Commands

| Message ID | Direction | Description |
|------------|-----------|-------------|
| `HANDSHAKE` | Bidirectional | Authentication challenge-response |
| `HARDWARE_DETECT` | CTrack → Proxy | Discover connected hardware |
| `CONFIG_DETECT` | CTrack → Proxy | Detect markers, 6DOF objects, probes |
| `CHECK_INIT` | CTrack → Proxy | Start tracking at specified frequency |
| `SHUTDOWN` | CTrack → Proxy | Stop tracking |
| `QUIT` | CTrack → Proxy | Terminate proxy process |

### Hardware Detection Example

**Request (CTrack → Proxy):**
```json
{
  "id": "HARDWARE_DETECT",
  "params": {}
}
```

**Response (Proxy → CTrack):**
```json
{
  "id": "HARDWARE_DETECT",
  "params": {
    "present": true,
    "num_trackers": 2,
    "names": ["Tracker1", "Tracker2"],
    "serial_numbers": ["SN001", "SN002"],
    "feedback": "Hardware detected successfully"
  }
}
```

### Configuration Detection Example

**Request (CTrack → Proxy):**
```json
{
  "id": "CONFIG_DETECT",
  "params": {}
}
```

**Response (Proxy → CTrack):**
```json
{
  "id": "CONFIG_DETECT",
  "params": {
    "MARKER": [
      {"name": "Marker1"},
      {"name": "Marker2"}
    ],
    "OBJECT6DOF": [
      {"name": "Probe1"}
    ]
  }
}
```

### Start Tracking Example

**Request (CTrack → Proxy):**
```json
{
  "id": "CHECK_INIT",
  "params": {
    "meas_freq": 100.0
  }
}
```

---

## Response Handling

### Message Responder Architecture

The system uses a publish-subscribe pattern for message routing.

```
┌──────────────────────────────────────────────────────────────────┐
│                      MessageResponder                            │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────┐    ┌─────────────────┐                     │
│  │  Subscription   │    │  Subscription   │    ...              │
│  │  ID: HANDSHAKE  │    │  ID: HW_DETECT  │                     │
│  │  Handler: func1 │    │  Handler: func2 │                     │
│  └────────┬────────┘    └────────┬────────┘                     │
│           │                      │                               │
│           ▼                      ▼                               │
│     RespondToMessage(msg) → matches ID → executes handler        │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

### Subscription Pattern

```cpp
// Handler signature
using Handler = std::function<CTrack::Reply(const CTrack::Message &)>;
using Reply = std::unique_ptr<CTrack::Message>;

// Register handler for message ID
driver->Subscribe(
    *TCPServer.GetMessageResponder(),
    TAG_COMMAND_HARDWAREDETECT,
    CTrack::MakeMemberHandler(driver.get(), &Driver::HardwareDetect)
);
```

### Request-Response Pattern

**Synchronous (blocking):**
```cpp
CTrack::Message request(TAG_COMMAND_HARDWAREDETECT, {});
CTrack::Message response;
bool success = proxyDevice->SendTrackRequest(request, response, TIMEOUTSECS);
```

**Asynchronous:**
```cpp
CTrack::Message request(TAG_COMMAND_HARDWAREDETECT, {});
std::future<CTrack::Message> future = proxyDevice->SendTrackRequestAsync(request);
// ... do other work ...
CTrack::Message response = future.get();
```

### Handler Implementation

```cpp
CTrack::Reply Driver::HardwareDetect(const CTrack::Message &message)
{
    std::string feedback;
    bool present = DetectHardware(feedback);

    return std::make_unique<CTrack::Message>(
        TAG_COMMAND_HARDWAREDETECT,
        {
            {ATTRIB_HARDWAREDETECT_PRESENT, present},
            {ATTRIB_HARDWAREDETECT_FEEDBACK, feedback},
            {ATTRIB_HARDWAREDETECT_NUM_TRACKERS, m_numTrackers},
            {ATTRIB_HARDWAREDETECT_NAMES, m_trackerNames}
        }
    );
}
```

---

## Data Streaming

### Measurement Data Format

Real-time measurement data uses binary double arrays (Code 0):

```
┌─────────────────────────────────────────────────────────────────┐
│                    Double Array Telegram                        │
├─────────────────────┬───────────────────────────────────────────┤
│  NumChannels        │  Channel Values                           │
│  uint16_t (2 bytes) │  double[] (NumChannels × 8 bytes)         │
└─────────────────────┴───────────────────────────────────────────┘
```

### Streaming Flow

```
┌─────────────────┐                              ┌─────────────────┐
│   CTrack        │                              │     Proxy       │
└────────┬────────┘                              └────────┬────────┘
         │                                                │
         │  CHECK_INIT {meas_freq: 100}                   │
         │───────────────────────────────────────────────►│
         │                                                │
         │                                       Start measurement loop
         │                                                │
         │◄─────────── DOUBLES [x1, y1, z1, ...] ─────────│
         │◄─────────── DOUBLES [x2, y2, z2, ...] ─────────│
         │◄─────────── DOUBLES [x3, y3, z3, ...] ─────────│
         │                    ...                         │
         │                                                │
         │  SHUTDOWN                                      │
         │───────────────────────────────────────────────►│
         │                                       Stop measurement loop
         │                                                │
```

### Proxy Implementation

```cpp
// Main loop in proxy
while (bContinueLoop)
{
    // Process incoming commands
    std::unique_ptr<CTCPGram> TCPGram;
    if (TCPServer.GetReceivePackage(TCPGram))
    {
        TCPServer.GetMessageResponder()->RespondToMessage(*TCPGram->GetMessage());
    }

    // Generate and send measurement data
    if (driver->Run())
    {
        std::unique_ptr<CTCPGram> gram =
            std::make_unique<CTCPGram>(driver->m_arDoubles);
        TCPServer.PushSendPackage(gram);
    }
}
```

### Buffer Architecture

**Send Pipeline:**
1. Application creates `CTCPGram` with measurement data
2. `PushSendPackage()` adds to send buffer
3. Communication thread retrieves via `GetSendPackage()`
4. `WriteSendTelegram()` transmits over socket

**Receive Pipeline:**
1. `ReadExtractTelegram()` reads from socket
2. Header parsed (5 bytes) to determine payload size
3. Payload assembled into complete telegram
4. `MessageResponder::RespondToMessage()` routes to handlers

---

## Connection Lifecycle

### State Diagram

```
                    ┌──────────────┐
                    │   Startup    │
                    └──────┬───────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  Create Listen Socket  │
              │  Bind to Port          │
              │  listen()              │
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  Waiting for           │◄────────────┐
              │  Connection            │             │
              └────────────┬───────────┘             │
                           │ accept()                │
                           ▼                         │
              ┌────────────────────────┐             │
              │  Connected             │             │
              │  - Handshake           │             │
              │  - Command/Response    │             │
              │  - Data Streaming      │             │
              └────────────┬───────────┘             │
                           │ disconnect              │
                           ▼                         │
              ┌────────────────────────┐             │
              │  Disconnected          │─────────────┘
              │  (Auto-reconnect)      │
              └────────────────────────┘
```

### Disconnection Detection

- `recv()` returns 0: Clean disconnect
- Socket error: Connection lost
- `OnDisconnectFunction` callback triggered
- Client mode automatically attempts reconnection

### Timeout Handling

- Default timeout: 60 seconds (`TIMEOUTSECS`)
- Request-response operations have configurable timeout
- Streaming has no timeout (continuous operation)

---

## Error Handling

### Error Categories

| Level | Handling |
|-------|----------|
| Thread | Caught via `std::exception`, stored in `m_ErrorString` |
| Socket | `CTRACK_THROW_SOCKET_ERROR` for network errors |
| Protocol | Invalid message format, missing parameters |
| Authentication | Handshake failure |

### Common Error Conditions

| Condition | Detection | Recovery |
|-----------|-----------|----------|
| Connection refused | `WSAECONNREFUSED` | Retry after 1 second |
| Connection reset | `recv()` returns 0 | Trigger disconnect callback |
| Timeout | Request timeout | Return failure to caller |
| Invalid message | JSON parse error | Log and ignore |

### Diagnostic Logging

```cpp
// Enable diagnostic callbacks
TCPServer.SetOnReceive([](auto &gram, bool send, int port) {
    // Log received telegrams
});

TCPServer.SetOnSend([](auto &gram, bool send, int port) {
    // Log sent telegrams
});
```

---

## Quick Reference

### Message Keywords

```cpp
// ProxyKeywords.h
TAG_HANDSHAKE              = "HANDSHAKE"
TAG_COMMAND_HARDWAREDETECT = "HARDWARE_DETECT"
TAG_COMMAND_CONFIGDETECT   = "CONFIG_DETECT"
TAG_COMMAND_CHECKINIT      = "CHECK_INIT"
TAG_COMMAND_SHUTDOWN       = "SHUTDOWN"
TAG_COMMAND_QUIT           = "QUIT"

ATTRIB_CHALLENGE                  = "challenge"
ATTRIB_HARDWAREDETECT_PRESENT     = "present"
ATTRIB_HARDWAREDETECT_FEEDBACK    = "feedback"
ATTRIB_HARDWAREDETECT_NUM_TRACKERS = "num_trackers"
ATTRIB_CHECKINIT_MEASFREQ         = "meas_freq"
```

### Key Classes

| Class | Location | Purpose |
|-------|----------|---------|
| `CTCPCommunication` | `Proxies/Libraries/TCP/` | TCP connection management |
| `CTCPGram` | `Proxies/Libraries/TCP/` | Telegram encoding/decoding |
| `Message` | `Proxies/Libraries/TCP/` | JSON message wrapper |
| `MessageResponder` | `Proxies/Libraries/TCP/` | Message routing |
| `CProxyDevice` | `CTrack_Data/` | Main app proxy interface |

### Default Configuration

| Parameter | Value |
|-----------|-------|
| Default Port | 40000 |
| Port Range | 40000 - 49999 |
| Default Timeout | 60 seconds |
| Listen Backlog | 1 |

---

## See Also

- [Proxy Development Guide](../Proxies/ProxyDevelopment.md)
- [Template Proxy](../Proxies/Template/)
- Source: `Proxies/Libraries/TCP/TCPCommunication.cpp`
- Source: `CTrack_Data/ProxyDevice.h`
