#ifndef __BOSONTILES_H__
#define __BOSONTILES_H__

#include "cell.h"

#include <qpixmap.h>

class QImage;

class BosonTiles : public QPixmap
{
public:
	/**
	 * Create an empty BosonTiles object. You want to use this if you are
	 * going to create a pixmap to be used in Boson.
	 *
	 * Use @ref loadTiles to write the single tiles into the pixmap and @ref
	 * save to save the pixmap.
	 **/
	BosonTiles();

	/**
	 * Construct a BosonTiles object from fileName. This loads the already
	 * created pixmap.
	 *
	 * You'll use this c'tor within the map editor and split the pixmap into
	 * several small pixmaps (the cells/tiles themselves) using @ref
	 * plainTile, @ref big1, @ref big2, @ref small and @ref tile.
	 **/
	BosonTiles(const QString& fileName);
	~BosonTiles();

	QPixmap plainTile(Cell::GroundType type);

	QPixmap big1(int bigNo, Cell::TransType trans, bool inverted); // bigNo = 0..4

	QPixmap big2(int bigNo, Cell::TransType trans, bool inverted); // bigNo = 0..4

	QPixmap small(int smallNo, Cell::TransType trans, bool inverted);
	
	// call this like the original fillGroundPixmap() in editorTopLevel.cpp
	QPixmap tile(int g);

	/**
	 * Load tiles from dir. This creates a pixmap which can be used as a
	 * ground in boson from a lot of small pixmaps (tiles).
	 *
	 * Don't use this if youre actualle playing boson but just to create
	 * the pixmap.
	 **/
	bool loadTiles(const QString& dir);

	/**
	 * Save a pixmap created using @ref loadTiles.
	 **/
	bool save(const QString& fileName);

protected:
	static int big_x(int g);
	static int big_y(int g);

	static int big_w();
	static int big_h();

	bool loadGround(int j, const QString& path);
	bool loadTransition(int gt);
	void putOne(int z, QImage& p, int xoffset = 0, int yoffset = 0);

	/**
	 * @return a name (e.g. "desert") for the specified groundType. Note
	 * that this name is necesary for creating the file path of the tiles so
	 * don't change it.
	 **/
	static QString groundType2Name(Cell::GroundType t);
	static QString transition2Name(Cell::TransType t);
	static QString trans_ext(int t);
	
private:
	QImage* mTilesImage; // only used by loadTiles()
};

#endif
