#include "bosonunitview.h"

#include "unit.h"
#include "player.h"
#include "unitproperties.h"
#include "speciestheme.h"

#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>

#include "bosonunitview.moc"

#define OVERVIEW_WIDTH 180
#define OVERVIEW_HEIGHT 110

class BosonUnitViewPrivate
{
public:
	BosonUnitViewPrivate()
	{
		mName = 0;
		mHealth = 0;
		mSpeed = 0;
		mOwner = 0;
		mId = 0;

		mOverview = 0;
		mViewNone = 0;
	}

	QLabel* mOverview;
	QPixmap* mViewNone;
	
	QLabel* mName;
	QLabel* mHealth;
	QLabel* mSpeed;
	QLabel* mOwner;
	QLabel* mId; // mostly for debugging
	QLabel* mDamage;
	QLabel* mRange;

};

// this unit view consists of 2 parts 
// - the big overview (aka 'preview') pixmap
// - the actual "unit view" - some QLabels describing the selected unit
BosonUnitView::BosonUnitView(QWidget* parent) : QFrame(parent)
{
 d = new BosonUnitViewPrivate;
 setFrameStyle(QFrame::Raised | QFrame::Panel);
 setLineWidth(5);
// setMinimumWidth(OVERVIEW_WIDTH);
// setMinimumHeight(OVERVIEW_HEIGHT);
 setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
// setFixedSize();//TODO
 QVBoxLayout* topLayout = new QVBoxLayout(this, 5);// FIXME 5 is hardcoded

 d->mViewNone = new QPixmap();
 d->mOverview = new QLabel(this, "preview");
 d->mOverview->setFixedSize(OVERVIEW_WIDTH, OVERVIEW_HEIGHT);
 setOverview(0);

 topLayout->addWidget(d->mOverview, 0, AlignHCenter);// FIXME this seems to be working but the pixmap is not centered in the label :-(
 QGridLayout* layout = new QGridLayout(topLayout, 10, 2);
 
 d->mName = new QLabel(this);
 layout->addMultiCellWidget(d->mName, 0, 0, 0, 1);
 
 d->mHealth = new QLabel(this);
 layout->addWidget(d->mHealth, 1, 0);
 
 d->mSpeed = new QLabel(this);
 layout->addWidget(d->mSpeed, 1, 1);

 d->mOwner = new QLabel(this);
 layout->addWidget(d->mOwner, 2, 0);

 d->mId = new QLabel(this);
 layout->addWidget(d->mId, 2, 1);

 d->mDamage = new QLabel(this);
 layout->addWidget(d->mDamage, 3, 0);

 d->mRange = new QLabel(this);
 layout->addWidget(d->mRange, 3, 0);
 

 setUnit(0);
 show();
}

BosonUnitView::~BosonUnitView()
{
 delete d->mViewNone;
 delete d;
}

void BosonUnitView::setUnit(Unit* unit)
{
 if (!unit) {
	// hide all labels
	hideAll();
	setOverview(0);
	return;
 }
 if (!unit->owner()) {
	kdError() << k_funcinfo << ": no owner" << endl;
	return;
 }
 if (!unit->owner()->speciesTheme()) {
	kdError() << k_funcinfo << ": No speciesTheme" << endl;
	return;
 }
 d->mName->setText(unit->unitProperties()->name());
 d->mHealth->setText(i18n("Health: %1").arg(unit->health()));
 d->mOwner->setText(i18n("Owner: %1").arg(unit->owner()->name()));
 d->mId->setText(i18n("Id : %1").arg(unit->id()));
 d->mDamage->setText(i18n("Damage : %1").arg(unit->damage())); // confusing name: not the damage this unit *has* but the damage the unit *makes*
 d->mRange->setText(i18n("Range: %1").arg(unit->range()));
 showGeneral();

 const UnitProperties* prop = unit->unitProperties();
 if (!prop) {
	kdError() << k_funcinfo << ": NULL unit properties" << endl;
	return;
 }
 if (prop->isMobile()) {
	d->mSpeed->setText(i18n("Speed: %1").arg(unit->speed()));
	showMobile();
	hideFacility();
 } else if (prop->isFacility()) {
	hideMobile();
	showFacility();
 } else {
	kdError() << "Internal error - neither mobile nor fix" << endl;
	return;
 }
 QPixmap* overview = unit->owner()->speciesTheme()->bigOverview(unit->type());
 setOverview(overview);
}

void BosonUnitView::hideAll()
{
 d->mName->hide();
 d->mHealth->hide();
 d->mOwner->hide();
 d->mId->hide();
 d->mRange->hide();
 d->mDamage->hide();

 hideMobile();
 hideFacility();
}

void BosonUnitView::hideMobile()
{ // hide all mobile unit labels
 d->mSpeed->hide();
}

void BosonUnitView::hideFacility()
{ // hide all facility labels
 // currenlty nothing
}

void BosonUnitView::showGeneral()
{
 d->mName->show();
 d->mHealth->show();
 d->mOwner->show();
 d->mId->show();
 d->mRange->show();
 d->mDamage->show();
}

void BosonUnitView::showMobile()
{
 d->mSpeed->show();
}

void BosonUnitView::showFacility()
{
 // currenlty nothing
}

void BosonUnitView::setOverview(QPixmap* p)
{
 if (!p) {
	d->mOverview->setPixmap(*d->mViewNone);
 } else {
	d->mOverview->setPixmap(*p);
 }
}

