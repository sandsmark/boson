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

static bool generateGLHeader(const QString& outputFileName, QFile& inputFile);
static bool generateResolveFile(const QString& outputFileName, const QStringList& inputFiles);

static bool parseInput(QFile& inputFile, QStringList* lines, QValueList<Function>* functions);
static bool parseFunction(QString line, Function* function);

int main(int argc, char** argv)
{
 QStringList inputFiles;
 QString outFileName;

 // 0 == write a .h header
 // 1 == write a .cpp file to resolve the symbols using glXGetProcAddressARB()
 int mode = -1;

 for (int i = 1; i < argc; i++) {
	if (strcmp(argv[i], "--input") == 0) {
		if (argc <= i + 1) {
			fprintf(stderr, "Expected input filename after --input\n");
			return 1;
		}
		i++;
		QString fileName = argv[i];
		inputFiles.append(fileName);
	} else if (strcmp(argv[i], "--outputheader") == 0) {
		if (mode != -1) {
			fprintf(stderr, "tried mixing different output modes\n");
			return 1;
		}
		mode = 0;
		if (argc <= i + 1) {
			fprintf(stderr, "Expected output filename after --outputheader\n");
			return 1;
		}
		i++;
		outFileName = argv[i];
	} else if (strcmp(argv[i], "--outputresolve") == 0) {
		if (mode != -1) {
			fprintf(stderr, "tried mixing different output modes\n");
			return 1;
		}
		mode = 1;
		if (argc <= i + 1) {
			fprintf(stderr, "Expected output filename after --outputresolve\n");
			return 1;
		}
		i++;
		outFileName = argv[i];
	}
 }
 if (mode == -1) {
	fprintf(stderr, "no --outputheader or --outputresolve filename given\n");
	return 1;
 }
 if (inputFiles.empty()) {
	fprintf(stderr, "no --input filename given\n");
	return 1;
 }
 if (outFileName.isEmpty()) {
	fprintf(stderr, "no --outputheader or --outputresolve filename given\n");
	return 1;
 }

 if (mode == 0) {
	if (inputFiles.count() != 1) {
		fprintf(stderr, "can have only 1 --input filename for --outputheader\n");
		return 1;
	}
	QString inputFileName = inputFiles[0];
	QFile inputFile(inputFileName);
	if (!inputFile.open(IO_ReadOnly)) {
		fprintf(stderr, "Could not open file %s\n", inputFileName.latin1());
		return 1;
	}
	if (!generateGLHeader(outFileName, inputFile)) {
		return 1;
	}
 } else if (mode == 1) {
	if (!generateResolveFile(outFileName, inputFiles)) {
		return 1;
	}
 }


 return 0;
}

static bool parseInput(QFile& inputFile, QStringList* lines, QValueList<Function>* functions)
{
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
		lines->append(QString::fromLatin1(""));
		continue;
	}
	if (line.startsWith("define") ||
			line.startsWith("ifndef") ||
			line.startsWith("ifdef") ||
			line.startsWith("endif") ||
			line.startsWith("include")) {
		lines->append(QString::fromLatin1("#") + line);
		continue;
	}
	if (line.startsWith("typedef ")) {
		lines->append(line);
		continue;
	}
	if (line.startsWith("function ")) {
		Function f;
		if (!parseFunction(line, &f)) {
			return false;
		}
		functions->append(f);
		continue;
	}

	fprintf(stderr, "parse error: unrecognized line %s\n", line.latin1());
	return false;
 }
 return true;
}

static bool parseFunction(QString line, Function* function)
{
 QString origLine = line;
 line = line.right(line.length() - QString::fromLatin1("function ").length());
 QStringList parts = QStringList::split('.', line, true);
 if (parts.count() != 3) {
	fprintf(stderr, "parse error: invalid function format %s\n", origLine.latin1());
	return false;
 }
 function->returnType = parts[0].stripWhiteSpace();
 function->name = parts[1].stripWhiteSpace();
 function->parameters = parts[2].stripWhiteSpace();
 return true;
}

