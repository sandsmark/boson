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

class HCorner {
public:
	HCorner()
	{
		h = 0.0f;
		willStartMountain = false;
	}
	float h;
	bool willStartMountain;
};
class MyMap {
public:
	MyMap(const BosonMap* realMap)
	{
		mMap = realMap;
		const int cornerWidth = realMap->width() + 1;
		const int cornerHeight = realMap->height() + 1;
		mCorners = new HCorner[(cornerWidth + 1) * (cornerHeight + 1)];
	}
	~MyMap()
	{
		delete[] mCorners;
	}

	int cornerWidth() const
	{
		return mMap->width() + 1;
	}
	int cornerHeight() const
	{
		return mMap->height() + 1;
	}
	void loadHeightsFromRealMap()
	{
		for (int x = 0; x < cornerWidth(); x++) {
			for (int y = 0; y < cornerHeight(); y++) {
				setHeightAtCorner(x, y, mMap->heightAtCorner(x, y));
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
		int index = mMap->cornerArrayPos(x, y);
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
		int index = mMap->cornerArrayPos(x, y);
		return mCorners[index].h;
	}

	void setStartMountainAtCorner(int x, int y, bool s)
	{
		int index = mMap->cornerArrayPos(x, y);
		mCorners[index].willStartMountain = s;
	}
	bool startMountainAtCorner(int x, int y) const
	{
		int index = mMap->cornerArrayPos(x, y);
		return mCorners[index].willStartMountain;
	}
private:
	HCorner* mCorners;
	const BosonMap* mMap;
};

class EditorRandomMapWidgetPrivate
{
public:
	EditorRandomMapWidgetPrivate()
	{
		mRandom = 0;

		mRandomHeightCount = 0;
		mChangeUpCount = 0;
		mChangeDownCount = 0;
		mChangeBy = 0;
		mHeightProbabilities = 0;

		mRandomMountainCount = 0;
	}
	KRandomSequence* mRandom;

	BoUfoNumInput* mRandomHeightCount;
	BoUfoNumInput* mChangeUpCount;
	BoUfoNumInput* mChangeDownCount;
	BoUfoNumInput* mChangeBy;
	BoUfoLabel* mHeightProbabilities;

	BoUfoNumInput* mRandomMountainCount;
};

EditorRandomMapWidget::EditorRandomMapWidget()
		: BoUfoWidget()
{
 d = new EditorRandomMapWidgetPrivate();
 mCanvas = 0;
 d->mRandom = new KRandomSequence();

 setLayoutClass(UHBoxLayout);

 BoUfoVBox* box = new BoUfoVBox();
 addWidget(box);

 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setStretch(1);
 addWidget(stretch);

 BoUfoLabel* header = new BoUfoLabel(i18n("Random Map generation"));
 box->addWidget(header);

 d->mRandomHeightCount = new BoUfoNumInput();
 d->mRandomHeightCount->setLabel(i18n("Random Count (height up/down): "));
 d->mRandomHeightCount->setRange(2.0f, 150.0f);
 box->addWidget(d->mRandomHeightCount);

 d->mChangeUpCount = new BoUfoNumInput();
 d->mChangeUpCount->setLabel(i18n("Change Up Count: "));
 d->mChangeUpCount->setRange(0.0f, 20.0f);
 box->addWidget(d->mChangeUpCount);

 d->mChangeDownCount = new BoUfoNumInput();
 d->mChangeDownCount->setLabel(i18n("Change Down Count: "));
 d->mChangeDownCount->setRange(0.0, 20.0);
 box->addWidget(d->mChangeDownCount);

 d->mChangeBy = new BoUfoNumInput();
 d->mChangeBy->setLabel(i18n("Change By: "));
 d->mChangeBy->setRange(0.0f, 5.0f);
 d->mChangeBy->setStepSize(0.1f);
 box->addWidget(d->mChangeBy);

 d->mRandomMountainCount = new BoUfoNumInput();
 d->mRandomMountainCount->setLabel(i18n("Random Count (mountain): "));
 d->mRandomMountainCount->setRange(0.0f, 2000.0f);
 box->addWidget(d->mRandomMountainCount);


 d->mHeightProbabilities = new BoUfoLabel();
 box->addWidget(d->mHeightProbabilities);

 connect(d->mRandomHeightCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateProbabilityLabels()));
 connect(d->mChangeUpCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateProbabilityLabels()));
 connect(d->mChangeDownCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateProbabilityLabels()));
 connect(d->mChangeBy, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateProbabilityLabels()));
 connect(d->mRandomMountainCount, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpdateProbabilityLabels()));

 d->mRandomHeightCount->setValue(70.0f);
 d->mChangeUpCount->setValue(1.0f);
 d->mChangeDownCount->setValue(1.0f);
 d->mChangeBy->setValue(0.5f);
 d->mRandomMountainCount->setValue(1000.0f);


 BoUfoPushButton* apply = new BoUfoPushButton(i18n("Apply"));
 box->addWidget(apply);
 connect(apply, SIGNAL(signalClicked()), this, SLOT(slotApply()));

 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 box->addWidget(stretch);

}

EditorRandomMapWidget::~EditorRandomMapWidget()
{
 delete d->mRandom;
 delete d;
}

