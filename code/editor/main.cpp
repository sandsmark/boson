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

#include "boeditor.h" 
 
int main(int argc, char* argv[])
{ 
	KApplication app(argc,argv,"BoEditor");  
 
/*	if (app.isRestored())
		RESTORE(BoEditorApp);
	else { */
		BoEditorApp* boEditor = new BoEditorApp( (argc>1)?argv[1]:0l);
		app.setMainWidget(boEditor);
		app.setTopWidget(boEditor);
		boEditor->show();
//		}  

	return app.exec();
}  
 
