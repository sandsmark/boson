/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "bosoncomputerio.h"

#include "player.h"

#include <kdebug.h>

#include "bosoncomputerio.moc"

BosonComputerIO::BosonComputerIO() : KGameComputerIO()
{
}

BosonComputerIO::BosonComputerIO(KPlayer* p) : KGameComputerIO(p)
{
}

BosonComputerIO::~BosonComputerIO()
{
}

void BosonComputerIO::reaction()
{
// kdDebug() << k_funcinfo << endl;

}

