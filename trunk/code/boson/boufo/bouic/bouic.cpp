/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>

static bool openFile(const QString& file, QDomDocument* doc);
static bool saveTo(const QString& fileBase, QDomDocument& doc);
static bool writeDeclaration(QTextStream& header, QDomElement& widgets, const QString& fileNameBase);
static bool writeImplementation(QTextStream& cpp, QDomElement& widgets, const QString& fileNameBase);
static bool writeConstructorImplementation(QTextStream& cpp, QDomElement& widgets, const QString& fileNameBase);
static bool writeCreateChildren(QTextStream& cpp, QDomElement& e, const QString& parent);
static bool writePropertiesList(QTextStream& cpp, QDomElement& root);

const char* g_headline = "/* TODO: write some clever stuff into the header. e.g. \"Do not edit this file by hand \" */ \n\n";

 // FIXME: atm the "name" attribute is the variable name. the "name" of the Widgets tag should become the classname of the resulting file
QString g_className;
QStringList g_addIncludes;

int main(int argc, char** argv)
{
 QString fileName;
 QString outFileNameBase;
 g_addIncludes.clear();
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
		outFileNameBase = argv[i];
	} else if (strcmp(argv[i], "--addinclude") == 0) {
		if (argc <= i + 1) {
			fprintf(stderr, "Expected header file --addinclude\n");
			return 1;
		}
		i++;
		g_addIncludes.append(argv[i]);
	}
 }
 if (fileName.isEmpty()) {
	fprintf(stderr, "no --input filename given\n");
	return 1;
 }
 if (outFileNameBase.isEmpty()) {
	fprintf(stderr, "no --output basename given\n");
	return 1;
 }
 QDomDocument doc("BoDesigner");
 if (!openFile(fileName, &doc)) {
	fprintf(stderr, "Error reading %s\n", fileName.latin1());
	return 1;
 }

 if (!saveTo(outFileNameBase, doc)) {
	fprintf(stderr, "could not output .cpp and .h files\n");
	return 1;
 }
 return 0;
}

bool openFile(const QString& fileName, QDomDocument* doc)
{
 if (fileName.isEmpty()) {
	fprintf(stderr, "No filename\n");
	return false;
 }
 QFile file(fileName);
 if (!file.open(IO_ReadOnly)) {
	fprintf(stderr, "Could not open file %s\n", fileName.latin1());
	return false;
 }
 if (!doc->setContent(&file)) {
	fprintf(stderr, "Could not parse file %s\n", fileName.latin1());
	return false;
 }
 file.close();
 QDomElement root = doc->documentElement();
 if (root.isNull()) {
	fprintf(stderr, "NULL root element\n");
	return false;
 }
 QDomElement widgets = root.namedItem("Widgets").toElement();
 if (widgets.isNull()) {
	fprintf(stderr, "NULL Widgets element\n");
	return false;
 }
 return true;
}

bool saveTo(const QString& fileBase, QDomDocument& doc)
{
 QFile fileCpp(fileBase + ".cpp");
 QFile fileH(fileBase + ".h");
 if (!fileCpp.open(IO_WriteOnly)) {
	fprintf(stderr, "Could not open %s.cpp for writing\n", fileBase.latin1());
	return false;
 }
 if (!fileH.open(IO_WriteOnly)) {
	fprintf(stderr, "Could not open %s.h for writing\n", fileBase.latin1());
	return false;
 }
 QTextStream cpp(&fileCpp);
 QTextStream header(&fileH);
 QDomElement root = doc.documentElement();
 QDomElement widgets = root.namedItem("Widgets").toElement();

 g_className = widgets.attribute("name");
 if (!writeDeclaration(header, widgets, fileBase)) {
	fprintf(stderr, "Unable to write declaration\n");
	return false;
 }
 if (!writeImplementation(cpp, widgets, fileBase)) {
	fprintf(stderr, "Unable to write implementation\n");
	return false;
 }
 return true;
}

