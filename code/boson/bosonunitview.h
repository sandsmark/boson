#ifndef __BOSONUNITVIEW_H__
#define __BOSONUNITVIEW_H__

#include <qframe.h>

class Unit;

class BosonUnitViewPrivate;
class BosonUnitView : public QFrame
{
	Q_OBJECT
public:
	BosonUnitView(QWidget* parent);
	~BosonUnitView();

	/**
	 * @param unit The unit to be shown or 0 for none
	 **/
	void setUnit(Unit* unit);

protected:

        /**
	 * Set the big overview pixmap.
	 * @param p The pixmap to be displayed. 0 for none.
	 **/
	void setOverview(QPixmap* p);


	void hideAll();
	void hideMobile();
	void hideFacility();
	void showGeneral();
	void showMobile();
	void showFacility();

private:
	BosonUnitViewPrivate* d;
};

#endif
