/***************************************************************************
                          bosonCanvas.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#ifndef BOSONFIELD_H 
#define BOSONFIELD_H 

#include <time.h>

#include <qintdict.h>

#include "common/msgData.h"
#include "common/unit.h"	// Facility

#include "playerUnit.h"		// playerMobUnit
#include "visualCanvas.h"
#include "bosonCell.h"

class QRect;
class QPainter;
class Unit;


#define RADAR_PULSE_PERIOD	150		// every RADAR_PULSE_PERIOD jiffies, the radar emits a bip

/** 
  * This class encapsulate the "physical" idea of the map : size, contents..
  */
class bosonCanvas : public visualCanvas, public CellMap
{
	Q_OBJECT

public:
	bosonCanvas( QPixmap, uint w, uint h, uint np);
	~bosonCanvas();

  void createMob(mobileMsg_t &);
  void unHideMob(mobileMsg_t &);
  void destroyMob(destroyedMsg_t &);
  void hideMob(destroyedMsg_t &);

  void createFix(facilityMsg_t &);
  void unHideFix(facilityMsg_t &);
  void destroyFix(destroyedMsg_t &);
  void hideFix(destroyedMsg_t &);

  void move(moveMsg_t &);
  void shooted(powerMsg_t &);
  void shoot(shootMsg_t &);
  void requestAction(void);
  void updateRess(unitRessMsg_t &);


	/** third (and last) communication layer : game */
	void handleGameMessage(bosonMsgTag, int, bosonMsgData *);

	/** configure the cell at the given point (i,j)
	 * reimplement(and uses) visualCanvas::setCell()
	 * */
	void	setCell(int i, int j, cell_t c);        // not virtual !  

	// CellMap functions
	virtual enum groundType	groundAt(QPoint p) { return ::ground( (cell_t)tile(p.x(), p.y()) ); }
	/** return the cell at (i,j) */
	virtual Cell&	ccell(int i, int j) { return cells[i+j*_width]; }

	/* concerning contents */
	playerFacility *getFacility(long key) { return facility.find(key); }

	// sound stuff
	void	play(char *);

	//private :
	QIntDict<playerMobUnit>	mobile;
	QIntDict<playerFacility> facility;

private:
	/* following are handler for all the messages coming from the server */
	void msg_map_discovered(cooMsg_t m);


	/** used to handle for of war */
	void		handleFOW(int, int);
	bosonCell&	cell(int i, int j) { return cells[i+j*_width]; }
	bool		cellKnown(int i, int j) { return tile(i,j)!=0;}

signals:
	void	oilUpdated(int);
	void	mineralUpdated(int);
	void	mobileNbUpdated(int);
	void	facilityNbUpdated(int);
	void	pingUpdated(int);

private:
	bosonCell	*cells;
	time_t	ping;
	time_t	last_sync;
	int	radar_pulse;
	int	my_fix;
	int	my_mobiles;
	uint	nb_player;
};

#endif // BOSONFIELD_H

