
#include "bosoncommandframe.h"

#include "visualunit.h"
#include "player.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosonunitview.h"

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qintdict.h>
#include <qsignalmapper.h>
#include <qpixmap.h>
#include <qscrollview.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include "defines.h"

#include "bosoncommandframe.moc"

#include "dunno.h" // FIXME

#define ORDERS_PER_COLUMN 3
#define ORDER_SPACING 3

class BosonCommandTiles : public QPixmap
{
public:
	BosonCommandTiles(const QString& fileName) : QPixmap(fileName)
	{
	}
	~BosonCommandTiles()
	{
	}
	enum Transition {
		TransUpLeft = 0,
		TransUpRight,
		TransDownLeft,
		TransDownRight,

		TransUp,
		TransDown,
		TransLeft,
		TransRight,

		TransUpLeftInverted,
		TransUpRightInverted,
		TransDownLeftInverted,
		TransDownRightInverted
	};

	QPixmap plainTile(Cell::GroundType type)
	{
		if (type <= Cell::GroundUnknown || type >= Cell::GroundLast) {
			kdError() << "invalid groundtype " << (int)type << endl;
			return QPixmap();
		}
		return tile((int)type);
	}

	QPixmap big1(int bigNo, Dunno::TransType trans, bool inverted) // bigNo = 0..4
	{
		return tile(Dunno::getBigTransNumber(trans, (inverted ? 4 : 0) + bigNo));
	}

	QPixmap big2(int bigNo, Dunno::TransType trans, bool inverted) // bigNo = 0..4
	{
		return tile(Dunno::getBigTransNumber(trans, (inverted ? 12 : 8) + bigNo));
	}

	QPixmap small(int smallNo, Dunno::TransType trans, bool inverted)
	{
		switch (smallNo) {
			case 0:
				return tile(Dunno::getTransNumber(trans,
						inverted ? TransUpLeftInverted
						: TransUpLeft));
			case 1:
				return tile(Dunno::getTransNumber(trans,
						inverted ? TransDown
						: TransUp));
			case 2:
				return tile(Dunno::getTransNumber(trans,
						inverted ? TransUpRightInverted
						: TransUpRight));
			case 3:
				return tile(Dunno::getTransNumber(trans,
						inverted ? TransRight
						: TransLeft));
			case 4:
				return tile(Dunno::getTransNumber(trans,
						inverted ? to(trans)
						: from(trans)));
			case 5:
				return tile(Dunno::getTransNumber(trans,
						inverted ? TransLeft
						: TransRight));
			case 6:
				return tile(Dunno::getTransNumber(trans,
						inverted ? TransDownLeftInverted
						: TransDownLeft));
			case 7:
				return tile(Dunno::getTransNumber(trans,
						inverted ? TransUp
						: TransDown));
			case 8:
				return tile(Dunno::getTransNumber(trans,
						inverted ? TransDownRightInverted
						: TransDownRight));
			default:
				kdError() << "Unknwon small tile " << smallNo << endl;
				return QPixmap();
		}
	}
	
	// call this like the original fillGroundPixmap() in editorTopLevel.cpp
	QPixmap tile(int g)
	{
		QPixmap p;
		if (Dunno::isBigTrans(g)) {
			p.resize(2 * BO_TILE_SIZE, 2 * BO_TILE_SIZE);
		} else {
			p.resize(BO_TILE_SIZE, BO_TILE_SIZE);
		}

		g<<=2;

		bitBlt(&p, 0, 0, this, big_x(g), big_y(g), BO_TILE_SIZE, BO_TILE_SIZE);

		// a big tile is 2*2 normal tiles - the upper left was painted
		// above. The following will paint the remaining 3 rects
		if (Dunno::isBigTrans(g>>2)) {
			g+=4;
			bitBlt(&p, BO_TILE_SIZE, 0, this, big_x(g), big_y(g),
					BO_TILE_SIZE, BO_TILE_SIZE);
			g+=4;
			bitBlt(&p, 0, BO_TILE_SIZE, this, big_x(g), big_y(g),
					BO_TILE_SIZE, BO_TILE_SIZE);
			g+=4;
			bitBlt(&p, BO_TILE_SIZE, BO_TILE_SIZE, this, big_x(g),
			big_y(g), BO_TILE_SIZE, BO_TILE_SIZE);
		}

		return p;

	}

protected:
	int big_w() const { return 32; }
	int big_x(int g) const { return ((g % big_w()) * BO_TILE_SIZE); }
	int big_y(int g) const { return ((g / big_w()) * BO_TILE_SIZE); }

