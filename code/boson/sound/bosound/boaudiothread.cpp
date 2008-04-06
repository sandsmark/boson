/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "boaudiothread.h"
#include "boaudiothread.moc"

#include "boaudiocommand.h"
#include "bosonaudio.h"
#include "bodebug.h"

#include <q3ptrqueue.h>
#include <qfile.h>


static BoAudioCommand* parseCommand(QString command);
static bool parseInt(QString& command, int* result);
static bool parseString(QString& command, QString* result);

static QString g_buffer;

class BoAudioThreadPrivate
{
public:
	BoAudioThreadPrivate()
	{
		mAudio = 0;
	}

	BosonAudio* mAudio;

	Q3PtrQueue<BoAudioCommand> mCommandQueue;
};

BoAudioThread::BoAudioThread()
{
 d = new BoAudioThreadPrivate;

 boDebug(200) << k_funcinfo << "construct BosonAudio" << endl;
 d->mAudio = BosonAudio::create();
 if (!d->mAudio) {
	boDebug(200) << k_funcinfo << "NULL BosonAudio constructed" << endl;
 } else {
	boDebug(200) << k_funcinfo << "BosonAudio constructed" << endl;
 }
}

BoAudioThread::~BoAudioThread()
{
 boDebug(200) << k_funcinfo << endl;
 delete d->mAudio;
 delete d;
 boDebug(200) << k_funcinfo << "done" << endl;
}

BosonAudio* BoAudioThread::audio() const
{
 return d->mAudio;
}

bool BoAudioThread::audioStarted() const
{
 bool ret = true;
 if (!audio()) {
	ret = false;
 }
 if (ret && audio()->isNull()) {
	ret = false;
 }
 return ret;
}

void BoAudioThread::processCommand()
{
 if (d->mCommandQueue.isEmpty()) {
	return;
 }
 BoAudioCommand* command = dequeueCommand();
 if (!command) {
	BO_NULL_ERROR(command);
	return;
 }
 executeCommand(command);
}

void BoAudioThread::executeCommand(BoAudioCommand* command)
{
 BO_CHECK_NULL_RET(command);

 audio()->executeCommand(command);
}

void BoAudioThread::enqueueCommand(BoAudioCommand* command)
{
 d->mCommandQueue.enqueue(command);
}

BoAudioCommand* BoAudioThread::dequeueCommand()
{
 return d->mCommandQueue.dequeue();
}

void BoAudioThread::slotReceiveStdin(int sock)
{
 if (g_buffer.length() > 2048) {
	// a command of 2 KB? no, I don't believe this!
	fprintf(stderr, "command too long\n");
	g_buffer = QString::null;
	return;
 }
 QFile readFile;
 readFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered, sock);
 int ch = readFile.getch();
 if (ch == -1) {
	return;
 }
 if (ch == '\n') {
	QString command = g_buffer;
	g_buffer = QString::null;
	BoAudioCommand* cmd = parseCommand(command);
	if (!cmd) {
		fprintf(stderr, "parsing error on command %s\n", command.latin1());
	} else {
		enqueueCommand(cmd);
	}
 } else {
	g_buffer.append((char)ch);
 }
 processCommand();
}

BoAudioCommand* parseCommand(QString command)
{
 // first the type
 int type = 0;
 int isSound = 0;
 QString species;
 int dataInt = 0;
 QString dataString1;
 QString dataString2;

 // AB: _all_ commands start with the type (number) - see BoAudioCommand::Command
 bool ok = parseInt(command, &type);
 if (!ok) {
	fprintf(stderr, "Could not parse type\n");
	return 0;
 }

 // the second argument is _always_ a 0 (music message) or a 1 (sound message).
 ok = parseInt(command, &isSound);
 if (!ok) {
	fprintf(stderr, "Could not parse sound/music tag\n");
	return 0;
 }
 if (isSound == 0) {
	// it is a music message.
	isSound = false;
 } else if (isSound == 1) {
	isSound = true;
	ok = parseString(command, &species);
	if (!ok) {
		fprintf(stderr, "Could not parse species\n");
		return 0;
	}
 } else {
	fprintf(stderr, "Invalid sound/music tag %d\n", isSound);
 }

 ok = parseInt(command, &dataInt);
 if (!ok) {
	fprintf(stderr, "Could not parse dataInt\n");
	return 0;
 }
 ok = parseString(command, &dataString1);
 if (!ok) {
	fprintf(stderr, "Could not parse dataString1\n");
	return 0;
 }
 ok = parseString(command, &dataString2);
 if (!ok) {
	fprintf(stderr, "Could not parse dataString2\n");
	return 0;
 }

 BoAudioCommand* cmd = 0;
 if (isSound) {
	cmd = new BoAudioCommand(type, species, dataInt, dataString1, dataString2);
 } else {
	cmd = new BoAudioCommand(type, dataInt, dataString1, dataString2);
 }
 return cmd;
}

bool parseInt(QString& command, int* result)
{
 bool ok = false;
 QString s;
 int index = command.find(' ');
 if (index >= 0) {
	s = command.left(index);
 } else {
	s = command;
 }
 if (s.isEmpty()) {
	fprintf(stderr, "Could not parse integer - command: %s\n", command.latin1());
	return 0;
 }
 *result = s.toInt(&ok);
 if (!ok) {
	fprintf(stderr, "Parsed value not an integer - command: %s\n", command.latin1());
 }
 if (index >= 0) {
	command = command.right(command.length() - index - 1);
 } else {
	command = QString::null;
 }
 return ok;
}

bool parseString(QString& command, QString* result)
{
 QString s;
 int index = command.find(' ');
 if (index >= 0) {
	s = command.left(index);
	command = command.right(command.length() - index - 1);
 } else {
	s = command;
	command = QString::null;
 }
 // note: an empty string is perfectly valid!
 *result = s;
 return true;
}

