/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "boinfo.h"
#include "bodebug.h"

#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>

#include <linux/version.h>

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
 QString cpu = readFile("/proc/cpuinfo");
 if (!cpu.isNull()) {
	insert(CPUString, cpu);
 }

 QString mtrr = readFile("/proc/mtrr");
 if (!mtrr.isNull()) {
	insert(BoInfo::MTRRString, mtrr);
 } else {
	insert(BoInfo::MTRRString, QString::fromLatin1(""));
 }

 QString osType = readFile("/proc/sys/kernel/ostype");
 if (!osType.isNull()) {
	insert(BoInfo::OSTypeString, osType);
 }
 insert(BoInfo::OSTypeString + BoInfo::CompileOffset, QString::fromLatin1("Linux")); // we are in a #ifdef __linux__

 QString kernelVersion = readFile("/proc/sys/kernel/osrelease");
 if (!kernelVersion.isNull()) {
	insert(OSVersionString, kernelVersion);
 }

 kernelVersion = UTS_RELEASE;
 insert(BoInfo::OSVersionString + BoInfo::CompileOffset, kernelVersion);

 QString kernelModules = readFile("/proc/modules");
 if (!kernelModules.isEmpty()) {
	QStringList list = QStringList::split('\n', kernelModules);
	QString nvidia = QString::fromLatin1("");
	QString tdfx = QString::fromLatin1("");
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i].contains(QRegExp("^tdfx\\s*"))) {
			if (!tdfx.isEmpty()) {
				boWarning() << k_funcinfo << "found more than one tdfx kernel modules" << endl;
				insert(BoInfo::KernelModuleTDFXString, tdfx);
			}
			tdfx = list[i];
		} else if (list[i].contains(QRegExp("^nvidia\\s*")) || list[i].contains(QRegExp("^NVdriver\\s*"))) {
			// AB: with version 4191 nvidia changed the name of the
			// kernel driver from "NVdriver" to "nvidia".
			// this might cause even more trouble for us...
			if (!nvidia.isEmpty()) {
				boWarning() << k_funcinfo << "found more than one nvidia kernel modules" << endl;
				insert(BoInfo::KernelModuleNVidiaString, nvidia);
			}
			nvidia = list[i];
		}
		// I am sure there are more cards out there which use a kernel
		// module (what about ATI cards for example?)
		// but I don't know the exact name of them. we'll add them one
		// day...
	}
	insert(BoInfo::KernelModuleTDFXString, tdfx);
	insert(BoInfo::KernelModuleNVidiaString, nvidia);
 }
}

float BoInfo::cpuSpeed() const
{
 QString cpu = getString(CPUString);
 if (cpu.isEmpty()) {
	return -1.0f;
 }
 QRegExp exp("^cpu MHz\\s*:\\s*");
 QStringList list = QStringList::split('\n', cpu);
 for (unsigned int i = 0; i < list.count(); i++) {
	if (exp.search(list[i]) >= 0) {
		QString s = list[i];
		s.replace(exp, QString::fromLatin1(""));
		bool ok = true;
		float ret = 0.0f;
		ret = s.toFloat(&ok);
		if (ok) {
			return ret;
		} else {
			// an error occured...
			// can ret still be valid?
			return -1.0f;
		}
	}
 }
 return -1.0f;
}

bool BoInfo::haveMtrr() const
{
 return !getString(MTRRString).isEmpty();
}