	Cell::GroundType from(Dunno::TransType trans) const
	{
		switch (trans) {
			case Dunno::TransGrassWater:
				return Cell::GroundGrass;
			case Dunno::TransGrassDesert:
				return Cell::GroundGrass;
			case Dunno::TransDesertWater:
				return Cell::GroundDesert;
			case Dunno::TransDeepWater:
				return Cell::GroundDeepWater;
			default:
				kdError() << "Unknown trans " << (int)trans << endl;
				return Cell::GroundUnknown;
		}
	}

	Cell::GroundType to(Dunno::TransType trans) const
	{
		switch (trans) {
			case Dunno::TransGrassWater:
				return Cell::GroundWater;
			case Dunno::TransGrassDesert:
				return Cell::GroundDesert;
			case Dunno::TransDesertWater:
				return Cell::GroundWater;
			case Dunno::TransDeepWater:
				return Cell::GroundWater;
			default:
				kdError() << "Unknown trans " << (int)trans
						<< endl;
				return Cell::GroundUnknown;
		}
	}

};

class OrderScrollView : public QScrollView
{
//	Q_OBJECT
public:
	OrderScrollView(QWidget* parent = 0)  : QScrollView(parent) 
	{
		setVScrollBarMode(QScrollView::AlwaysOn);
	}

	~OrderScrollView() {}
	
protected:

};

class BosonCommandFramePrivate
{
public:
	BosonCommandFramePrivate()
	{
		mScrollView = 0;
		mOrderMap = 0;
		mOrderLayout = 0;
		mOrderWidget = 0;

		mUnitView = 0;

		mTiles = 0;

		mTransRef = 0;
		mInverted = 0;

		mOwner = 0;
	}

	QIntDict<QPushButton> mOrderButton;
	QWidget* mOrderWidget;
	OrderScrollView* mScrollView;
	QSignalMapper* mOrderMap;

	QVBoxLayout* mOrderLayout;
	QPtrList<QHBoxLayout> mHOrderLayoutList;

	BosonUnitView* mUnitView;

	BosonCommandTiles* mTiles;

	QComboBox* mTransRef;
	QCheckBox* mInverted;

	BosonCommandFrame::OrderType mOrderType; // plain tiles, facilities, mob units, ...

	Player* mOwner;
};

BosonCommandFrame::BosonCommandFrame(QWidget* parent, bool editor) : QFrame(parent)
{
 init();
 if (editor) {
	initEditor();
 }

 QVBoxLayout* layout = new QVBoxLayout(this);

 d->mUnitView = new BosonUnitView(this);
 layout->addWidget(d->mUnitView, 0);

 if (d->mTransRef) {
	layout->addWidget(d->mTransRef);
 }
 if (d->mInverted) {
	layout->addWidget(d->mInverted);
 }


// the order buttons
 d->mScrollView = new OrderScrollView(this);
 layout->addWidget(d->mScrollView, 1);
 d->mOrderWidget = new QWidget(d->mScrollView->viewport());
 d->mScrollView->addChild(d->mOrderWidget);
// d->mOrderWidget->setMinimumWidth(d->mScrollView->viewport()->width()); // might cause problems if scrollview is resized. maybe subclass QScrollView
 
// d->mOrderLayout = new QVBoxLayout(d->mScrollView->viewport(), ORDER_SPACING, ORDER_SPACING);
 d->mOrderLayout = new QVBoxLayout(d->mOrderWidget, 0, ORDER_SPACING);
 

 d->mOrderMap = new QSignalMapper(d->mOrderWidget);
 connect(d->mOrderMap, SIGNAL(mapped(int)), this, SLOT(slotHandleOrder(int)));

}

void BosonCommandFrame::init()
{
 d = new BosonCommandFramePrivate;

 setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
 setMinimumSize(230, 200); // FIXME hardcoded value

 setFrameStyle(QFrame::Raised | QFrame::Panel);
 setLineWidth(5);

}

void BosonCommandFrame::initEditor()
{
 d->mTransRef = new QComboBox(this);
 connect(d->mTransRef, SIGNAL(activated(int)), this, SLOT(slotRedrawTiles()));
 d->mTransRef->insertItem(i18n("Grass/Water"), (int)Dunno::TransGrassWater);
 d->mTransRef->insertItem(i18n("Grass/Desert"), (int)Dunno::TransGrassDesert);
 d->mTransRef->insertItem(i18n("Desert/Water"), (int)Dunno::TransDesertWater);
 d->mTransRef->insertItem(i18n("Deep Water"), (int)Dunno::TransDeepWater);

 d->mInverted = new QCheckBox(this);
 d->mInverted->setText(i18n("Invert"));
 connect(d->mInverted, SIGNAL(toggled(bool)), this, SLOT(slotRedrawTiles()));

}

