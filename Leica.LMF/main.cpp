
#include "LeicaDriver.h"
#include "../Libraries/StressTest/StressTest.h"
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

void leicaTestCode()
{
    try
    {
        LMF::Tracker::Connection ^ con = gcnew LMF::Tracker::Connection();
        if (con != nullptr)
        {
            std::string                        IPAdres("AT960LRSimulator#506432");
            System::String ^ ipAddress = gcnew System::String("AT960LRSimulator#506432");
            LMF::Tracker::Tracker ^ tracker =                con->Connect(ipAddress);
        }
    }
    catch (std::exception &e)
    {
        PrintError(e.what());
    }
    catch (LMF::Tracker::ErrorHandling::LmfException ^ ex)
    {
        System::Console::WriteLine("LMF Error getting targets: {0}", ex->Description); // [cite: 788]
    }
}

int main(int argc, char *argv[])
{
    CTrack::InitLogging("");
    SetConsoleTabText("Leica.LMF");
    SetConsoleTabBackgroundColor(CYAN);
#ifdef _DEBUG
//    leicaTestCode();
#endif

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
        PrintInfo("z : start stress test");
        PrintInfo("y : stop stress test");
    }

    // startup server object - use native LeicaDriver wrapper for IDriver compatibility
    std::unique_ptr<LeicaDriver>      driver = std::make_unique<LeicaDriver>();
    CCommunicationObject              TCPServer;
    std::vector<CTrack::Subscription> subscriptions;
    std::unique_ptr<CTrack::Message>  manualMessage;
    std::unique_ptr<StressTest>       stressTest;
    bool                              bContinueLoop = true;
    std::vector<std::string>          names({"Simulator"}), serialNumbers({"506432"}), IPAddresses({"AT960LRSimulator#506432"}), types({"AT960LRSimulator"}),
        comments({"using simulator"});

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
        TAG_COMMAND_HARDWAREDETECT, [&driver](const CTrack::Message &message) -> CTrack::Reply { return driver->HardwareDetect(message); })));
    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(
        TAG_COMMAND_CONFIGDETECT, [&driver](const CTrack::Message &message) -> CTrack::Reply { return driver->ConfigDetect(message); })));
    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(
        TAG_COMMAND_CHECKINIT, [&driver](const CTrack::Message &message) -> CTrack::Reply { return driver->CheckInitialize(message); })));
    subscriptions.emplace_back(std::move(TCPServer.GetMessageResponder()->Subscribe(
        TAG_COMMAND_SHUTDOWN, [&driver](const CTrack::Message &message) -> CTrack::Reply { return driver->ShutDown(message); })));

    // start server
    TCPServer.Open(TCP_SERVER, PortNumber);
    PrintInfo("Server started on port " + std::to_string(PortNumber));

    // Initialize stress test
    stressTest = std::make_unique<StressTest>(driver.get(), TCPServer.GetMessageResponder());

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
            // Skip driver->Run() if stress test is handling tracking
            bool stressTestTracking = stressTest && stressTest->IsRunning() && stressTest->IsTracking();
            if (!stressTestTracking && driver->Run())
            {
                std::vector<double> values;
                if (driver->GetValues(values))
                {
                    std::string valueString;
                    for (const auto& value : values)
                    {
                        valueString += fmt::format(" {:.3f} ", value);
                    }
                    PrintInfo(valueString);
                    std::unique_ptr<CTCPGram> TCPGRam = std::make_unique<CTCPGram>(values);
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
                    case 'z':
                    {
                        if (stressTest && !stressTest->IsRunning())
                        {
                            stressTest->Start();
                        }
                        else if (stressTest && stressTest->IsRunning())
                        {
                            PrintWarning("Stress test already running - press 'y' to stop");
                        }
                    };
                    break;
                    case 'y':
                    {
                        if (stressTest && stressTest->IsRunning())
                        {
                            stressTest->Stop();
                        }
                        else
                        {
                            PrintWarning("Stress test is not running");
                        }
                    };
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
    // Stop stress test if running before shutdown
    if (stressTest && stressTest->IsRunning())
    {
        PrintInfo("Stopping stress test before shutdown...");
        stressTest->Stop();
    }
    stressTest.reset();

    PrintInfo("Closing server");
    TCPServer.Close();
    return 0;
}
