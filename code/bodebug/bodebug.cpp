/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// note the copyright above: this is LGPL!

#include "bodebug.h"
#include "bodebug.moc"

#include "bodebuglog.h"

#include <KGlobal>
#include <KComponentData>

#include <QHash>

#include <unistd.h> // getpid

class BoDebugPrivate
{
public:
  BoDebugPrivate()
  {
    mDummyProgramName = QString("<unknown program name>").toLocal8Bit();
  }
  QByteArray programName()
  {
    if (!mProgramName.isEmpty())
    {
      return mProgramName;
    }
    if (KGlobal::hasMainComponent())
    {
      mProgramName = KGlobal::mainComponent().componentName().toLocal8Bit();
      return mProgramName;
    }
    return mDummyProgramName;
  }
  QHash<int, QByteArray> mHash;
  QByteArray mProgramName;
  QByteArray mDummyProgramName;
};
K_GLOBAL_STATIC(BoDebugPrivate, myBoDebug)


// AB some parts of this function have been shamelessy stolen from KDE's kdebug.cpp, revision 739605
void BoDebugObject::finalizeMessage()
{
  QByteArray areaName;
  if (mArea != 0)
  {
    areaName = BoDebug::findAreaName(mArea);
  }
  QByteArray programName = myBoDebug->programName();
  bool haveAreaName = false;
  if (areaName.isEmpty())
  {
    haveAreaName = false;
  }
  else
  {
    haveAreaName = true;
  }
  QByteArray funcinfo = stripFuncinfoDown(mFuncinfo);
  fprintf(stderr, "%s(%d)%s%s %s %s\n",
      programName.constData(),
      getpid(),
      haveAreaName ? "/" : "",
      haveAreaName ? areaName.constData() : "",
      funcinfo.constData(),
      mMessage.toLocal8Bit().constData()
  );
  fflush(stderr);

  BoDebugLog* log = BoDebugLog::debugLog();
  if (log)
  {
    QString areaName;

    // TODO: add funcinfo, line, file parameters and use the original message
    // (without file, line, funcinfo data).
    // -> displaying file/funcinfo/line separately may be very useful
    log->addEntry(mMessage, mArea, areaName, mType);
  }
}

// AB: shamelessy stolen from KDE's kdebug.cpp, revision 739605
QByteArray BoDebugObject::stripFuncinfoDown(const char* funcinfo) const
{
# ifdef Q_CC_GNU
            // strip the function info down to the base function name
            // note that this throws away the template definitions,
            // the parameter types (overloads) and any const/volatile qualifiers
            QByteArray info = funcinfo;
            int pos = info.indexOf('(');
            Q_ASSERT_X(pos != -1, "kDebug",
                       "Bug in kDebug(): I don't know how to parse this function name");
            while (info.at(pos - 1) == ' ')
                // that '(' we matched was actually the opening of a function-pointer
                pos = info.indexOf('(', pos + 1);

            info.truncate(pos);
            // gcc 4.1.2 don't put a space between the return type and
            // the function name if the function is in an anonymous namespace
            int index = 1;
            forever {
                index = info.indexOf("<unnamed>::", index);
                if ( index == -1 )
                    break;

                if ( info.at(index-1) != ':' )
                    info.insert(index, ' ');

                index += strlen("<unnamed>::");
            }
            pos = info.lastIndexOf(' ');
            if (pos != -1) {
                int startoftemplate = info.lastIndexOf('<');
                if (startoftemplate != -1 && pos > startoftemplate &&
                    pos < info.lastIndexOf(">::"))
                    // we matched a space inside this function's template definition
                    pos = info.lastIndexOf(' ', startoftemplate);
            }

            if (pos + 1 == info.length())
                // something went wrong, so gracefully bail out
                return QByteArray(funcinfo);
            return QByteArray(info.constData() + pos + 1);
# else
            return QByteArray(funcinfo);
# endif
}

QByteArray BoDebug::findAreaName(int area)
{
  if (!myBoDebug->mHash.contains(area))
  {
    return QString("<unregistered area %1>").arg(area).toLocal8Bit();
  }
  return myBoDebug->mHash[area];
}

void BoDebug::registerAreaName(int area, QString name)
{
  QByteArray b = name.toLocal8Bit();
  if (myBoDebug->mHash.contains(area))
  {
    boError() << "hash already contains area " << area;
    return;
  }
  myBoDebug->mHash.insert(area, name.toLocal8Bit());
}

/*
 * vim: et sw=2
 
 */
