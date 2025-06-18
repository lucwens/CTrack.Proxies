
#include "../Libraries/TCP/TCPCommunication.h"
#include "../Libraries/TCP/TCPTelegram.h"
#include "../Libraries/Utility/NetworkError.h"
#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/Logging.h"
#include "../Libraries/Utility/os.h"
#include "../Libraries/Utility/filereader.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/Utility/CommandLineParameters.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "DriverVicon.h"
#include "../../CTrack_Data/ProxyHandshake.h"

#include <conio.h>
#include <iostream>
#include <memory>
#include <string>
#include <format>

int main(int argc, char *argv[])
{
    CTrack::InitLogging("");
    SetConsoleTabText("Vicon");
    SetConsoleTabBackgroundColor(MAGENTA);

    
    //
    // command line parameters
    unsigned short PortNumber(40001);
    bool           showConsole{false};

    CommandLineParameters parameters(argc, argv);

    if (parameters.isInitializedFromJson())
    {
        PortNumber  = parameters.getInt(TCPPORT, 40001);
        showConsole = parameters.getBool(SHOWCONSOLE, false);
    }
    PortNumber = FindAvailableTCPPortNumber(PortNumber);

    SetConsoleVisible(showConsole);
    if (showConsole)
    {
        PrintInfo("Big loop starting");
        PrintInfo("q : quit");
        PrintInfo("h : hardware detect");
        PrintInfo("c : configuration detect");
        PrintInfo("s : start track");
        PrintInfo("t : stop track");
    }

    // startup server object
    std::unique_ptr<DriverVicon>      driver = std::make_unique<DriverVicon>();
    CCommunicationObject              TCPServer;
    std::vector<CTrack::Subscription> subscriptions;
    std::unique_ptr<CTrack::Message>  manualMessage;
    bool                              bContinueLoop = true;

    TCPServer.SetOnConnectFunction([](SOCKET, size_t numConnections) { PrintInfo("connected : {}", numConnections); });
    TCPServer.SetOnDisconnectFunction([](SOCKET, size_t numConnections) { PrintWarning("DISCONNNECTED : {}", numConnections); });
#if !defined(CTRACK_UI) && !defined(_DEBUG)
    subscriptions.emplace_back(std::move(TCPServer.Subscribe(TAG_HANDSHAKE, &ProxyHandShake::ProxyHandShake)));
#endif

    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_QUIT,
                      [&bContinueLoop](const CTrack::Message &) -> CTrack::Reply
                      {
                          bContinueLoop = false;
                          return nullptr;
                      });
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_HANDSHAKE, &ProxyHandShake::ProxyHandShake);
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_HARDWAREDETECT, CTrack::MakeMemberHandler(driver.get(), &DriverVicon::HardwareDetect));
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_CONFIGDETECT, CTrack::MakeMemberHandler(driver.get(), &DriverVicon::ConfigDetect));
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_CHECKINIT, CTrack::MakeMemberHandler(driver.get(), &DriverVicon::CheckInitialize));
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_SHUTDOWN, CTrack::MakeMemberHandler(driver.get(), &DriverVicon::ShutDown));

    TCPServer.Open(TCP_SERVER, PortNumber);
    PrintInfo("Server started on port {}", PortNumber);


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
                std::string         ValueString, FullLine;
                std::vector<double> arValues;
                if (driver->GetValues(arValues))
                {
                    std::unique_ptr<CTCPGram> TCPGRam = std::make_unique<CTCPGram>(arValues);
                    TCPServer.PushSendPackage(TCPGRam);
                }
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
                        double AcquisitionRate(50.0);
//                         std::cout << "Enter the measurement frequency" << std::endl;
//                         std::cin >> AcquisitionRate;
                        manualMessage                                         = std::make_unique<CTrack::Message>(TAG_COMMAND_CHECKINIT);
                        manualMessage->GetParams()[ATTRIB_CHECKINIT_MEASFREQ] = AcquisitionRate;
                    };
                    break;
                    case 't':
                        manualMessage = std::make_unique<CTrack::Message>(TAG_COMMAND_SHUTDOWN);
                        break;
                    case 'o':
                    {
                        SetConsoleVisible(true);
                    };
                    break;
                    case 'x':
                    {
                        SetConsoleVisible(false);
                    };
                    break;
                }
            }
        }
        catch (const std::exception &e)
        {
            PrintError("An error occurred : {}", e.what());
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
