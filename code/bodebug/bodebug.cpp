/*  This file is part of the Boson game. It was originally part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// Include our header without NDEBUG defined to avoid having the kDebugInfo
// functions inlined to noops (which would then conflict with their definition
// here).

#include "bodebug.h"
#include "bodebug.moc"

#ifdef NDEBUG
#undef boDebug
#undef boBacktrace
#endif

#include "bodebugdcopiface.h"
#include "bodebuglog.h"

#include <kapplication.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <qmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qintdict.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdlib.h> // abort
#include <unistd.h> // getpid
#include <stdarg.h> // vararg stuff
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <kconfig.h>
#include "kstaticdeleter.h"
#include <config.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

class BoDebugEntry;

class BoDebugEntry
{
public:
    BoDebugEntry (int n, QString d) {number=n; descr=d;}
    unsigned int number;
    QString descr;
};

static QIntDict<BoDebugEntry> *KDebugCache;

static KStaticDeleter< QIntDict<BoDebugEntry> > kdd;

static QString getDescrFromNum(unsigned int _num)
{
  if (!KDebugCache)
  {
    kdd.setObject(KDebugCache, new QIntDict<BoDebugEntry>);
    // Do not call this deleter from ~KApplication
    KGlobal::unregisterStaticDeleter(&kdd);
    KDebugCache->setAutoDelete(true);
  }

  BoDebugEntry *ent = KDebugCache->find( _num );
  if ( ent )
  {
    return ent->descr;
  }

  if ( !KDebugCache->isEmpty() ) // areas already loaded
  {
    return QString::null;
  }

  QString filename(locate("config","bodebug.areas"));
  if (filename.isEmpty())
  {
    qDebug("oops - bodebug.areas not found! check your installation!");
    QMessageBox::critical( 0L, i18n("Fatal error"), i18n("bodebug.areas not found!\nCheck your installation!"));
    exit(1);
  }
  QFile file(filename);
  if (!file.open(IO_ReadOnly))
  {
    qWarning("Couldn't open %s", filename.local8Bit().data());
    file.close();
    return "";
  }

  unsigned long number = 0;
  bool longOK;

  QTextStream *ts = new QTextStream(&file);
  ts->setEncoding( QTextStream::Latin1 );
  while (!ts->eof())
  {
    const QString data(ts->readLine());
    int i = 0;
    int len = data.length();

    QChar ch = data[0];
    if (ch == '#' || ch.isNull())
    {
      continue;
    }
    while (ch.isSpace())
    {
      if (!(i < len))
      {
        continue;
      }
      ++i;
      ch = data[i];
    }
    if (ch.isNumber())
    {
      int numStart = i ;
      while (ch.isNumber())
      {
        if (!(i < len))
        {
          continue;
        }
        ++i;
        ch = data[i];
      }
      number = data.mid(numStart,i).toULong(&longOK);
    }
    while (ch.isSpace())
    {
      if (!(i < len))
      {
        continue;
      }
      ++i;
      ch = data[i];
    }
    const QString description(data.mid(i, len));
    //qDebug("number: [%i] description: [%s]", number, description.latin1());

    KDebugCache->insert(number, new BoDebugEntry(number,description));
  }

  delete ts;
  file.close();

  ent = KDebugCache->find( _num );
  if ( ent )
  {
      return ent->descr;
  }

  return QString::null;
}


struct boDebugPrivate {
  boDebugPrivate() :
      oldarea(), config(0) { }

  ~boDebugPrivate() { delete config; }

  QString aAreaName;
  unsigned int oldarea;
  KConfig *config;
};

static boDebugPrivate *boDebug_data = 0;
static KStaticDeleter<boDebugPrivate> pcd;

static void kDebugBackend( unsigned short nLevel, unsigned int nArea, const QString& _output)
{
  if ( !boDebug_data )
  {
      pcd.setObject(boDebug_data, new boDebugPrivate());
      // Do not call this deleter from ~KApplication
      KGlobal::unregisterStaticDeleter(&pcd);
  }

  if (!boDebug_data->config && KGlobal::_instance )
  {
      boDebug_data->config = new KConfig("bodebugrc", false, false);
      boDebug_data->config->setGroup("0");

      //AB: this is necessary here, otherwise all output with area 0 won't be
      //prefixed with anything, unless something with area != 0 is called before
      if ( KGlobal::_instance )
      {
        boDebug_data->aAreaName = KGlobal::instance()->instanceName();
      }
  }

  if (boDebug_data->config && boDebug_data->oldarea != nArea)
  {
    boDebug_data->config->setGroup( QString::number(static_cast<int>(nArea)) );
    boDebug_data->oldarea = nArea;
    if ( nArea > 0 && KGlobal::_instance )
    {
      boDebug_data->aAreaName = getDescrFromNum(nArea);
    }
    if ((nArea == 0) || boDebug_data->aAreaName.isEmpty())
    {
      if ( KGlobal::_instance )
      {
        boDebug_data->aAreaName = KGlobal::instance()->instanceName();
      }
    }
  }

  BoDebugLog* log = BoDebugLog::debugLog();
  if (log)
  {
    log->addEntry(_output, nArea, boDebug_data->aAreaName, nLevel);
  }

  QString sOutput;
  int nPriority = 0;
  QString aCaption;

  /* Determine output */

  QString key;
  switch( nLevel )
  {
    case BoDebug::KDEBUG_INFO:
      key = "InfoOutput";
      aCaption = "Info";
      nPriority = LOG_INFO;
      sOutput = "";
      break;
    case BoDebug::KDEBUG_WARN:
      key = "WarnOutput";
      aCaption = "Warning";
      nPriority = LOG_WARNING;
      sOutput = "WARNING: ";
      break;
    case BoDebug::KDEBUG_FATAL:
      key = "FatalOutput";
      aCaption = "Fatal Error";
      nPriority = LOG_CRIT;
      sOutput = "FATAL: ";
      break;
    case BoDebug::KDEBUG_ERROR:
    default:
      /* Programmer error, use "Error" as default */
      key = "ErrorOutput";
      aCaption = "Error";
      nPriority = LOG_ERR;
      sOutput = "ERROR: ";
      break;
  }

  sOutput += _output;
  QCString local8Bit = sOutput.local8Bit();
  const char* data = local8Bit.data();

  short nOutput = boDebug_data->config ? boDebug_data->config->readNumEntry(key, 2) : 2;

  // If the application doesn't have a QApplication object it can't use
  // a messagebox.
  if (!kapp && (nOutput == 1))
  {
    nOutput = 2;
  }

  // Output
  switch( nOutput )
  {
    case 0: // File
    {
      QString aKey;
      switch( nLevel )
      {
        case BoDebug::KDEBUG_INFO:
          aKey = "InfoFilename";
          break;
        case BoDebug::KDEBUG_WARN:
          aKey = "WarnFilename";
          break;
        case BoDebug::KDEBUG_FATAL:
          aKey = "FatalFilename";
          break;
        case BoDebug::KDEBUG_ERROR:
        default:
          aKey = "ErrorFilename";
          break;
      }
      QString aOutputFileName = boDebug_data->config->readEntry(aKey, "bodebug.dbg");

      const int BUFSIZE = 4096;
      char buf[BUFSIZE] = "";
      int nSize;
      if ( !boDebug_data->aAreaName.isEmpty() )
      {
        nSize = snprintf( buf, BUFSIZE, "%s: %s", boDebug_data->aAreaName.ascii(), data);
      }
      else
      {
        nSize = snprintf( buf, BUFSIZE, "%s", data);
      }

      QFile aOutputFile( aOutputFileName );
      aOutputFile.open( IO_WriteOnly | IO_Append );
      if ( ( nSize == -1 ) || ( nSize >= BUFSIZE ) )
      {
        aOutputFile.writeBlock( buf, BUFSIZE-1 );
      }
      else
      {
        aOutputFile.writeBlock( buf, nSize );
      }
      aOutputFile.close();
      break;
    }
    case 1: // Message Box
    {
      // Since we are in kdecore here, we cannot use KMsgBox and use
      // QMessageBox instead
      if ( !boDebug_data->aAreaName.isEmpty() )
      {
        aCaption += QString("(") + boDebug_data->aAreaName + ")";
      }
      QMessageBox::warning( 0L, aCaption, data, i18n("&OK") );
      break;
    }
    case 2: // Shell
    {
      FILE *output;
      /* we used to use stdout for debug
      if (nPriority == LOG_INFO)
        output = stderr;
      else */
      output = stderr;
      // Uncomment this to get the pid of the app in the output (useful for e.g. kioslaves)
      // if ( !boDebug_data->aAreaName.isEmpty() ) fprintf( output, "%d %s: ", (int)getpid(), boDebug_data->aAreaName.ascii() );
      if ( !boDebug_data->aAreaName.isEmpty() )
      {
        fprintf( output, "%s: ", boDebug_data->aAreaName.ascii() );
      }
      fputs(data, output);
      break;
    }
    case 3: // syslog
    {
      syslog( nPriority, "%s", data);
      break;
    }
    case 4: // nothing
    {
      break;
    }
    case 5: // emit signal
    {
      // we won't emit anything unless BoDebug::mDebug is non-NULL.
      // the user will need to call BoDebug::self() in order to connect
      // any signal to BoDebug. So if it is NULL, the signal won't be
      // used anyway.
      if (BoDebug::selfNonCreate())
      {
        emit BoDebug::self()->emitSignal(boDebug_data->aAreaName, data, (BoDebug::DebugLevels)nLevel);
      }
      break;
    }
  }

  // check if we should abort
  if( ( nLevel == BoDebug::KDEBUG_FATAL )
      && ( !boDebug_data->config || boDebug_data->config->readNumEntry( "AbortFatal", 1 ) ) )
  {
    abort();
  }
}

