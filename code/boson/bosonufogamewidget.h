/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONUFOGAMEWIDGET_H
#define BOSONUFOGAMEWIDGET_H

#include "boufo/boufo.h"
#include <GL/gl.h>

class BosonGLMiniMap;
class BoSelection;
class PlayerIO;
class Player;
class BosonBigDisplayBase;
class BosonCanvas;
class BoGameCamera;
class BoMatrix;
class BosonGroundTheme;
class bofixed;
template<class T> class BoRect;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoVector4;
typedef BoRect<bofixed> BoRectFixed;
typedef BoRect<float> BoRectFloat;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector2<float> BoVector2Float;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;
typedef BoVector4<bofixed> BoVector4Fixed;
typedef BoVector4<float> BoVector4Float;

class BosonUfoGameWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoGameWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BosonUfoGameWidget(const BoMatrix& modelview, const BoMatrix& projection, const GLfloat* viewFrustum, const GLint* viewport, BosonBigDisplayBase* display);
	virtual ~BosonUfoGameWidget();

	void setGLMiniMap(BosonGLMiniMap* m);
	void setGroundTheme(BosonGroundTheme*);
	void updateUfoLabels();
	void addChatMessage(const QString& message);

	void setGameMode(bool mode);
	bool isChatVisible() const;

	void setLocalPlayerIO(PlayerIO*);
	PlayerIO* localPlayerIO() const;

public slots:
	void slotShowPlaceFacilities(PlayerIO*);
	void slotShowPlaceMobiles(PlayerIO*);
	void slotShowPlaceGround();

signals:
	void signalPlaceGround(unsigned int, unsigned char*);
	void signalPlaceUnit(unsigned int, Player*);

protected:
	BosonBigDisplayBase* display() const;

	/**
	 * @return @ref BosonBigDisplayBase::selection
	 **/
	BoSelection* selection() const;
	const BosonCanvas* canvas() const;
	const BoVector3Fixed& cursorCanvasVector() const;
	BoGameCamera* camera() const;


protected:
	void updateUfoLabelPathFinderDebug();
	void updateUfoLabelMatricesDebug();
	void updateUfoLabelItemWorkStatistics();
	void updateUfoLabelOpenGLCamera();
	void updateUfoLabelRenderCounts();
	void updateUfoLabelAdvanceCalls();
	void updateUfoLabelTextureMemory();

private:
	void initUfoWidgets();

private:
	BosonUfoGameWidgetPrivate* d;
};


#endif

