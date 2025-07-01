
#include "LeicaDriver.h"
#include "../Libraries/TCP/TCPCommunication.h"
#include "../Libraries/TCP/TCPTelegram.h"
#include "../Libraries/Utility/errorException.h"
#include "../Libraries/Utility/NetworkError.h"
#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/os.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/Utility/CommandLineParameters.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/Utility/Logging.h"
#include "../../CTrack_Data/ProxyHandshake.h"

#include <msclr/gcroot.h>
#include <conio.h>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char *argv[])
{
    CTrack::InitLogging("");
    SetConsoleTabText("Template");
    SetConsoleTabBackgroundColor(CYAN);

    unsigned short PortNumber(40001);
    bool           showConsole{true};

    CommandLineParameters parameters(argc, argv);

    if (parameters.isInitializedFromJson())
    {
        PortNumber  = parameters.getInt(TCPPORT, 40001);
        showConsole = parameters.getBool(SHOWCONSOLE, false);
    }
    PortNumber = FindAvailableTCPPortNumber(PortNumber);

    ShowConsole(showConsole);
    if (showConsole)
    {
        PrintInfo("Big loop starting");
        PrintInfo("q : quit");
        PrintInfo("h : hardware detect");
        PrintInfo("c : configuration detect");
        PrintInfo("s : start track");
        PrintInfo("t : stop track");
        PrintInfo("p : simulate push trigger button");
        PrintInfo("v : simulate push validate button");
        PrintInfo("i : print info");
        PrintInfo("l : report last coordinates");
    }

    // startup server object
    CLeicaLMFDriver ^ driver                       = gcnew  CLeicaLMFDriver();
    msclr::gcroot<CLeicaLMFDriver ^>  driverHandle = driver;
    CCommunicationObject              TCPServer;
    std::vector<CTrack::Subscription> subscriptions;
    std::unique_ptr<CTrack::Message>  manualMessage;
    bool                              bContinueLoop = true;

    // responders & handlers
    TCPServer.SetOnConnectFunction([](SOCKET, size_t numConnections) { PrintInfo("connected : {}", numConnections); });
    TCPServer.SetOnDisconnectFunction([](SOCKET, size_t numConnections) { PrintWarning("DISCONNNECTED : {}", numConnections); });

    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(TAG_COMMAND_QUIT,
                                                                                    [&bContinueLoop](const CTrack::Message &) -> CTrack::Reply
                                                                                    {
                                                                                        bContinueLoop = false;
                                                                                        return nullptr;
                                                                                    })));
    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(TAG_HANDSHAKE, &ProxyHandShake::ProxyHandShake)));
    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(
        TAG_COMMAND_HARDWAREDETECT, [&driverHandle](const CTrack::Message &message) -> CTrack::Reply { return driverHandle->HardwareDetect(message); })));
    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(
        TAG_COMMAND_CONFIGDETECT, [&driverHandle](const CTrack::Message &message) -> CTrack::Reply { return driverHandle->ConfigDetect(message); })));
    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(
        TAG_COMMAND_CHECKINIT, [&driverHandle](const CTrack::Message &message) -> CTrack::Reply { return driverHandle->CheckInitialize(message); })));
    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(
        TAG_COMMAND_SHUTDOWN, [&driverHandle](const CTrack::Message &message) -> CTrack::Reply { return driverHandle->ShutDown(message); })));

    // start server
    TCPServer.Open(TCP_SERVER, PortNumber);
    PrintInfo("Server started on port " + std::to_string(PortNumber));

    while (bContinueLoop)
    {
        try
        {

            //------------------------------------------------------------------------------------------------------------------
            /*
            Respond to incoming messages
            */
            //------------------------------------------------------------------------------------------------------------------
            std::unique_ptr<CTCPGram> TCPGram;
            bool                      bAvailable = TCPServer.GetReceivePackage(TCPGram);
            if (bAvailable)
            {
                switch (TCPGram->GetCode())
                {
                    switch (TCPGram->GetCode())
                    {
                        case TCPGRAM_CODE_COMMAND:
                        {
                            PrintError("should not receive any commands here anymore");
                        };
                        break;
                        case TCPGRAM_CODE_TEST_BIG:
                        {
                            std::vector<char> arChar = TCPGram->GetData();
                            PrintInfo("Received a big packet of {} long values", arChar.size());
                        };
                        default:
                            PrintError("Unknown TCPgram received");
                            break;
                    }
                }
            }
            if (manualMessage)
            {
                TCPServer.GetMessageResponder()->RespondToMessage(*manualMessage);
                manualMessage.reset();
            }

            //------------------------------------------------------------------------------------------------------------------
            /*
            Running
            */
            //------------------------------------------------------------------------------------------------------------------
            if (driver->Run())
            {
                std::string valueString;
                for each (double value in driver->m_arDoubles)
                {
                    valueString += fmt::format(" {:.3f} ", value);
                }
                PrintInfo(valueString);
#ifdef _MANAGED
                std::unique_ptr<CTCPGram> TCPGRam;
                TCPGRam.reset(new CTCPGram(driver->m_arDoubles));
                TCPServer.PushSendPackage(TCPGRam);
#endif
            }

            //------------------------------------------------------------------------------------------------------------------
            /*
            Respond to direct user input
            */
            //------------------------------------------------------------------------------------------------------------------
            if (_kbhit())
            {
                char c = tolower(_getch());
                switch (c)
                {
                    case 'q':
                        manualMessage = std::make_unique<CTrack::Message>(TAG_COMMAND_QUIT);
                        break;
                    case 'h':
                        manualMessage = std::make_unique<CTrack::Message>(TAG_COMMAND_HARDWAREDETECT);
                        break;
                    case 'c':
                        manualMessage = std::make_unique<CTrack::Message>(TAG_COMMAND_CONFIGDETECT);
                        break;
                    case 's':
                    {
                        double AcquisitionRate(1.0);
                        std::cout << "Enter the measurement frequency" << std::endl;
                        std::cin >> AcquisitionRate;
                        manualMessage                                         = std::make_unique<CTrack::Message>(TAG_COMMAND_CHECKINIT);
                        manualMessage->GetParams()[ATTRIB_CHECKINIT_MEASFREQ] = AcquisitionRate;
                    };
                    break;
                    case 't':
                        manualMessage = std::make_unique<CTrack::Message>(TAG_COMMAND_SHUTDOWN);
                        break;
                }
            }
        }
        catch (const std::exception &e)
        {
            PrintError("An error occurred : %s", e.what());
            std::unique_ptr<CTCPGram> TCPGRam = std::make_unique<CTCPGram>(e);
            TCPServer.PushSendPackage(TCPGRam);
        }
        catch (...)
        {
            PrintError("An unknown error occurred");
            std::unique_ptr<CTCPGram> TCPGRam = std::make_unique<CTCPGram>("An unknown error occurred", TCPGRAM_CODE_ERROR);
            TCPServer.PushSendPackage(TCPGRam);
        }
    }
    PrintInfo("Closing server");
    TCPServer.Close();
    return 0;
}
