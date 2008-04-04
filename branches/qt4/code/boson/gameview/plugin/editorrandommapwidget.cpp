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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "editorrandommapwidget.h"
#include "editorrandommapwidget.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../../no_player.h"
#include "../../gameengine/bosoncanvas.h"
#include "../../gameengine/bosonmap.h"
#include "../../gameengine/playerio.h"
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
		factor = 1.0f;
	}
	HCorner& operator=(const HCorner& c)
	{
		h = c.h;
		willStartMountain = c.willStartMountain;
		dir = c.dir;
		factor = c.factor;

		return *this;
	}
	float h;
	bool willStartMountain;

	int dir;
	float factor;
};
class MyMap {
public:
	MyMap(int cornerWidth, int cornerHeight)
	{
		mCornerWidth = cornerWidth;
		mCornerHeight = cornerHeight;
		mCorners = new HCorner[(mCornerWidth + 1) * (mCornerHeight + 1)];
	}
	MyMap(const MyMap& map)
	{
		mCornerWidth = map.cornerWidth();
		mCornerHeight = map.cornerHeight();
		mCorners = new HCorner[(mCornerWidth + 1) * (mCornerHeight + 1)];

		copyFrom(map);
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
				setFactorAtCorner(x, y, 1.0f);
			}
		}
	}

	/**
	 * Scale all heights in this map to be inside the valid @ref BosonMap
	 * heights.
	 *
	 * If all heights are already valid, no scaling is applied.
	 **/
	void scaleHeights()
	{
		float min = 0.0f;
		float max = 0.0f;
		for (int x = 0; x < cornerWidth(); x++) {
			for (int y = 0; y < cornerHeight(); y++) {
				float h = heightAtCorner(x, y);
				if (h < min) {
					min = h;
				}
				if (h > max) {
					max = h;
				}
			}
		}

		float realMax = 18.75;
		float realMin = -13.125;

		float scalePos = 1.0f;
		float scaleNeg = 1.0f;
		if (max > realMax) {
			scalePos = realMax / max;
		}
		if (min < realMin) {
			// AB: both are negative, so scaleNeg is positive
			scaleNeg = realMin / min;
		}
		if (scalePos == 1.0f && scaleNeg == 1.0f) {
			// nothing to scale
			boDebug() << "all heights valid - no scaling" << endl;
			return;
		}
		float scale = scalePos;
		if (scaleNeg < scale) {
			scale = scaleNeg;
		}
		boDebug() << "scaling of " << scalePos << " for positive and of " << scaleNeg << " for negative heights requested. Using " << scale << " for all heights." << endl;
		for (int x = 0; x < cornerWidth(); x++) {
			for (int y = 0; y < cornerHeight(); y++) {
				float h = heightAtCorner(x, y);
				setHeightAtCorner(x, y, h * scale);
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

	void setFactorAtCorner(int x, int y, float f)
	{
		int index = cornerArrayPos(x, y);
		mCorners[index].factor = f;
	}
	float factorAtCorner(int x, int y) const
	{
		int index = cornerArrayPos(x, y);
		return mCorners[index].factor;
	}
private:
	HCorner* mCorners;
	int mCornerWidth;
	int mCornerHeight;
};

/**
 * Implementation of the diamond square algorithm.
 * Idea from
 * Game Programming Gems 1, Chapter 4.18, by Jason Shankel. Code completely by
 * me, I did not use the sample code in any way.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
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
	void diamondSquare2(MyMap& map, int x1, int x2, int y1, int y2);

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

/**
 * Particle Deposition algorithm that is intended to create mountains. Idea from
 * Game Programming Gems 1, Chapter 4.19, by Jason Shankel. Code completely by
 * me, I did not use the sample code in any way.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ParticleDeposition
{
public:
	ParticleDeposition()
	{
		mParticleHeight = 0.5f;
		mNumberOfParticles = 100;

		setParticleHeight(0.5f);
		setNumberOfParticles(1000);
	}
	~ParticleDeposition()
	{
	}
	void setParticleHeight(float h)
	{
		mParticleHeight = h;
	}
	void setNumberOfParticles(int n)
	{
		mNumberOfParticles = n;
	}

	void particleDeposition(MyMap& map, const QPoint& start);

protected:
	bool moveParticle(MyMap& map, int x, int y, float particleHeight, QPoint* dest);

	bool neighbor(const MyMap& map, int i, int* x, int* y) const
	{
		switch (i)
		{
			case 0:
				if (*x - 1 >= 0) {
					*x = *x - 1;
					return true;
				}
				return false;
			case 1:
				if (*y - 1 >= 0) {
					*y = *y - 1;
					return true;
				}
				return false;
			case 2:
				if (*x - 1 >= 0 && *y - 1 >= 0) {
					*x = *x - 1;
					*y = *y - 1;
					return true;
				}
				return false;
			case 3:
				if (*x + 1 < map.cornerWidth()) {
					*x = *x + 1;
					return true;
				}
				return false;
			case 4:
				if (*y + 1 < map.cornerHeight()) {
					*y = *y + 1;
					return true;
				}
				return false;
			case 5:
				if (*x + 1 < map.cornerWidth() && *y + 1 < map.cornerHeight()) {
					*x = *x + 1;
					*y = *y + 1;
					return true;
				}
				return false;
			case 6:
				if (*x + 1 < map.cornerWidth() && *y - 1 >= 0) {
					*x = *x + 1;
					*y = *y - 1;
					return true;
				}
				return false;
			case 7:
				if (*x - 1 >= 0 && *y + 1 < map.cornerHeight()) {
					*x = *x - 1;
					*y = *y + 1;
					return true;
				}
				return false;
			default:
				boError() << k_funcinfo << "invalid parameter" << endl;
				return false;
		}
		return false;
	}

private:
	KRandomSequence mRandom;
	float mParticleHeight;
	int mNumberOfParticles;
};

class MountainSimple
{
public:
	MountainSimple()
	{
		mMaxHeight = 10.0f;
		mMultiplyHeightWithRandomFactor = true;
		mMultiplyWidthWithRandomFactor = true;
		mMultiplyWidthWithRandomHeightFactor = true;
		mWidthX = 10.0f;
		mWidthY = 10.0f;

		setMaxHeight(10.0f);
		setMultiplyHeightWithRandomFactor(true);
		setMultiplyWidthWithRandomFactor(true);
		setMultiplyWidthWithRandomHeightFactor(true);
		setWidthX(10.0f);
		setWidthY(10.0f);
	}

	void setMaxHeight(float h)
	{
		mMaxHeight = h;
	}
	void setMultiplyHeightWithRandomFactor(bool m)
	{
		mMultiplyHeightWithRandomFactor = m;
	}
	void setMultiplyWidthWithRandomFactor(bool m)
	{
		mMultiplyWidthWithRandomFactor = m;
	}
	void setMultiplyWidthWithRandomHeightFactor(bool m)
	{
		mMultiplyWidthWithRandomHeightFactor = m;
	}
	void setWidthX(float x)
	{
		mWidthX = x;
	}
	void setWidthY(float y)
	{
		mWidthY = y;
	}
	void createMountain(MyMap& map, const QPoint& start);

protected:
	/**
	 * @return A factor describing the distance to the maxHeight corner. 0.0
	 * means maximum distance, 1.0 means zero distance.
	 *
	 * @param x x-coordinate of the corner
	 * @param y y-coordinate of the corner
	 * @param maxHeightX x-coordinate of the corner with the maximum height
	 * (e.g. the center of the mountain)
	 * @param maxHeightY y-coordinate of the corner with the maximum height
	 * (e.g. the center of the mountain)
	 * @param widthX The width in x direction of the mountain
	 * @param widthY The width in y direction of the mountain
	 **/
	float linearFactorOfCorner(int x, int y, int maxHeightX, int maxHeightY, int widthX, int widthY) const;


	float heightAtCorner2(float factor, float factor2, float height2, float maxHeight) const;

private:
	KRandomSequence mRandom;
	float mMaxHeight;
	bool mMultiplyHeightWithRandomFactor;
	bool mMultiplyWidthWithRandomFactor;
	bool mMultiplyWidthWithRandomHeightFactor;
	float mWidthX;
	float mWidthY;
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

		mSelectMountainCreation = 0;
		mSimpleMountainCreationButton = 0;
		mParticleDepositionMountainCreationButton = 0;
		mDiamondSquareMountainCreationButton = 0;

		mSimpleMountainCreation = 0;
		mSimpleMountainMaxHeight = 0;
		mSimpleMountainWidthX = 0;
		mSimpleMountainWidthY = 0;

		mParticleDepositionMountainCreation = 0;
		mParticlesCount = 0;
		mParticlesHeight = 0;

		mDiamondSquareMountainCreation = 0;
		mMountainDiamondSquareDHeight = 0;
		mMountainDiamondSquareR = 0;

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

	BoUfoButtonGroupWidget* mSelectMountainCreation;
	BoUfoRadioButton* mSimpleMountainCreationButton;
	BoUfoRadioButton* mParticleDepositionMountainCreationButton;
	BoUfoRadioButton* mDiamondSquareMountainCreationButton;

	BoUfoWidget* mSimpleMountainCreation;
	BoUfoNumInput* mSimpleMountainMaxHeight;
	BoUfoNumInput* mSimpleMountainWidthX;
	BoUfoNumInput* mSimpleMountainWidthY;

	BoUfoWidget* mParticleDepositionMountainCreation;
	BoUfoNumInput* mParticlesCount;
	BoUfoNumInput* mParticlesHeight;

	BoUfoWidget* mDiamondSquareMountainCreation;
	BoUfoNumInput* mMountainDiamondSquareDHeight;
	BoUfoNumInput* mMountainDiamondSquareR;

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
 boDebug() << k_funcinfo << endl;
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

 d->mSelectMountainCreation = new BoUfoButtonGroupWidget();
 parent->addWidget(d->mSelectMountainCreation);
 d->mSimpleMountainCreationButton = new BoUfoRadioButton(i18n("Simple algorithm"));
 d->mParticleDepositionMountainCreationButton = new BoUfoRadioButton(i18n("Particle deposition algorithm"));
 d->mDiamondSquareMountainCreationButton = new BoUfoRadioButton(i18n("Diamond square algorithm"));
 d->mSimpleMountainCreationButton->setSelected(true);
// d->mParticleDepositionMountainCreationButton->setSelected(true);
// d->mDiamondSquareMountainCreationButton->setSelected(true);
 d->mSelectMountainCreation->addWidget(d->mSimpleMountainCreationButton);
 d->mSelectMountainCreation->addWidget(d->mParticleDepositionMountainCreationButton);
 d->mSelectMountainCreation->addWidget(d->mDiamondSquareMountainCreationButton);


 d->mSimpleMountainCreation = new BoUfoWidget();
 parent->addWidget(d->mSimpleMountainCreation);
 d->mSimpleMountainMaxHeight = new BoUfoNumInput();
 d->mSimpleMountainMaxHeight->setLabel(i18n("Max Height"));
 d->mSimpleMountainMaxHeight->setRange(5.0f, 20.0f);
 d->mSimpleMountainMaxHeight->setValue(15.0f);
 d->mSimpleMountainCreation->addWidget(d->mSimpleMountainMaxHeight);

 d->mSimpleMountainWidthX = new BoUfoNumInput();
 d->mSimpleMountainWidthX->setLabel(i18n("Width in X direction"));
 d->mSimpleMountainWidthX->setRange(5.0f, 40.0f);
 d->mSimpleMountainWidthX->setValue(10.0f);
 d->mSimpleMountainCreation->addWidget(d->mSimpleMountainWidthX);

 d->mSimpleMountainWidthY = new BoUfoNumInput();
 d->mSimpleMountainWidthY->setLabel(i18n("Width in Y direction"));
 d->mSimpleMountainWidthY->setRange(5.0f, 40.0f);
 d->mSimpleMountainWidthY->setValue(10.0f);
 d->mSimpleMountainCreation->addWidget(d->mSimpleMountainWidthY);

 d->mParticleDepositionMountainCreation = new BoUfoWidget();
 parent->addWidget(d->mParticleDepositionMountainCreation);

 d->mParticlesCount = new BoUfoNumInput();
 d->mParticlesCount->setLabel(i18n("Particle Count: "));
 d->mParticlesCount->setRange(100.0f, 10000.0f);
 d->mParticlesCount->setValue(500.0f);
 d->mParticleDepositionMountainCreation->addWidget(d->mParticlesCount);

 d->mParticlesHeight = new BoUfoNumInput();
 d->mParticlesHeight->setLabel(i18n("Particle height: "));
 d->mParticlesHeight->setRange(0.1f, 2.0f);
 d->mParticlesHeight->setStepSize(0.1f);
 d->mParticlesHeight->setValue(0.5f);
 d->mParticleDepositionMountainCreation->addWidget(d->mParticlesHeight);

 d->mDiamondSquareMountainCreation = new BoUfoWidget();
 parent->addWidget(d->mDiamondSquareMountainCreation);

 d->mMountainDiamondSquareDHeight = new BoUfoNumInput();
 d->mMountainDiamondSquareDHeight->setLabel(i18n("\"dHeight\" in diamond square: "));
 d->mMountainDiamondSquareDHeight->setRange(0.0f, 100.0f);
 d->mMountainDiamondSquareDHeight->setStepSize(0.5f);
 d->mMountainDiamondSquareDHeight->setValue(30.0f);
 d->mDiamondSquareMountainCreation->addWidget(d->mMountainDiamondSquareDHeight);

 d->mMountainDiamondSquareR = new BoUfoNumInput();
 d->mMountainDiamondSquareR->setLabel(i18n("\"r\" in diamond square: "));
 d->mMountainDiamondSquareR->setRange(0.0f, 10.0f);
 d->mMountainDiamondSquareR->setStepSize(0.1f);
 d->mMountainDiamondSquareR->setValue(1.0f);
 d->mDiamondSquareMountainCreation->addWidget(d->mMountainDiamondSquareR);



 BoUfoWidget* generalConfiguration = new BoUfoWidget();
 parent->addWidget(generalConfiguration);
 d->mRandomMountainCount = new BoUfoNumInput();
 d->mRandomMountainCount->setLabel(i18n("Random Count (place mountain): "));
 d->mRandomMountainCount->setRange(0.0f, 2000.0f);
 generalConfiguration->addWidget(d->mRandomMountainCount);

 d->mMountainProbabilities = new BoUfoLabel();
 generalConfiguration->addWidget(d->mMountainProbabilities);

 connect(d->mRandomMountainCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateMountainProbabilityLabels()));
 d->mRandomMountainCount->setValue(1000.0f);


 connect(d->mSelectMountainCreation, SIGNAL(signalButtonActivated(BoUfoRadioButton*)),
		this, SLOT(slotMountainCreationChanged(BoUfoRadioButton*)));
 slotMountainCreationChanged(d->mSelectMountainCreation->selectedButton());

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

 map.scaleHeights();

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
 BoUfoRadioButton* b = d->mSelectMountainCreation->selectedButton();
 if (!b) {
	boWarning() << k_funcinfo << "no mountain creation algorithm selected" << endl;
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
 map.loadHeightsFromRealMap(realMap);

 // do a BFS on all corners
 QValueList<QPoint> mountains;
 QValueList<QPoint> queue;
 cornersBFS(map, &queue);
 while (!queue.isEmpty()) {
	QPoint p = queue.front();
	queue.pop_front();

	if (lrint(d->mRandomMountainCount->value()) > 0) {
		int r = d->mRandom->getLong(lrint(d->mRandomMountainCount->value()));
		if (r == 0) {
			mountains.append(p);
		}
	}
 }
 // remove mountains that are too close to other mountains
#if 0
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
#endif

 bool isSimple = false;
 bool isParticleDeposition = false;
 bool isDiamondSquare  = false;
 if (b == d->mSimpleMountainCreationButton) {
	isSimple = true;
 } else if (b == d->mParticleDepositionMountainCreationButton) {
	isParticleDeposition = true;
 } else if (b == d->mDiamondSquareMountainCreationButton) {
	isDiamondSquare = true;
 } else {
	boError() << k_funcinfo << "unknown button selected" << endl;
	return;
 }

 for (QValueList<QPoint>::iterator it = mountains.begin(); it != mountains.end(); ++it) {
	if (isSimple) {
		createMountainSimple(map, *it);
	} else if (isParticleDeposition) {
		createMountainParticleDeposition(map, *it);
	} else if (isDiamondSquare) {
		createMountainDiamondSquare(map, *it);
	}
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

void EditorRandomMapWidget::createMountainSimple(MyMap& map, const QPoint& start)
{
 MountainSimple simple;
 simple.setMaxHeight(d->mSimpleMountainMaxHeight->value());
 simple.setMultiplyHeightWithRandomFactor(true);
 simple.setMultiplyWidthWithRandomFactor(true);
 simple.setMultiplyWidthWithRandomHeightFactor(true);
 simple.setWidthX(d->mSimpleMountainWidthX->value());
 simple.setWidthY(d->mSimpleMountainWidthY->value());

 simple.createMountain(map, start);
}

void EditorRandomMapWidget::createMountainParticleDeposition(MyMap& map, const QPoint& start)
{
 ParticleDeposition pd;
 pd.setParticleHeight(d->mParticlesHeight->value());
 pd.setNumberOfParticles((int)d->mParticlesCount->value());
 pd.particleDeposition(map, start);
}

void EditorRandomMapWidget::createMountainDiamondSquare(MyMap& map, const QPoint& start)
{
 const int size = 32;
 if (start.x() < size || start.x() + size >= map.cornerWidth()) {
	boDebug() << k_funcinfo << "won't start mountain at x=" << start.x() << endl;
	return;
 }
 if (start.y() < size || start.y() + size >= map.cornerHeight()) {
	boDebug() << k_funcinfo << "won't start mountain at y=" << start.y() << endl;
	return;
 }
#if 1
 DiamondSquare diamond;
 diamond.setDHeight(d->mMountainDiamondSquareDHeight->value());
 diamond.setR(d->mMountainDiamondSquareR->value());
 diamond.diamondSquare2(map, start.x() - size/2, start.x() + size/2, start.y() - size/2, start.y() + size/2);
#endif
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
 d->mHeightProbabilities->setText(i18n(
			 	"Prob(Change height UP by %1 at corner) = %2\n"
			 	"Prob(Change height DOWN by %3 at corner) = %4\n")
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
 bool isSimple = false;
 bool isDiamondSquare = false;
 if (button == d->mSimpleTerrainCreationButton) {
	isSimple = true;
 } else if (button == d->mDiamondSquareTerrainCreationButton) {
	isDiamondSquare = true;
 } else if (!button) {
	boWarning() << k_funcinfo << "no button selected" << endl;
 } else {
	boError() << k_funcinfo << "unknown button selected" << endl;
 }
 d->mSimpleTerrainCreation->setVisible(isSimple);
 d->mDiamondSquareTerrainCreation->setVisible(isDiamondSquare);
}

void EditorRandomMapWidget::slotMountainCreationChanged(BoUfoRadioButton* button)
{
 bool isSimple = false;
 bool isParticleDeposition = false;
 bool isDiamondSquare = false;
 if (button == d->mSimpleMountainCreationButton) {
	isSimple = true;
 } else if (button == d->mParticleDepositionMountainCreationButton) {
	isParticleDeposition = true;
 } else if (button == d->mDiamondSquareMountainCreationButton) {
	isDiamondSquare = true;
 } else if (!button) {
	boWarning() << k_funcinfo << "no button selected" << endl;
 } else {
	boError() << k_funcinfo << "unknown button selected" << endl;
 }
 d->mSimpleMountainCreation->setVisible(isSimple);
 d->mParticleDepositionMountainCreation->setVisible(isParticleDeposition);
 d->mDiamondSquareMountainCreation->setVisible(isDiamondSquare);
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

void DiamondSquare::diamondSquare2(MyMap& origMap, int x1, int x2, int y1, int y2)
{
 int dx = x2 - x1;
 int dy = y2 - y1;
 if (x1 < dx/2 || x2 + dx/2 >= origMap.cornerWidth()) {
	boWarning() << "invalid x parameters " << x1 << " " << x2 << endl;
	return;
 }
 if (y1 < dy/2 || y2 + dy/2 >= origMap.cornerHeight()) {
	boWarning() << "invalid y parameters" << endl;
	return;
 }

 if (dx != dy) {
	boWarning() << k_funcinfo << "invalid paramters" << endl;
 }

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

#if 0

 // initial values
 mMap->setHeightAtCorner(0, 0, 0.0f);
 mMap->setHeightAtCorner(mMap->cornerWidth() - 1, 0, 0.0f);
 mMap->setHeightAtCorner(0, mMap->cornerHeight() - 1, 0.0f);
 mMap->setHeightAtCorner(mMap->cornerWidth() - 1, mMap->cornerHeight() - 1, 0.0f);
#else

 for (int x = 0; x < origMap.cornerWidth(); x++) {
	for (int y = 0; y < origMap.cornerHeight(); y++) {
		mMap->setHeightAtCorner(x, y, origMap.heightAtCorner(x, y));
	}
 }
 for (int x = 0; x < origMap.cornerWidth(); x++) {
	for (int y = origMap.cornerHeight(); y < mMap->cornerHeight(); y++) {
		mMap->setHeightAtCorner(x, y, origMap.heightAtCorner(x, origMap.cornerHeight() - 1));
	}
 }
 for (int x = origMap.cornerWidth(); x < mMap->cornerWidth(); x++) {
	for (int y = 0; y < origMap.cornerHeight(); y++) {
		mMap->setHeightAtCorner(x, y, origMap.heightAtCorner(origMap.cornerWidth() - 1, y));
	}
 }
 for (int x = origMap.cornerWidth(); x < mMap->cornerWidth(); x++) {
	for (int y = origMap.cornerHeight(); y < mMap->cornerHeight(); y++) {
		mMap->setHeightAtCorner(x, y, origMap.heightAtCorner(origMap.cornerWidth() - 1, origMap.cornerHeight() - 1));
	}
 }

#endif

 float dHeight = mOrigDHeight;

 // AB: note that dx == dy is important here
 int lod = (dx - 1) / 2;
 while (lod >= 1) {
	// "diamond step"
	for (int x = x1 + lod; x < x2; x += 2 * lod) {
		for (int y = y1 + lod; y < y2; y += 2 * lod) {
			diamondStepCorner(x, y, lod, dHeight);
		}
	}

	// "square step"
	for (int x = x1 + lod; x < x2; x += 2 * lod) {
		for (int y = 0; y < y2; y += 2 * lod) {
			squareStepCorner(x, y, lod, dHeight);
		}
	}
	for (int x = 0; x < x2; x += 2 * lod) {
		for (int y = y1 + lod; y < y2; y += 2 * lod) {
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

void ParticleDeposition::particleDeposition(MyMap& map, const QPoint& start)
{
 // TODO: instead of a number of particles, we could also use a certain height
 // of the start point as stop condition
 // TODO: randomize the number of particles slightly, e.g. use a factor in
 // [0.5;1.5] that mNumberOfParticles is multiplied by
 int numberOfParticles = mNumberOfParticles;
 for (int count = 0; count < numberOfParticles; count++) {
	float particleHeight = mParticleHeight;

#if 0
	// our particles do not have a fixed size
	double factor = mRandom.getDouble() + 0.5;
	particleHeight *= factor;
#endif

	float cornerHeight = map.heightAtCorner(start.x(), start.y());
	cornerHeight += particleHeight;

	map.setHeightAtCorner(start.x(), start.y(), cornerHeight);

	QPoint pos = start;
	QPoint dest;

	while (moveParticle(map, pos.x(), pos.y(), particleHeight, &dest)) {
#if 0
		boDebug() << count << " from " << pos.x() << "x" << pos.y()
				<< " to " << dest.x() << "x" << dest.y()
				<< " from "
				<< map.heightAtCorner(pos.x(), pos.y())
				<< " to "
				<< map.heightAtCorner(dest.x(), dest.y())
				<< endl;
#endif

		pos = dest;
	}
 }
}

bool ParticleDeposition::moveParticle(MyMap& map, int x, int y, float particleHeight, QPoint* dest)
{
#if 0
 // with a certain (low) probability stop without moving any further.
 int r = mRandom.getLong(1000);
 if (r == 0) {
	// do not move any further
	return false;
 }
#endif

 if (!dest) {
	BO_NULL_ERROR(dest);
	return false;
 }
 float cornerHeight = map.heightAtCorner(x, y);

 QValueList<int> candidates;
 for (int i = 0; i < 8; i++) {
	int nx = x;
	int ny = y;
	if (neighbor(map, i, &nx, &ny)) {
		const float epsilon = 0.0001f;
		if (map.heightAtCorner(nx, ny) + particleHeight + epsilon < cornerHeight) {
			candidates.append(i);
		}
	}
 }
 if (candidates.count() == 0) {
	// don't move the particle any further
	return false;
 }

 int target = mRandom.getLong(candidates.count());
 int destX = x;
 int destY = y;
 if (!neighbor(map, candidates[target], &destX, &destY)) {
	boError() << k_funcinfo << "internal error" << endl;
	return false;
 }

 map.setHeightAtCorner(x, y, map.heightAtCorner(x, y) - particleHeight);
 map.setHeightAtCorner(destX, destY, map.heightAtCorner(destX, destY) + particleHeight);

 dest->setX(destX);
 dest->setY(destY);
 return true;
}

void MountainSimple::createMountain(MyMap& map, const QPoint& start)
{
 QPoint p = start;

 float heightFactor = 1.0f;
 if (mMultiplyHeightWithRandomFactor) {
	// heightFactor <= 1.0f is ensured, so maxHeight will never be exceeded
	heightFactor = (float)mRandom.getDouble();
 }
 float height = mMaxHeight * heightFactor;

 float widthXBase = mWidthX;
 float widthYBase = mWidthY;
 if (mMultiplyWidthWithRandomFactor) {
	// note that the factor is allowed to be larger than 1
	// (also note, that this implementation may not necessarily use this)

	float r;
	r = mRandom.getDouble() + 0.875;
//	r = mRandom.getDouble() / 8.0 + 0.875;
	widthXBase *= r;
	r = mRandom.getDouble() + 0.875;
//	r = mRandom.getDouble() / 8.0 + 0.875;
	widthYBase *= r;
 }
 if (mMultiplyWidthWithRandomHeightFactor) {
	widthXBase *= heightFactor;
	widthYBase *= heightFactor;
 }
 int widthX = (int)widthXBase;
 int widthY = (int)widthYBase;

 // we use a dummy map with height = 0.0f everywhere to create the mountain
 MyMap tmpMap(map.cornerWidth(), map.cornerHeight());
 tmpMap.resetHeights();

 int startX = QMAX(0, p.x() - widthX);
 int endX = QMIN(p.x() + widthX, map.cornerWidth() - 1);
 int startY = QMAX(0, p.y() - widthY);
 int endY = QMIN(p.y() + widthY, map.cornerHeight() - 1);
 bool quadratic = false; // factor * factor
 bool sqrtf_linear = true; // sqrtf(factor) * factor
 bool randomize = true;
 bool usePreviousCorners = true;
 for (int x = startX; x <= endX; x++) {
	for (int y = startY; y <= endY; y++) {
		float factor = linearFactorOfCorner(x, y, p.x(), p.y(), widthX, widthY);

		if (quadratic) {
			factor = factor * factor;
		} else if (sqrtf_linear) {
			factor = sqrtf(factor) * factor;
		}

		if (randomize) {
			// randomize the factor slightly
			// we get a random value in [0.875;1] and multiply factor
			// by it
			float randomFactor = (float)mRandom.getDouble() / 8;
			randomFactor += 0.875f;
			factor *= randomFactor;
		}

		tmpMap.setFactorAtCorner(x, y, factor);


		float h = 0.0f;
		if (usePreviousCorners) {
			// macro that returns the height that the corner 2, which is
			// defined by (a,b) "thinks" that should be at corner (x,y)
			#define HEIGHT2(a, b) \
				heightAtCorner2(factor, \
					tmpMap.factorAtCorner(a, b), \
					tmpMap.heightAtCorner(a, b), \
					height \
				)

			int c = 0;
			float average = 0.0f;
			if (x - 1 >= 0 && x - 1 >= startX) {
				average += HEIGHT2(x - 1, y);
				c++;
				if (y - 1 >= 0 && y - 1 >= startY) {
					average += HEIGHT2(x - 1, y - 1);
					c++;
				}
				if (y + 1 < tmpMap.cornerHeight() && y + 1 <= endY) {
					average += HEIGHT2(x - 1, y + 1);
					c++;
				}
				average /= (float)c;
			} else {
				// no neighbors available.
				average = heightAtCorner2(factor, -1.0f, 0.0f, height);
			}

			#undef HEIGHT2
			h = average;
		} else {
			h = height * factor;
		}


		tmpMap.setHeightAtCorner(x, y, h);
	}
 }

 // apply the generated corner to the official map
 for (int x = startX; x <= endX; x++) {
	for (int y = startY; y <= endY; y++) {
		float h = map.heightAtCorner(x, y);

		h += tmpMap.heightAtCorner(x, y);

		map.setHeightAtCorner(x, y, h);
	}
 }
}

float MountainSimple::linearFactorOfCorner(int x, int y, int maxHeightX, int maxHeightY, int widthX, int widthY) const
{
 int dx = QABS(x - maxHeightX);
 int dy = QABS(y - maxHeightY);

 // dist is element of [0 ; sqrt(widthX^2 + widthY^2)]
 float dist = sqrtf(dx * dx + dy * dy);

 // factor is element of [0 ; 1]
 float factor = dist / (sqrtf(widthX * widthX + widthY * widthY));
 factor = QMIN(factor, 1.0f);

 // now dx==dy==0              => factor==1
 // and dx==widthX, dy==widthY => factor==0
 // this means linear interpolation of the height from the top of
 // the mountain to all other points.
 factor = 1.0f - factor;

 return factor;
}

float MountainSimple::heightAtCorner2(float factor, float factor2, float height2, float maxHeight) const
{
 // AB: the real equation for the height is:
 //   h = maxHeight * factor
 //
 // however due to randomizing, it might make sense to take the
 // actual height values of the neighbor corners which have been
 // calculated already into account.
 // for any (other) corner (x', y') with factor f2, we can
 // calculate the height of this corner with the height of the
 // other corner like:
 //   h = cornerHeight(x',y') + (factor - f2) * maxHeight
 //
 // if no randomizing is applied, the the value of h is the same with both
 // approaches, assuming that (x',y') has been calculated the same way.
 //
 // note that if no neighbor corner does exist, only the first equation can be
 // used.
 if (factor2 < 0.0f) {
	return maxHeight * factor;
 }
 return height2 + (factor - factor2) * maxHeight;
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

