/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2003-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef GLOBAL_H
#define GLOBAL_H

/**
 * Here you won't find any classes. These are mostly enums which are used at
 * different places in boson by several reasons. Examples are mostly
 * configuration items.
 *
 * I am usually against global variables. Please try not to add any here. There
 * is usually a better way.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/

enum CursorMode {
	CursorOpenGL = 0,
	CursorKDE = 1
};

enum Direction {
	North = 0,
	NorthEast = 1,
	East = 2,
	SouthEast = 3,
	South = 4,
	SouthWest = 5,
	West = 6,
	NorthWest = 7,
	DirNone = 100 // used by BosonPath
};

enum CellType {
	CellPlain = 2,
	CellSmall = 3,
	CellBig1 = 4,
	CellBig2 = 5
};

enum CursorType {
	CursorAttack = 0,
	CursorMove = 1,
	CursorDefault = 2
};


/**
 * Describes unit's action such as attack, move or stop
 **/
enum UnitAction {
	ActionInvalid = 0,

	ActionUnitStart = 1, // Start of unit actions
	ActionMove = 2,
	ActionStop = 3,
	ActionFollow = 4,
	ActionHarvest = 5,
	ActionRepair = 6,
	ActionAttack = 7,
	ActionUnitEnd = 29, // End of unit actions

	ActionWeaponStart = 30, // Start of weapon actions
	ActionAttackGround = 31,
	ActionLayMine = 32,
	ActionDropBomb = 33,
	ActionWeaponEnd = 49, // End of weapon actions

	ActionProduceStart = 50, // Start of produce actions
	ActionProduceUnit = 51,
	ActionProduceTech = 52,
	ActionStopProduceUnit = 53,
	ActionStopProduceTech = 54,
	ActionProduceEnd = 69, // End of produce actions

	ActionChangeHeight = 100, // change height of a corner of a cell (editor only)
	ActionPlacementPreview = 101
};


enum UnitSoundEvent {
	SoundOrderMove = 0,
	SoundOrderAttack = 1,
	SoundOrderSelect = 2,
	SoundReportProduced = 3,
	SoundReportDestroyed = 4,
	SoundReportUnderAttack = 5
};

enum WeaponSoundEvent {
	SoundWeaponShoot = 0,
	SoundWeaponHit = 1,
	SoundWeaponFly = 2
};

enum SoundEvent {
	SoundReportMinimapActivated = 0,
	SoundReportMinimapDeactivated = 1
};

enum ProductionType {
	ProduceNothing = 0,
	ProduceUnit = 1,
	ProduceTech = 2
};

enum CameraAction {
	CameraMove = 0,
	CameraZoom = 1,
	CameraRotate = 2
};


enum UnitAnimationModes {
	// the numbers are just cosmetic and do not need to be in order.
	// the idle mode is an exception - it MUST be 0.
	// AB: TODO: add BosonItem::disableAnimation for the
	// construction animations! they are done differently
	UnitAnimationIdle = 0, // default mode
	UnitAnimationMove = 1,
	UnitAnimationMine = 2,
	UnitAnimationRefine = 3,
	UnitAnimationConstruction = 4,
	UnitAnimationWreckage = 100
};

enum RenderFlags {
	Default = 0,
	DepthOnly = 256
};

enum ScrollDirection {
	ScrollUp = 0,
	ScrollRight = 1,
	ScrollDown = 2,
	ScrollLeft = 3
};

// these are added to QEvent::User
enum BoQtEvent {
	QtEventAdvanceCall = 1,
	QtEventAdvanceMessageCompleted = 2
};

#endif

