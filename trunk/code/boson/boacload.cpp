/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boacload.h"

#include "bo3dtools.h"
#include "bomesh.h"
#include "bomaterial.h"
#include "bosonmodel.h"
#include "bosonmodelloader.h"
#include "bodebug.h"

#include <qptrlist.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qfile.h>
#include <qtextstream.h>

#include <stdlib.h>


#define SURFACE_SHADED (1<<4)
#define SURFACE_TWOSIDED (1<<5)


// Small helper classes to hold some values
class ACFace
{
  public:
    ACFace()
    {
      smooth = false;
      twosided = false;
      material = 0;
      numpoints = 0;
      vertexindex = 0;
      texu = 0;
      texv = 0;
    }
    ~ACFace()
    {
      delete[] vertexindex;
      delete[] texu;
      delete[] texv;
    }

    bool smooth;
    bool twosided;
    int material;

    int numpoints;
    int* vertexindex;
    float* texu;
    float* texv;
};

class ACObject
{
  public:
    ACObject()
    {
      texrepX = 1.0;
      texrepY = 1.0;
      texoffX = 0.0;
      texoffY = 0.0;
      loc.set(0, 0, 0);
      numvert = 0;
      vertices = 0;
      numfaces = 0;
      faces = 0;
      numkids = 0;
      kids = 0;
      type = Poly;
    }
    ~ACObject()
    {
      delete[] vertices;
      delete[] faces;
    }

    enum Type { World = 1, Group, Poly };


    QString name;
    Type type;

    QString texture;

    float texrepX;
    float texrepY;
    float texoffX;
    float texoffY;

    BoVector3Float loc;

    int numvert;
    BoVector3Float* vertices;

    int numfaces;
    ACFace* faces;

    int numkids;
    ACObject* kids;
};



QValueVector<QString> splitString(const QString& str)
{
 QValueVector<QString> tokens;  // All tokens
 QString current;  // Token being processed atm

 for (unsigned int i = 0; i < str.length(); i++) {
	if (!str[i].isSpace()) {
		if (str[i] == '"') {
			// Quoted word(s). Loop until we find ending quote.
			i++;
			while (str[i] != '"') {
				current += str[i];
				i++;
			}
			// Token will be added once we find delimiter.
			// This means that e.g. my"f o o"bar will be treated as one token.
		} else if (str[i] == '\'') {
			// Same as above
			i++;
			while (str[i] != '\'') {
				current += str[i];
				i++;
			}
		} else {
			// Normal character. Just add it to current
			current += str[i];
		}
	} else {
		// We have a space (= delimiter)
		if (!current.isEmpty()) {
			tokens.append(current);
			current = QString();
		}
	}
 }
 // Add last token
 if (!current.isEmpty()) {
	tokens.append(current);
 }

 return tokens;
}



BoACLoad::BoACLoad(const QString& dir, const QString& file, BosonModelLoaderData* data)
{
  init();
  mDirectory = dir;
  mFile = file;
  mData = data;
}

void BoACLoad::init()
{
}

BoACLoad::~BoACLoad()
{
  boDebug(100) << k_funcinfo << endl;
}

const QString& BoACLoad::baseDirectory() const
{
  return mDirectory;
}

QString BoACLoad::file() const
{
  return baseDirectory() + mFile;
}

