#ifndef __KSPRITETOOLTIP_H__
#define __KSPRITETOOLTIP_H__

#include <qtooltip.h>

class QCanvasView;
class QCanvasItem;

class KSpriteToolTipPrivate;
class KSpriteToolTip : public QToolTip
{
public:
	KSpriteToolTip(QCanvasView* v);
	virtual ~KSpriteToolTip();

	void add(int rtti, const QString& tip);
	void add(QCanvasItem* item, const QString& tip);
	
protected:
	virtual void maybeTip(const QPoint& pos);

private:
	QCanvasView* mView;
	KSpriteToolTipPrivate* d;
};

#endif
