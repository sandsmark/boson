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

#ifndef BO_H
#define BO_H

#include <qvaluelist.h>
#include <qptrlist.h>

class KConfig;
class QString;
class SpeciesTheme;
class BosonParticleSystemProperties;
class BoVector3;
class BoVector4;

/**
 * @short Namespace for little helper functions used in many places
 * If some function or piece of code is used in many places, add it here
 **/
namespace Bo
{
  /**
   * Loads list of unsinged long int's from KConfig (which only supports loading
   * list of _int's_)
   **/
  QValueList<unsigned long int> readUnsignedLongNumList(KConfig* cfg, QString key);
  
  /**
   * Loads list of float's from KConfig
   **/
  QValueList<float> readFloatNumList(KConfig* cfg, QString key);

  /**
   * Loads BoVector3 from KConfig
   **/
  BoVector3 readBoVector3(KConfig* cfg, QString key);

  /**
   * Loads BoVector4 from KConfig
   **/
  BoVector4 readBoVector4(KConfig* cfg, QString key);

  QPtrList<BosonParticleSystemProperties> loadParticleSystemProperties(KConfig* cfg, QString key, SpeciesTheme* theme);
}

#endif // BO_H
