/*  This file is part of the Boson game. It was originally part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2000-2002 Stephan Kulow (coolo@kde.org)
                  2002-2008 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef BODEBUG_H
#define BODEBUG_H

#include <KDebug>

class AreaData;

/**
 * Output a NULL pointer error for @p p (including the variable name).
 * Won't check whether p is actually NULL.
 *
 * Usually you will use BO_CHECK_NULL* instead.
 **/
#define BO_NULL_ERROR(p) boError() << "NULL pointer: " << #p;

/**
 * Ensure that the pointer p is non-NULL. If it is NULL output an error. will
 * also output the supplied variable name.
 **/
#define BO_CHECK_NULL(p) if (!p) { BO_NULL_ERROR(p) }

/**
 * Just like BO_CHECK_NULL, but will also return the current function (without
 * return value)
 **/
#define BO_CHECK_NULL_RET(p) if (!p) { BO_NULL_ERROR(p) return; }

/**
 * Just like BO_CHECK_NULL, but will also return the current function, return
 * value is 0.
 **/
#define BO_CHECK_NULL_RET0(p) if (!p) { BO_NULL_ERROR(p) return 0; }

// make boDebug behave like kDebug
#define boDebug BoDebugObject(QtDebugMsg, __FILE__, __LINE__, Q_FUNC_INFO)
#define boWarning BoDebugObject(QtWarningMsg, __FILE__, __LINE__, Q_FUNC_INFO)
#define boError BoDebugObject(QtCriticalMsg, __FILE__, __LINE__, Q_FUNC_INFO)
#define boBacktrace kBacktrace

//#warning TODO: BoDebugLog support


class BoDebugObject
{
public:
  inline BoDebugObject(QtMsgType t, const char* file, int line, const char* funcinfo)
    : mType(t), mFile(file), mLine(line), mFuncinfo(funcinfo), mArea(0)
  {
  }

  inline QDebug operator()(int area = 0)
  {
    mArea = area;
    return QDebug(&mMessage);
  }
  inline ~BoDebugObject()
  {
    finalizeMessage();
  }

protected:
  void finalizeMessage();
  QByteArray stripFuncinfoDown(const char* funcinfo) const;

private:
  QString mMessage;
  QtMsgType mType;
  const char* mFile;
  int mLine;
  const char* mFuncinfo;
  int mArea;
};

namespace BoDebug
{
  void registerAreaName(int area, QString name);
  QByteArray findAreaName(int area);
  AreaData* findAreaData(int area);
};


#if 0
// TODO: must be ported to Qt4, if we want to use signals. do we?
//#warning TODO: port BoDebug for signals?
#include <qobject.h>
class BoDebug : public QObject
{
  Q_OBJECT
public:
  BoDebug();
  ~BoDebug();

  enum DebugLevels {
    KDEBUG_INFO = 0,
    KDEBUG_WARN = 1,
    KDEBUG_ERROR = 2,
    KDEBUG_FATAL = 3
  };

  /**
   * Create and return the BoDebug object. You should use this method in
   * your program!
   * @return The BoDebug instance of your program. The instance will be
   * created if not present.
   **/
  static BoDebug* self();

  //AB: workaround. we need this in kDebugBackend. we should make that
  //function a method of BoDebug instead, so that we can access mDebug
  //directly. maybe we make kDebugBackend a friend of this class instead
  /**
   * @internal
   * Do <em>not</em> use this in your program!
   * @return The global BoDebug instance. Does <em>not</em> create the
   * instance if not present and will return NULL in that case.
   **/
  static BoDebug* selfNonCreate();

  /**
   * Calles internally. Emits @ref notify.
   **/
  void emitSignal(const QString& area, const char* data, int level)
  {
    emit notify(area, data, level);
  }

  /**
   * Disable the use of debug areas, all debug output is treated as if it
   * belonged to area 0. I.e. boDebug(x) is always equal to boDebug().
   *
   * This also means that the bodebug.areas file does @em not have to be loaded
   * and thus doesnt need to be installed.
   **/
  static void disableAreas() { mUseAreas = false; }
  static bool useAreas() { return mUseAreas; }

signals:
  /**
   * @param area The string that belongs to the specified debug area. The
   * string is specified in the bodebug.areas file. Usually this is simply
   * the instance name of the application, e.g. "boson" for boson if you
   * use boDebug() << endl; or boDebug(0) << endl;
   * @param data The text of the debug output. Including the prefix, such
   * as "ERROR: " or "WARNING: ".
   * @param level What kind of debug output this is - see @ref
   * DebugLevels. The value depends on the function you called, e.g.
   * boWarning() will always generate @ref KDEBUG_WARN.
   **/
  void notify(const QString& area, const char* data, int level);

private:
  static BoDebug* mDebug;
  class BoDebugPrivate;
  BoDebugPrivate* d;
  static bool mUseAreas;
};
#endif

#if 0
// AB: when you remove this #if 0, remove it from the .cpp as well (next to
// NDEBUG)
#ifdef NDEBUG
#define boDebug bondDebug
#define boBacktrace bondBacktrace
#endif
#endif

#endif

/*
 * vim: et sw=2
 */
