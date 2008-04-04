/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONCANVASSTATISTICS_H
#define BOSONCANVASSTATISTICS_H

class BosonCanvas;

template<class T> class QPtrList;
template<class T> class QValueList;
template<class T> class QPtrVector;
template<class T1, class T2> class QMap;


class BosonCanvasStatisticsPrivate;

/**
 * This class provides statistics about a @ref BosonCanvas object (such as item
 * counts).
 **/
class BosonCanvasStatistics
{
public:
	BosonCanvasStatistics(BosonCanvas* parent);
	~BosonCanvasStatistics();

	/**
	 * @return @ref BosonCanvas::width
	 **/
	unsigned int mapHeight() const;

	/**
	 * @return @ref BosonCanvas::height
	 **/
	unsigned int mapWidth() const;

	/**
	 * WARNING: you must call @ref updateItemCount before calling this!
	 * Otherwise you'll get old results!
	 * @return The number of items on the canvas with @p rtti. See @ref
	 * BosonItem::rtti. Note that <em>all</em> units get added to @p rtti =
	 * @ref RTTI::UnitStart!
	 **/
	unsigned int itemCount(int rtti) const;

	/**
	 * Update the values for @ref itemCount. This function iterates over all
	 * items and therefore might take a little bit time. Don't call it when
	 * you render frames or so!
	 **/
	void updateItemCount();

	unsigned int allItemsCount() const;

	/**
	 * Increase the number work-count for @p work by @p by. For non-Unit items
	 * (which therefore do no have a "work") you can use -1 for  @p work.
	 *
	 * See also @ref workCounts and @ref resetWorkCounts
	 **/
	void increaseWorkCountBy(int work, int by);

	/**
	 * This is a counter of all work kinds (see @ref Unit::advanceWork) of
	 * an advance call. Whenever a @ref Unit is advanced, it's work is
	 * counted (see @ref increaseWorkCount). Here you can retrieve the sum
	 * of the previous advance call.
	 * @return A map with all work counts. -1 is the dummy work-count,
	 * which is used for non-unit items (i.e. those that have no kind of
	 * work).
	 **/
	QMap<int, int>* workCounts() const;

	/**
	 * Reset all (!) work-counts to 0. This includes the "dummy" work -1.
	 *
	 * See also @ref increaseWorkCount and @ref workCounts
	 **/
	void resetWorkCounts();

private:
	BosonCanvasStatisticsPrivate* d;
	BosonCanvas* mCanvas;
};

#endif
