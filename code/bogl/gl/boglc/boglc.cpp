/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qstringlist.h>

struct Function {
	QCString name;
	QCString returnType;
	QCString parameters;
};

static void writeHeader(QTextStream& output, const QString& inputFileName);

int main(int argc, char** argv)
{
 QString fileName;
 QString outFileName;
 for (int i = 1; i < argc; i++) {
	if (strcmp(argv[i], "--input") == 0) {
		if (argc <= i + 1) {
			fprintf(stderr, "Expected input filename after --input\n");
			return 1;
		}
		i++;
		fileName = argv[i];
	} else if (strcmp(argv[i], "--output") == 0) {
		if (argc <= i + 1) {
			fprintf(stderr, "Expected output basename after --output\n");
			return 1;
		}
		i++;
		outFileName = argv[i];
	}
 }
 if (fileName.isEmpty()) {
	fprintf(stderr, "no --input filename given\n");
	return 1;
 }
 if (outFileName.isEmpty()) {
	fprintf(stderr, "no --output filename given\n");
	return 1;
 }
 QFile inputFile(fileName);
 if (!inputFile.open(IO_ReadOnly)) {
	fprintf(stderr, "Could not open file %s\n", fileName.latin1());
	return 1;
 }

 QStringList lines;
 QValueList<Function> functions;

 QTextStream input(&inputFile);
 while (!input.atEnd()) {
	QString line = input.readLine();
	if (line.isNull()) {
		break;
	}
	if (line.startsWith("#")) {
		continue;
	}
	if (line.isEmpty()) {
		lines.append(QString::fromLatin1(""));
		continue;
	}
	if (line.startsWith("define") ||
			line.startsWith("ifndef") ||
			line.startsWith("ifdef") ||
			line.startsWith("endif") ||
			line.startsWith("include")) {
		lines.append(QString::fromLatin1("#") + line);
		continue;
	}
	if (line.startsWith("typedef ")) {
		lines.append(line);
		continue;
	}
	if (line.startsWith("function ")) {
		QString origLine = line;
		line = line.right(line.length() - QString::fromLatin1("function ").length());
		QStringList parts = QStringList::split('.', line, true);
		if (parts.count() != 3) {
			fprintf(stderr, "parse error: invalid function format %s\n", origLine.latin1());
			return 1;
		}
		Function f;
		f.returnType = parts[0].stripWhiteSpace();
		f.name = parts[1].stripWhiteSpace();
		f.parameters = parts[2].stripWhiteSpace();
		functions.append(f);
		continue;
	}

	fprintf(stderr, "parse error: unrecognized line %s\n", line.latin1());
	return 1;
 }

 QFile outputFile(outFileName);
 if (!outputFile.open(IO_WriteOnly)) {
	fprintf(stderr, "Could not open file %s\n", outFileName.latin1());
	return 1;
 }
 QFileInfo info(outFileName);
 QString includeGuard = info.fileName().upper().replace('.', "_");
 QTextStream output(&outputFile);
 writeHeader(output, fileName);
 output << "#ifndef " << includeGuard << "\n";
 output << "#define " << includeGuard << "\n";
 output << "\n";
 output << "#ifndef BOGL_H\n";
 output << "#error Never include this file directly! Include bogl.h instead!\n";
 output << "#endif\n";
 output << "\n";
 output << "#include \"bogl/bogl_do_dlopen.h\"\n";
 output << "\n";

 for (QStringList::iterator it = lines.begin(); it != lines.end(); ++it) {
	output << *it << "\n";
 }

 if (!functions.isEmpty()) {
	output << "\n";
	output << "extern \"C\" {\n";
	// TODO: support for comments and newlines between functions!

	output << "\t// typedefs\n";
	for (unsigned int i = 0; i < functions.count(); i++) {
		Function f = functions[i];
		output << "\ttypedef " << f.returnType << " ";
		output << "(*_" << f.name << ")";
		output << "(" << f.parameters << ");\n";
	}

	output << "\n";
	output << "\n";

	output << "\t// Function pointers\n";
	for (unsigned int i = 0; i < functions.count(); i++) {
		QString name = functions[i].name;
		output << "\textern _" << name << " bo_" << name << ";\n";
	}
	output << "}\n";

	output << "\n";
	output << "\n";

	output << "#if BOGL_DO_DLOPEN\n";
	for (unsigned int i = 0; i < functions.count(); i++) {
		QString name = functions[i].name;
		output << "#define " << name << " bo_" << name << "\n";
	}
	output << "#endif // BOGL_DO_DLOPEN\n";
 }

 output << "\n";
 output << "#endif // " << includeGuard << "\n";

 return 0;
}

// "header" as in "footer", not ".h file".
static void writeHeader(QTextStream& output, const QString& inputFileName)
{
 output << "// This file was generated from " << inputFileName << "\n";
 output << "// Do not edit this file (all changes will be lost)!" << "\n";
}


