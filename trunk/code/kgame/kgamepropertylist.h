/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KGAMEPROPERTYLIST_H_
#define __KGAMEPROPERTYLIST_H_

#include <qvaluelist.h>

#include <bodebug.h>

#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"

// AB: things NOT supported of KGamePropertyBase:
//     1. policies. an instance of this class is always PolicyClean (recommended
//        anyway!)
//     2. emitting signals. objects of this class will never emit signals.
//        if you call setEmittingSignal(true) on this, you get undefined
//        behaviour.
template<class type>
class KGamePropertyList : public QValueList<type>, public KGamePropertyBase
{
public:
    KGamePropertyList() :QValueList<type>(), KGamePropertyBase()
    {
      setEmittingSignal(false);
    }

    KGamePropertyList( const KGamePropertyList<type> &a ) : QValueList<type>(a)
    {
      setEmittingSignal(false);
    }
    ~KGamePropertyList()
    {
      if(id() == -1)
      {
        // AB: very bad: it can be used just fine, but saving/loading won't work
        //     -> bug is not always noticed.
        boError() << k_funcinfo << "KGamePropertyList has never been registered! probably a bug!" << endl;
      }
    }

    void load(QDataStream& s)
    {
      QValueList<type>::clear();
      Q_UINT32 size;
      type data;
      s >> size;

      for (Q_UINT32 i=0; i < size; i++)
      {
        s >> data;
        QValueList<type>::append(data);
      }
    }

    void save(QDataStream &s)
    {
      type data;
      Q_UINT32 size = QValueList<type>::count();
      s << size;
      QValueListIterator<type> it;
      for(it = QValueList<type>::begin(); it != QValueList<type>::end(); ++it)
      {
        data = *it;
        s << data;
      }
    }

};

#endif

/*
 * vim: et sw=4
 */
