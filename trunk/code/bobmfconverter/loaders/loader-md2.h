/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef LOADERMD2_H
#define LOADERMD2_H


#include "loader.h"
#include "bo3dtools.h"

class Model;
class Mesh;
class Face;


/**
 * MD2 is a model file format pretty popular for games. It has nothing to do
 * with the MD4 algorithm.
 *
 * It is used by Quake2.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class LoaderMD2 : public Loader
{
public:
	LoaderMD2(Model* m, LOD* l, const QString& file);
	virtual ~LoaderMD2();

	virtual bool load();


protected:

};

#endif
