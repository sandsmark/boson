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
#include <KConfig>
#include <KConfigGroup>
#include <KMessage>

#include <QHash>
#include <QFile>

#include <unistd.h> // getpid()
#include <syslog.h> // syslog()

enum OutputMode
{
  OutputFile = 0,
  OutputMessageBox = 1,
  OutputShell = 2,
  OutputSyslog = 3,
  OutputOff = 4,
  OutputCustom = 5,
  OutputLast/* must be LAST entry */
};

struct AreaData
{
  AreaData()
  {
  }
  AreaData(const QByteArray& name_)
    : name(name_)
  {
    for (int i = 0; i < 4; i++)
    {
      outputMode[i] = OutputShell;
    }
  }
  AreaData(const AreaData& a)
  {
    *this = a;
  }
  AreaData& operator=(const AreaData& a)
  {
    name = a.name;
    for (int i = 0; i < 4; i++)
    {
      outputMode[i] = a.outputMode[i];
      outputFileName[i] = a.outputFileName[i];
    }
    return *this;
  }
  QByteArray name;
  OutputMode outputMode[4];
  QString outputFileName[4];
};

class BoDebugPrivate
{
public:
  BoDebugPrivate()
  {
    mDummyProgramName = QString("<unknown program name>").toLocal8Bit();
    mConfig = 0;

    mHash.insert(0, AreaData());
  }
  ~BoDebugPrivate()
  {
    delete mConfig;
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
  void loadConfig()
  {
    if (mConfig)
    {
      return;
    }
    mConfig = new KConfig("bodebugrc");
    foreach (int area, mHash.keys())
    {
      KConfigGroup group = mConfig->group(QString::number(area));
      QHash<int, AreaData>::iterator it = mHash.find(area);
      int info = group.readEntry("InfoOutput", (int)OutputShell);
      int warn = group.readEntry("WarnOutput", (int)OutputShell);
      int error = group.readEntry("ErrorOutput", (int)OutputShell);
      int fatal = group.readEntry("FatalOutput", (int)OutputShell);
      (*it).outputMode[0] = ensureValidMode(info);
      (*it).outputMode[1] = ensureValidMode(warn);
      (*it).outputMode[2] = ensureValidMode(error);
      (*it).outputMode[3] = ensureValidMode(fatal);

      (*it).outputFileName[0] = group.readEntry("InfoFilename", QString());
      (*it).outputFileName[1] = group.readEntry("WarnFilename", QString());
      (*it).outputFileName[2] = group.readEntry("ErrorFilename", QString());
      (*it).outputFileName[3] = group.readEntry("FatalFilename", QString());

      qDebug() << "area" << area << "mode=" << (*it).outputMode[0];
    }
  }
  void clearConfig()
  {
    delete mConfig;
    mConfig = 0;
  }
  OutputMode ensureValidMode(int mode)
  {
    if (mode < 0 || mode >= OutputLast)
    {
      return OutputShell;
    }
    return (OutputMode)mode;
  }


  QHash<int, AreaData> mHash;
  QByteArray mProgramName;
  QByteArray mDummyProgramName;

  KConfig* mConfig;
};
K_GLOBAL_STATIC(BoDebugPrivate, myBoDebug)


// AB some parts of this function have been shamelessy stolen from KDE's kdebug.cpp, revision 739605
void BoDebugObject::finalizeMessage()
{
  AreaData* areaData = BoDebug::findAreaData(mArea);
  QByteArray areaName;
  OutputMode outputMode;
  QString outputFile;
  if (areaData)
  {
    areaName = areaData->name;
    int index = (int)mType;
    index = qMin(index, 3);
    outputMode = areaData->outputMode[index];
    outputFile = areaData->outputFileName[index];
  }
  else
  {
    outputMode = OutputShell;
  }
  if (outputMode == OutputOff)
  {
    return;
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

  BoDebugLog* log = BoDebugLog::debugLog();
  if (log)
  {
    QString areaName;

    // TODO: add funcinfo, line, file parameters and use the original message
    // (without file, line, funcinfo data).
    // -> displaying file/funcinfo/line separately may be very useful
    log->addEntry(mMessage, mArea, areaName, mType);
  }

  QByteArray funcinfo = stripFuncinfoDown(mFuncinfo);
  switch (outputMode)
  {
    case OutputShell:
      fprintf(stderr, "%s(%d)%s%s %s %s\n",
          programName.constData(),
          getpid(),
          haveAreaName ? "/" : "",
          haveAreaName ? areaName.constData() : "",
          funcinfo.constData(),
          mMessage.toLocal8Bit().constData()
      );
      fflush(stderr);
      break;
    case OutputMessageBox:
    {
      QString caption;
      switch(mType)
      {
        case QtDebugMsg:
          caption = QString("Info (%1)").arg(areaName.constData());
          break;
        case QtWarningMsg:
          caption = QString("Warning (%1)").arg(areaName.constData());
          break;
        default:
        case QtCriticalMsg:
          caption = QString("Error (%1)").arg(areaName.constData());
          break;
        case QtFatalMsg:
          caption = QString("Fatal Error(%1)").arg(areaName.constData());
          break;

      }
      // AB: shamelessy stolen from KDE's kdebug.cpp#
      KMessage::message(KMessage::Information, mMessage, caption);
      break;
    }
    case OutputFile:
    {
      QFile file(outputFile);
      if (!file.open(QFile::WriteOnly | QFile::Append))
      {
        qCritical() << Q_FUNC_INFO << "could not open file " << outputFile;
        return;
      }
      file.write(mMessage.toLocal8Bit());
      file.putChar('\n');
      file.close();
      break;
    }
    case OutputSyslog:
    {
      int priority;
      switch (mType)
      {
        case QtDebugMsg:
          priority = LOG_DEBUG;
          break;
        case QtWarningMsg:
          priority = LOG_WARNING;
          break;
        default:
        case QtCriticalMsg:
          priority = LOG_ERR;
          break;
        case QtFatalMsg:
          priority = LOG_CRIT;
          break;

      }
      syslog(priority, "%s", mMessage.toLocal8Bit().constData());
      break;
    }
    case OutputCustom:
      // currently unused.
      break;
    case OutputOff:
      break;
    case OutputLast:
      // error! should never be used.
      qCritical() << Q_FUNC_INFO << "oops: OutputLast should not be used.";
      break;
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
  return myBoDebug->mHash[area].name;
}

AreaData* BoDebug::findAreaData(int area)
{
  if (!myBoDebug->mConfig && KGlobal::hasMainComponent()) // we need a mainComponent for KConfig
  {
    myBoDebug->loadConfig();
  }
  if (!myBoDebug->mHash.contains(area))
  {
    return 0;
  }
  return &myBoDebug->mHash.find(area).value();
}

void BoDebug::registerAreaName(int area, QString name)
{
  QByteArray b = name.toLocal8Bit();
  if (myBoDebug->mHash.contains(area))
  {
    boError() << "hash already contains area " << area;
    return;
  }
  myBoDebug->mHash.insert(area, AreaData(name.toLocal8Bit()));
  myBoDebug->clearConfig();
}

/*
 * vim: et sw=2
 
 */
