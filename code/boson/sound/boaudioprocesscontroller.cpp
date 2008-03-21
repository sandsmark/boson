/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "boaudioprocesscontroller.h"
#include "boaudioprocesscontroller.moc"

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"

#include <boaudiocommand.h>

#include <kprocess.h>

#include <qstringlist.h>
#include <qptrqueue.h>

#include <stdio.h>
#include <string.h>

class BoAudioProcessControllerPrivate
{
public:
	BoAudioProcessControllerPrivate()
	{
		mProcess = 0;
		mBuffer = 0;
	}
	KProcess* mProcess;
	QPtrQueue<BoAudioCommand> mCommandQueue;
	char* mBuffer;
};

BoAudioProcessController::BoAudioProcessController()
{
 d = new BoAudioProcessControllerPrivate;
}

BoAudioProcessController::~BoAudioProcessController()
{
 delete d->mProcess;
 delete[] d->mBuffer;
 delete d;
}

bool BoAudioProcessController::start()
{
 if (!d->mProcess) {
	d->mProcess = new KProcess();
	connect(d->mProcess, SIGNAL(receivedStdout(KProcess*, char*, int)),
			this, SLOT(slotShowStdout(KProcess*, char*, int)));
	connect(d->mProcess, SIGNAL(receivedStderr(KProcess*, char*, int)),
			this, SLOT(slotShowStderr(KProcess*, char*, int)));
	connect(d->mProcess, SIGNAL(processExited(KProcess*)),
			this, SLOT(slotProcessExited(KProcess*)));
	connect(d->mProcess, SIGNAL(wroteStdin(KProcess*)),
			this, SLOT(slotWroteStdin(KProcess*)));
 } else if (d->mProcess->isRunning()) {
	boWarning(200) << k_funcinfo << "process already running" << endl;
	return true;
 }
 QString processPath = QString::null;
#warning FIXME: check for existance ; use KStandardDirs
 processPath = "bosonaudioprocess"; // will be found in $KDEDIR/bin

 *(d->mProcess) << processPath;

 boDebug(200) << k_funcinfo << "starting process" << endl;
 return d->mProcess->start(KProcess::NotifyOnExit, KProcess::All);
}

void BoAudioProcessController::slotShowStdout(KProcess* , char* buffer, int length)
{
 QString s = QString::fromLatin1(buffer, length);
 QStringList list = QStringList::split('\n', s);
 QStringList::Iterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	s = QString::fromLatin1("Process: ") + *it + '\n';
	fprintf(stdout, s.latin1());
 }
}

void BoAudioProcessController::slotShowStderr(KProcess* , char* buffer, int length)
{
 QString s = QString::fromLatin1(buffer, length);
 QStringList list = QStringList::split('\n', s);
 QStringList::Iterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	s = QString::fromLatin1("Process: ") + *it + '\n';
	fprintf(stderr, s.latin1());
 }
}

bool BoAudioProcessController::isRunning() const
{
 if (!d->mProcess) {
	return false;
 }
 return d->mProcess->isRunning();
}

void BoAudioProcessController::sendCommand(BoAudioCommand* command)
{
 if (!command) {
	// nothing to send
	return;
 }
 if (!isRunning()) {
	boWarning(200) << k_funcinfo << "not running" << endl;
	return;
 }
 if (d->mBuffer && !d->mCommandQueue.isEmpty()) {
	d->mCommandQueue.enqueue(command);
	return;
 }

 QString buffer;
 buffer += QString::number(command->type());
 buffer += ' ';
 if (command->species().isEmpty()) {
	buffer += QString::number(0);
 } else {
	buffer += QString::number(1);
	buffer += ' ';
	buffer += command->species();
 }
 buffer += ' ';
 buffer += QString::number(command->dataInt());
 buffer += ' ';
 if (command->dataString1().isEmpty()) {
	// send nothing
 } else {
	buffer += command->dataString1();
 }
 buffer += ' ';
 if (command->dataString2().isEmpty()) {
	// send nothing
 } else {
	buffer += command->dataString2();
 }

// boDebug(200) << k_funcinfo << "sending: " << buffer << endl;
 buffer += '\n';
 delete[] d->mBuffer;
 d->mBuffer = new char[buffer.length()];
 memcpy(d->mBuffer, buffer.latin1(), buffer.length());
 bool ok = d->mProcess->writeStdin(d->mBuffer, buffer.length());
 if (!ok) {
	boWarning(200) << k_funcinfo << "Unable to send the command to the process! (will retry later)" << endl;
	d->mCommandQueue.enqueue(command);
 } else {
	delete command;
 }
}

void BoAudioProcessController::slotProcessExited(KProcess*)
{
 while (!d->mCommandQueue.isEmpty()) {
	BoAudioCommand* c = d->mCommandQueue.dequeue();
	delete c;
 }
 delete[] d->mBuffer;
 d->mBuffer = 0;
}

void BoAudioProcessController::slotWroteStdin(KProcess*)
{
 delete[] d->mBuffer;
 d->mBuffer = 0;
 if (!d->mCommandQueue.isEmpty()) {
	sendCommand(d->mCommandQueue.dequeue());
 }
}