void EditorRandomMapWidget::slotApply()
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(canvas());
 BosonMap* map = canvas()->map();
 BO_CHECK_NULL_RET(map);

 BosonLocalPlayerInput* input = 0;
 input = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 BO_CHECK_NULL_RET(input);
 boDebug() << k_funcinfo << endl;

 QValueList< QPair<QPoint, bofixed> > heights;
 createHeights(&heights);

 boDebug() << k_funcinfo << "new heights calculated. sending..." << endl;
 input->changeHeight(heights);
 boDebug() << k_funcinfo << "sending completed. new values will be applied soon (asynchronously)." << endl;


 boDebug() << k_funcinfo << "done" << endl;
}


void EditorRandomMapWidget::createHeights(QValueList< QPair<QPoint, bofixed> >* heights)
{
 BO_CHECK_NULL_RET(canvas());
 BosonMap* realMap = canvas()->map();
 BO_CHECK_NULL_RET(realMap);

 MyMap map(realMap);
 map.resetHeights();

 const int cornerWidth = map.cornerWidth();
 const int cornerHeight = map.cornerHeight();

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
 queue.append(QPoint(0, 0));
 while (!queue.isEmpty()) {
	QPoint p = queue.front();
	queue.pop_front();

	if (p.x() <= p.y() && p.y() + 1 < cornerHeight) {
		queue.append(QPoint(p.x(), p.y() + 1));
	}
	if (p.x() == p.y()) {
		if (p.x() + 1 < cornerWidth && p.y() + 1 < cornerHeight) {
			queue.append(QPoint(p.x() + 1, p.y() + 1));
		}
	}
	if (p.x() >= p.y() && p.x() + 1 < cornerWidth) {
		queue.append(QPoint(p.x() + 1, p.y()));
	}

	float hmax = 0.0;
	float hmin = 0.0;
	float havg = 0.0;
	int c = 0;
	float h = 0.0;
	if (p.x() < p.y()) {
		if (p.x() > 0) {
			h = map.heightAtCorner(p.x() - 1, p.y());
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
		if (p.x() > 0 && p.y() > 0) {
			h = map.heightAtCorner(p.x() - 1, p.y() - 1);
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
		if (p.y() > 0) {
			h = map.heightAtCorner(p.x(), p.y() - 1);
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
	} else if (p.x() == p.y()) {
		if (p.x() > 0 && p.y() > 0) {
			h = map.heightAtCorner(p.x() - 1, p.y() - 1);
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
		if (p.x() > 0) {
			h = map.heightAtCorner(p.x() - 1, p.y());
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
	} else {
		if (p.x() > 0) {
			h = map.heightAtCorner(p.x() - 1, p.y());
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
		if (p.x() > 0 && p.y() > 0) {
			h = map.heightAtCorner(p.x() - 1, p.y() - 1);
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
		if (p.x() > 0 && p.y() < cornerHeight) {
			h = map.heightAtCorner(p.x() - 1, p.y() + 1);
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
		if (p.y() < cornerHeight) {
			h = map.heightAtCorner(p.x(), p.y() + 1);
			havg += h;
			hmax = QMAX(hmax, h);
			hmin = QMIN(hmin, h);
			c++;
		}
	}
	if (c == 0) {
		havg = 0.0f;
	} else {
		havg /= (float)c; // average height in surrounding cells that have already been visited by this algorithm
	}

//	h = hmax;
	h = havg;

	int r = d->mRandom->getLong(randomHeightCount);
	if (r < changeUpCount) {
		h += changeBy;
	} else if (r < changeUpCount + changeDownCount) {
		h -= changeBy;
	}
	map.setHeightAtCorner(p.x(), p.y(), h);

	r = d->mRandom->getLong(lrint(d->mRandomMountainCount->value()));
	if (r == 0) {
		map.setStartMountainAtCorner(p.x(), p.y(), true);
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
	int endX = QMIN(p.x() + radius, cornerWidth - 1);
	int startY = QMAX(0, p.y() - radius);
	int endY = QMIN(p.y() + radius, cornerHeight - 1);
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

 for (int x = 0; x < map.cornerWidth(); x++) {
	for (int y = 0; y < map.cornerHeight(); y++) {
		QPoint p(x, y);
		bofixed height = map.heightAtCorner(x, y);

		heights->append(QPair<QPoint, bofixed>(p, height));
	}
 }

 for (int x = 0; x < map.cornerWidth(); x++) {
	for (int y = 0; y < map.cornerHeight(); y++) {
		QPoint p(x, y);
		bofixed height = map.heightAtCorner(x, y);

		heights->append(QPair<QPoint, bofixed>(p, height));
	}
 }
}

void EditorRandomMapWidget::slotUpdateProbabilityLabels()
{
 if (lrint(d->mRandomHeightCount->value() < lrint(d->mChangeUpCount->value() + d->mChangeDownCount->value()))) {
	d->mHeightProbabilities->setText(i18n("Invalid values (height count must be <= changeUpCount + changeDownCount)"));
	return;
 }
 float changeBy = d->mChangeBy->value();
 float probUp = d->mChangeUpCount->value() / d->mRandomHeightCount->value();
 float probDown = d->mChangeDownCount->value() / d->mRandomHeightCount->value();
 float probMountain = 0.0f;
 if (lrint(d->mRandomMountainCount->value()) > 0) {
	probMountain = 1.0f / d->mRandomMountainCount->value();
 }
 d->mHeightProbabilities->setText(i18n(
			 	"Prob(Change height UP by %1 at corner) = %2\n"
			 	"Prob(Change height DOWN by %3 at corner) = %4\n"
			 	"Prob(Start mountain at corner) = %5")
				.arg(changeBy)
				.arg(probUp)
				.arg(changeBy)
				.arg(probDown)
				.arg(probMountain));
}

