/***************************************************************************
                          main.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : ?????
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : capricel@enst.fr                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "boson.h" 
 
int main(int argc, char* argv[])
{ 
	KApplication app(argc,argv,"Boson");  
 
/*	if (app.isRestored())
		RESTORE(BosonApp);
	else { */
		BosonApp* boson = new BosonApp( (argc>1)?argv[1]:0l);
		app.setMainWidget(boson);
		app.setTopWidget(boson);
		boson->show();
//		}  

	return app.exec();
}  
 
