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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
 */
class bodbgstream {
 public:
  /**
   * @internal
   */
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
     */
    bodbgstream &operator<<(bool i)  {
	if (!print) return *this;
	output += QString::fromLatin1(i ? "true" : "false");
	return *this;
    }
    /**
     * Prints the given value.
     * @param i the short to print
     * @return this stream
     */
    bodbgstream &operator<<(short i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(i); output += tmp;
	return *this;
    }
    /**
     * Prints the given value.
     * @param i the unsigned short to print
     * @return this stream
     */
    bodbgstream &operator<<(unsigned short i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    /**
     * Prints the given value.
     * @param i the char to print
     * @return this stream
     */
    bodbgstream &operator<<(char i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(int(i)); output += tmp;
	return *this;
    }
    /**
     * Prints the given value.
     * @param i the unsigned char to print
     * @return this stream
     */
    bodbgstream &operator<<(unsigned char i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(static_cast<unsigned int>(i)); output += tmp;
        return *this;
    }
    /**
     * Prints the given value.
     * @param i the int to print
     * @return this stream
     */
    bodbgstream &operator<<(int i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(i); output += tmp;
	return *this;
    }
    /**
     * Prints the given value.
     * @param i the unsigned int to print
     * @return this stream
     */
    bodbgstream &operator<<(unsigned int i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    /**
     * Prints the given value.
     * @param i the long to print
     * @return this stream
     */
    bodbgstream &operator<<(long i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    /**
     * Prints the given value.
     * @param i the unsigned long to print
     * @return this stream
     */
    bodbgstream &operator<<(unsigned long i) {
        if (!print) return *this;
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
    bodbgstream &operator<<(const QString& string) {
	if (!print) return *this;
	output += string;
	if (output.at(output.length() -1 ) == '\n')
	    flush();
	return *this;
    }
    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    bodbgstream &operator<<(const char *string) {
	if (!print) return *this;
	output += QString::fromUtf8(string);
	if (output.at(output.length() - 1) == '\n')
	    flush();
	return *this;
    }
    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    bodbgstream &operator<<(const QCString& string) {
        *this << string.data();
        return *this;
    }
    /**
     * Prints the given value.
     * @param p a pointer to print (in number form)
     * @return this stream
     */
    bodbgstream& operator<<(const void * p) {
        form("%p", p);
        return *this;
    }
    /**
     * Invokes the given function.
     * @param f the function to invoke
     * @return the return value of @p f
     */
    bodbgstream& operator<<(BODBGFUNC f) {
	if (!print) return *this;
	return (*f)(*this);
    }
    /**
     * Prints the given value.
     * @param d the double to print
     * @return this stream
     */
    bodbgstream& operator<<(double d) {
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
  /// EMpty constructor.
    bondbgstream() {}
    ~bondbgstream() {}
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(short int )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(unsigned short int )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(char )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(unsigned char )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(int )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(unsigned int )  { return *this; }
    /**
     * Does nothing.
     */
    void flush() {}
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(const QString& ) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(const QCString& ) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream &operator<<(const char *) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream& operator<<(const void *) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream& operator<<(void *) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream& operator<<(double) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream& operator<<(long) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream& operator<<(unsigned long) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream& operator<<(BONDBGFUNC) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    bondbgstream& operator << (QWidget*) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
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

#ifdef NDEBUG
#define boDebug bondDebug
#define boBacktrace bondBacktrace
#endif

#endif

