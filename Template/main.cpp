
#include "../Libraries/Driver/IDriver.h"
#include "../Libraries/TCP/TCPCommunication.h"
#include "../Libraries/TCP/TCPTelegram.h"
#include "../Libraries/Utility/errorHandling.h"
#include "../Libraries/Utility/NetworkError.h"
#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/StringUtilities.h"
#include "../Libraries/XML/ProxyKeywords.h"
#include "../Libraries/XML/TinyXML_AttributeValues.h"

#include <conio.h>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char *argv[])
{
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
    PrintInfo("t : start track");
    PrintInfo("s : stop track");
    PrintInfo("p : simulate push trigger button");
    PrintInfo("v : simulate push validate button");
    PrintInfo("i : print info");
    PrintInfo("l : report last coordinates");

    // startup server object
    CCommunicationObject    TCPServer;
    std::unique_ptr<Driver> driver = std::make_unique<Driver>();
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
                        std::cout << "Unknown TCPgram received" << endl;
                        break;
                }
            }

            if (!Command.empty())
            {
                if (Command == TAG_COMMAND_QUIT)
                {
                    std::cout << "Quit" << endl;
                    bContinueLoop = false;
                }
                if (Command == TAG_COMMAND_HARDWAREDETECT)
                {
                    std::unique_ptr<TiXmlElement> Response = driver->HardwareDetect(TCP_XML_Input);
                    std::unique_ptr<CTCPGram>     TCPGRam  = std::make_unique<CTCPGram>(Response, TCPGRAM_CODE_COMMAND);
                    TCPServer.PushSendPackage(TCPGRam);
                }
                if (Command == TAG_COMMAND_CONFIGDETECT)
                {
                    std::unique_ptr<TiXmlElement> Response = driver->ConfigDetect(TCP_XML_Input);
                    std::unique_ptr<CTCPGram>     TCPGRam  = std::make_unique<CTCPGram>(Response, TCPGRAM_CODE_COMMAND);
                    TCPServer.PushSendPackage(TCPGRam);
                }
                if (Command == TAG_COMMAND_CHECKINIT)
                {
                    std::unique_ptr<TiXmlElement> Response = driver->CheckInitialize(TCP_XML_Input);
                    std::unique_ptr<CTCPGram>     TCPGRam  = std::make_unique<CTCPGram>(Response, TCPGRAM_CODE_COMMAND);
                    TCPServer.PushSendPackage(TCPGRam);
                }
                if (Command == TAG_COMMAND_SHUTDOWN)
                {
                    std::unique_ptr<TiXmlElement> Response = driver->Stop();
                    std::unique_ptr<CTCPGram>     TCPGRam  = std::make_unique<CTCPGram>(Response, TCPGRAM_CODE_COMMAND);
                    TCPServer.PushSendPackage(TCPGRam);
                }
                Command.clear();
            }
            //------------------------------------------------------------------------------------------------------------------
            /*
            Running
            */
            //------------------------------------------------------------------------------------------------------------------
            if (driver->Run())
            {
                std::unique_ptr<CTCPGram> TCPGRam;
                //            TCPGRam.reset(new CTCPGram(driver->m_arDoubles));
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
                        bContinueLoop = false;
                        break;
                    case 'h':
                        Command = TAG_COMMAND_HARDWAREDETECT;
                        break;
                    case 'c':
                        Command = TAG_COMMAND_CONFIGDETECT;
                        break;
                    case 't':
                    {
                        double AcquisitionRate(1.0);
                        Command = TAG_COMMAND_CHECKINIT;
                        std::cout << "Enter the measurement frequency" << endl;
                        cin >> AcquisitionRate;
                        TCP_XML_Input = std::make_unique<TiXmlElement>(TAG_COMMAND_CHECKINIT);
                        if (TCP_XML_Input)
                        {
                            GetSetAttribute(TCP_XML_Input.get(), ATTRIB_CHECKINIT_MEASFREQ, AcquisitionRate, XML_WRITE);
                        }
                    };
                    break;
                    case 's':
                        Command = TAG_COMMAND_SHUTDOWN;
                        break;
                }
            }
        }
        catch (const std::exception &e)
        {
            PrintError(e.what());
        }
        catch (...)
        {
            PrintError("An unknown error occurred");
        }
    }
    PrintInfo("Closing server");
    TCPServer.Close();
    return 0;
}
