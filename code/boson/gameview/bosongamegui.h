/*
    This file is part of the Boson game
    Copyright (C) 2004-2008 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONGAMEGUI_H
#define BOSONGAMEGUI_H

#include <bogl.h>
#include <QWidget>

class BosonMiniMap;
class BoSelection;
class PlayerIO;
class Player;
class BosonCanvas;
class BoGameCamera;
class BoMatrix;
class BoFrustum;
class BosonGroundTheme;
class BosonGameFPSCounter;
class BoDebugMessage;
class Boson;
class bofixed;
template<class T> class BoRect2;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoVector4;
typedef BoRect2<bofixed> BoRect2Fixed;
typedef BoRect2<float> BoRect2Float;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector2<float> BoVector2Float;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;
typedef BoVector4<bofixed> BoVector4Fixed;
typedef BoVector4<float> BoVector4Float;

class BosonGameGUIPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGameGUI : public QWidget
{
	Q_OBJECT
public:
	BosonGameGUI(const BoMatrix& modelview, const BoMatrix& projection, const BoFrustum& viewFrustum, const GLint* viewport, QWidget* parent);
	virtual ~BosonGameGUI();

	void bosonObjectCreated(Boson* boson);
	void bosonObjectAboutToBeDestroyed(Boson* boson);

	void setCursorWidgetPos(const QPoint* pos);
	void setCursorRootPos(const QPoint* pos);
	void setCursorCanvasVector(const BoVector3Fixed* v);
	void setSelection(BoSelection* s);
	void setCanvas(const BosonCanvas* c);
	void setCamera(BoGameCamera* c);
	void setGameFPSCounter(BosonGameFPSCounter* counter);

	void setGroundTheme(BosonGroundTheme*);
	void updateLabels();
	void addChatMessage(const QString& message);

	void setGameMode(bool mode);
	bool isChatVisible() const;

	void setLocalPlayerIO(PlayerIO*);
	PlayerIO* localPlayerIO() const;

	BosonMiniMap* miniMapWidget() const;

public slots:
	void slotShowPlaceFacilities(PlayerIO*);
	void slotShowPlaceMobiles(PlayerIO*);
	void slotShowPlaceGround();

protected slots:
	void slotBoDebugOutput(const BoDebugMessage&);
	void slotBoDebugWarning(const BoDebugMessage&);
	void slotBoDebugError(const BoDebugMessage&);

signals:
	void signalSelectionChanged(BoSelection*);
	void signalPlaceGround(unsigned int, unsigned char*);
	void signalPlaceUnit(unsigned int, Player*);

protected:
	/**
	 * @return @ref BosonGameView::selection
	 **/
	BoSelection* selection() const;
	const BosonCanvas* canvas() const;
	const QPoint& cursorWidgetPos() const;
	const BoVector3Fixed& cursorCanvasVector() const;
	BoGameCamera* camera() const;


protected:
	void updateLabelPathFinderDebug();
	void updateLabelMatricesDebug();
	void updateLabelItemWorkStatistics();
	void updateLabelOpenGLCamera();
	void updateLabelRenderCounts();
	void updateLabelAdvanceCalls();
	void updateLabelTextureMemory();
	void updateLabelMemoryUsage();
	void updateLabelCPUUsage();

private:
	void initWidgets();

private:
	BosonGameGUIPrivate* d;
};


#endif

