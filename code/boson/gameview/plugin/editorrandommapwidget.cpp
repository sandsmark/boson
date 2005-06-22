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

#include <qtimer.h>
#include <qvaluelist.h>
#include <qpoint.h>

class EditorRandomMapWidgetPrivate
{
public:
	EditorRandomMapWidgetPrivate()
	{
	}
};

EditorRandomMapWidget::EditorRandomMapWidget()
		: BoUfoWidget()
{
 d = new EditorRandomMapWidgetPrivate();
 mCanvas = 0;

 setLayoutClass(UHBoxLayout);

 BoUfoVBox* box = new BoUfoVBox();
 addWidget(box);

 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setStretch(1);
 addWidget(stretch);

 BoUfoLabel* header = new BoUfoLabel(i18n("Random Map generation"));
 box->addWidget(header);

 BoUfoPushButton* apply = new BoUfoPushButton(i18n("Apply"));
 box->addWidget(apply);
 connect(apply, SIGNAL(signalClicked()), this, SLOT(slotApply()));

 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 box->addWidget(stretch);

}

EditorRandomMapWidget::~EditorRandomMapWidget()
{
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
 BosonMap* map = canvas()->map();
 BO_CHECK_NULL_RET(map);

 class HCorner {
	public:
		float h;
 };

 int cornerWidth = map->width() + 1;
 int cornerHeight = map->height() + 1;

 HCorner* corners = new HCorner[(cornerWidth + 1) * (cornerHeight + 1)];
 for (int x = 0; x < cornerWidth; x++) {
	for (int y = 0; y < cornerHeight; y++) {
		corners[x * cornerHeight + y].h = map->heightAtCorner(x, y);
	}
 }

 // do a BFS on all cells
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

	double h = 0.0;
	h += ((double)(p.x() * p.y())) * 100 / ((double)(cornerWidth * cornerHeight));
	bofixed height = h;

	heights->append(QPair<QPoint, bofixed>(p, height));
 }
}