bool writeDeclaration(QTextStream& header, QDomElement& root, const QString& fileNameBase)
{
 header << g_headline;

 header << "#ifndef " << fileNameBase.upper() << "_H\n";
 header << "#define " << fileNameBase.upper() << "_H\n";
 header << "\n";

 // AB: this is hardcoded and will probably stay like this
 header << "#include \"boufo/boufo.h\"\n";
 header << "\n";

 header << "class " << g_className << " : public " << root.attribute("ClassName") << "\n{\n";

 // AB: since we support BoUfoWidget derived classes only, our widgets are
 // always QObjects.
 header << "\tQ_OBJECT\n";

 header << "public:\n";

 // we always provide one c'tor and a d'tor.
 header << "\t" << g_className << "();\n";
 header << "\t~" << g_className << "();\n";
 header << "\n";

 // AB: all member widgets are public. we do this because if we'd use protected,
 // then we would always have to derive this widget to use it - or we'd have to
 // add set/get methods using the designer for every single property that we'd
 // like to change at any time. that would be very inconvenient.
 header << "public: // member variables\n";
 QDomNodeList list = root.elementsByTagName("Widget");
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	header << "\t" << e.attribute("ClassName") << "* " << e.attribute("name") << ";\n";
 }

 header << "};\n\n";
 header << "#endif\n";

 return true;
}

// TODO: use i18n() whenever a string that is to be translated occurs.
// especially on all label->setText() calls or so.
bool writeImplementation(QTextStream& cpp, QDomElement& root, const QString& fileNameBase)
{
 cpp << g_headline;

 cpp << "#include \"" << fileNameBase << ".h\"\n";
 cpp << "#include \"" << fileNameBase << ".moc\"\n";
 cpp << "\n";
 cpp << "#include <qmap.h>\n"; // for loading the properties
 cpp << "\n";
 for (unsigned int i = 0; i < g_addIncludes.count(); i++) {
	cpp << "#include <" << g_addIncludes[i] << ">\n";
 }
 cpp << "\n";

 cpp << g_className << "::" << g_className << "()\n";
 cpp << "\t: " << root.attribute("ClassName") << "()\n";
 cpp << "{\n";
 if (!writeConstructorImplementation(cpp, root, fileNameBase)) {
	fprintf(stderr, "Could not write c'tor implementation\n");
	return false;
 }
 cpp << "}\n\n";

 cpp << g_className << "::~" << g_className << "()\n";
 cpp << "{\n";
 // not much to do, as libufo handles deletions
 cpp << "}\n\n";


 return true;
}

bool writeConstructorImplementation(QTextStream& cpp, QDomElement& root, const QString& fileNameBase)
{
 // TODO: this->loadPropertiesFromXML(root.attributes())

 cpp << " QMap<QString, QString> properties;\n";
 if (!writePropertiesList(cpp, root)) {
	return false;
 }
 cpp << " loadProperties(properties);\n";

 return writeCreateChildren(cpp, root, QString::null);
}

bool writeCreateChildren(QTextStream& cpp, QDomElement& root, const QString& parent)
{
 QDomNode n = root.firstChild();
 while (!n.isNull()) {
	QDomElement e = n.toElement();
	n = n.nextSibling();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Widget") {
		continue;
	}
	QString className = e.attribute("ClassName");
	QString name = e.attribute("name");
	if (className.isEmpty()) {
		fprintf(stderr, "empty ClassName\n");
		return false;
	}
	if (name.isEmpty()) {
		fprintf(stderr, "empty name\n");
		return false;
	}
	cpp << " " << name << " = new " << className << "();\n";

	if (!writePropertiesList(cpp, e)) {
		fprintf(stderr, "could not write properties list\n");
		return false;
	}
	cpp << " " << name << "->loadProperties(properties);\n";

	cpp << " ";
	if (!parent.isEmpty()) {
		cpp << parent << "->";
	}
	cpp << "addWidget(" << name << ");\n";
	cpp << "\n";

	if (!writeCreateChildren(cpp, e, name)) {
		return false;
	}
 }
 return true;
}

bool writePropertiesList(QTextStream& cpp, QDomElement& root)
{
 // AB: atm all attributes of a Widgets or Widget tag are properties. if
 // that changes one day, we should consider placing the properties into a
 // separate child tag.
 cpp << " properties.clear();\n";
 QDomNamedNodeMap attributes = root.attributes();
 for (unsigned int i = 0; i < attributes.count(); i++) {
	QDomAttr a = attributes.item(i).toAttr();
	cpp << " properties.insert(\"" << a.name() << "\", \"" << a.value() << "\");\n";
 }
 return true;
}

