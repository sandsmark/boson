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

#include "editorrandommapwidget.h"
#include "editorrandommapwidget.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../../no_player.h"
#include "../../bosoncanvas.h"
#include "../../bosonmap.h"
#include "../../playerio.h"
#include "../bosonlocalplayerinput.h"
#include <bodebug.h>

#include <klocale.h>
#include <krandomsequence.h>
#include <kapplication.h>

#include <qtimer.h>
#include <qvaluelist.h>
#include <qpoint.h>

#include <math.h>

// returns a queue with _ALL_ cells of the map into retQueue, using a BFS.
static void cornersBFS(const MyMap& map, QValueList<QPoint>* retQueue);

class HCorner {
public:
	HCorner()
	{
		h = 0.0f;
		willStartMountain = false;
		dir = 0;
	}
	HCorner& operator=(const HCorner& c)
	{
		h = c.h;
		willStartMountain = c.willStartMountain;
		dir = c.dir;

		return *this;
	}
	float h;
	bool willStartMountain;

	int dir;
};
class MyMap {
public:
	MyMap(int cornerWidth, int cornerHeight)
	{
		mCornerWidth = cornerWidth;
		mCornerHeight = cornerHeight;
		mCorners = new HCorner[(mCornerWidth + 1) * (mCornerHeight + 1)];
	}
	~MyMap()
	{
		delete[] mCorners;
	}

	void copyFrom(const MyMap& map)
	{
		if (map.cornerWidth() < cornerWidth()) {
			boError() << k_funcinfo << "cannot copy" << endl;
			return;
		}
		if (map.cornerHeight() < cornerHeight()) {
			boError() << k_funcinfo << "cannot copy" << endl;
			return;
		}
		for (int x = 0; x < cornerWidth(); x++) {
			for (int y = 0; y < cornerHeight(); y++) {
				int index1 = cornerArrayPos(x, y);
				int index2 = map.cornerArrayPos(x, y);
				mCorners[index1] = map.mCorners[index2];
			}
		}
	}

	int cornerWidth() const
	{
		return mCornerWidth;
	}
	int cornerHeight() const
	{
		return mCornerHeight;
	}
	void loadHeightsFromRealMap(const BosonMap* map)
	{
		for (int x = 0; x < cornerWidth(); x++) {
			for (int y = 0; y < cornerHeight(); y++) {
				setHeightAtCorner(x, y, map->heightAtCorner(x, y));
			}
		}
	}
	void resetHeights()
	{
		for (int x = 0; x < cornerWidth(); x++) {
			for (int y = 0; y < cornerHeight(); y++) {
				setHeightAtCorner(x, y, 0.0f);
			}
		}
	}

	void setHeightAtCorner(int x, int y, float h)
	{
		if (x < 0 || x >= cornerWidth()) {
			boError() << k_funcinfo << "invalid x: " << x << endl;
			return;
		}
		if (y < 0 || y >= cornerHeight()) {
			boError() << k_funcinfo << "invalid y: " << y << endl;
			return;
		}
		int index = cornerArrayPos(x, y);
		mCorners[index].h = h;
	}
	float heightAtCorner(int x, int y) const
	{
		if (x < 0 || x >= cornerWidth()) {
			boError() << k_funcinfo << "invalid x: " << x << endl;
			return 0.0f;
		}
		if (y < 0 || y >= cornerHeight()) {
			boError() << k_funcinfo << "invalid y: " << y << endl;
			return 0.0f;
		}
		int index = cornerArrayPos(x, y);
		return mCorners[index].h;
	}
	void setHeightChangeDirectionAtCorner(int x, int y, int dir)
	{
		if (x < 0 || x >= cornerWidth()) {
			boError() << k_funcinfo << "invalid x: " << x << endl;
			return;
		}
		if (y < 0 || y >= cornerHeight()) {
			boError() << k_funcinfo << "invalid y: " << y << endl;
			return;
		}
		int index = cornerArrayPos(x, y);
		mCorners[index].dir = dir;
	}
	int heightChangeDirectionAtCorner(int x, int y) const
	{
		if (x < 0 || x >= cornerWidth()) {
			boError() << k_funcinfo << "invalid x: " << x << endl;
			return 0;
		}
		if (y < 0 || y >= cornerHeight()) {
			boError() << k_funcinfo << "invalid y: " << y << endl;
			return 0;
		}
		int index = cornerArrayPos(x, y);
		return mCorners[index].dir;
	}