bodbgstream &perror( bodbgstream &s) { return s << QString::fromLocal8Bit(strerror(errno)); }
bodbgstream boDebug(int area) { return bodbgstream(area, BoDebug::KDEBUG_INFO); }
bodbgstream boDebug(bool cond, int area) { if (cond) return bodbgstream(area, BoDebug::KDEBUG_INFO); else return bodbgstream(0, 0, false); }

bodbgstream boError(int area) { return bodbgstream(area, BoDebug::KDEBUG_ERROR); }
bodbgstream boError(bool cond, int area) { if (cond) return bodbgstream(area, BoDebug::KDEBUG_ERROR); else return bodbgstream(0,0,false); }
bodbgstream boWarning(int area) { return bodbgstream(area, BoDebug::KDEBUG_WARN); }
bodbgstream boWarning(bool cond, int area) { if (cond) return bodbgstream(area, BoDebug::KDEBUG_WARN); else return bodbgstream(0,0,false); }
bodbgstream boFatal(int area) { return bodbgstream(area, BoDebug::KDEBUG_FATAL); }
bodbgstream boFatal(bool cond, int area) { if (cond) return bodbgstream(area, BoDebug::KDEBUG_FATAL); else return bodbgstream(0,0,false); }

void bodbgstream::flush()
{
  if (output.isEmpty() || !print)
  {
    return;
  }
  kDebugBackend( level, area, output );
  output = QString::null;
}

