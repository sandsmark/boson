

#include "unit.h"
#include "unitproperties.h"
#include "player.h"
#include "speciestheme.h"

#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertyhandler.h>

#include <kdebug.h>

#include <qrect.h>
#include <qptrlist.h>


class UnitPrivate
{
public:
	UnitPrivate()
	{
		mOwner = 0;
	}

	Player* mOwner;
	QString mName;//note: do NOT send over network (i18n)!

	KGamePropertyHandler mProperties;

	KGameProperty<unsigned long int> mHealth;
	KGameProperty<unsigned long int> mArmor;
	KGameProperty<unsigned long int> mShields;
	KGameProperty<unsigned long int> mId; // is a KGameProperty clever here?
	KGameProperty<unsigned long int> mCost;
	KGameProperty<unsigned long int> mRange;
	KGameProperty<long int> mDamage; // can also repair (negative value)
	KGameProperty<double> mSpeed;
	KGameProperty<unsigned int> mReload;

	KGameProperty<int> mType; // *only* touched on construction (at least currently ;))
	KGamePropertyInt mWork;
};


Unit::Unit(int type)
{
 d = new UnitPrivate;

// PolicyLocal?
 d->mHealth.registerData(IdHealth, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Health");
 d->mArmor.registerData(IdArmor, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Armor");
 d->mShields.registerData(IdShields, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Shields");
 d->mId.registerData(IdId, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "ID"); // perhaps use dataHandler()->id() instead
 d->mCost.registerData(IdCost, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Cost");
 d->mType.registerData(IdType, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Type");
 d->mWork.registerData(IdWork, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Work");
 d->mSpeed.registerData(IdSpeed, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Speed");
 d->mDamage.registerData(IdDamage, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Damage");
 d->mRange.registerData(IdRange, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Range");
 d->mReload.registerData(IdReload, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "ReloadTime");


 d->mWork.setLocal((int)WorkNone);
 d->mType.setLocal((int)type);
 d->mHealth.setLocal(0); // initially destroyed
 d->mShields.setLocal(0); // doesn't have any shields
 d->mArmor.setLocal(0); // doesn't have any armor
 d->mId.setLocal(0);
 d->mCost.setLocal(0);
 d->mDamage.setLocal(0);
 d->mRange.setLocal(0);
 d->mReload.setLocal(0);
}

Unit::~Unit()
{
// kdDebug() << "~Unit" << endl;
 dataHandler()->clear();
 delete d;
// kdDebug() << "~Unit done" << endl;
}

const QString& Unit::name() const
{ // FIXME: remove
 if (!unitProperties()) {
	return QString::null;
 }
 return unitProperties()->name();
}

unsigned long int Unit::health() const
{
 return d->mHealth;
}

void Unit::setHealth(unsigned long int h)
{
 d->mHealth = h;
}

unsigned long int Unit::shields() const
{
 return d->mShields;
}

unsigned long int Unit::armor() const
{
 return d->mArmor;
}

void Unit::setId(unsigned long int id)
{
 d->mId = id;
}

unsigned long int Unit::id() const
{
 return d->mId;
}

unsigned long int Unit::cost() const
{
 return d->mCost;
}

void Unit::setCost(unsigned long int c)
{
 d->mCost = c;
}

void Unit::setArmor(unsigned long int a)
{
 d->mArmor = a;
}

void Unit::setShields(unsigned long int s)
{
 d->mShields = s;
}

Player* Unit::owner() const
{
 return d->mOwner;
}

KGamePropertyHandler* Unit::dataHandler() const
{
 return &d->mProperties;
}

int Unit::type() const
{
 return d->mType.value();
}

void Unit::setOwner(Player* owner)
{
 d->mOwner = owner;
}

void Unit::setWork(WorkType work)
{
 d->mWork = (int)work;
}

Unit::WorkType Unit::work() const
{
 return (WorkType)d->mWork.value();
}

void Unit::setSpeed(double speed)
{
 d->mSpeed = speed;
}

double Unit::speed() const
{
 return d->mSpeed;
}

long int Unit::damage() const
{
 return d->mDamage;
}

void Unit::setDamage(long int damage)
{
 d->mDamage = damage;
}

unsigned long int Unit::range() const
{
 return d->mRange;
}

void Unit::setRange(unsigned long int range)
{
 d->mRange= range;
}

void Unit::setReload(unsigned int reload)
{
 d->mReload = reload;
}

unsigned int Unit::reload() const
{
 return d->mReload;
}

bool Unit::save(QDataStream& stream)
{
 bool ret = dataHandler()->save(stream);
 return ret;
}

bool Unit::load(QDataStream& stream)
{
 bool ret = dataHandler()->load(stream);
 return ret;
}

const UnitProperties* Unit::unitProperties() const
{
 Player* o = owner();
 if (!o) {
	kdError() << "Unit::unitProperties(): NULL owner" << endl;
	return 0;
 }
 SpeciesTheme* theme = o->speciesTheme();
 if (!theme) {
	kdError() << "Unit::unitProperties(): NULL theme" << endl;
	return 0;
 }
 return theme->unitProperties(type());
}

SpeciesTheme* Unit::speciesTheme() const
{
 if (!owner()) {
	kdWarning() << "Unit::speciesTheme(): NULL owner" << endl;
	return 0;
 }
 return owner()->speciesTheme();
}

bool Unit::isFacility() const
{
 return unitProperties()->isFacility();
}

bool Unit::isMobile() const
{
 return unitProperties()->isMobile();
}