	void setStartMountainAtCorner(int x, int y, bool s)
	{
		int index = cornerArrayPos(x, y);
		mCorners[index].willStartMountain = s;
	}
	bool startMountainAtCorner(int x, int y) const
	{
		int index = cornerArrayPos(x, y);
		return mCorners[index].willStartMountain;
	}
	inline int cornerArrayPos(int x, int y) const
	{
		return BoMapCornerArray::arrayPos(x, y, cornerWidth());
	}
private:
	HCorner* mCorners;
	int mCornerWidth;
	int mCornerHeight;
};

/**
 * Implementation of the diamond square algorithm
 **/
class DiamondSquare
{
public:
	DiamondSquare()
	{
		mMap = 0;
		mOrigDHeight = 0.0f;
		mR = 1.0f;
		mPow2_R = 0.5f;

		setR(1.0f);
		setDHeight(0.0f);

		// AB heights are possible in [-13.125;18.75]
		// so it makes sense to pick a dHeight somewhere close to that.
		// (note that at most 0.5*dHeight is added to a height value in
		// one iteration)
		// setDHeight(w * h);
		// setDHeight(w);
		setDHeight(30.0f);
	}
	~DiamondSquare()
	{
		delete mMap;
	}

	void diamondSquare(MyMap& map);

	void setDHeight(float d)
	{
		mOrigDHeight = d;
	}
	void setR(float r)
	{
		mR = r;
		mPow2_R = powf(2.0f, -mR);
	}

protected:
#if 0
	bool diamondStep(int x1, int x2, int y1, int y2, float dHeight);
	bool squareStep(int x1, int x2, int y1, int y2, float dHeight);
	void squareStepCorner(int x, int y, float dHeight);
#endif


	// diamond/square step at a specific corner.
	// lod := (current rectangle width) / 2
	void diamondStepCorner(int x, int y, int lod, float dHeight);
	void squareStepCorner(int x, int y, int lod, float dHeight);

private:
	MyMap* mMap;
	float mOrigDHeight;
	float mR;
	float mPow2_R;
	KRandomSequence mRandom;
};

class EditorRandomMapWidgetPrivate
{
public:
	EditorRandomMapWidgetPrivate()
	{
		mRandom = 0;

		mSelectTerrainCreation = 0;
		mSimpleTerrainCreationButton = 0;
		mDiamondSquareTerrainCreationButton = 0;

		mSimpleTerrainCreation = 0;
		mRandomHeightCount = 0;
		mChangeUpCount = 0;
		mChangeDownCount = 0;
		mChangeBy = 0;
		mHeightProbabilities = 0;

		mDiamondSquareTerrainCreation = 0;
		mDiamondSquareDHeight = 0;
		mDiamondSquareR = 0;

		mRandomMountainCount = 0;
		mMountainProbabilities = 0;
	}
	KRandomSequence* mRandom;

	BoUfoButtonGroupWidget* mSelectTerrainCreation;
	BoUfoRadioButton* mSimpleTerrainCreationButton;
	BoUfoRadioButton* mDiamondSquareTerrainCreationButton;

	BoUfoWidget* mSimpleTerrainCreation;
	BoUfoNumInput* mRandomHeightCount;
	BoUfoNumInput* mChangeUpCount;
	BoUfoNumInput* mChangeDownCount;
	BoUfoNumInput* mChangeBy;
	BoUfoLabel* mHeightProbabilities;

	BoUfoWidget* mDiamondSquareTerrainCreation;
	BoUfoNumInput* mDiamondSquareDHeight;
	BoUfoNumInput* mDiamondSquareR;

	BoUfoNumInput* mRandomMountainCount;
	BoUfoLabel* mMountainProbabilities;
};

