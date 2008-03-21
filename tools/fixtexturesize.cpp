/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

// AB: remember to set $QTDIR before compiling this or your compiler won't find
// this
#include <qimage.h>
#include <qfile.h>
#include <stdlib.h>

int nextPower2(int);
void usage(const char* command);

int main(int argc, const char** argv)
{
 QStringList files;
 if (argc < 2) {
	usage(argv[0]);
	exit(0);
 }
 for (int i = 1; i < argc; i++) {
	files.append(argv[i]);
 }
 for (unsigned int i = 0; i < files.count(); i++) {
	QImage img(files[i]);
	if (img.isNull()) {
		qWarning("Could not load %s", files[i].latin1());
	}
	int w = nextPower2(img.width());
	int h = nextPower2(img.height());
	if (w == img.width() && h == img.height()) {
		qDebug("Won't modify %s", files[i].latin1());
	} else {
		qDebug("Rescale %s to %dx%d", files[i].latin1(), w, h);
		img = img.scale(w, h, QImage::ScaleFree);
		if (!img.save(files[i], QImage::imageFormat(files[i]))) {
			qFatal("Could not save %s", files[i].latin1());
			exit(1);
		}
	}
 }
 return 0;
}

void usage(const char* command)
{
 printf("Usage: %s [files...]\n", command);
}

int nextPower2(int n)
{
 if (n <= 0) {
	return 1;
 }
 int i = 1;
 while (n > i) {
	i *= 2;
 }
 return i;
}