bodbgstream &bodbgstream::form(const char *format, ...)
{
  char buf[4096];
  va_list arguments;
  va_start( arguments, format );
  vsnprintf( buf, sizeof(buf), format, arguments );
  va_end(arguments);
  *this << buf;
  return *this;
}

bodbgstream::~bodbgstream()
{
  if (!output.isEmpty())
  {
    fprintf(stderr, "ASSERT: debug output not ended with \\n\n");
    *this << "\n";
  }
}

bodbgstream& bodbgstream::operator << (QWidget* widget)
{
  QString string, temp;
  // -----
  if(widget==0)
  {
    string=(QString)"[Null pointer]";
  }
  else
  {
    temp.setNum((ulong)widget, 16);
    string=(QString)"["+widget->className()+" pointer "
        + "(0x" + temp + ")";
    if(widget->name(0)==0)
    {
      string += " to unnamed widget, ";
    }
    else
    {
      string += (QString)" to widget " + widget->name() + ", ";
    }
    string += "geometry="
        + QString().setNum(widget->width())
        + "x"+QString().setNum(widget->height())
        + "+"+QString().setNum(widget->x())
        + "+"+QString().setNum(widget->y())
        + "]";
  }
  if (!print)
  {
    return *this;
  }
  output += string;
  if (output.at(output.length() -1 ) == '\n')
  {
    flush();
  }
  return *this;
}

QString boBacktrace(int levels)
{
  QString s;
#ifdef HAVE_BACKTRACE
  void* trace[256];
  int n = backtrace(trace, 256);
  char** strings = backtrace_symbols (trace, n);

  if ( levels != -1 )
    n = QMIN( n, levels );
  s = "[\n";

  for (int i = 0; i < n; ++i)
    s += QString::number(i) +
        QString::fromLatin1(": ") +
        QString::fromLatin1(strings[i]) + QString::fromLatin1("\n");
  s += "]\n";
  free (strings);
#endif
  return s;
}

QString boBacktrace()
{
  return boBacktrace(-1 /*all*/);
}

void boClearDebugConfig()
{
  delete boDebug_data->config;
  boDebug_data->config = 0;
}


static KStaticDeleter<BoDebug> sd;
BoDebug* BoDebug::mDebug = 0;

BoDebug::BoDebug() : QObject(0)
{
}

BoDebug::~BoDebug()
{
}

BoDebug* BoDebug::self()
{
 if (!mDebug)
 {
   mDebug = new BoDebug();
   sd.setObject(mDebug);
 }
 return mDebug;
}

BoDebug* BoDebug::selfNonCreate()
{
 return mDebug;
}

// Needed for --enable-final
#ifdef NDEBUG
#define boDebug kndDebug
#endif

/*
 * vim: et sw=2
 */