EditorRandomMapWidget::EditorRandomMapWidget()
		: BoUfoWidget()
{
 d = new EditorRandomMapWidgetPrivate();
 mCanvas = 0;
 d->mRandom = new KRandomSequence();

 setLayoutClass(UHBoxLayout);

 BoUfoVBox* terrainBox = new BoUfoVBox();
 addWidget(terrainBox);

 BoUfoVBox* mountainBox = new BoUfoVBox();
 addWidget(mountainBox);

 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setStretch(1);
 addWidget(stretch);

 BoUfoLabel* header = new BoUfoLabel(i18n("Random Map generation"));
 terrainBox->addWidget(header);


 initTerrainCreationGUI(terrainBox);
 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 terrainBox->addWidget(stretch);


 initMountainCreationGUI(mountainBox);
 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 mountainBox->addWidget(stretch);
}

EditorRandomMapWidget::~EditorRandomMapWidget()
{
 delete d->mRandom;
 delete d;
}

void EditorRandomMapWidget::initTerrainCreationGUI(BoUfoWidget* parent)
{
 d->mSelectTerrainCreation = new BoUfoButtonGroupWidget();
 parent->addWidget(d->mSelectTerrainCreation);
 d->mSimpleTerrainCreationButton = new BoUfoRadioButton(i18n("Simple terrain algorithm"));
 d->mDiamondSquareTerrainCreationButton = new BoUfoRadioButton(i18n("Diamond square terrain algorithm"));
 d->mDiamondSquareTerrainCreationButton->setSelected(true);
 d->mSelectTerrainCreation->addWidget(d->mSimpleTerrainCreationButton);
 d->mSelectTerrainCreation->addWidget(d->mDiamondSquareTerrainCreationButton);

 d->mSimpleTerrainCreation = new BoUfoWidget();
 parent->addWidget(d->mSimpleTerrainCreation);

 d->mRandomHeightCount = new BoUfoNumInput();
 d->mRandomHeightCount->setLabel(i18n("Random Count (height up/down): "));
 d->mRandomHeightCount->setRange(2.0f, 150.0f);
 d->mSimpleTerrainCreation->addWidget(d->mRandomHeightCount);

 d->mChangeUpCount = new BoUfoNumInput();
 d->mChangeUpCount->setLabel(i18n("Change Up Count: "));
 d->mChangeUpCount->setRange(0.0f, 20.0f);
 d->mSimpleTerrainCreation->addWidget(d->mChangeUpCount);

 d->mChangeDownCount = new BoUfoNumInput();
 d->mChangeDownCount->setLabel(i18n("Change Down Count: "));
 d->mChangeDownCount->setRange(0.0, 20.0);
 d->mSimpleTerrainCreation->addWidget(d->mChangeDownCount);

 d->mChangeBy = new BoUfoNumInput();
 d->mChangeBy->setLabel(i18n("Change By: "));
 d->mChangeBy->setRange(0.0f, 5.0f);
 d->mChangeBy->setStepSize(0.1f);
 d->mSimpleTerrainCreation->addWidget(d->mChangeBy);


 d->mHeightProbabilities = new BoUfoLabel();
 d->mSimpleTerrainCreation->addWidget(d->mHeightProbabilities);

 connect(d->mRandomHeightCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateHeightProbabilityLabels()));
 connect(d->mChangeUpCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateHeightProbabilityLabels()));
 connect(d->mChangeDownCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateHeightProbabilityLabels()));
 connect(d->mChangeBy, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateHeightProbabilityLabels()));

 d->mRandomHeightCount->setValue(70.0f);
 d->mChangeUpCount->setValue(1.0f);
 d->mChangeDownCount->setValue(1.0f);
 d->mChangeBy->setValue(0.5f);



 d->mDiamondSquareTerrainCreation = new BoUfoWidget();
 parent->addWidget(d->mDiamondSquareTerrainCreation);

 d->mDiamondSquareDHeight = new BoUfoNumInput();
 d->mDiamondSquareDHeight->setLabel(i18n("\"dHeight\" in diamond square: "));
 d->mDiamondSquareDHeight->setRange(0.0f, 100.0f);
 d->mDiamondSquareDHeight->setStepSize(0.5f);
 d->mDiamondSquareDHeight->setValue(30.0f);
 d->mDiamondSquareTerrainCreation->addWidget(d->mDiamondSquareDHeight);

 d->mDiamondSquareR = new BoUfoNumInput();
 d->mDiamondSquareR->setLabel(i18n("\"r\" in diamond square: "));
 d->mDiamondSquareR->setRange(0.0f, 10.0f);
 d->mDiamondSquareR->setStepSize(0.1f);
 d->mDiamondSquareR->setValue(1.0f);
 d->mDiamondSquareTerrainCreation->addWidget(d->mDiamondSquareR);

 connect(d->mSelectTerrainCreation, SIGNAL(signalButtonActivated(BoUfoRadioButton*)),
		this, SLOT(slotTerrainCreationChanged(BoUfoRadioButton*)));
 slotTerrainCreationChanged(d->mSelectTerrainCreation->selectedButton());


 BoUfoPushButton* apply = new BoUfoPushButton(i18n("Create Terrain"));
 parent->addWidget(apply);
 connect(apply, SIGNAL(signalClicked()), this, SLOT(slotCreateTerrain()));
}

