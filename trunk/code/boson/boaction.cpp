/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boaction.h"

#include "unit.h"
#include "speciesdata.h"
#include "bodebug.h"

#include <ksimpleconfig.h>
#include <qstring.h>
#include <qptrlist.h>


/***  BoAction  ***/

BoAction::BoAction(KSimpleConfig* cfg, const QString& name, SpeciesData* theme)
{
  cfg->setGroup(name);
  mId = name;  // is that needed?
  mPixmap = theme->pixmap(cfg->readEntry("Pixmap", ""));
  if(!mPixmap)
  {
    boWarning() << k_funcinfo << "NULL pixmap" << endl;
  }
  mText = cfg->readEntry("Text", "");
  // TODO: load shortcut aka hotkey
}

BoAction::BoAction(const QString& name, QPixmap* pixmap, const QString& text/*, hotkey*/)
{
  mId = name;
  mPixmap = pixmap;
  mText = text;
}



/*** BoSpecificAction  ***/

BoSpecificAction::BoSpecificAction(const BoAction* action)
{
  mAllUnits = new QPtrList<Unit>();
  reset();
  mAction = action;
  mOk = true;
}

BoSpecificAction::BoSpecificAction(const BoSpecificAction& action)
{
  mAllUnits = new QPtrList<Unit>();
  reset();
  *this = action;
}

BoSpecificAction::BoSpecificAction()
{
  mAllUnits = new QPtrList<Unit>();
  reset();
}

BoSpecificAction::~BoSpecificAction()
{
  delete mAllUnits;
}

void BoSpecificAction::reset()
{
  mAction = 0;
  mProductionId = 0;
  mProductionOwner = 0;
  mType = ActionInvalid;
  mUnit = 0;
  mAllUnits->clear();
  mWeapon = 0;
  mOk = false;
}

ProductionType BoSpecificAction::productionType() const
{
  if(mType == ActionProduceUnit || mType == ActionStopProduceUnit)
  {
    return ProduceUnit;
  }
  else if(mType == ActionProduceTech || mType == ActionStopProduceTech)
  {
    return ProduceTech;
  }
  else
  {
    return ProduceNothing;
  }
}

void BoSpecificAction::setUnit(Unit* u)
{
  mUnit = u;
  if(mUnit)
  {
    mProductionOwner = mUnit->ownerIO();
  }
  // Don't reset owner if unit is 0 (needed for editor)
}

void BoSpecificAction::setAllUnits(const QPtrList<Unit>& units, Unit* leader)
{
  *mAllUnits = units;
  setUnit(leader);
}

const QPtrList<Unit>& BoSpecificAction::allUnits() const
{
  return *mAllUnits;
}

void BoSpecificAction::operator=(const BoSpecificAction& a)
{
  mAction = a.mAction;
  mProductionId = a.mProductionId;
  mProductionOwner = a.mProductionOwner;
  mType = a.mType;
  mUnit = a.mUnit;
  *mAllUnits = *a.mAllUnits;
  mWeapon = a.mWeapon;
  mOk = a.mOk;
}

/*
 * vim: et sw=2
 */
