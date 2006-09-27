/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "loader-md2.h"

#include "debug.h"
#include "mesh.h"
#include "material.h"
#include "lod.h"
#include "model.h"
#include "frame.h"
#include "texture.h"

#include <qptrlist.h>
#include <qptrvector.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qtextstream.h>


#define SURFACE_SHADED (1<<4)
#define SURFACE_TWOSIDED (1<<5)

// AB: see http://linux.ucla.edu/~phaethon/q3a/formats/md2-schoenblum.html
//     or http://www.icculus.org/homepages/phaethon/q3/formats/md2-schoenblum.html

class MD2Header
{
public:
	bool load(QDataStream& stream);

	static const int MD2_MAX_TRIANGLES = 4096;
	static const int MD2_MAX_VERTICES = 2048;
	static const int MD2_MAX_TEXTURE_COORDINATES = 2048;
	static const int MD2_MAX_FRAMES = 512;
	static const int MD2_MAX_SKINS = 32;

	Q_INT32 mMagic;
	Q_INT32 mVersion;
	Q_INT32 mSkinWidth;
	Q_INT32 mSkinHeight;
	Q_INT32 mFrameSize;
	Q_INT32 mNumSkins; // number of textures
	Q_INT32 mNumVertices;
	Q_INT32 mNumTexCoords;
	Q_INT32 mNumTriangles;
	Q_INT32 mNumGLCommands;
	Q_INT32 mNumFrames;
	Q_INT32 mOffsetSkins;
	Q_INT32 mOffsetTexCoords;
	Q_INT32 mOffsetTriangles;
	Q_INT32 mOffsetFrames;
	Q_INT32 mOffsetGLCommands;
	Q_INT32 mOffsetEnd;

	// header size: 17 * 4 = 68 bytes
};


class MD2TriangleVertex
{
public:
	bool load(QDataStream& stream)
	{
		stream >> mVertex[0];
		stream >> mVertex[1];
		stream >> mVertex[2];
		stream >> mLightNormalIndex;
		return true;
	}

	Q_UINT8 mVertex[3];
	Q_UINT8 mLightNormalIndex;
};

class MD2Frame
{
public:
	MD2Frame()
	{
		mTriangleVertices = 0;
	}
	~MD2Frame()
	{
		delete[] mTriangleVertices;
	}
	bool load(QDataStream& stream, int numVertices)
	{
		stream >> mScale[0];
		stream >> mScale[1];
		stream >> mScale[2];
		stream >> mTranslate[0];
		stream >> mTranslate[1];
		stream >> mTranslate[2];
		for (int i  = 0; i < 16; i++) {
			stream >> mName[i];
		}
		delete[] mTriangleVertices;
		mTriangleVertices = new MD2TriangleVertex[numVertices];
		for (int i = 0; i < numVertices; i++) {
			if (!mTriangleVertices[i].load(stream)) {
				return false;
			}
		}
		return true;
	}

	float mScale[3];
	float mTranslate[3];
	Q_INT8 mName[16];
	MD2TriangleVertex* mTriangleVertices;
};

class MD2Triangle
{
public:
	bool load(QDataStream& stream)
	{
		stream >> mVertexIndices[0];
		stream >> mVertexIndices[1];
		stream >> mVertexIndices[2];
		stream >> mTextureIndices[0];
		stream >> mTextureIndices[1];
		stream >> mTextureIndices[2];
		return true;
	}

	Q_INT16 mVertexIndices[3];
	Q_INT16 mTextureIndices[3];
};

class MD2Skin // a texture
{
public:
	bool load(QDataStream& stream)
	{
		for (int i = 0; i < 64; i++) {
			stream >> mName[i];
		}
		return true;
	}
	Q_INT8 mName[64];

};

class MD2TextureCoordinate
{
public:
	bool load(QDataStream& stream)
	{
		stream >> mS;
		stream >> mT;
		return true;
	}

	Q_INT16 mS, mT;
};

class MD2GLCommandVertex
{
public:
	bool load(QDataStream& stream)
	{
		stream >> mS;
		stream >> mT;
		stream >> mVertexIndex;
		return true;
	}

	float mS;
	float mT;
	Q_INT32 mVertexIndex;
};

class MD2GLCommand
{
public:
	MD2GLCommand()
	{
		mVertices = 0;
	}
	~MD2GLCommand()
	{
		delete[] mVertices;
	}

