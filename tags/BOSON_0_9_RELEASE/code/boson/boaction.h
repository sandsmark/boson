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

#ifndef BOACTION_H
#define BOACTION_H

#include "global.h"

#include <qstring.h>

class QPixmap;
class KSimpleConfig;

class Player;
class Unit;
class SpeciesData;
class BosonWeaponProperties;



/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoAction
{
  public:
    BoAction(KSimpleConfig* cfg, const QString& name, SpeciesData* theme);
    BoAction(const QString& name, QPixmap* pixmap, const QString& text/*, hotkey*/);

    const QString& id() const { return mId; }
    int hotkey() const { return mHotkey; }
    QPixmap* pixmap() const { return mPixmap; }
    const QString& text() const { return mText; }

  private:
    QString mId;  // Id aka name
    QPixmap* mPixmap;
    int mHotkey;
    QString mText;  // Full text to show in tooltip
};

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoSpecificAction
{
  public:
    BoSpecificAction(BoAction* action);
    BoSpecificAction();

    // These are meant to be used by commandframe only
    long unsigned int productionId() const  { return mProductionId; }
    void setProductionId(long unsigned int id)  { mProductionId = id; }
    Player* productionOwner() const  { return mProductionOwner; }
    void setProductionOwner(Player* owner)  { mProductionOwner = owner; }

    Unit* unit() const  { return mUnit; }
    // Production owner is also set to u->owner()
    void setUnit(Unit* u);

    BosonWeaponProperties* weapon() const  { return mWeapon; }
    void setWeapon(BosonWeaponProperties* w)  { mWeapon = w; }

    UnitAction type() const  { return mType; }
    void setType(UnitAction type)  { mType = type; }

    ProductionType productionType() const;

    bool isUnitAction() const
    {
      return mType >= ActionUnitStart && mType <= ActionUnitEnd;
    }
    bool isWeaponAction() const
    {
      return mType >= ActionWeaponStart && mType <= ActionWeaponEnd;
    }
    bool isProduceAction() const
    {
      return mType >= ActionProduceStart && mType <= ActionProduceEnd;
    }

    bool ok() const  { return mOk; }
    void reset();
    void operator=(const BoSpecificAction& a);

    BoAction* action() const  { return mAction; }
    const QString& id() const { return mAction->id(); }
    int hotkey() const { return mAction->hotkey(); }
    QPixmap* pixmap() const { return mAction->pixmap(); }
    const QString& text() const { return mAction->text(); }


  private:
    BoAction* mAction;
    long unsigned int mProductionId;
    UnitAction mType;
    Unit* mUnit;
    Player* mProductionOwner;
    BosonWeaponProperties* mWeapon;
    bool mOk;
};

#endif // BOACTION_H

/*
 * vim: et sw=2
 */