void EditorRandomMapWidget::initMountainCreationGUI(BoUfoWidget* parent)
{
 BoUfoLabel* label = new BoUfoLabel("Mountains");
 parent->addWidget(label);

 d->mRandomMountainCount = new BoUfoNumInput();
 d->mRandomMountainCount->setLabel(i18n("Random Count (mountain): "));
 d->mRandomMountainCount->setRange(0.0f, 2000.0f);

 parent->addWidget(d->mRandomMountainCount);

 d->mMountainProbabilities = new BoUfoLabel();
 parent->addWidget(d->mMountainProbabilities);

 connect(d->mRandomMountainCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateMountainProbabilityLabels()));

 d->mRandomMountainCount->setValue(1000.0f);


 BoUfoPushButton* apply = new BoUfoPushButton(i18n("Create Mountains"));
 parent->addWidget(apply);
 connect(apply, SIGNAL(signalClicked()), this, SLOT(slotCreateMountains()));
}

void EditorRandomMapWidget::slotCreateTerrain()
{
 BoUfoRadioButton* b = d->mSelectTerrainCreation->selectedButton();
 if (!b) {
	boWarning() << k_funcinfo << "no terrain creation algorithm selected" << endl;
	return;
 }

 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(canvas());
 BosonMap* realMap = canvas()->map();
 BO_CHECK_NULL_RET(realMap);

 BosonLocalPlayerInput* input = 0;
 input = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 BO_CHECK_NULL_RET(input);
 boDebug() << k_funcinfo << endl;

 MyMap map(realMap->width() + 1, realMap->height() + 1);
 map.resetHeights();

 if (b == d->mSimpleTerrainCreation) {
	createHeightsSimple(map);
 } else if (d->mDiamondSquareTerrainCreation) {
	createHeightsDiamondSquare(map);
 } else {
	boError() << k_funcinfo << "unknown button selected" << endl;
	return;
 }

 QValueList< QPair<QPoint, bofixed> > heights;
 for (int x = 0; x < map.cornerWidth(); x++) {
	for (int y = 0; y < map.cornerHeight(); y++) {
		QPoint p(x, y);
		bofixed height = map.heightAtCorner(x, y);

		heights.append(QPair<QPoint, bofixed>(p, height));
	}
 }

 boDebug() << k_funcinfo << "new heights calculated. sending..." << endl;
 input->changeHeight(heights);
 boDebug() << k_funcinfo << "sending completed. new values will be applied soon (asynchronously)." << endl;


 boDebug() << k_funcinfo << "done" << endl;
}