	bool load(QDataStream& stream, int num)
	{
		if (num > 0) {
			// triangle strips
			mIsStrip = true;
		} else {
			// triangle fans
			mIsStrip = false;
			num = -num;
		}
		if (num > 4096) {
			// MD2 files must not have more than 4096 triangles, so
			// we cannot have even more GL commands.
			boError(100) << k_funcinfo << "tried to load " << num << " GL commands" << endl;
			return false;
		}
//		boDebug(100) << k_funcinfo << "strip/fan with " << num << " vertices" << endl;
		mVertices = new MD2GLCommandVertex[num];
		for (int i = 0; i < num; i++) {
			mVertices[i].load(stream);
		}
		return true;
	}

	bool mIsStrip; // TRUE: strip, FALSE: fan
	MD2GLCommandVertex* mVertices;
};


bool MD2Header::load(QDataStream& stream)
{
 stream >> mMagic;
 Q_INT32 magic =
	((((int)'I'))
	+ (((int)'D') << 8)
	+ (((int)'P') << 16)
	+ (((int)'2') << 24));
 if (mMagic != magic) {
	boError(100) << k_funcinfo << "invalid magix number. read: " << mMagic << " want: " << magic << endl;
	return false;
 }
 stream >> mVersion;
 if (mVersion != 8) {
	boError(100) << k_funcinfo << "invalid version. have: " << mVersion << " want: 8" << endl;
	return false;
 }
 stream >> mSkinWidth;
 stream >> mSkinHeight;
 stream >> mFrameSize;
 stream >> mNumSkins;
 stream >> mNumVertices;
 stream >> mNumTexCoords;
 stream >> mNumTriangles;
 stream >> mNumGLCommands;
 stream >> mNumFrames;
 stream >> mOffsetSkins;
 stream >> mOffsetTexCoords;
 stream >> mOffsetTriangles;
 stream >> mOffsetFrames;
 stream >> mOffsetGLCommands;
 stream >> mOffsetEnd;

 if (mNumSkins > MD2_MAX_SKINS) {
	boError(100) << k_funcinfo << "too many skins: " << mNumSkins << endl;
	return false;
 }
 if (mNumVertices > MD2_MAX_VERTICES) {
	boError(100) << k_funcinfo << "too many vertices: " << mNumVertices << endl;
	return false;
 }
 if (mNumTexCoords > MD2_MAX_TEXTURE_COORDINATES) {
	boError(100) << k_funcinfo << "too many texture coordinates: " << mNumTexCoords << endl;
	return false;
 }
 if (mNumTriangles > MD2_MAX_TRIANGLES) {
	boError(100) << k_funcinfo << "too many triangles: " << mNumTriangles << endl;
	return false;
 }
 if (mNumFrames > MD2_MAX_FRAMES) {
	boError(100) << k_funcinfo << "too many frames: " << mNumFrames << endl;
	return false;
 }

#if 0
 boDebug(100) << "num skins: " << mNumSkins << endl;
 boDebug(100) << "num GLCommands: " << mNumGLCommands << endl;
 boDebug(100) << "offsetGLCommands: " << mOffsetGLCommands << endl;
 boDebug(100) << "offsetGLEnd: " << mOffsetEnd << endl;
#endif

 int pos = sizeof(MD2Header);
 if (mOffsetSkins != pos) {
	boError(100) << k_funcinfo << "unexpected skin offset " << mOffsetSkins << " expected " << pos << endl;
	return false;
 }
 pos += (sizeof(MD2Skin) * mNumSkins);
 if (mOffsetTexCoords!= pos) {
	boError(100) << k_funcinfo << "unexpected TexCoord offset " << mOffsetTexCoords << " expected " << pos << endl;
	return false;
 }
 pos += (sizeof(MD2TextureCoordinate) * mNumTexCoords);
 if (mOffsetTriangles != pos) {
	boError(100) << k_funcinfo << "unexpected triangle offset " << mOffsetTriangles << " expected " << pos << endl;
	return false;
 }
 pos += (sizeof(MD2Triangle) * mNumTriangles);
 if (mOffsetFrames != pos) {
	boError(100) << k_funcinfo << "unexpected frames offset " << mOffsetFrames << " expected " << pos << endl;
	return false;
 }
 pos += ((6 * 4 + 16) * mNumFrames + sizeof(MD2TriangleVertex) * mNumFrames * mNumVertices);
 if (mOffsetGLCommands != pos) {
	boError(100) << k_funcinfo << "unexpected GLCommands offset " << mOffsetGLCommands << " expected " << pos << endl;
	return false;
 }
 pos += (sizeof(Q_UINT32) * mNumGLCommands); // AB: note that sizeof(Q_UINT32) != sizeof(MD2GLCommandVertex). the numGLCommand stores number of 32bit variables that are used.
 if (mOffsetEnd != pos) {
	boError(100) << k_funcinfo << "unexpected end offset " << mOffsetEnd << " expected " << pos << endl;
	return false;
 }
 return true;
}


