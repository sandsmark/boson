/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOSONCOMMANDFRAMEINTERFACE_H
#define BOSONCOMMANDFRAMEINTERFACE_H

#include <qframe.h>

class Player;
class BoSelection;
class BosonGroundTheme;
class BoSpecificAction;
class Unit;


// TODO: BosonWidgetBase::initCommandFrame() wants the frame to be a QWidget
// TODO: BosonWidgetBase d'tor needs to delete the cmdframe
// TODO: createCommandFrame()


class BosonCommandFrameInterface;
class BosonCommandFrameFactoryBase
{
public:
	BosonCommandFrameFactoryBase()
	{
	}

	virtual BosonCommandFrameInterface* createCommandFrame(QWidget* parent, bool game) = 0;
};

/**
 * This class is supposed to be used in actual boson code. You should not
 * #include any header from the commandframe directory.
 *
 * @short An interface to @ref BosonCommandFrameBase
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCommandFrameInterface : public QFrame // FIXME: we should use QObject only, once we ported to OpenGL
{
	Q_OBJECT
public:
	BosonCommandFrameInterface(QWidget* parent, const char* name = 0);
	virtual ~BosonCommandFrameInterface();

	/**
	 * Set the factory for @ref createCommandFrame.
	 *
	 * This approach is useful to avoid linking to the commandframe library.
	 * All calls to the commandframe go to the BosonCommandFrameInterface,
	 * only the @ref createCommandFrame method requires more commandframe
	 * definitions
	 **/
	static void setFactory(BosonCommandFrameFactoryBase*);

	static BosonCommandFrameInterface* createCommandFrame(QWidget* parent, bool game);

	/**
	 * @param p The player whose units can be produced here.
	 **/
	virtual void setLocalPlayer(Player* p) = 0;

	/**
	 * @return Current selection
	 **/
	virtual BoSelection* selection() const = 0;

	/**
	 * Editor mode only.
	 **/
	virtual void setGroundTheme(BosonGroundTheme*) = 0;

	/**
	 * Editor mode only. Display "place ground" widgets.
	 **/
	virtual void placeGround()  = 0;

	/**
	 * Editor mode only. Place all mobile units for the specified player
	 * into the command frame.
	 **/
	virtual void placeMobiles(Player*) = 0;

	/**
	 * Editor mode only. Place all facilities for the specified player
	 * into the command frame.
	 **/
	virtual void placeFacilities(Player*) = 0;

public slots:
	/**
	 * Should be called when the production of the factory changes, i.e. is
	 * stopped/paused or started.
	 **/
	virtual void slotUpdateProduction(Unit* factory) = 0;

	/**
	 * One of the most important methods in this class
	 * This is called whenever selection changes, i.e. when unit(s) is selected
	 * or unselected.
	 **/
	virtual void slotSelectionChanged(BoSelection*) = 0;

signals:
	// AB: maybe use PlayerIO instead of Player parameter
	void signalPlaceUnit(unsigned long int unitType, Player* owner);

	/**
	 * @param textureCount See @ref BosonGroundTheme::textureCount
	 * @param alpha An array (of size @þ textureCount) defining how much of
	 * every texture should get displayed. 255 is maximum, 0 is nothing.
	 **/
	void signalPlaceGround(unsigned int textureCount, unsigned char* alpha);

	/**
	 * Emitted when user clicks on action button (e.g move). Also used for
	 * the placement preview, when the player clicks on a constructed
	 * facility and wants it to be placed on the map.
	 */
	void signalAction(const BoSpecificAction& action);

	/**
	 * This unit should become the only selected unit. See @ref
	 * BosonOrderButton::signalSelectUnit
	 **/
	void signalSelectUnit(Unit* unit);

private:
	static BosonCommandFrameFactoryBase* mFactory;
};

#endif
