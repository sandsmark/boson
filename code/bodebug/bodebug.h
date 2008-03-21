/*  This file is part of the Boson game. It was originally part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2000-2002 Stephan Kulow (coolo@kde.org)
                  2002 Andreas Beckermann (b_mann@gmx.de)

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

#include <qstring.h>
class QWidget;

class bodbgstream;
class bondbgstream;
typedef bodbgstream & (*BODBGFUNC)(bodbgstream &); // manipulator function
typedef bondbgstream & (*BONDBGFUNC)(bondbgstream &); // manipulator function

#ifdef __GNUC__
#define k_funcinfo "[" << __PRETTY_FUNCTION__ << "] "
#else
#define k_funcinfo "[" << __FILE__ << ":" << __LINE__ << "] "
#endif

#define k_lineinfo "[" << __FILE__ << ":" << __LINE__ << "] "


// 4 macros for boson:

/**
 * Output a NULL pointer error for @p p (including the variable name).
 * Won't check whether p is actually NULL.
 *
 * Usually you will use BO_CHECK_NULL* instead.
 **/
#define BO_NULL_ERROR(p) boError() << k_funcinfo << "NULL pointer: " << #p << endl;

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

/**
 * bodbgstream is a text stream that allows you to print debug messages.
 * Using the overloaded "<<" operator you can send messages. Usually 
 * you do not create the bodbgstream yourself, but use @ref boDebug()
 * @ref boWarning(), @ref boError() or @ref boFatal to obtain one.
 *
 * Example:
 * <pre>
 *    int i = 5;
 *    boDebug() << "The value of i is " << i << endl;
 * </pre>
 * @see bondbgstream
 **/
class bodbgstream
{
 public:
   /**
    * @internal
    **/
   bodbgstream(unsigned int _area, unsigned int _level, bool _print = true) :
      area(_area), level(_level),  print(_print) { }
   bodbgstream(const char * initialString, unsigned int _area, unsigned int _level, bool _print = true) :
      output(QString::fromLatin1(initialString)), area(_area), level(_level),  print(_print) { }
   /// Copy constructor
   bodbgstream(bodbgstream &str) :
      output(str.output), area(str.area), level(str.level), print(str.print) { str.output.truncate(0); }
   bodbgstream(const bodbgstream &str) :
      output(str.output), area(str.area), level(str.level), print(str.print) {}
   ~bodbgstream();

   /**
    * Prints the given value.
    * @param i the boolean to print (as "true" or "false")
    * @return this stream
    **/
   bodbgstream &operator<<(bool i)
   {
     if (!print)
     {
       return *this;
     }
     output += QString::fromLatin1(i ? "true" : "false");
     return *this;
   }
   /**
    * Prints the given value.
    * @param i the short to print
    * @return this stream
    */
   bodbgstream &operator<<(short i)
   {
     if (!print)
     {
       return *this;
     }
     QString tmp; tmp.setNum(i); output += tmp;
     return *this;
   }
   /**
    * Prints the given value.
    * @param i the unsigned short to print
    * @return this stream
    */
   bodbgstream &operator<<(unsigned short i)
   {
     if (!print)
     {
       return *this;
     }
     QString tmp; tmp.setNum(i); output += tmp;
     return *this;
   }
   /**
    * Prints the given value.
    * @param i the char to print
    * @return this stream
    */
   bodbgstream &operator<<(char i)
   {
     if (!print)
     {
       return *this;
     }
     QString tmp; tmp.setNum(int(i)); output += tmp;
     return *this;
   }
   /**
    * Prints the given value.
    * @param i the unsigned char to print
    * @return this stream
    */
   bodbgstream &operator<<(unsigned char i)
   {
     if (!print)
     {
       return *this;
     }
     QString tmp; tmp.setNum(static_cast<unsigned int>(i)); output += tmp;
     return *this;
   }
   /**
    * Prints the given value.
    * @param i the int to print
    * @return this stream
    */
   bodbgstream &operator<<(int i)
   {
     if (!print)
     {
       return *this;
     }
     QString tmp; tmp.setNum(i); output += tmp;
     return *this;
   }
   /**
    * Prints the given value.
    * @param i the unsigned int to print
    * @return this stream
    */
   bodbgstream &operator<<(unsigned int i)
   {
     if (!print)
     {
       return *this;
     }
     QString tmp; tmp.setNum(i); output += tmp;
     return *this;
   }
   /**
    * Prints the given value.
    * @param i the long to print
    * @return this stream
    */
   bodbgstream &operator<<(long i)
   {
     if (!print)
     {
       return *this;
     }
     QString tmp; tmp.setNum(i); output += tmp;
     return *this;
   }
   /**
    * Prints the given value.
    * @param i the unsigned long to print
    * @return this stream
    */
   bodbgstream &operator<<(unsigned long i)
   {
     if (!print)
     {
       return *this;
     }
     QString tmp; tmp.setNum(i); output += tmp;
     return *this;
   }
   /**
    * Flushes the output.
    */
   void flush(); //AB: if this was virtual we wouldn't have to fork these files!

