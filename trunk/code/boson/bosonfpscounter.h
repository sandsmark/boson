/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONFPSCOUNTER_H
#define BOSONFPSCOUNTER_H

#include <qobject.h>

struct timeval;

class BosonFPSCounterPrivate;
/**
 * @short An FPS counter
 * This class is not supposed to be used outside of the main GL widget. Use for
 * example @ref BosonGameFPSCounter instead, which provides access to the
 * methods relevant for in-game use (but not to e.g. @ref startFrame and @ref
 * endFrame which must not be used there)
 **/
class BosonFPSCounter : public QObject
{
	Q_OBJECT
public:
	BosonFPSCounter(QObject* parent);
	~BosonFPSCounter();

	/**
	 * Tell the counter that a new frame is being started. Must not be
	 * called again, unless @ref endFrame has been called before.
	 **/
	void startFrame();

	/**
	 * Tell the counter that the current frame is completely rendered now.
	 * See also @ref startFrame.
	 **/
	void endFrame();

	/**
	 * Tell the counter that the current frame will not be rendered
	 * completely, but only partially (e.g. because the game engine needs
	 * additional CPU power).
	 *
	 * Note that @ref endFrame still needs to be called. See also @ref
	 * startFrame, which starts the current frame.
	 **/
	void skipFrame();

	/**
	 * @return @ref fps, but additionally caches the values. The cache is
	 * updated automatically once per second.
	 *
	 * This method can be useful to display the FPS on the screen, as it
	 * does not change every frame, but only once a second.
	 **/
	double cachedFps(double* skippedPerSeond = 0);

	/**
	 * @return The frames per second, computed using all frames that have
	 * been rendered since a certain point in time shortly ago (one or two
	 * seconds usually).
	 * Note that if this is called while a frame is being rendered, the
	 * currently rendered frame is NOT counted.
	 **/
	double fps(double* skippedPerSeond = 0) const;

	/**
	 * @return The frames per second, computed using all frames that have
	 * been rendered since @p since. This method counts all frames that are
	 * currently stored in this class.
	 *
	 * The return value of this method is mostly equivalent to @ref
	 * countFramesSince divided by the number of seconds in @p since
	 * (since.tv_sec).
	 **/
	double fps(const struct timeval& since, double* skippedPerSecond = 0) const;

	void reset();

	/**
	 * @return The number of frames since @p since.
	 **/
	unsigned int countFramesSince(const struct timeval& since, unsigned int* skippedFrames = 0) const;

	/**
	 * @return The maximum number of seconds after which the frames are
	 * deleted internally. @ref fps can approximate only using frames that
	 * are younger than this value.
	 **/
	int maximumAge() const
	{
		return mMaximumAge;
	}

	/**
	 * See also @ref maximumAge. You usually do not need to change the
	 * default (currently at 20 seconds).
	 **/
	void setMaximumAge(int seconds)
	{
		mMaximumAge = seconds;
	}

	/**
	 * @return The frames per second value, if @p frames frames were rendered
	 * between @p now and @p since
	 **/
	double calculateFPS(unsigned int frames, const struct timeval& now, const struct timeval& since) const;

	/**
	 * @return The number of us since the last frame was rendered (reminder:
	 * 1s == 1000,000us), or a very large value if no frame is stored.
	 * @param onlyNonSkippedFrames If FALSE, this returns the time since the
	 * last frame. If TRUE, this returns the time since the last frame that
	 * was not skipped.
	 **/
	long long int timeSinceLastFrame(bool onlyNonSkippedFrames = false) const;

signals:
	void signalSkipFrame();

protected:
	void cleanOldFrames();

private:
	BosonFPSCounterPrivate* d;
	int mMaximumAge;
};

/**
 * This class is a small frontend to @ref BosonFPSCounter that may be used in
 * the game widget directly.
 **/
// AB: this class can be used to e.g. add some game relevant info, like "models
// per frame", or "average time that canvas rendering took".
// The relevant methods should be added to the fps counter and this class
// should call these there.
class BosonGameFPSCounter : public QObject
{
	Q_OBJECT
public:
	BosonGameFPSCounter(BosonFPSCounter* parent);

	/**
	 * @return A const object that provides read-only access to the @ref
	 * BosonFPSCounter. This can be used for example to read the @ref
	 * BosonFPSCounter::fps.
	 **/
	const BosonFPSCounter* counter() const
	{
		return mFPSCounter;
	}

	/**
	 * Tell the @ref BosonFPSCounter that the current frame is being
	 * skipped. See also @ref BosonFPSCounter::skipFrame
	 **/
	void skipFrame();

	/**
	 * @return See @ref BosonFPSCounter::cachedFps
	 **/
	double cachedFps(double* skippedPerSeond = 0);

private:
	BosonFPSCounter* mFPSCounter;
};


#endif

