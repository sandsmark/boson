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

#include <qthread.h>
#include <qfile.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "boaudiothread.h"
#include "boaudiocommand.h"
#include "bosonaudiointerface.h"
#include "../boversion.h"

#include <stdlib.h>
#include <unistd.h>

BoAudioCommand* parseCommand(QString command);
bool parseInt(QString& command, int* result);
bool parseString(QString& command, QString* result);

static const char* description = "Boson Audio Process";

int main(int argc, char **argv)
{
 // we have to use KApplication, as we use kapp->random() in BosonSound!
 KAboutData about("bosonaudioprocess",
		"Boson Audio Process",
		BOSON_VERSION_STRING,
		description,
		KAboutData::License_GPL,
		"(C) 2003 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor("Andreas Beckermann", I18N_NOOP("Coding"), "b_mann@gmx.de");
 KCmdLineArgs::init(argc, argv, &about);
 KApplication app(false, false);


// this is the actual sound part:
 BoAudioThread* t = new BoAudioThread();
 t->start();

 // and now the communication with the other process.
 QFile readFile;
 readFile.open(IO_ReadOnly | IO_Raw, stdin);
 // we may want to write to stdout to let boson know about errors!
 QFile writeFile;
 writeFile.open(IO_ReadOnly | IO_Raw, stdout);

 QString command;
 QString buffer;
 //while (t->running()) {
 while (1){
	while (readFile.atEnd()) {
		usleep(500);
	}
	if (buffer.length() > 2048) {
		// a command of 2 KB? no, I don't believe this!
		fprintf(stderr, "command too long\n");
		buffer = QString::null;
		continue;
	}
	int ch = readFile.getch();
	if (ch == -1) {
		continue;
	}
	if (ch == '\n') {
		command = buffer;
		buffer = QString::null;
		BoAudioCommand* cmd = parseCommand(command);
		if (!cmd) {
			fprintf(stderr, "parsing error on command %s\n", command.latin1());
		} else {
			t->enqueueCommand(cmd);
		}
	} else {
		buffer.append((char)ch);
	}
 }

 readFile.close();
 writeFile.close();
 delete t;

 return 0;
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
 bool ok = parseInt(command, &type);
 if (!ok) {
	fprintf(stderr, "Could not parse type\n");
	return 0;
 }
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

