/***************************************************************************
                         boFile.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Feb 14 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : orzel@yalbi.com                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef BOFILE_H 
#define BOFILE_H 

#include <qstring.h>

#include "../common/unitType.h"
#include "../common/groundType.h"


class QDataStream;
class QFile;
class mobileMsg_t;
class facilityMsg_t;
class Cell;
//class QString;


#define stateAssert(s)								\
		if ((s)!=BFstate) {						\
			logf(LOG_ERROR, "boFile : bad BFstate %d, while %d assumed, line %d",\
				(s), BFstate, __LINE__);				\
			error = true; return;					\
		}

class boFile 
{

public:
	boFile();
virtual	~boFile();

protected :

	bool	openRead(const char *);
	bool	openWrite(const char *);
	bool	Close();

/* header */
	QString		worldName;
	int		map_width;
	int		map_height;
	int		nbPlayer;
	int		nbMobiles;
	int		nbFacilities;

	void	load(Cell &);
	void	load(mobileMsg_t &);
	void	load(facilityMsg_t &);

	void	write(Cell &);
	void	write(mobileMsg_t &m);
	void	write(facilityMsg_t &f);

	bool	isOk() {return !error; }


protected:

	enum {
		None,		// nothing has happened
		Read,		// a file is being read
		Write,		// a file is being written
	}		BFstate;
	QDataStream	*stream;
	QFile		*f;
	bool		error;

};
 

#endif // BOFILE_H