LoaderMD2::LoaderMD2(Model* m, LOD* l, const QString& file) : Loader(m, l, file)
{
 boDebug(100) << k_funcinfo << file << endl;
}

LoaderMD2::~LoaderMD2()
{
 boDebug(100) << k_funcinfo << endl;
}


bool LoaderMD2::load()
{
 boDebug(100) << k_funcinfo << endl;
 if (filename().isEmpty()) {
	boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
	return false;
 }
 if (!model()) {
	BO_NULL_ERROR(model());
	return false;
 }

 QFile f(filename());
 if (!f.open(IO_ReadOnly)) {
	boError(100) << k_funcinfo << "can't open " << filename() << endl;;
	return false;
 }
 QDataStream stream(&f);
 stream.setByteOrder(QDataStream::LittleEndian);
 MD2Header header;
 if (!header.load(stream)) {
	boError(100) << k_funcinfo << "unable to load .md2 header" << endl;
	return false;
 }
 QPtrList<MD2Skin> skins;
 skins.setAutoDelete(true);
 for (int i = 0; i < header.mNumSkins; i++) {
	MD2Skin* skin = new MD2Skin();
	skins.append(skin);
	if (!skin->load(stream)) {
		return false;
	}
 }
 QPtrVector<MD2TextureCoordinate> textureCoordinates(header.mNumTexCoords);
 textureCoordinates.setAutoDelete(true);
 for (int i = 0; i < header.mNumTexCoords; i++) {
	MD2TextureCoordinate* t = new MD2TextureCoordinate();
	textureCoordinates.insert(i, t);
	if (!t->load(stream)) {
		return false;
	}
 }
 QPtrList<MD2Triangle> triangles;
 triangles.setAutoDelete(true);
 for (int i = 0; i < header.mNumTriangles; i++) {
	MD2Triangle* t = new MD2Triangle();
	triangles.append(t);
	if (!t->load(stream)) {
		return false;
	}
 }
 QPtrList<MD2Frame> frames;
 frames.setAutoDelete(true);
 for (int i = 0; i < header.mNumFrames; i++) {
	MD2Frame* f = new MD2Frame();
	frames.append(f);
	if (!f->load(stream, header.mNumVertices)) {
		return false;
	}
 }


 // AB: GL commands are a little different than the rest.
 //     the "GL commands" are a set of triangle strips/fans
 Q_INT32 num;
 int pos = 0;
 QPtrList<MD2GLCommand> commands;
 commands.setAutoDelete(true);
 while (pos < header.mNumGLCommands * 4) { // we have mNumGLCommands * 4 bytes
	stream >> num;
	if (num != 0) {
		MD2GLCommand* c = new MD2GLCommand();
		commands.append(c);
		if (!c->load(stream, num)) {
			boError(100) << k_funcinfo << "could not load GL command" << endl;
			return false;
		}
	}
	pos += 4; // the "num" variable
	pos += num * sizeof(MD2GLCommandVertex);
	if (num == 0 && pos < header.mNumGLCommands * 4) {
		boWarning(100) << k_funcinfo << "end of list reached, but not all data read. is that valid in MD2 files?" << endl;
		break;
	}
 }



 // convert data into our own data structures

 Material* material = new Material();
 // TODO: find texture. (AB: I guess .md2 without any texture makes little
 //                          sense, as .md2 provides texture coordinates in any
 //                          case)
 //       -> note: .md2 supports multiple skins, but apparently we can only use
 //                one of them at once. they seem to be used for different
 //                "teams".
 //                this also leads to a new problem: some models might not
 //                specify a skin at all (as it is not relevant for the .md2
 //                data) but use one anyway, e.g. use model.md2 for the model
 //                and use a model.tga texture for it.
 model()->addMaterial(material);
 QString texture;
 if (header.mNumSkins > 0) {
	MD2Skin* skin = skins.at(0);
	if (!skin) {
		BO_NULL_ERROR(skin);
		return false;
	}
	texture = (const char*)skin->mName;

	boDebug() << k_funcinfo << "model requested texture " << texture << endl;
 } else {
	// TODO: search in $KDEDIR/boson/themes/textures/ ?
	boWarning() << k_funcinfo << "model requested no texture - searching for texture in model directory" << endl;
	// search for texture files named <model>.foobar in the directory where
	// <model>.md2 resides.
	QFileInfo fileInfo(filename());
	QDir dir(fileInfo.dirPath(true));
	dir.setNameFilter(fileInfo.baseName() + ".*");
	QStringList list = dir.entryList();
	list.remove(fileInfo.fileName());
	QStringList listLower;
	for (QStringList::iterator it = list.begin(); it != list.end(); ++it) {
		listLower.append((*it).lower());
	}

	QStringList exts; // only lowercase extensions here - we match uppercase automatically
	exts.append(".png");
	exts.append(".jpg");
	exts.append(".jpeg");
	exts.append(".bmp");
	exts.append(".xbm");
	exts.append(".xpm");
	exts.append(".gif");
	exts.append(".pnm");
	exts.append(".tga");
	exts.append(".pcx");
	QString baseNameLower = fileInfo.baseName().lower();
	for (unsigned int i = 0; texture.isNull() && i < exts.count(); i++) {
		int index = listLower.findIndex(baseNameLower + exts[i]);
		if (index >= 0) {
			// AB: we check against listLower, but we return the
			// actual filename from list
			// -> this may even be uppercase.
			texture = list[index];
		}
	}

	if (texture.isNull()) {
		boWarning(100) << k_funcinfo << "no texture found for model " << filename() << endl;
	} else {
		boDebug(100) << k_funcinfo << "using texture " << texture << endl;
	}
 }
 if (!texture.isEmpty()) {
	material->setTexture(model()->getTexture(texture));
 }

 for (int i = 0; i < header.mNumFrames; i++) {
	MD2Frame* md2Frame = frames.at(i);
	if (!md2Frame) {
		BO_NULL_ERROR(md2Frame);
		return false;
	}

	lod()->createFrame();
	Frame* frame = lod()->frame(i);
	if (!frame) {
		BO_NULL_ERROR(frame);
		return false;
	}
	frame->allocateNodes(1);

	Mesh* mesh = new Mesh();
	mesh->setName((const char*)md2Frame->mName);
	lod()->addMesh(mesh);
	frame->setMesh(0, mesh);

	mesh->setMaterial(material);
	mesh->allocateVertices(header.mNumVertices);
	for (int i = 0; i < header.mNumVertices; i++) {
		Vertex* vertex = mesh->vertex(i);
		if (!vertex) {
			BO_NULL_ERROR(vertex);
			return false;
		}
		MD2TriangleVertex* md2Vertex = &md2Frame->mTriangleVertices[i];
		float x = (((float)md2Vertex->mVertex[0]) * md2Frame->mScale[0]) + md2Frame->mTranslate[0];
		float y = (((float)md2Vertex->mVertex[1]) * md2Frame->mScale[1]) + md2Frame->mTranslate[1];
		float z = (((float)md2Vertex->mVertex[2]) * md2Frame->mScale[2]) + md2Frame->mTranslate[2];
		vertex->pos = BoVector3Float(x, y, z);

		// AB: the texture coordinate is assigned later
	}

	// AB: our code requires that all frames (at least the very first)
	//     reference all meshes. however in MD2 there is one mesh per frame
	//     -> we cannot support animations correctly atm.
	//        bobmfconverter will remove all frames but the very first.
	mesh->allocateFaces(header.mNumTriangles);
	for (int i = 0; i < header.mNumTriangles; i++) {
		Face* face = mesh->face(i);
		if (!face) {
			BO_NULL_ERROR(face);
			return false;
		}
		MD2Triangle* md2Triangle = triangles.at(i);
		if (!md2Triangle) {
			BO_NULL_ERROR(md2Triangle);
			return false;
		}
		face->setVertexCount(3);
		for (int j = 0; j < 3; j++) {
			int vertexIndex = md2Triangle->mVertexIndices[j];
			int textureIndex = md2Triangle->mTextureIndices[j];
			if (vertexIndex < 0) {
				boError(100) << k_funcinfo << vertexIndex << endl;
				return false;
			}
			if (textureIndex < 0) {
				boError(100) << k_funcinfo << textureIndex << endl;
				return false;
			}
			Vertex* v = mesh->vertex(vertexIndex);
			if (!v) {
				BO_NULL_ERROR(v);
				return false;
			}

			// AB: the texture coordinate is still missing from the
			//     vertex
			if (textureIndex >= header.mNumTexCoords) {
				boError(100) << k_funcinfo << "textureIndex too large: " << textureIndex << endl;
				return false;
			}
			MD2TextureCoordinate* tex = textureCoordinates[textureIndex];
			float s = ((float)tex->mS) / ((float)header.mSkinWidth);
			float t = ((float)tex->mT) / ((float)header.mSkinHeight);
			v->tex = BoVector2Float(s, t);

			face->setVertex(j, v);
		}

		// face->smoothgroup = ?; // do we need this ?
	}
 }

 return true;
}


