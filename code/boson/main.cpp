
#include "boson.h" 
 
int main(int argc, char* argv[]) { 
  KApplication app(argc,argv,"boson");  
 
  if (app.isRestored())
    { 
      RESTORE(BosonApp);
    }
  else 
    {
      BosonApp* boson = new BosonApp;
	app.setMainWidget(boson);
	app.setTopWidget(boson);
	boson->show();
    }  
  return app.exec();
}  
 
