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

#include "boinfo.h"
#include "bocurrentinfo.h"

#include "bodebug.h"
#include "../../bomemory/bodummymemory.h"

#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>

/* Needed to use sysctl correctly */
/* should have some autoconf macro here to make sure 
   these headers exists
 */
#include <sys/param.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/linker.h>



/* cut-n-paste from boinfo_linux.cpp file */
QString readFile(const char* fileName)
{
 QFile file(fileName);
 if (!file.exists()) {
     return QString::null;
 }
 if (!file.open(IO_ReadOnly)) {
     return QString::null;
 }
 QString string;
 char buf[512];
 while (file.readLine(buf, sizeof(buf) - 1) > 0) {
     string += buf;
 }
 return string;
}

void BoInfo::updateOSInfo()
{
    char osrelease[64]; 
    char ostype[64];
    size_t len;
    QString nvModule = QString("nvidia.ko");

    /* Get CPU info. All the boot messages are in /var/run/dmesg.boot, which
     * contains the cpu. A line like so, should appear
     *    CPU: AMD Athlon(tm) processor (953.78-MHz 686-class CPU)
     * This is where we grab the CPU speed. I don't know of a better way
     */
    QString cpu = readFile("/var/run/dmesg.boot");
    if (!cpu.isNull()) {
        QRegExp exp_begin("^CPU.*\\(");
        QRegExp exp_end("-MH.*$");
        QStringList list = QStringList::split('\n', cpu);
        for (unsigned int i = 0; i < list.count(); i++) {
            if (exp_begin.search(list[i]) >= 0) {
                QString s = list[i];
                s.replace(exp_begin, QString::fromLatin1(""));
                s.replace(exp_end, QString::fromLatin1(""));
                insert(CPUString, s);
            }
        }
    }

    /* get the OSType */
    len = sizeof(ostype);
    if (sysctlbyname("kern.ostype", &ostype, &len, NULL, NULL)  == -1)
    {
        insert(OSTypeString, QString("Unable to determine"));
    }
    else
    {
        insert(OSTypeString, QString(ostype));
    }

    /* get the OSRelease */
    len = sizeof(osrelease);
    if (sysctlbyname("kern.osrelease", &osrelease, &len, NULL, NULL) == -1)
    {
        insert(OSVersionString, QString("Unable to determine"));
    }
    else
    {
        insert(OSVersionString, QString(osrelease));
    }

    /* See which modules are loaded in the kernel */
    kldfind(nvModule.latin1());
    if (errno != ENOENT)
    {
        /* module is loaded */
        insert(BoInfo::KernelModuleNVidiaString, nvModule);
    }
}

float BoInfo::cpuSpeed() const
{
    bool ok = true;
    float ret = 0.0f;
    QString cpu = getString(CPUString);
    ret = cpu.toFloat(&ok);
    if (ok) {
        return ret;
    } else {
        // an error occured...
        // can ret still be valid?
        return -1.0f;
    }
}

bool BoInfo::haveMtrr() const
{
 return false;
}

bool BoCurrentInfo::memoryInUse(QString*, QString*, QString*,
		QString*, QString*, QString*, QString*,
		QString*) const
{
 return false;
}

bool BoCurrentInfo::cpuTime(unsigned long int*, unsigned long int*, long int*, long int*) const
{
 return false;
}

/*
 * vim: et sw=4
 */
