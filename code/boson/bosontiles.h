#ifndef __BOSONTILES_H__
#define __BOSONTILES_H__

#include "cell.h"

#include <qpixmap.h>

class BosonTiles : public QPixmap
{
public:
	BosonTiles(const QString& fileName);
	~BosonTiles();

	QPixmap plainTile(Cell::GroundType type);

	QPixmap big1(int bigNo, Cell::TransType trans, bool inverted); // bigNo = 0..4

	QPixmap big2(int bigNo, Cell::TransType trans, bool inverted); // bigNo = 0..4

	QPixmap small(int smallNo, Cell::TransType trans, bool inverted);
	
	// call this like the original fillGroundPixmap() in editorTopLevel.cpp
	QPixmap tile(int g);

protected:
	int big_w() const;
	int big_x(int g) const;
	int big_y(int g) const;
};

#endif
