#ifndef __BOSONMINIMAP_H__
#define __BOSONMINIMAP_H__

#include <qwidget.h>

class Player;
class Unit;
class BosonMap;

class QPixmap;
class QPainter;
class QPaintEvent;
class QMouseEvent;

class BosonMiniMapPrivate;

class BosonMiniMap : public QWidget
{
	Q_OBJECT
public:
	BosonMiniMap(QWidget* parent);
	~BosonMiniMap();

	QPixmap* ground() const;

	int mapWidth() const;
	int mapHeight() const;

	void setMap(BosonMap* map);
	void initMap();
	

signals:
	void signalReCenterView(const QPoint& pos);

public slots:
	void slotCreateMap(int w, int h);
	/**
	  * @param x The x - coordinate of the cell
	  * @param y The x - coordinate of the cell
	  * @param groundType The type of the cell. See @ref Cell::GroundType
	  * @param b Unused
	  **/
	void slotAddCell(int x, int y, int groundType, unsigned char b);
	void slotAddUnit(Unit* unit, int x, int y);

	void slotMoveRect(int x, int y);
	void slotResizeRect(int w, int h);

	void slotMoveUnit(Unit* unit, double oldX, double oldY);
	void slotUnitDestroyed(Unit* unit);

protected:
	void setPoint(int x, int y, const QColor& color);
	virtual void paintEvent(QPaintEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	
private:
	BosonMiniMapPrivate* d;
};
#endif