bool BoACLoad::loadModel()
{
  boDebug(100) << k_funcinfo << endl;
  if(mFile.isEmpty() || mDirectory.isEmpty())
  {
    boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
    return false;
  }
  if(!mData)
  {
    BO_NULL_ERROR(mData);
    return false;
  }

  // Code taken from AC3DLoader
  QFile f(file());
  if(!f.open(IO_ReadOnly))
  {
    boError() << k_funcinfo << "can't open " << file() << endl;;
    return false;
  }

  QTextStream stream(&f);
  QString line;
  int i = 1;
  line = stream.readLine(); // line of text excluding '\n'
  i++;

  if(line.left(4) != "AC3D")
  {
    boError() << k_funcinfo << file() << "is not a valid AC3D file." << endl;
    f.close();
    return false;
  }

  // Base (world) object
  ACObject* obj = new ACObject;
  bool objloaded = false;

  // Load AC3D file into memory
  while(!stream.atEnd())
  {
    // Read next line
    line = stream.readLine();

    if(line.left(8) == "MATERIAL")
    {
      if(!loadMaterial(line))
      {
        return false;
      }
    }
    else if(line.left(6) == "OBJECT")
    {
      // Load object
      if(objloaded)
      {
        boError() << k_funcinfo << "Multiple world objects?!" << endl;
        return false;
      }
      if(!loadObject(stream, obj))
      {
        return false;
      }
      objloaded = true;
    }

    i++;
  }

  f.close();


  // Convert AC3D objects into meshes and adds them to a frame
  // Note that materials are already BoMaterials - we don't use special temporary
  //  objects for them
  // Create frame
  BoFrame* frame = mData->frame(mData->addFrame());
  if(!frame)
  {
    BO_NULL_ERROR(frame);
    return false;
  }

  int numobjects = 0;
  countObjects(obj, &numobjects);
  frame->allocMeshes(numobjects);


  translateObject(obj, BoVector3Float());

  int index = 0;
  convertIntoMesh(frame, &index, obj);


  // Delete temporary objects
  delete obj;


  boDebug(100) << k_funcinfo << "loaded from " << file() << endl;
  return true;
}

bool BoACLoad::loadObject(QTextStream& stream, ACObject* obj)
{
  QString line;
  QValueVector<QString> tokens;

  while(!stream.atEnd())
  {
    // Read next line
    line = stream.readLine();
    tokens = splitString(line);
    if(tokens.count() == 0)
    {
      // Probably an empty line
      continue;
    }

    if(tokens[0].lower() == "data")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected 'data <number>'" << endl;
      }
      else
      {
        int len = tokens[1].toInt();
        if (len > 0)
        {
          char* str = new char[len + 1];
          stream.readRawBytes(str, len);
          delete[] str;
        }
      }
    }
    else if(tokens[0].lower() == "name")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected quoted name" << endl;
      }
      else
      {
        obj->name = tokens[1];
      }
    }
    else if(tokens[0].lower() == "texture")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected quoted texture name" << endl;
      }
      else
      {
        obj->texture = tokens[1];
      }
    }
    else if(tokens[0].lower() == "texrep")
    {
      if(tokens.count() != 3)
      {
        boError() << k_funcinfo << "expected 'texrep <float> <float>'" << endl;
      }
      else
      {
        obj->texrepX = tokens[1].toFloat();
        obj->texrepY = tokens[1].toFloat();
      }
    }
    else if(tokens[0].lower() == "texoff")
    {
      if(tokens.count() != 3)
      {
        boError() << k_funcinfo << "expected 'texoff <float> <float>'" << endl;
      }
      else
      {
        obj->texoffX = tokens[1].toFloat();
        obj->texoffY = tokens[1].toFloat();
      }
    }
    else if(tokens[0].lower() == "rot")
    {
      boError() << k_funcinfo << "'rot' is not yet supported!!!" << endl;
    }
    else if(tokens[0].lower() == "loc")
    {
      if(tokens.count() != 4)
      {
        boError() << k_funcinfo << "expected 'loc <float> <float> <float>'" << endl;
      }
      else
      {
        obj->loc = BoVector3Float(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat());
      }
    }
    else if(tokens[0].lower() == "numvert")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected 'numvert <int>'" << endl;
      }
      else
      {
        obj->numvert = tokens[1].toInt();
        obj->vertices = new BoVector3Float[obj->numvert];
        for(int i = 0; i < obj->numvert; i++)
        {
          float x, y, z;
          stream >> x >> y >> z;
          obj->vertices[i].set(x, y, z);
        }
        // Read end of the line
        line = stream.readLine();
      }
    }
    else if(tokens[0].lower() == "numsurf")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected 'numsurf <int>'" << endl;
      }
      else
      {
        obj->numfaces = tokens[1].toInt();
        obj->faces = new ACFace[obj->numfaces];
        for(int i = 0; i < obj->numfaces; i++)
        {
          if(!loadFace(stream, &obj->faces[i]))
          {
            return false;
          }
        }
      }
    }
    else if(tokens[0].lower() == "kids")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected 'kids <num>'" << endl;
      }
      else
      {
        obj->numkids = tokens[1].toInt();
        if(obj->numkids > 0)
        {
          obj->kids = new ACObject[obj->numkids];
          for(int i = 0; i < obj->numkids; i++)
          {
            // Load 'OBJECT <type>' line (type isn't used yet)
            line = stream.readLine();
            if(!loadObject(stream, &obj->kids[i]))
            {
              return false;
            }
          }
        }
      }
      // Kids token ends object section, so we're done with this object
      return true;
    }

  }

  return true;
}

