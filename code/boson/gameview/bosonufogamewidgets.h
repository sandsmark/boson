/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONUFOGAMEWIDGETS_H
#define BOSONUFOGAMEWIDGETS_H

#include "../boufo/boufo.h"
#include "../bo3dtools.h"

class BosonCanvas;
class PlayerIO;
class UnitProperties;
class BosonCursor;
class BoGameCamera;
class BosonGameFPSCounter;

class BosonUfoCanvasWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoCanvasWidget : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoCanvasWidget();
	virtual ~BosonUfoCanvasWidget();

	void setGameGLMatrices(const BoGLMatrices*);
	void setCamera(BoGameCamera* c);
	void setLocalPlayerIO(PlayerIO* io);
	void setCanvas(const BosonCanvas* canvas);

	void setParticlesDirty(bool);

	virtual void paintWidget();

	void quitGame();

private:
	BosonUfoCanvasWidgetPrivate* d;
};

class BosonUfoPlacementPreviewWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoPlacementPreviewWidget : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoPlacementPreviewWidget();
	virtual ~BosonUfoPlacementPreviewWidget();

	void setGameGLMatrices(const BoGLMatrices*);
	void setCanvas(BosonCanvas* canvas);
	const BosonCanvas* canvas() const;
	void setCursorCanvasVector(const BoVector3Fixed& cursorCanvasVector);
	const BoVector3Fixed& cursorCanvasVector() const;
	void setLocalPlayerIO(PlayerIO* io);
	PlayerIO* localPlayerIO() const;

	virtual void paintWidget();

	void quitGame();

	/**
	 * @param prop The unit that should get placed or NULL if none.
	 * @param canPlace Whether @p prop can be placed at the current cursor
	 * position (current == the moment when @ref BosonGameViewInputBase::updatePlacementPreviewData
	 * has been called)
	 **/
	void setPlacementPreviewData(const UnitProperties* prop, bool canPlace, bool freeMode, bool useCollisionDetection);

	/**
	 * Same as above - but this will make a cell placement preview, instead
	 * of a unit placement preview.
	 **/
	void setPlacementCellPreviewData(unsigned int textureCount, unsigned char* alpha, bool canPlace);

public slots:
	void slotSetPlacementPreviewData(const UnitProperties* prop, bool canPlace, bool freeMode = false, bool useCollisionDetection = true);
	void slotSetPlacementCellPreviewData(unsigned int textureCount, unsigned char* alpha, bool canPlace);

	void slotLockAction(bool locked, int actionType);

protected:
	void renderPlacementPreview();

private:
	BosonUfoPlacementPreviewWidgetPrivate* d;
};


class BoLineVisualization
{
public:
	BoLineVisualization()
	{
		color.set(1.0f, 1.0f, 1.0f, 1.0f);
		timeout = 60;
		pointsize = 1.0f;
	}

	QValueList<BoVector3Fixed> points;
	BoVector4Float color;
	int timeout;
	float pointsize;
};

class BosonUfoLineVisualizationWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoLineVisualizationWidget : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoLineVisualizationWidget();
	virtual ~BosonUfoLineVisualizationWidget();

	void setGameGLMatrices(const BoGLMatrices*);
	void setCanvas(const BosonCanvas* mCanvas);
	const BosonCanvas* canvas() const;

	virtual void paintWidget();

public slots:
	void slotAdvance(unsigned int advanceCallsCount, bool advanceFlag);

protected slots:
	void slotAddLineVisualization(const QValueList<BoVector3Fixed>& points, const BoVector4Float& color, bofixed pointSize, int timeout, bofixed zOffset);

protected:
	void advanceLineVisualization();
	void addLineVisualization(BoLineVisualization v);

private:
	BosonUfoLineVisualizationWidgetPrivate* d;
};



class BosonUfoCursorWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoCursorWidget : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoCursorWidget();
	virtual ~BosonUfoCursorWidget();

	virtual void paintWidget();

	void setGameGLMatrices(const BoGLMatrices*);
	void setCursorWidgetPos(const QPoint* pos);
	BosonCursor* cursor() const;
	void setCanvas(BosonCanvas* mCanvas);
	BosonCanvas* canvas() const;

public slots:
	void slotChangeCursor(int mode, const QString& cursorDir);

signals:
	/**
	 * See @ref BosonCursorCollection::signalSetWidgetCursor
	 **/
	void signalSetWidgetCursor(BosonCursor* c);

private:
	BosonUfoCursorWidgetPrivate* d;
};



class BosonUfoSelectionRectWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoSelectionRectWidget : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoSelectionRectWidget();
	virtual ~BosonUfoSelectionRectWidget();

	virtual void paintWidget();

	void setGameGLMatrices(const BoGLMatrices*);
	void setCanvas(BosonCanvas* mCanvas);
	BosonCanvas* canvas() const;

public slots:
	void slotSelectionRectVisible(bool visible);
	void slotSelectionRectChanged(const QRect& rect);


private:
	BosonUfoSelectionRectWidgetPrivate* d;
};


class FPSGraphData;
class BosonUfoFPSGraphWidgetPrivate;
/**
 * @short A widget to paint a graph from FPS (or other) data
 * This widget paints a graph (i.e. a set of connected lines) from FPS data.
 * Although it is named like this and primarily intended to be used for
 * frame-per-second data, it can also handle any other data (I just dont know a
 * more generic but still descriptive name for this class).
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoFPSGraphWidget : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoFPSGraphWidget();
	~BosonUfoFPSGraphWidget();

	virtual void paintWidget();

	void setGameGLMatrices(const BoGLMatrices*);
	void setGameFPSCounter(const BosonGameFPSCounter* c);

protected:
	void paintFPS(const FPSGraphData& data);

protected slots:
	void slotAddData();

private:
	BosonUfoFPSGraphWidgetPrivate* d;
};


class BosonUfoProfilingGraphWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoProfilingGraphWidget : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoProfilingGraphWidget();
	~BosonUfoProfilingGraphWidget();

	virtual void paintWidget();

	void setGameGLMatrices(const BoGLMatrices*);

protected slots:
	void slotUpdateData();
	void slotSetUpdateInterval(int);

protected:
	void resetProfilingTypes();
	void ensureLabels(unsigned int count);

private:
	BosonUfoProfilingGraphWidgetPrivate* d;
};


#endif