   /**
    * Prints the given value.
    * @param string the string to print
    * @return this stream
    */
   bodbgstream &operator<<(const QString& string)
   {
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
   /**
    * Prints the given value.
    * @param string the string to print
    * @return this stream
    */
   bodbgstream &operator<<(const char *string)
   {
     if (!print)
     {
       return *this;
     }
     output += QString::fromUtf8(string);
     if (output.at(output.length() - 1) == '\n')
     {
       flush();
     }
     return *this;
   }
   /**
    * Prints the given value.
    * @param string the string to print
    * @return this stream
    */
   bodbgstream &operator<<(const QCString& string)
   {
     *this << string.data();
     return *this;
   }
   /**
    * Prints the given value.
    * @param p a pointer to print (in number form)
    * @return this stream
    */
   bodbgstream& operator<<(const void * p)
   {
     form("%p", p);
     return *this;
   }
   /**
    * Invokes the given function.
    * @param f the function to invoke
    * @return the return value of @p f
    */
   bodbgstream& operator<<(BODBGFUNC f)
   {
     if (!print)
     {
       return *this;
     }
     return (*f)(*this);
   }
   /**
    * Prints the given value.
    * @param d the double to print
    * @return this stream
    */
   bodbgstream& operator<<(double d)
   {
     QString tmp; tmp.setNum(d); output += tmp;
     return *this;
   }
   /**
    * Prints the string @p format which can contain
    * printf-style formatted values.
    * @param format the printf-style format
    * @return this stream
    */
   bodbgstream &form(const char *format, ...);
   /** Operator to print out basic information about a QWidget.
    *  Output of class names only works if the class is moc'ified.
    * @param widget the widget to print
    * @return this stream
    */
   bodbgstream& operator << (QWidget* widget);

 private:
   QString output;
   unsigned int area, level;
   bool print;
};

/**
 * Prints an "\n". 
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 */
inline bodbgstream &endl( bodbgstream &s) { s << "\n"; return s; }
/**
 * Flushes the stream.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 */
inline bodbgstream &flush( bodbgstream &s) { s.flush(); return s; }
bodbgstream &perror( bodbgstream &s);

/**
 * bondbgstream is a dummy variant of @ref bodbgstream. All functions do
 * nothing.
 * @see bondDebug()
 */
class bondbgstream {
 public:
   /// Empty constructor.
   bondbgstream() {}
   ~bondbgstream() {}
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(short int )  { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(unsigned short int )  { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(char )  { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(unsigned char )  { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(int )  { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(unsigned int )  { return *this; }
   /**
    * Does nothing.
    **/
   void flush() {}
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(const QString& ) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(const QCString& ) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &operator<<(const char *) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    */
   bondbgstream& operator<<(const void *) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream& operator<<(void *) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream& operator<<(double) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream& operator<<(long) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream& operator<<(unsigned long) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream& operator<<(BONDBGFUNC) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream& operator << (QWidget*) { return *this; }
   /**
    * Does nothing.
    * @return this stream
    **/
   bondbgstream &form(const char *, ...) { return *this; }
};

/**
 * Does nothing.
 * @param a stream
 * @return the given @p s
 */
inline bondbgstream &endl( bondbgstream & s) { return s; }
/**
 * Does nothing.
 * @param a stream
 * @return the given @p s
 */
inline bondbgstream &flush( bondbgstream & s) { return s; }
inline bondbgstream &perror( bondbgstream & s) { return s; }

/**
 * Returns a debug stream. You can use it to print debug
 * information.
 * @param area an id to identify the output, 0 for default
 * @see bondDebug()
 */
bodbgstream boDebug(int area = 0);
bodbgstream boDebug(bool cond, int area = 0);
/**
 * Returns a backtrace.
 * @return a backtrace
 */
QString boBacktrace();
/**
 * Returns a backtrace.
 * @param levels the number of levels of the backtrace
 * @return a backtrace
 * @since 3.1
 */
QString boBacktrace(int levels);
/**
 * Returns a dummy debug stream. The stream does not print anything.
 * @param area an id to identify the output, 0 for default
 * @see boDebug()
 */
inline bondbgstream bondDebug(int = 0) { return bondbgstream(); }
inline bondbgstream bondDebug(bool , int  = 0) { return bondbgstream(); }
inline QString bondBacktrace() { return QString::null; }

/**
 * Returns a warning stream. You can use it to print warning
 * information.
 * @param area an id to identify the output, 0 for default
 */
bodbgstream boWarning(int area = 0);
bodbgstream boWarning(bool cond, int area = 0);
/**
 * Returns an error stream. You can use it to print error
 * information.
 * @param area an id to identify the output, 0 for default
 */
bodbgstream boError(int area = 0);
bodbgstream boError(bool cond, int area = 0);
/**
 * Returns a fatal error stream. You can use it to print fatal error
 * information.
 * @param area an id to identify the output, 0 for default
 */
bodbgstream boFatal(int area = 0);
bodbgstream boFatal(bool cond, int area = 0);

/**
 * Deletes the bodebugrc cache and therefore forces bodebug to reread the
 * config file
 */
void boClearDebugConfig();


// AB: TODO document
// AB: TODO integrate some of the internal .cpp functions here?
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
};

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
