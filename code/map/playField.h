/***************************************************************************
                         playField.h  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Feb 14 1999
                                           
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

#ifndef PLAYFIELD_H 
#define PLAYFIELD_H 

#include "../common/unitType.h"


class Cell;
class QDataStream;


struct origMobile {
	mobType	t;
	int	who;
	int	x;
	int	y;
	};

struct origFacility {
   facilityType	t;
	int	who;
	int	x;
	int	y;
	};


struct origPeople {
	int		nbMobiles;
	int		nbFacilities;
	origMobile	*mobile;
	origFacility	*facility;
	};


struct bosonMap {
	Cell **cells;
	int	width;
	int	height;
	};

/* this is the callBack to create the XXXXCell array */
/* that should, of course, inherits Cell (common/Cell.h )*/
typedef Cell ** createCellFunc(int width, int height);

class playField //: public  QDataStream
{

 public:
	playField(const char *name);
	~playField();

	bool	load	(createCellFunc *);
	bool	write	(void);

 private:

/*	QDataStream& operator<< (origMobile *);
	QDataStream& operator>> (origMobile *);

	QDataStream& operator<< (origFacility *);
	QDataStream& operator>> (origFacility *);

	QDataStream& operator<< (Cell *);
	QDataStream& operator>> (Cell *); */

	bool	load(origMobile &);
	bool	load(origFacility &);
	bool	load(Cell  &);
	bool	loadMap(createCellFunc *);
	bool	loadPeople();
	void	write(origMobile &);
	void	write(origFacility &);
	void	write(Cell  &);
	void	writeMap();
	void	writePeople();

	QDataStream	*stream;
	const char	*filename;

public :
	int		nbPlayer;
	bosonMap	map;
	origPeople	people;

/*	int	nbMobiles;
	int	nbFacilities;
	origMobile *mobile;
	origFacility *facility; */

};
 

#endif // PLAYFIELD_H

