
#include <kapp.h>
#include "boserver.h" 
 
int main(int argc, char* argv[]) { 
  KApplication app(argc,argv,"boson");  
 
/*  if (app.isRestored())
    { 
      RESTORE(BosonServer);
    }
  else */
    {
      BosonServer* server = new BosonServer("basic.bpf");
      server->show();
    }  
  return app.exec();
}  
 
