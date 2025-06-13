#include "Driver.h"
#include "../Libraries/TCP/TCPCommunication.h"
#include "../Libraries/TCP/TCPTelegram.h"
#include "../Libraries/Utility/errorException.h"
#include "../Libraries/Utility/NetworkError.h"
#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/Logging.h"
#include "../Libraries/Utility/filereader.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../../CTrack_Data/ProxyHandshake.h"

#include <conio.h>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>

int main(int argc, char *argv[])
{
    CTrack::InitLogging("");
    SetConsoleTabText("Template");
    SetConsoleTabBackgroundColor(GREEN);

    //
    // parse port
    unsigned short PortNumber(40001);
    if (argc >= 2)
        PortNumber = atoi(argv[1]);
    PortNumber = FindAvailableTCPPortNumber(PortNumber);

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
    PrintInfo("b : send big TCP package");

    // startup server object
    std::unique_ptr<Driver>           driver = std::make_unique<Driver>();
    CCommunicationObject              TCPServer;
    std::vector<CTrack::Subscription> subscriptions;
    std::unique_ptr<CTrack::Message>  manualMessage;
    bool                              bContinueLoop = true;

    // responders & handlers
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
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_HANDSHAKE,  &ProxyHandShake::ProxyHandShake);
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_HARDWAREDETECT, CTrack::MakeMemberHandler(driver.get(), &Driver::HardwareDetect));
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_CONFIGDETECT, CTrack::MakeMemberHandler(driver.get(), &Driver::ConfigDetect));
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_CHECKINIT, CTrack::MakeMemberHandler(driver.get(), &Driver::CheckInitialize));
    driver->Subscribe(*TCPServer.GetMessageResponder(), TAG_COMMAND_SHUTDOWN, CTrack::MakeMemberHandler(driver.get(), &Driver::ShutDown));

    // start server
    TCPServer.Open(TCP_SERVER, PortNumber);
    PrintInfo("Server started on port {}", PortNumber);

    while (bContinueLoop)
    {
        try
        {
            // auto                      v = FileReader::ReadNumbersFromFile("C:\\CTrack-Software\\Testing\\AMT with headers.txt");
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
                //                 std::string ValueString, FullLine;
                //                 for (auto &value : driver->m_arDoubles)
                //                 {
                //                     ValueString = fmt::format("{:10.3f}", value);
                //                     FullLine += ValueString + " ";
                //                 };
                //                 PrintInfo(FullLine);
                std::unique_ptr<CTCPGram> TCPGRam = std::make_unique<CTCPGram>(driver->m_arDoubles);
                TCPServer.PushSendPackage(TCPGRam);
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
                    case 'p':
                        driver->PressTriggerButton();
                        break;
                    case 'v':
                        driver->PressValidateButton();
                        break;
                    case 'b':
                    {
                        size_t                     NumValues = 1000000;
                        std::vector<unsigned long> arLong;
                        arLong.resize(NumValues);
                        std::generate_n(arLong.begin(), arLong.size(), [n = 1]() mutable { return n++; });

                        std::vector<char> arChar;
                        arChar.swap(*reinterpret_cast<std::vector<char> *>(&arLong));
                        std::unique_ptr<CTCPGram> TCPGRam = std::make_unique<CTCPGram>(std::move(arChar), TCPGRAM_CODE_TEST_BIG);
                        TCPServer.PushSendPackage(TCPGRam);
                        PrintInfo("Sending a big packet of {} long values", NumValues);
                    };
                    break;
                    case 'e':
                    {
                        throw(std::exception("Fucking Hell"));
                    };
                    break;
                    case 'w':
                    {
                        std::unique_ptr<CTCPGram> TCPGRam = std::make_unique<CTCPGram>(CTrack::Message("warning", {{"message", "The system is running hot"}}));
                        TCPServer.PushSendPackage(TCPGRam);
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