void EditorRandomMapWidget::slotCreateMountains()
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(canvas());
 BosonMap* realMap = canvas()->map();
 BO_CHECK_NULL_RET(realMap);

 BosonLocalPlayerInput* input = 0;
 input = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 BO_CHECK_NULL_RET(input);
 boDebug() << k_funcinfo << endl;

 MyMap map(realMap->width() + 1, realMap->height() + 1);
 map.loadHeightsFromRealMap(realMap);

 createMountains(map);

 QValueList< QPair<QPoint, bofixed> > heights;
 for (int x = 0; x < map.cornerWidth(); x++) {
	for (int y = 0; y < map.cornerHeight(); y++) {
		QPoint p(x, y);
		bofixed height = map.heightAtCorner(x, y);

		heights.append(QPair<QPoint, bofixed>(p, height));
	}
 }

 boDebug() << k_funcinfo << "new heights calculated. sending..." << endl;
 input->changeHeight(heights);
 boDebug() << k_funcinfo << "sending completed. new values will be applied soon (asynchronously)." << endl;


 boDebug() << k_funcinfo << "done" << endl;
}

void EditorRandomMapWidget::createHeightsSimple(MyMap& map)
{
 const int randomHeightCount = lrint(d->mRandomHeightCount->value());
 const int changeUpCount = lrint(d->mChangeUpCount->value());
 const int changeDownCount = lrint(d->mChangeDownCount->value());
 if (changeUpCount + changeDownCount > randomHeightCount) {
	boError() << k_funcinfo << "changeUpCount + changeDownCount must be <= randomHeightCount" << endl;
	return;
 }
 float changeBy = d->mChangeBy->value();

 // do a BFS on all corners
 QValueList<QPoint> queue;
 cornersBFS(map, &queue);
 while (!queue.isEmpty()) {
	QPoint p = queue.front();
	queue.pop_front();

	int changedNeighborsDownCount = 0;
	int changedNeighborsUpCount = 0;

	float hmax = 0.0f;
	float hmin = 0.0f;
	float havg = 0.0f;
	int c = 0;
	float h = 0.0f;
#if 0
	if (p.x() - 1 >= 0 && p.y() - 1 >= 0) {
		h = map.heightAtCorner(p.x() - 1, p.y() - 1);
		havg += h;
		hmax = QMAX(hmax, h);
		hmin = QMIN(hmin, h);
		c++;

		switch (map.heightChangeDirectionAtCorner(p.x() - 1, p.y() - 1)) {
			case 1:
				changedNeighborsUpCount++;
				break;
			case 2:
				changedNeighborsDownCount++;
				break;
			default:
				break;
		}
	}
#endif
	if (p.x() - 1 >= 0) {
		h = map.heightAtCorner(p.x() - 1, p.y());
		havg += h;
		hmax = QMAX(hmax, h);
		hmin = QMIN(hmin, h);
		c++;

		switch (map.heightChangeDirectionAtCorner(p.x() - 1, p.y())) {
			case 1:
				changedNeighborsUpCount++;
				break;
			case 2:
				changedNeighborsDownCount++;
				break;
			default:
				break;
		}
	}
	if (p.y() - 1 >= 0) {
		h = map.heightAtCorner(p.x(), p.y() - 1);
		havg += h;
		hmax = QMAX(hmax, h);
		hmin = QMIN(hmin, h);
		c++;

		switch (map.heightChangeDirectionAtCorner(p.x(), p.y() - 1)) {
			case 1:
				changedNeighborsUpCount++;
				break;
			case 2:
				changedNeighborsDownCount++;
				break;
			default:
				break;
		}
	}
#if 0
	if (p.x() - 1 >= 0 && p.y() + 1 < cornerHeight) {
		h = map.heightAtCorner(p.x() - 1, p.y() + 1);
		havg += h;
		hmax = QMAX(hmax, h);
		hmin = QMIN(hmin, h);
		c++;

		switch (map.heightChangeDirectionAtCorner(p.x() - 1, p.y() + 1)) {
			case 1:
				changedNeighborsUpCount++;
				break;
			case 2:
				changedNeighborsDownCount++;
				break;
			default:
				break;
		}
	}
#endif
	if (c == 0) {
		havg = 0.0f;
	} else {
		havg /= (float)c; // average height in surrounding cells that have already been visited by this algorithm
	}

	h = 0.0f;

	bool useAverage = true;
	h = havg;
//	h = hmax;

	int r = d->mRandom->getLong(randomHeightCount);
	bool changeUp = (r < changeUpCount);
	bool changeDown = (!changeUp && r < changeUpCount + changeDownCount);
	if (changedNeighborsUpCount > 0) {
		changeDown = false;
	}
	if (changedNeighborsDownCount > 0) {
		changeUp = false;
	}
	if (changeUp) {
		if (!useAverage) {
			h = hmax;
		}
		h += changeBy;
		map.setHeightChangeDirectionAtCorner(p.x(), p.y(), 1);
	} else if (changeDown) {
		if (!useAverage) {
			h = hmin;
		}
		h -= changeBy;
		map.setHeightChangeDirectionAtCorner(p.x(), p.y(), 2);
	} else {
		map.setHeightChangeDirectionAtCorner(p.x(), p.y(), 0);
	}
	map.setHeightAtCorner(p.x(), p.y(), h);
 }
}

