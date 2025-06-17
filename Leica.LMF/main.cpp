
#include "LeicaDriver.h"
#include "../Libraries/TCP/TCPCommunication.h"
#include "../Libraries/TCP/TCPTelegram.h"
#include "../Libraries/Utility/errorException.h"
#include "../Libraries/Utility/NetworkError.h"
#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/os.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"
#include "../Libraries/Utility/Logging.h"

#ifndef _DEBUG
#include "../../CTrack_Data/ProxyHandshake.h"
#endif

#include <conio.h>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char *argv[])
{
    CTrack::InitLogging("");
    //
    // parse port
    unsigned short                PortNumber(40014);
    std::string                   Command;
    std::unique_ptr<TiXmlElement> TCP_XML_Input;
    if (argc >= 2)
        PortNumber = atoi(argv[1]);

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

    // startup server object
    CCommunicationObject TCPServer;
    CLeicaLMFDriver      driver;
    // diagnostics on the TCP server
   
    TCPServer.Open(TCP_SERVER, PortNumber);
    PrintInfo("Server started on port " + std::to_string(PortNumber));

    bool bContinueLoop = true;

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
                        TCP_XML_Input = TCPGram->GetXML();
                        if (TCP_XML_Input)
                        {
                            // CString XMLString = XML_To_String(XMLElement.get());
                            Command = ToUpperCase(TCP_XML_Input->Value());
                        }
                    };
                    break;
                    default:
                        PrintError("Unknown TCPgram received ");
                }
            }

            if (!Command.empty())
            {
                std::unique_ptr<TiXmlElement> Response;
                if (Command == TAG_COMMAND_QUIT)
                {
                    PrintInfo("Quit");
                    bContinueLoop = false;
                }
                if (Command == TAG_COMMAND_HARDWAREDETECT)
                {
                    Response = driver.HardwareDetect(TCP_XML_Input);
                }
                if (Command == TAG_COMMAND_CONFIGDETECT)
                {
                    Response = driver.ConfigDetect(TCP_XML_Input);
                }
                if (Command == TAG_COMMAND_CHECKINIT)
                {
                    std::string xmlstring = XMLToString(TCP_XML_Input);
                    Response              = driver.CheckInitialize(TCP_XML_Input);
                }
                if (Command == TAG_COMMAND_SHUTDOWN)
                {
                    Response = driver.ShutDown();
                }
                if (Command == TAG_COMMAND_QUIT)
                {
                    PrintInfo("Quit");
                    bContinueLoop = false;
                }

                std::string               xmlstring = XMLToString(Response);
                std::unique_ptr<CTCPGram> TCPGRam   = std::make_unique<CTCPGram>(Response, TCPGRAM_CODE_COMMAND);
                TCPServer.PushSendPackage(TCPGRam);
                Command.clear();
            }
            //------------------------------------------------------------------------------------------------------------------
            /*
            Running
            */
            //------------------------------------------------------------------------------------------------------------------
            if (driver.Run())
            {
                std::string valueString;
                for each (double value in driver.m_arDoubles)
                {
                    valueString += fmt::format(" {:.3f} ", value);
                }
                PrintInfo(valueString);
#ifdef _MANAGED
                std::unique_ptr<CTCPGram> TCPGRam;
                TCPGRam.reset(new CTCPGram(driver.m_arDoubles));
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
                        bContinueLoop = false;
                        break;
                    case 'h':
                        Command = TAG_COMMAND_HARDWAREDETECT;
                        break;
                    case 'c':
                        Command = TAG_COMMAND_CONFIGDETECT;
                        break;
                    case 's':
                    {
                        double AcquisitionRate(1.0);
                        Command = TAG_COMMAND_CHECKINIT;
                        PrintCommand("Enter the measurement frequency");
                        std::cin >> AcquisitionRate;
                        TCP_XML_Input = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);
                        if (TCP_XML_Input)
                        {
                            GetSetAttribute(TCP_XML_Input.get(), ATTRIB_CHECKINIT_MEASFREQ, AcquisitionRate, XML_WRITE);
                        }
                    };
                    break;
                    case 't':
                        Command = TAG_COMMAND_SHUTDOWN;
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
            PrintError("An error occurred : %s", e.what());
            TCPServer.PushSendPackage(std::make_unique<CTCPGram>(e));
        }
        catch (...)
        {
            PrintError("An unknown error occurred");
            TCPServer.PushSendPackage(std::make_unique<CTCPGram>(std::exception("An unknown error occurred")));
        }
    }
    PrintInfo("Closing server");
    TCPServer.Close();
    return 0;
}
