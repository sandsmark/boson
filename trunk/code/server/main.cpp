/***************************************************************************
                          boson.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : ????
                                           
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

#include <kapp.h>

#include "boserver.h" 
 
int main(int argc, char* argv[])
{ 

	KApplication app(argc,argv,"boson");  
 
/*	if (app.isRestored()) 
		RESTORE(BosonServer);
	else { */
	BosonServer* server = new BosonServer("basic.bpf");
	server->show();
//}  
	return app.exec();
}  
 
