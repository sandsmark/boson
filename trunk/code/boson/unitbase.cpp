

#include "unitbase.h"

#include "unitproperties.h"
#include "player.h"
#include "speciestheme.h"

#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertyhandler.h>

#include <kdebug.h>

#include <qrect.h>
#include <qptrlist.h>


class UnitBasePrivate
{
public:
	UnitBasePrivate()
	{
	}

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


UnitBase::UnitBase(int type)
{
 d = new UnitBasePrivate;
 mOwner = 0;

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

UnitBase::~UnitBase()
{
// kdDebug() << k_funcinfo << endl;
 dataHandler()->clear();
 delete d;
// kdDebug() << k_funcinfo << " done" << endl;
}

const QString& UnitBase::name() const
{ // FIXME: remove
 if (!unitProperties()) {
	return QString::null;
 }
 return unitProperties()->name();
}

unsigned long int UnitBase::health() const
{
 return d->mHealth;
}

void UnitBase::setHealth(unsigned long int h)
{
 d->mHealth = h;
}

unsigned long int UnitBase::shields() const
{
 return d->mShields;
}

unsigned long int UnitBase::armor() const
{
 return d->mArmor;
}

void UnitBase::setId(unsigned long int id)
{
 d->mId = id;
}

unsigned long int UnitBase::id() const
{
 return d->mId;
}

unsigned long int UnitBase::cost() const
{
 return d->mCost;
}

void UnitBase::setCost(unsigned long int c)
{
 d->mCost = c;
}

void UnitBase::setArmor(unsigned long int a)
{
 d->mArmor = a;
}

void UnitBase::setShields(unsigned long int s)
{
 d->mShields = s;
}

KGamePropertyHandler* UnitBase::dataHandler() const
{
 return &d->mProperties;
}

int UnitBase::type() const
{
 return d->mType.value();
}

void UnitBase::setOwner(Player* owner)
{
 mOwner = owner;
}

void UnitBase::setWork(WorkType work)
{
 d->mWork = (int)work;
}

UnitBase::WorkType UnitBase::work() const
{
 return (WorkType)d->mWork.value();
}

void UnitBase::setSpeed(double speed)
{
 d->mSpeed = speed;
}

double UnitBase::speed() const
{
 return d->mSpeed;
}

long int UnitBase::damage() const
{
 return d->mDamage;
}

void UnitBase::setDamage(long int damage)
{
 d->mDamage = damage;
}

unsigned long int UnitBase::range() const
{
 return d->mRange;
}

void UnitBase::setRange(unsigned long int range)
{
 d->mRange= range;
}

void UnitBase::setReload(unsigned int reload)
{
 d->mReload = reload;
}

unsigned int UnitBase::reload() const
{
 return d->mReload;
}

bool UnitBase::save(QDataStream& stream)
{
 bool ret = dataHandler()->save(stream);
 return ret;
}

bool UnitBase::load(QDataStream& stream)
{
 bool ret = dataHandler()->load(stream);
 return ret;
}

const UnitProperties* UnitBase::unitProperties() const
{
 SpeciesTheme* theme = speciesTheme();
 if (!theme) {
	kdError() << k_funcinfo << ": NULL theme" << endl;
	return 0;
 }
 return theme->unitProperties(type());
}

SpeciesTheme* UnitBase::speciesTheme() const
{
 if (!owner()) {
	kdWarning() << k_funcinfo << ": NULL owner" << endl;
	return 0;
 }
 return owner()->speciesTheme();
}

bool UnitBase::isFacility() const
{
 return unitProperties()->isFacility();
}

bool UnitBase::isMobile() const
{
 return unitProperties()->isMobile();
}
