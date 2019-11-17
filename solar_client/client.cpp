#include "kNet.h"
#include "kNet/DebugMemoryLeakCheck.h"

using namespace kNet;

// Define a MessageID for our a custom message.
const message_id_t cHelloMessageID = 32;

BottomMemoryAllocator bma;
char com[100];
std::string mess;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
          std::cout << "Usage: " << argv[0] << " server-ip-1 [server-ip-2 ...]" << std::endl;
          return 0;
    }

  kNet::SetLogChannels(LogUser | LogInfo | LogError);
  EnableMemoryLeakLoggingAtExit();

    Network network;
  // const unsigned short cServerPort = 32000;

  Ptr(MessageConnection) connection1;
  Ptr(MessageConnection) connection2;
  Ptr(MessageConnection) connection3;
  Ptr(MessageConnection) connection4;
  Ptr(MessageConnection) connection5;

  connection1 = network.Connect(argv[1], 32000, SocketOverUDP,  NULL);
  if (argc > 2) {
    connection2 = network.Connect(argv[2], 32001, SocketOverUDP,  NULL);
    if (argc > 3) {
      connection3 = network.Connect(argv[3], 32002, SocketOverUDP,  NULL);
      if (argc > 4) {
        connection4 = network.Connect(argv[4], 32003, SocketOverUDP,  NULL);
        if (argc > 5) {
          connection5 = network.Connect(argv[5], 32004, SocketOverUDP,  NULL);
        }
      }
    }
  }


  std::cin.getline(com,sizeof(com));
  while (com[0]!='X')
  {
          if (connection1)
          {
                  connection1->SendMessage(cHelloMessageID, true, true, 100, 0, com, strlen(com));
                  printf("message sent: [%s]\n",com);

          }
          if (connection2)
          {
                  connection2->SendMessage(cHelloMessageID, true, true, 100, 0, com, strlen(com));
                  printf("message sent: [%s]\n",com);

          }
          if (connection3)
          {
                  connection3->SendMessage(cHelloMessageID, true, true, 100, 0, com, strlen(com));
                  printf("message sent: [%s]\n",com);

          }
          if (connection4)
          {
                  connection4->SendMessage(cHelloMessageID, true, true, 100, 0, com, strlen(com));
                  printf("message sent: [%s]\n",com);

          }
          if (connection5)
          {
                  connection5->SendMessage(cHelloMessageID, true, true, 100, 0, com, strlen(com));
                  printf("message sent: [%s]\n",com);

          }
    std::cin.getline(com,sizeof(com));
  }
   
    return 0;
}