BosonCommandFrame::~BosonCommandFrame()
{
 d->mHOrderLayoutList.clear();
 if (d->mTiles) {
	delete d->mTiles;
 }
 delete d;
}

void BosonCommandFrame::initOrderButtons(unsigned int no)
{
 if (d->mOrderButton.count() >= no) {
	return;
 }
 for (unsigned int i = 0; i < no; i++) {
	if (!d->mOrderButton[i]) {
		QHBoxLayout* h = 0;
		if (d->mHOrderLayoutList.count() > i / ORDERS_PER_COLUMN) {
			h = d->mHOrderLayoutList.at(i / ORDERS_PER_COLUMN);
		} else {
			h = new QHBoxLayout(d->mOrderLayout, ORDER_SPACING);
			d->mHOrderLayoutList.append(h);
		}
		QPushButton* b = new QPushButton(d->mOrderWidget);
		h->addWidget(b);
		b->hide();
		d->mOrderButton.insert(i, b);
		connect(b, SIGNAL(clicked()), d->mOrderMap, SLOT(map()));
		d->mOrderMap->setMapping(b, i);
	}
 }
}


void BosonCommandFrame::slotShowSingleUnit(VisualUnit* unit)
{
 if (!unit) {
	kdError() << "BosonCommandFrame::slotShowSingleUnit: NULL unit "
			<< "selected" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	kdWarning() << "BosonCommandFrame::slotShowSingleUnit: unit is "
			<< "destroyed" << endl;
	return;
 }
 if (!unit->owner()) {
	kdError() << "BosonCommandFrame::slotUnitSelected: unit has no owner" 
			<< endl;
	return;
 }
 SpeciesTheme* theme = unit->owner()->speciesTheme();
 if (!theme) {
	kdError() << "BosonCommandFrame::slotUnitSelected: owner has no "
			<< "species theme" << endl;
	return;
 }
 QPixmap* p = theme->bigOverview(unit->type());
 if (!p) {
	kdError() << "BosonCommandFrame::slotUnitSelected: unit has no big"
			<< "overview in this theme" << endl;
	return;
 }

 d->mUnitView->setUnit(unit);
}

void BosonCommandFrame::slotSetConstruction(VisualUnit* unit)
{
 if (!unit) {
	kdError() << "EditorCommandFrame::slotSetConstruction(): NULL unit"
			<< endl;
	return;
 }
 Player* owner = unit->owner();
 if (!owner) {
	kdError() << "BosonComandFrame::slotSetConstruction(): no owner" << endl;
	return;
 }

 // don't display construction items of units of other players. TODO: must be
 // fixed!
 if (owner->isVirtual()) { // a virtual player is a not-local player. FIXME: computer player isn't virtual!
	kdDebug() << "BosonCommandFrame::slotSetConstruction(): we are not the owner (FIXME)" << endl;
	return;
 }
 setUnitOrders(unit->type(), owner);
}

void BosonCommandFrame::hideOrderButtons()
{
 QIntDictIterator<QPushButton> it(d->mOrderButton);
 while (it.current()) {
	it.current()->hide();
	++it;
 }
}

void BosonCommandFrame::setOrderPixmap(unsigned int id, const QPixmap& p)
{
 if (!d->mOrderButton[id]) {
	kdError() << "cannot find order button " << id << endl;
	return;
 }
 d->mOrderButton[id]->setPixmap(p);
 d->mOrderButton[id]->setFixedSize(d->mOrderButton[id]->sizeHint());
 d->mOrderButton[id]->show();
}

void BosonCommandFrame::setOrderTooltip(unsigned int id, const QString& text)
{
 if (!d->mOrderButton[id]) {
	kdError() << "cannot find order button " << id << endl;
	return;
 }
 QToolTip::remove(d->mOrderButton[id]);
 QToolTip::add(d->mOrderButton[id], text);
}

void BosonCommandFrame::setUnitOrders(int unitType, Player* owner)
{
 hideOrderButtons();
 if (!owner) {
	kdError() << "setUnitOrders(): NULL owner" << endl;
	return;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	kdError() << "setUnitOrders(): NULL species theme" << endl;
	return;
 }
 const UnitProperties* prop = theme->unitProperties(unitType);
 if (!prop) {
	kdError() << "setUnitOrders(): NULL unitProperties" << endl;
	return;
 }
 if (!prop->canProduce()) {
	return;
 }
 QValueList<int> produceList = prop->produceList();
 setOrderButtons(produceList, owner);
}