void EditorRandomMapWidget::createHeightsDiamondSquare(MyMap& map)
{
 DiamondSquare diamond;
 diamond.setR(d->mDiamondSquareR->value());
 diamond.setDHeight(d->mDiamondSquareDHeight->value());
 diamond.diamondSquare(map);
}

void EditorRandomMapWidget::createMountains(MyMap& map)
{
 // do a BFS on all corners
 QValueList<QPoint> queue;
 cornersBFS(map, &queue);
 while (!queue.isEmpty()) {
	QPoint p = queue.front();
	queue.pop_front();

	if (lrint(d->mRandomMountainCount->value()) > 0) {
		int r = d->mRandom->getLong(lrint(d->mRandomMountainCount->value()));
		if (r == 0) {
			map.setStartMountainAtCorner(p.x(), p.y(), true);
		}
	}
 }

 // now actually place the mountains
 QValueList<QPoint> mountains;
 for (int x = 0; x < map.cornerWidth(); x++) {
	for (int y = 0; y < map.cornerHeight(); y++) {
		if (map.startMountainAtCorner(x, y)) {
			mountains.append(QPoint(x, y));
		}
	}
 }

 // remove mountains that are too close to other mountains
 for (QValueList<QPoint>::iterator it = mountains.begin(); it != mountains.end(); ++it) {
	QPoint p = *it;

	bool removedAllTooClose = true;
	do {
		removedAllTooClose = true;
		QValueList<QPoint>::iterator it2 = it;
		++it2;
		for (; it2 != mountains.end(); ++it2) {
			if (it2 == it) {
				continue;
			}
			QPoint p2 = *it2;
			int dx = QABS(p2.x() - p.x());
			int dy = QABS(p2.y() - p.y());
			// AB: sqrt probably not necessary, but at this point we dont
			// care _that_ much about speed
			float dist = sqrtf(dx * dx + dy * dy);

			const float maxDist = 0.0f; // TODO: currently unused
			if (dist < maxDist) {
				mountains.remove(it2);

				// somewhat ugly (slow) but who cares.
				removedAllTooClose = false;
				break;
			}
		}
	} while (!removedAllTooClose);
 }

 for (QValueList<QPoint>::iterator it = mountains.begin(); it != mountains.end(); ++it) {
	QPoint p = *it;

	// TODO: should be randomized
	float maxHeight = 20.0f; // TODO: numinput
	float heightFactor = (float)d->mRandom->getDouble();
	float height = maxHeight * heightFactor;

	float radiusFactor = d->mRandom->getDouble() / 8.0 + 0.875;
	int radius = (int)(10 * heightFactor * radiusFactor);

	int startX = QMAX(0, p.x() - radius);
	int endX = QMIN(p.x() + radius, map.cornerWidth() - 1);
	int startY = QMAX(0, p.y() - radius);
	int endY = QMIN(p.y() + radius, map.cornerHeight() - 1);
//	boDebug() << startX << " " << endX << "   " << startY << " " << endY << endl;
	for (int x = startX; x <= endX; x++) {
		for (int y = startY; y <= endY; y++) {
			int dx = QABS(x - p.x());
			int dy = QABS(y - p.y());

			// dist is element of [0 ; sqrt(2)*radius]
			float dist = sqrtf(dx * dx + dy * dy);

			// factor is element of [0 ; 1]
			float factor = dist / (sqrtf(2.0f) * radius);
			factor = QMIN(factor, 1.0f);

			// now dx==dy==0      => factor==1
			// and dx==dy==radius => factor==0
			factor = 1.0f - factor;

			float h = map.heightAtCorner(x, y);

			// randomize the factor slightly
			// we get a random value in [0.875;1] and multiply factor
			// by it
			float randomFactor = (float)d->mRandom->getDouble() / 8;
			randomFactor += 0.875f;
			factor *= randomFactor;

			h += height * factor;

			map.setHeightAtCorner(x, y, h);
		}
	}
 }


}