static bool generateGLHeader(const QString& outputFileName, QFile& inputFile)
{
 QStringList lines;
 QValueList<Function> functions;

 if (!parseInput(inputFile, &lines, &functions)) {
	return false;
 }

 QFile outputFile(outputFileName);
 if (!outputFile.open(IO_WriteOnly)) {
	fprintf(stderr, "Could not open file %s\n", outputFileName.latin1());
	return false;
 }
 QFileInfo inputInfo(inputFile);
 QFileInfo outputInfo(outputFileName);
 QString includeGuard = outputInfo.fileName().upper().replace('.', "_");
 QTextStream output(&outputFile);
 output << "// This file was generated from " << inputInfo.fileName() << "\n";
 output << "// Do not edit this file (all changes will be lost)!" << "\n";
 output << "\n";
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

	for (unsigned int i = 0; i < functions.count(); i++) {
		QString name = functions[i].name;
		output << "#define " << name << " bo_" << name << "\n";
	}
 }

 output << "\n";
 output << "#endif // " << includeGuard << "\n";

 return true;
}

static bool generateResolveFile(const QString& outputFileName, const QStringList& inputFiles)
{
 QStringList lines;
 QValueList< QValueList<Function> > allFunctions;

 for (int i = 0; i < inputFiles.count(); i++) {
	QValueList<Function> functions;
	QFile inputFile(inputFiles[i]);
	if (!inputFile.open(IO_ReadOnly)) {
		fprintf(stderr, "Could not open file %s\n", inputFiles[i].latin1());
		return false;
	}
	if (!parseInput(inputFile, &lines, &functions)) {
		return false;
	}

	allFunctions.append(functions);
 }

 QFile outputFile(outputFileName);
 if (!outputFile.open(IO_WriteOnly)) {
	fprintf(stderr, "Could not open file %s\n", outputFileName.latin1());
	return false;
 }
 QFileInfo outputInfo(outputFile);

 QTextStream output(&outputFile);
 output << "// This file was generated from *.boglc files\n";
 output << "// Do not edit this file (all changes will be lost)!" << "\n";
 output << "\n";
 output << "#ifndef QT_CLEAN_NAMESPACE\n";
 output << "#define QT_CLEAN_NAMESPACE\n";
 output << "#endif\n";
 output << "\n";
 output << "#include <bogl.h>\n";
 output << "#include <boglx.h>\n";
 output << "\n";

 // function pointer declarations
 output << "extern \"C\" {\n";
 for (int fileIndex = 0; fileIndex < allFunctions.count(); fileIndex++) {
	if (allFunctions[fileIndex].isEmpty()) {
		continue;
	}
	if (fileIndex > 0) {
		output << "\n";
	}
	output << "\t// from " << QFileInfo(inputFiles[fileIndex]).fileName() << "\n";
	QValueList<Function> functions = allFunctions[fileIndex];
	for (int i = 0; i < functions.count(); i++) {
		QString name = functions[i].name;
		output << "\t_" << name << " bo_" << name << ";\n";
	}
 }
 output << "}\n";
 output << "\n";

 output << "bool bogl_resolveSymbols_GL()\n";
 output << "{\n";
 for (int fileIndex = 0; fileIndex < allFunctions.count(); fileIndex++) {
	if (allFunctions[fileIndex].isEmpty()) {
		continue;
	}
	if (fileIndex > 0) {
		output << "\n";
	}
	output << "\t// from " << QFileInfo(inputFiles[fileIndex]).fileName() << "\n";
	QValueList<Function> functions = allFunctions[fileIndex];
	for (int i = 0; i < functions.count(); i++) {
		QString name = functions[i].name;
		output << "\tbo_" << name << " = (_" << name << ")glXGetProcAddressARB((const GLubyte*)\"" << name << "\");\n";
	}
 }
 output << "\n";
 output << "\treturn true;\n";
 output << "}\n";
 output << "\n";

 return true;
}

