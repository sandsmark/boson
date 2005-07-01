/*
    This file is part of the Boson game
    Copyright (C) 2003 Rivo Laks (rivolaks@hot.ee)

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
template<class T> class QPtrList;

class PlayerIO;
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
    BoSpecificAction(const BoAction* action);
    BoSpecificAction(const BoSpecificAction&);
    BoSpecificAction();
    ~BoSpecificAction();

    // These are meant to be used by commandframe only
    long unsigned int productionId() const  { return mProductionId; }
    void setProductionId(long unsigned int id)  { mProductionId = id; }
    PlayerIO* productionOwner() const  { return mProductionOwner; }
    void setProductionOwner(PlayerIO* owner)  { mProductionOwner = owner; }

    Unit* unit() const  { return mUnit; }
    // Production owner is also set to u->owner()
    void setUnit(Unit* u);

    /**
     * Set the list of units the action applies to. @p leader is the leader of
     * the selection (for example the first unit in the list).
     *
     * The leader of the selection is used for actions that don't make sense on
     * multiple units (e.g. when producing something). It is equivalent to
     * calling @ref setUnit(leader) after setAllUnits.
     **/
    void setAllUnits(const QPtrList<Unit>& units, Unit* leader = 0);
    const QPtrList<Unit>& allUnits() const;

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

    const BoAction* action() const  { return mAction; }
    const QString& id() const
    {
      if (!mAction)
      {
        return QString::null;
      }
      return mAction->id();
    }
    int hotkey() const
    {
      if (!mAction)
      {
        return 0;
      }
      return mAction->hotkey();
    }
    QPixmap* pixmap() const
    {
      if (!mAction)
      {
        return 0;
      }
      return mAction->pixmap();
    }
    const QString& text() const
    {
      if (!mAction)
      {
        return QString::null;
      }
      return mAction->text();
    }


  private:
    const BoAction* mAction;
    long unsigned int mProductionId;
    UnitAction mType;
    Unit* mUnit;
    QPtrList<Unit>* mAllUnits;
    PlayerIO* mProductionOwner;
    BosonWeaponProperties* mWeapon;
    bool mOk;
};

#endif // BOACTION_H

/*
 * vim: et sw=2
 */
