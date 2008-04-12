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

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"

#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3TextStream>

#include <linux/version.h>

QString readFile(QString fileName)
{
 QFile file(fileName);
 if (!file.exists()) {
	return QString();
 }
 if (!file.open(QIODevice::ReadOnly)) {
	return QString();
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

#warning FIXME: use uname(); !
 QString osType = readFile("/proc/sys/kernel/ostype");
 if (!osType.isNull()) {
	insert(BoInfo::OSTypeString, osType);
 }
 insert(BoInfo::OSTypeString + BoInfo::CompileOffset, QString::fromLatin1("Linux")); // we are in a #ifdef __linux__

 QString kernelVersion = readFile("/proc/sys/kernel/osrelease");
 if (!kernelVersion.isNull()) {
	insert(OSVersionString, kernelVersion);
 }

#ifdef UTS_RELEASE
 kernelVersion = UTS_RELEASE;
#else
 kernelVersion = "(Unknown)";
#endif
 insert(BoInfo::OSVersionString + BoInfo::CompileOffset, kernelVersion);

 QString kernelModules = readFile("/proc/modules");
 if (!kernelModules.isEmpty()) {
	QStringList list = kernelModules.split('\n');
	QString nvidia = QString::fromLatin1("");
	QString tdfx = QString::fromLatin1("");
	for (int i = 0; i < list.count(); i++) {
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
 QStringList list = cpu.split('\n');
 for (int i = 0; i < list.count(); i++) {
	if (exp.indexIn(list[i]) >= 0) {
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


bool BoCurrentInfo::memoryInUse(QString* vmSize, QString* vmLck, QString* vmRSS,
		QString* vmData, QString* vmStk, QString* vmExe, QString* vmLib,
		QString* vmPTE) const
{
 int pid = (int)getpid();
 QString status = readFile(QString("/proc/%1/status").arg(pid));
 if (status.isEmpty()) {
	return false;
 }
 // we need the lines starting with Vm only
 QStringList list = status.split('\n');
 QStringList vm = list.filter(QRegExp("^Vm\\S+:.*"));
 if (vm.count() == 0) {
	boError() << k_funcinfo << "file could be opened, but Vm* entries could not be read" << endl;
	return false;
 }

 if (vmSize) {
	QString* string = vmSize;
	QStringList l = vm.filter(QRegExp("^VmSize:.*"));
	if (l.count() == 0) {
		*string = QString();
	} else {
		*string = l[0].remove(QRegExp("^Vm\\S*\\s*"));
	}
 }
 if (vmLck) {
	QString* string = vmLck;
	QStringList l = vm.filter(QRegExp("^VmLck:.*"));
	if (l.count() == 0) {
		*string = QString();
	} else {
		*string = l[0].remove(QRegExp("^Vm\\S*\\s*"));
	}
 }
 if (vmRSS) {
	QString* string = vmRSS;
	QStringList l = vm.filter(QRegExp("^VmRSS:.*"));
	if (l.count() == 0) {
		*string = QString();
	} else {
		*string = l[0].remove(QRegExp("^Vm\\S*\\s*"));
	}
 }
 if (vmData) {
	QString* string = vmData;
	QStringList l = vm.filter(QRegExp("^VmData:.*"));
	if (l.count() == 0) {
		*string = QString();
	} else {
		*string = l[0].remove(QRegExp("^Vm\\S*\\s*"));
	}
 }
 if (vmStk) {
	QString* string = vmStk;
	QStringList l = vm.filter(QRegExp("^VmStk:.*"));
	if (l.count() == 0) {
		*string = QString();
	} else {
		*string = l[0].remove(QRegExp("^Vm\\S*\\s*"));
	}
 }
 if (vmExe) {
	QString* string = vmExe;
	QStringList l = vm.filter(QRegExp("^VmExe:.*"));
	if (l.count() == 0) {
		*string = QString();
	} else {
		*string = l[0].remove(QRegExp("^Vm\\S*\\s*"));
	}
 }
 if (vmLib) {
	QString* string = vmLib;
	QStringList l = vm.filter(QRegExp("^VmLib:.*"));
	if (l.count() == 0) {
		*string = QString();
	} else {
		*string = l[0].remove(QRegExp("^Vm\\S*\\s*"));
	}
 }
 if (vmPTE) {
	QString* string = vmPTE;
	QStringList l = vm.filter(QRegExp("^VmPTE:.*"));
	if (l.count() == 0) {
		*string = QString();
	} else {
		*string = l[0].remove(QRegExp("^Vm\\S*\\s*"));
	}
 }
 return true;
}


/**
 * This is a helper class that reads data from /proc/PID/stat
 **/
class ProcPIDStatInfo
{
public:
	bool read(int pid);
	void print();

	int mPID;
	QString mName;
	char mState;
	int mPPID;
	int mPGrpP;
	int mSession;
	int mTTY; // tty_nr
	int mTTYOwner; // tpgid
	unsigned long mFlags;
	unsigned long mMinFlt;
	unsigned long mCMinFlt;
	unsigned long mMajFlt;
	unsigned long mCMajFlt;
	unsigned long mUTime;
	unsigned long mSTime;
	long mCUTime;
	long mCSTime;
	long mPriority;
	long mNice;
	long mItRealValue;
	unsigned long mStartTime;
	unsigned long mVSize;
	long mRSS;
	unsigned long mRLim;
	unsigned long mStartCode;
	unsigned long mEndCode;
	unsigned long mStartStack;
	unsigned long mKStkesp;
	unsigned long mKstkeip;
	unsigned long mSignal;
	unsigned long mBlocked;
	unsigned long mSigIgnore;
	unsigned long mSigCatch;
	unsigned long mWChan;
	unsigned long mNSwap; // according to man proc, this is NOT maintained
	unsigned long mCNSwap; // (not maintained either, as it uses nswap)
	int mExitSignal;
	int mProcessor;

};

void ProcPIDStatInfo::print()
{
 boDebug() << "PID: "<< mPID << " name: " << mName << " state: " << mState << endl
		<< "Parent PID: " << mPPID << " process groupd ID: " << mPGrpP << endl
		<< "Session: " << mSession << " tty: " << mTTY << " tty owner: " << mTTYOwner << endl
		<< "Flags: " << mFlags << " minflt: " << mMinFlt << " cminflt: " << mCMinFlt << " majflt: " << mMajFlt << " cmajflt: " << mCMajFlt << endl
		<< "utime: " << mUTime << " stime: " << mSTime << " cutime: " << mCUTime << " cstime: " << mCSTime << endl
		<< "priority: " << mPriority << " nice: " << mNice << " itrealvalue: " << mItRealValue << endl
		<< "starttime: " << mStartTime << " vsize: " << mVSize << " RSS: " << mRSS << endl
		<< "rlim: " << mRLim << " startcode: " << mStartCode << " endcode: " << mEndCode << endl
		<< "kstkesp: " << mKStkesp << " kstkeip: " << mKstkeip << endl
		<< " signal: " << mSignal << " blocked: " << mBlocked << "sigignore: " << mSigIgnore << " sigcatch: " << mSigCatch << endl
		<< "wchan: " << mWChan << " nswap (not maintained, see man proc): " << mNSwap << " cnswap: " << mCNSwap << endl
		<< "exitsignal: " << mExitSignal << " processor: " << mProcessor << endl;
}

bool ProcPIDStatInfo::read(int pid)
{
 QFile file(QString("/proc/%1/stat").arg(pid));
 if (!file.open(QIODevice::ReadOnly)) {
	return false;
 }
 Q3TextStream s(&file);
 s.setf(Q3TextStream::dec);
 s >> mPID >> mName >> mState;
 s >> mPPID >> mPGrpP >> mSession >> mTTY >> mTTYOwner;
 s >> mFlags;
 s >> mMinFlt >> mCMinFlt >> mMajFlt>> mCMajFlt;
 s >> mUTime >> mSTime;
 s >> mCUTime >> mCSTime;
 long zero; // hardcoded to 0 according to man page .. is that true? seems to be 1 here
 s >> mPriority >> mNice >> zero >> mItRealValue;
 s >> mStartTime >> mVSize;
 s >> mRSS;
 s >> mRLim >> mStartCode >> mEndCode >> mStartStack;
 s >> mKStkesp >> mKstkeip >> mSignal >> mBlocked;
 s >> mSigIgnore >> mSigCatch >> mWChan >> mNSwap >> mCNSwap;
 s >> mExitSignal >> mProcessor;
 return true;
}

bool BoCurrentInfo::cpuTime(unsigned long int* utime, unsigned long int* stime, long int* cutime, long int* cstime) const
{
 int pid = (int)getpid();
 ProcPIDStatInfo info;
 if (!info.read(pid)) {
	return false;
 }
 if (utime) {
	*utime = info.mUTime;
 }
 if (stime) {
	*stime = info.mSTime;
 }
 if (cutime) {
	*cutime = info.mCUTime;
 }
 if (cstime) {
	*cstime = info.mCSTime;
 }
 return true;
}