void BosonCommandFrame::setOrderButtons(QValueList<int> produceList, Player* owner)
{
 hideOrderButtons();
 initOrderButtons(produceList.count());
 for (unsigned int i = 0; i < produceList.count(); i++) {
	setOrderButton(i, produceList[i], owner);
 }
 
}

void BosonCommandFrame::setOrderButton(unsigned int button, int unitType, Player* owner)
{
 if (!owner) {
	kdError() << "BosonComandFrame::setOrderButton(): no owner" << endl;
	return;
 }
 if (!owner->speciesTheme()) {
	kdError() << "BosonCommandFrame::setOrderButton(): player has no "
			<< " species theme" << endl;
	return;
 }

 QPixmap* small = owner->speciesTheme()->smallOverview(unitType);
 if (!small) {
	kdError() << "BosonCommandFrame::setOrderButton(): cannot find small "
			<< "overview for " << unitType << endl;
	return;
 }
 setOrderPixmap(button, *small);

 const UnitProperties* prop = owner->speciesTheme()->unitProperties(unitType);
 if (!prop) {
	kdError() << "No unit properties for " << unitType << endl;
	return;
 }
 setOrderTooltip(button, prop->name());
}

void BosonCommandFrame::slotHandleOrder(int index)
{
 switch (d->mOrderType) {
	case PlainTiles:
		break;
	case Small:
		break;
	case Big1:
		break;
	case Big2:
		break;
	case Facilities:
		#warning Unit::facilityType is obsolete
//		emit signalUnitSelected((int)Unit::facilityType(index), 0, d->mOwner);
		break;
	case Mobiles:
		#warning Unit::mobileType is obsolete
//		emit signalUnitSelected((int)Unit::mobileType(index), 0, d->mOwner);
		break;
	default:
		kdError() << "unexpected construction index " << index << endl;
		break;
 }
}

void BosonCommandFrame::slotEditorConstruction(int index, Player* owner)
{
 if (index == -1) {
	hideOrderButtons();
 }
 if (!owner) {
	kdError() << "NULL owner" << endl;
	return;
 }
 SpeciesTheme* theme = owner->speciesTheme();
 if (!theme) {
	kdError() << "NULL theme" << endl;
	return;
 }
 d->mOrderType = (OrderType)index;
 switch (d->mOrderType) {
	case PlainTiles:
	case Small:
	case Big1:
	case Big2:
		slotRedrawTiles(); // rename: redrawtiles
		break;
	case Mobiles:
		setOrderButtons(theme->allMobiles(), owner);
		break;
	case Facilities:
		setOrderButtons(theme->allFacilities(), owner);
		break;
	default:
		kdError() << "BosonCommandFrame::slotEditorConstruction(): "
				<< "Invalid index " << index << endl;
		return;
 }
}

void BosonCommandFrame::slotEditorLoadTiles(const QString& fileName)
{
 QString themePath = locate("data", QString("boson/themes/grounds/%1").arg(fileName));
 d->mTiles = new BosonCommandTiles(themePath);
 if (d->mTiles->isNull()) {
	kdError() << "Could not load " << fileName << endl;
	return;
 }
}

void BosonCommandFrame::slotRedrawTiles()
{
 bool inverted = d->mInverted->isChecked();
 Dunno::TransType trans = (Dunno::TransType)d->mTransRef->currentItem();
 // trans is one of TRANS_GW, TRANS_GD, TRANS_DW, TRANS_DWD ans specifies the
 // tile type (desert/water and so on)
 switch (d->mOrderType) {
	case PlainTiles:
		hideOrderButtons();
		initOrderButtons(Cell::GroundLast - 1);
		for (int i = 0; i < 5; i++) {
			QPixmap p = d->mTiles->plainTile((Cell::GroundType)(i + 1));
			setOrderPixmap(i, p);
		}
		break;
	case Small:
		hideOrderButtons();
		initOrderButtons(9);
		for (int i = 0; i < 9; i++) {
			setOrderPixmap(i, d->mTiles->small(i, trans, inverted));
		}
		break;
	case Big1:
		hideOrderButtons();
		initOrderButtons(4);
		for (int i = 0; i < 4; i++) {
			QPixmap p = d->mTiles->big1(i, trans, inverted);
			setOrderPixmap(i, p);
		}
		break;
	case Big2:
		hideOrderButtons();
		initOrderButtons(4);
		for (int i = 0; i < 4; i++) {
			QPixmap p = d->mTiles->big2(i, trans, inverted);
			setOrderPixmap(i, p);
		}
		break;
	case Facilities:
		break;
	case Mobiles:
		break;
	default:
		kdError() << "unexpected construction index " << index << endl;
		break;
 }
}

