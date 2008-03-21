/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann <b_mann@gmx.de>

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
#ifndef BOCURRENTINFO_H
#define BOCURRENTINFO_H

#include <qstring.h>

class BoInfo;

/**
 * @short Provide current information about the system
 *
 * While @ref BoInfo is meant to provide static information about a system, such
 * as installed libraries, CPU clock, installed RAM and so on, this class is
 * meant to provide information about dynamic data.
 *
 * The data this class operates on can change at any time and therefore it makes
 * little or no sense to store them in the @ref BoInfo database.
 *
 * This class is primarily meant to be used in-game, whereas @ref BoInfo is
 * primarily meant to provide an information log about the system. This class
 * can be used for example to retrieve the current system load. The game may
 * reduce certain effects, depending on that.
 *
 * Note that in addition to dynamic data, this class can also be used to provide
 * static data that may be useful for in-game use. One example may be the CPU
 * speed.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCurrentInfo
{
public:
	BoCurrentInfo();
	~BoCurrentInfo();

	BoInfo* boInfo() const
	{
		return mInfo;
	}

	/**
	 * Returns the memory that is currently in use in the parameters.
	 *
	 * WARNING: these values can be very confusing, if they are not read
	 * correctly. The most important value is @p vmData, which returns the
	 * size of the data segment of this application. However it does NOT
	 * necessarily return the amount of memory in use! The value depends
	 * heavily on the paging strategy of the operating system.
	 *
	 * If a value is not provided by any reason, the parameter is either
	 * not touched at all, or set to @ref QString::null.
	 *
	 * A NULL pointer parameter is not touched by this method.
	 *
	 * @return TRUE, if the data could be read, FALSE otherwise.
	 **/
	bool memoryInUse(QString* vmSize, QString* vmLck, QString* vmRSS,
			QString* vmData, QString* vmStk, QString* vmExe, QString* vmLib,
			QString* vmPTE) const;

	/**
	 * Returns the number of jiffies that the process has been scheduled.
	 *
	 * Note that this may be very linux dependent - I don't know other
	 * systems good enough to implement an API that might be portable.
	 *
	 * See man 5 proc for the meaning of the parameters.
	 *
	 * @return TRUE if the valeus could be read, otherwise FALSE
	 **/
	bool cpuTime(unsigned long int* utime, unsigned long int* stime, long int* cutime, long int* cstime) const;

	float cpuSpeed() const;


private:
	BoInfo* mInfo;
};


#endif