bool BoACLoad::loadFace(QTextStream& stream, ACFace* face)
{
  QString line;
  QValueVector<QString> tokens;

  while(!stream.atEnd())
  {
    // Read next line
    line = stream.readLine();
    tokens = splitString(line);
    if(tokens.count() == 0) {
      // Probably an empty line
      continue;
    }

    if(tokens[0].lower() == "surf")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected 'SURF <flags>'" << endl;
      }
      else
      {
        long int flags;
        // AC3D saves flags as e.g. "0x20", but we have to cut 0x from the
        //  beginning to make QString::toLong() work
        if(tokens[1][1] == 'x')
        {
          flags = tokens[1].right(tokens[1].length() - 2).toLong(0, 16);
        }
        else
        {
          flags = tokens[1].toLong(0, 16);
        }
        if(flags & SURFACE_SHADED)
        {
          face->smooth = true;
        }
        if(flags & SURFACE_TWOSIDED)
        {
          face->twosided = true;
        }
      }
    }
    else if(tokens[0].lower() == "mat")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected 'mat <index>'" << endl;
      }
      else
      {
        face->material = tokens[1].toInt();
      }
    }
    else if(tokens[0].lower() == "refs")
    {
      if(tokens.count() != 2)
      {
        boError() << k_funcinfo << "expected 'refs <num>'" << endl;
      }
      else
      {
        face->numpoints = tokens[1].toInt();
        face->vertexindex = new int[face->numpoints];
        face->texu = new float[face->numpoints];
        face->texv = new float[face->numpoints];
        int vertex;
        float u, v;
        for(int i = 0; i < face->numpoints; i++)
        {
          stream >> vertex >> u >> v;
          face->vertexindex[i] = vertex;
          face->texu[i] = u;
          face->texv[i] = v;
        }
        // Read end of the line
        line = stream.readLine();
      }
      // Refs entry _should_ be the last entry (line) of a surface in a file.
      //  Next line should already belong to the next surface or to something
      //  else. Problem here is that there's no way to check if it really is so.
      // FIXME: can we somehow check if this surface really ends here?
      return true;
    }

  }

  return true;
}

bool BoACLoad::loadMaterial(const QString& line)
{
  // Parse AC3D material
  QValueVector<QString> tokens = splitString(line);

  if(tokens.count() != 22)
  {
    boError() << k_funcinfo << "expected 21 params after \"MATERIAL\" - line " << line << endl;
    return false;
  }
  else
  {
    mData->addMaterial();
    BoVector4Float color;
    BoMaterial* mat = mData->material(mData->materialCount() - 1);
    mat->setName(tokens[1]);
    color.set(tokens[3].toFloat(), tokens[4].toFloat(), tokens[5].toFloat(), 1.0);
    mat->setDiffuse(color);

    color.set(tokens[7].toFloat(), tokens[8].toFloat(), tokens[9].toFloat(), 1.0);
    mat->setAmbient(color);

    // Emissive color isn't supported by boson atm

    color.set(tokens[15].toFloat(), tokens[16].toFloat(), tokens[17].toFloat(), 1.0);
    mat->setSpecular(color);

    mat->setShininess(tokens[19].toFloat());
    mat->setTransparency(tokens[21].toFloat());
  }
  return true;
}

