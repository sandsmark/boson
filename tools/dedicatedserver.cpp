#include <qapplication.h>

#include <kdebug.h>
#include <kgame/kmessageserver.h>


#define BOSON_COOKIE 992
#define DEFAULT_PORT 5454
#define MAX_CLIENTS 10


int main(int argc, char **argv)
{
  QApplication a(argc, argv, false);

  // Create message server
  KMessageServer* server = new KMessageServer(BOSON_COOKIE);
  if(!server->initNetwork(DEFAULT_PORT))
  {
    kdDebug() << k_funcinfo << "Couldn't init network using port " << DEFAULT_PORT << endl;
    return 1;
  }

  // Set some params
  server->setMaxClients(MAX_CLIENTS);

  return a.exec();
}