void EditorRandomMapWidget::slotUpdateHeightProbabilityLabels()
{
 if (lrint(d->mRandomHeightCount->value() < lrint(d->mChangeUpCount->value() + d->mChangeDownCount->value()))) {
	d->mHeightProbabilities->setText(i18n("Invalid values (height count must be <= changeUpCount + changeDownCount)"));
	return;
 }
 float changeBy = d->mChangeBy->value();
 float probUp = d->mChangeUpCount->value() / d->mRandomHeightCount->value();
 float probDown = d->mChangeDownCount->value() / d->mRandomHeightCount->value();
#if 0
 float probMountain = 0.0f;
 if (lrint(d->mRandomMountainCount->value()) > 0) {
	probMountain = 1.0f / d->mRandomMountainCount->value();
 }
#endif
 d->mHeightProbabilities->setText(i18n(
			 	"Prob(Change height UP by %1 at corner) = %2\n"
			 	"Prob(Change height DOWN by %3 at corner) = %4\n")
//			 	"Prob(Start mountain at corner) = %5")
				.arg(changeBy)
				.arg(probUp)
				.arg(changeBy)
				.arg(probDown));
}

void EditorRandomMapWidget::slotUpdateMountainProbabilityLabels()
{
 float probMountain = 0.0f;
 if (lrint(d->mRandomMountainCount->value()) > 0) {
	probMountain = 1.0f / d->mRandomMountainCount->value();
 }
 d->mMountainProbabilities->setText(i18n("Prob(Start mountain at corner) = %5")
				.arg(probMountain));
}

void EditorRandomMapWidget::slotTerrainCreationChanged(BoUfoRadioButton* button)
{
 boDebug() << k_funcinfo << button << endl;
 bool isSimple = false;
 bool isDiamondSquare = false;
 if (button == d->mSimpleTerrainCreationButton) {
	boDebug() << k_funcinfo << "isSimple" << endl;
	isSimple = true;
 } else if (button == d->mDiamondSquareTerrainCreationButton) {
	boDebug() << k_funcinfo << "isDiamond" << endl;
	isDiamondSquare = true;
 } else if (!button) {
	boWarning() << k_funcinfo << "no button selected" << endl;
 } else {
	boError() << k_funcinfo << "unknown button selected" << endl;
 }
 d->mSimpleTerrainCreation->setVisible(isSimple);
 d->mDiamondSquareTerrainCreation->setVisible(isDiamondSquare);
}



