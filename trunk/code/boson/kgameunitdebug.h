#ifndef __KGAMEUNITDEBUG_H__
#define __KGAMEUNITDEBUG_H__

#include <qwidget.h>

class Boson;
class KGameUnitDebugPrivate;
class VisualUnit;

class KGameUnitDebug : public QWidget
{
	Q_OBJECT
public:
	KGameUnitDebug(QWidget* parent);
	~KGameUnitDebug();

	void setBoson(Boson*);

protected:
	void addUnit(VisualUnit* unit);

protected slots:
	void slotUpdate();

private:
	KGameUnitDebugPrivate* d;
};

#endif
