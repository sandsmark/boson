/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bo.h"

#include <kconfig.h>
#include <kdebug.h>

#include <qstring.h>

#include "speciestheme.h"
#include "bosonparticlemanager.h"
#include "bo3dtools.h"

QValueList<unsigned long int> Bo::readUnsignedLongNumList(KConfig* cfg, QString key)
{
  QValueList<unsigned long int> list;
  QValueList<int> tmplist = cfg->readIntListEntry(key);
  QValueList<int>::Iterator it;
  for(it = tmplist.begin(); it != tmplist.end(); it++)
  {
    list.append((unsigned long int)(*it));
  }
  return list;
}

QValueList<float> Bo::readFloatNumList(KConfig* cfg, QString key)
{
  QStringList strlist = cfg->readListEntry(key);
  QValueList<float> list;
  for(QStringList::Iterator it = strlist.begin(); it != strlist.end(); it++)
  {
    list.append((*it).toFloat());
  }
  return list;
}

BoVector3 Bo::readBoVector3(KConfig* cfg, QString key)
{
  QValueList<float> list = Bo::readFloatNumList(cfg, key);
  if(list.count() == 0)
  {
    // Probably value wasn't specified. Default to 0;0;0
    return BoVector3();
  }
  else if(list.count() != 3)
  {
    kdError() << k_funcinfo << "BoVector3 entry must have 3 floats, not " << list.count() << endl;
    return BoVector3();
  }
  return BoVector3(list[0], list[1], list[2]);
}

BoVector4 Bo::readBoVector4(KConfig* cfg, QString key)
{
  QValueList<float> list = Bo::readFloatNumList(cfg, key);
  if(list.count() == 0)
  {
    // Probably value wasn't specified. Default to 0;0;0;0
    return BoVector4();
  }
  else if(list.count() != 4)
  {
    kdError() << k_funcinfo << "BoVector4 entry must have 4 floats, not " << list.count() << endl;
    return BoVector4();
  }
  return BoVector4(list[0], list[1], list[2], list[3]);
}

QPtrList<BosonParticleSystemProperties> Bo::loadParticleSystemProperties(KConfig* cfg, QString key, SpeciesTheme* theme)
{
  QPtrList<BosonParticleSystemProperties> props;
  if(!cfg->hasKey(key))
  {
    return props;
  }
  QValueList<unsigned long int> list = Bo::readUnsignedLongNumList(cfg, key);
  QValueList<unsigned long int>::Iterator it;
  for(it = list.begin(); it != list.end(); it++)
  {
    props.append(theme->particleSystemProperties(*it));
  }
  return props;
}