void DiamondSquare::diamondSquare(MyMap& origMap)
{
 int w = 1;
 int h = 1;
 while (w < (origMap.cornerWidth() - 1)) {
	w *= 2;
 }
 while (h < (origMap.cornerHeight() - 1)) {
	h *= 2;
 }
 if (w > h) {
	h = w;
 } else {
	w = h;
 }

 // new map has 2^n * 2^n cells, i.e. 2^(n+1) * 2^(n+1) corners
 // -> this makes things easier. we will later cut this to original size
 w++;
 h++;
 delete mMap;
 mMap = new MyMap(w, h);

 boDebug() << k_funcinfo << w << "x" << h << endl;
 boDebug() << k_funcinfo << "r=" << mR << " => 2^-r=" << mPow2_R << endl;
 boDebug() << k_funcinfo << "dheight=" << mOrigDHeight << endl;

 // initial values
 mMap->setHeightAtCorner(0, 0, 0.0f);
 mMap->setHeightAtCorner(mMap->cornerWidth() - 1, 0, 0.0f);
 mMap->setHeightAtCorner(0, mMap->cornerHeight() - 1, 0.0f);
 mMap->setHeightAtCorner(mMap->cornerWidth() - 1, mMap->cornerHeight() - 1, 0.0f);

 float dHeight = mOrigDHeight;

 // AB: note that w == h is important here
 int lod = (w - 1) / 2;
 while (lod >= 1) {
	// "diamond step"
	for (int x = lod; x < w; x += 2 * lod) {
		for (int y = lod; y < h; y += 2 * lod) {
			diamondStepCorner(x, y, lod, dHeight);
		}
	}

	// "square step"
	for (int x = lod; x < w; x += 2 * lod) {
		for (int y = 0; y < h; y += 2 * lod) {
			squareStepCorner(x, y, lod, dHeight);
		}
	}
	for (int x = 0; x < w; x += 2 * lod) {
		for (int y = lod; y < h; y += 2 * lod) {
			squareStepCorner(x, y, lod, dHeight);
		}
	}


	dHeight *= mPow2_R;
	lod /= 2;
 }

 // copy to original map (also cuts to original size)
 origMap.copyFrom(*mMap);

 delete mMap;
 mMap = 0;
}

void DiamondSquare::diamondStepCorner(int x, int y, int lod, float dHeight)
{
 int x1 = x - lod;
 int x2 = x + lod;
 int y1 = y - lod;
 int y2 = y + lod;

 // height at the 4 corner points
 float totalHeight = 0.0f;
 totalHeight += mMap->heightAtCorner(x1, y1);
 totalHeight += mMap->heightAtCorner(x1, y2);
 totalHeight += mMap->heightAtCorner(x2, y1);
 totalHeight += mMap->heightAtCorner(x2, y2);
 float averageHeight = totalHeight / 4.0f;
 double random = mRandom.getDouble() - 0.5; // -> [-0.5;0.5[
 float height = averageHeight + random * dHeight;

 mMap->setHeightAtCorner(x, y, height);
}

void DiamondSquare::squareStepCorner(int x, int y, int lod, float dHeight)
{
 float c = 0.0f;
 float totalHeight = 0.0f;
 if (x - lod >= 0) {
	totalHeight += mMap->heightAtCorner(x - lod, y);
	c += 1.0f;
 }
 if (y - lod >= 0) {
	totalHeight += mMap->heightAtCorner(x, y - lod);
	c += 1.0f;
 }
 if (x + lod <= mMap->cornerWidth() - 1) {
	totalHeight += mMap->heightAtCorner(x + lod, y);
	c += 1.0f;
 }
 if (y + lod <= mMap->cornerHeight() - 1) {
	totalHeight += mMap->heightAtCorner(x, y + lod);
	c += 1.0f;
 }
 float averageHeight = totalHeight / c;
 double random = mRandom.getDouble() - 0.5;
 float height = averageHeight + random * dHeight;

 mMap->setHeightAtCorner(x, y, height);
}

void cornersBFS(const MyMap& map, QValueList<QPoint>* retQueue)
{
 retQueue->clear();
 QValueList<QPoint> queue;
 queue.append(QPoint(0, 0));
 retQueue->append(QPoint(0, 0));
#define ENQUEUE(x) queue.append(x); retQueue->append(x);
#define DEQUEUE queue.front(); queue.pop_front();
 while (!queue.isEmpty()) {
	QPoint p = DEQUEUE;

	if (p.y() + 1 < map.cornerHeight()) {
		ENQUEUE(QPoint(p.x(), p.y() + 1));
	}
	if (p.y() == 0 && p.x() + 1 < map.cornerWidth()) {
		ENQUEUE(QPoint(p.x() + 1, p.y()));
	}
 }

#undef ENQUEUE
#undef DEQUEUE
}