bool BoACLoad::convertIntoMesh(BoFrame* f, int* index, ACObject* obj)
{
  // First load children
  for(int i = 0; i < obj->numkids; i++)
  {
    if(!convertIntoMesh(f, index, &obj->kids[i]))
    {
      return false;
    }
  }

  // Make sure ACObject has some faces/vertices
  // Note that it's ok to have none - e.g. if object is a group
  if(obj->numfaces == 0)
  {
    return true;
  }
  if(obj->numvert == 0)
  {
    boError() << k_funcinfo << "No vertices for object with " << obj->numfaces << " faces?!" << endl;
    return false;
  }

  // Construct a BoMesh
  BoMesh* boMesh = new BoMesh(obj->numfaces, obj->name);
  mData->addMesh(boMesh);

  // Set mesh's material
  int matindex = obj->faces[0].material;
  bool haddifferentmaterials = false;
  boMesh->setMaterial(mData->material(matindex));
  mData->material(matindex)->setTwoSided(obj->faces[0].twosided);  // sucks
  if(!obj->texture.isEmpty())
  {
    if(!mData->material(matindex)->textureName().isEmpty() && (mData->material(matindex)->textureName() != obj->texture))
    {
      // This material has different textures
      boWarning() << k_funcinfo << "Multiple textures use with material " << mData->material(matindex)->name() << endl;
    }
    else
    {
      mData->material(matindex)->setTextureName(obj->texture);
    }
  }
  // Check if it's a teamcolored-mesh
  boMesh->setIsTeamColor(obj->name.find("teamcolor", 0, false) == 0);


  // Load faces
  // FIXME: properly handle points with same texel and vertex coords
  boMesh->allocatePoints(obj->numfaces * 3);
  int pointindex = 0;
  QValueVector<BoVector3Float> vertices(obj->numfaces * 3);
  QValueVector<BoVector3Float> texels(obj->numfaces * 3);

  for(int i = 0; i < obj->numfaces; i++)
  {
    BoFace face;
    int points[3];

    if(obj->faces[i].material != matindex)
    {
      haddifferentmaterials = true;
    }

    vertices[pointindex] = obj->vertices[obj->faces[i].vertexindex[0]];
    texels[pointindex] = BoVector3Float(obj->faces[i].texu[0], obj->faces[i].texv[0], 0);
    points[0] = pointindex++;
    vertices[pointindex] = obj->vertices[obj->faces[i].vertexindex[1]];
    texels[pointindex] = BoVector3Float(obj->faces[i].texu[1], obj->faces[i].texv[1], 0);
    points[1] = pointindex++;
    vertices[pointindex] = obj->vertices[obj->faces[i].vertexindex[2]];
    texels[pointindex] = BoVector3Float(obj->faces[i].texu[2], obj->faces[i].texv[2], 0);
    points[2] = pointindex++;

    face.setPointIndex(points);
    face.setSmoothGroup(obj->faces[i].smooth ? 1 : 0);
    boMesh->setFace(i, face);
  }
  boMesh->setVertices(vertices);
  boMesh->setTexels(texels);

  if(haddifferentmaterials)
  {
    boWarning() << k_funcinfo << "Mesh " << obj->name << " had different materials!" << endl;
  }

  // Add mesh to frame
  f->setMesh(*index, boMesh);
  f->matrix(*index)->translate(obj->loc);
  (*index)++;

  return true;
}

void BoACLoad::countObjects(ACObject* obj, int* count)
{
  // First count children
  for(int i = 0; i < obj->numkids; i++)
  {
    countObjects(&obj->kids[i], count);
  }

  if(obj->numfaces > 0)
  {
    (*count)++;
  }
}

void BoACLoad::translateObject(ACObject* obj, const BoVector3Float& trans)
{
  // Add trans to our loc
  obj->loc += trans;

  // Translate children
  for(int i = 0; i < obj->numkids; i++)
  {
    translateObject(&obj->kids[i], obj->loc);
  }
}

