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
#include "bosongroundtheme.h"

#include "bosontexturearray.h"
#include "bosonmap.h"
#include "bodebug.h"
#include "defines.h"

#include <qimage.h>

BosonGroundTheme::BosonGroundTheme()
{
 mTextures = 0;
}

BosonGroundTheme::~BosonGroundTheme()
{
 delete mTextures;
}

bool BosonGroundTheme::loadGroundTheme(BosonMap* map, QString dir)
{
 if (!map) {
	BO_NULL_ERROR(map);
	return false;
 }
 if (map->textureCount() == 0) {
	boWarning() << k_funcinfo << "no textures to load in map" << endl;
	return true;
 }
 // dir is e.g. /opt/kde3/share/apps/boson/themes/grounds/earth/ -> "earth" is
 // the important part!
 if (dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }
 if (mTilesDir == dir) {
	boDebug() << k_funcinfo << "already loaded from " << dir << ". skipping..." << endl;
	// we have already loaded this. no need to do it again.
	return true;
 }
 if (mTextures) {
	boWarning() << k_funcinfo << "already loaded ?!" << endl;
 }
 delete mTextures;
 mTextures = 0;
 mTilesDir = dir;

 QValueList<QImage> images;
 for (unsigned int i = 0; i < map->textureCount(); i++) {
	QImage image = loadTextureImage(dir, map->groundType(i), map->amountOfLand(i), map->amountOfWater(i));
	if (image.isNull()) {
		boError() << k_funcinfo << "Null image" << endl;
		return false;
	}
	images.append(image);
 }

 mTextures = new BosonTextureArray(images);

 return true;
}

QImage BosonGroundTheme::loadTextureImage(const QString& dir, int groundType, unsigned char amountOfLand, unsigned char amountOfWater)
{
 QString file = dir + groundType2Name(groundType) + QString("-00.png");
 QImage image(file);
 if (image.isNull()) {
	boWarning() << k_funcinfo << "Could not find texture image for " << groundType << ". expected file: " << file << endl;


	// if the groundType cannot be found we should try to find a close
	// replacement using amountOfLand/Water.
	Q_UNUSED(amountOfLand);
	Q_UNUSED(amountOfWater);

	// load dummy image.
	image = QImage(64, 64, 32);
	image.fill(Qt::green.rgb());
 }
 return image;
}

QString BosonGroundTheme::groundType2Name(int groundType)
{
#warning TODO: add an enum
 switch (groundType) {
	case 0: // GroundGrass
		return QString::fromLatin1("grass");
	case 1: // GroundDesert
		return QString::fromLatin1("desert");
	case 2: // GroundWater
		return QString::fromLatin1("water");
	default:
		boError() << "Invalid GroundType " << groundType << endl;
		break;
 }
 return QString::null;
}

