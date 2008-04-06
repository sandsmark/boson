/*
    This file is part of the Boson game
    Copyright (C) 2005-2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "loader-ac.h"

#include "debug.h"
#include "mesh.h"
#include "material.h"
#include "lod.h"
#include "model.h"
#include "frame.h"
#include "texture.h"

#include <q3ptrlist.h>
#include <qstringlist.h>
#include <q3valuevector.h>
#include <qfile.h>
#include <q3textstream.h>
//Added by qt3to4:
#include <Q3ValueList>


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


Q3ValueVector<QString> splitString(const QString& str)
{
 Q3ValueVector<QString> tokens;  // All tokens
 QString current;  // Token being processed atm

 for (int i = 0; i < str.length(); i++) {
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




LoaderAC::LoaderAC(Model* m, LOD* l, const QString& file) : Loader(m, l, file)
{
  boDebug(100) << k_funcinfo << endl;
}

LoaderAC::~LoaderAC()
{
  boDebug(100) << k_funcinfo << endl;
}


bool LoaderAC::load()
{
  boDebug(100) << k_funcinfo << endl;
  if(filename().isEmpty())
  {
    boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
    return false;
  }
  if(!model())
  {
    BO_NULL_ERROR(model());
    return false;
  }

  // Code taken from AC3DLoader
  QFile f(filename());
  if(!f.open(QIODevice::ReadOnly))
  {
    boError() << k_funcinfo << "can't open " << filename() << endl;;
    return false;
  }

  Q3TextStream stream(&f);
  QString line;
  int i = 1;
  line = stream.readLine(); // line of text excluding '\n'
  i++;

  if(line.left(4) != "AC3D")
  {
    boError() << k_funcinfo << filename() << "is not a valid AC3D file." << endl;
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
      mMaterialLines.append(line);
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
  lod()->createFrame();
  Frame* frame = lod()->frame(0);
  if(!frame)
  {
    BO_NULL_ERROR(frame);
    return false;
  }

  int numobjects = 0;
  countObjects(obj, &numobjects);
  frame->allocateNodes(numobjects);


  translateObject(obj, BoVector3Float());

  int index = 0;
  convertIntoMesh(frame, &index, obj);


  // Delete temporary objects
  delete obj;


  boDebug(100) << k_funcinfo << "loaded from " << filename() << endl;
  return true;
}

bool LoaderAC::loadObject(Q3TextStream& stream, ACObject* obj)
{
  QString line;
  Q3ValueVector<QString> tokens;

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

    if(tokens[0].toLower() == "data")
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
    else if(tokens[0].toLower() == "name")
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
    else if(tokens[0].toLower() == "texture")
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
    else if(tokens[0].toLower() == "texrep")
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
    else if(tokens[0].toLower() == "texoff")
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
    else if(tokens[0].toLower() == "rot")
    {
      boError() << k_funcinfo << "'rot' is not yet supported!!!" << endl;
    }
    else if(tokens[0].toLower() == "loc")
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
    else if(tokens[0].toLower() == "numvert")
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
    else if(tokens[0].toLower() == "numsurf")
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
    else if(tokens[0].toLower() == "kids")
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

bool LoaderAC::loadFace(Q3TextStream& stream, ACFace* face)
{
  QString line;
  Q3ValueVector<QString> tokens;

  while(!stream.atEnd())
  {
    // Read next line
    line = stream.readLine();
    tokens = splitString(line);
    if(tokens.count() == 0) {
      // Probably an empty line
      continue;
    }

    if(tokens[0].toLower() == "surf")
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

        // flags:
        // The first 4 bits (flags & 0xF) is the type
        // (0 = polygon, 1 = closedline, 2 = line)
        // -> see http://www.ac3d.org/ac3d/man/ac3dfileformat.html
        int type = (flags & 0xF);
        if(type != 0)
        {
          boError() << k_funcinfo << "type != polygon (0) not supported!" << endl;
          return false;
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
    else if(tokens[0].toLower() == "mat")
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
    else if(tokens[0].toLower() == "refs")
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

Material* LoaderAC::requestMaterial(const QString& line, const QString& texture)
{
  if(!mMaterialLines.contains(line))
  {
    boError() << k_funcinfo << "requested unknown material line" << endl;
    return 0;
  }
  Q3ValueList<Material*> list = mLine2Materials[line];
  for(Q3ValueList<Material*>::iterator it = list.begin(); it != list.end(); ++it)
  {
    if(!(*it)->texture() && texture.isEmpty())
    {
      return *it;
    }
    if((*it)->texture() && (*it)->texture()->filename() == texture)
    {
      return *it;
    }
  }

  if(!loadMaterial(line))
  {
    boError() << k_funcinfo << "could not load material from line " << line << endl;
    return 0;
  }
  Material* mat = model()->material(model()->materialCount() - 1);
  mat->setTexture(model()->getTexture(texture));
  mLine2Materials[line].append(mat);
  return mat;
}

bool LoaderAC::loadMaterial(const QString& line)
{
  // Parse AC3D material
  Q3ValueVector<QString> tokens = splitString(line);

  if(tokens.count() != 22)
  {
    boError() << k_funcinfo << "expected 21 params after \"MATERIAL\" - line " << line << endl;
    return false;
  }
  else
  {
    Material* mat = new Material;
    BoVector4Float color;
    mat->setName(tokens[1]);
    color.set(tokens[3].toFloat(), tokens[4].toFloat(), tokens[5].toFloat(), 1.0);
    mat->setDiffuse(color);

    color.set(tokens[7].toFloat(), tokens[8].toFloat(), tokens[9].toFloat(), 1.0);
    mat->setAmbient(color);

    // Emissive color isn't supported by boson atm

    color.set(tokens[15].toFloat(), tokens[16].toFloat(), tokens[17].toFloat(), 1.0);
    mat->setSpecular(color);

    mat->setShininess(tokens[19].toFloat());
#warning FIXME: transparency
#if 0
    mat->setTransparency(tokens[21].toFloat());
#endif

    model()->addMaterial(mat);
  }
  return true;
}

bool LoaderAC::convertIntoMesh(Frame* f, int* index, ACObject* obj)
{
  if(!f) {
    BO_NULL_ERROR(f);
    return false;
  }
  if(!index) {
    BO_NULL_ERROR(index);
    return false;
  }
  if(!obj) {
    BO_NULL_ERROR(obj);
    return false;
  }
  if(!lod()) {
    BO_NULL_ERROR(lod());
    return false;
  }
  if(!model()) {
    BO_NULL_ERROR(model());
    return false;
  }
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

  // Construct a Mesh
  Mesh* boMesh = new Mesh();
  boMesh->setName(obj->name);
  lod()->addMesh(boMesh);

  // Set mesh's material
  int ac3dMatIndex = obj->faces[0].material;
  QString ac3dMatLine;
  if(ac3dMatIndex >= 0 && ac3dMatIndex < mMaterialLines.count())
  {
    ac3dMatLine = mMaterialLines[ac3dMatIndex];
  }
  bool hadDifferentMaterials = false;
  Material* material = requestMaterial(ac3dMatLine, obj->texture);
  if(!material)
  {
    BO_NULL_ERROR(material);
    return false;
  }
  boMesh->setMaterial(material);
#warning FIXME: twosided
#if 0
  material->setTwoSided(obj->faces[0].twosided);  // sucks
#endif
  if(!obj->texture.isEmpty())
  {
    if(material->texture() && (material->texture()->filename() != obj->texture))
    {
      // This material has different textures
      boWarning(100) << k_funcinfo << "Multiple textures use with material " << material->name() << endl;
    }
    else
    {
      material->setTexture(model()->getTexture(obj->texture));
    }
  }
  // Check if it's a teamcolored-mesh
  boMesh->setIsTeamColor(obj->name.indexOf("teamcolor", 0, Qt::CaseInsensitive) == 0);


  // Load faces
  // FIXME: properly handle points with same texel and vertex coords
  int bosonVertexCount = 0;
  int bosonFacesCount = 0;
  for(int i = 0; i < obj->numfaces; i++)
  {
    int points = obj->faces[i].numpoints;
    if(points < 3)
    {
      // a point or a line
      boWarning(100) << k_funcinfo << "faces with numpoints < 3 not yet supported (have " << obj->faces[i].numpoints << ")" << endl;
      continue;
    }
    bosonVertexCount += points;
    bosonFacesCount += (points - 2);
  }
  boMesh->allocateVertices(bosonVertexCount);
  boMesh->allocateFaces(bosonFacesCount);

  for(int i = 0; i < obj->numfaces; i++)
  {
    if(obj->faces[i].material != ac3dMatIndex)
    {
      hadDifferentMaterials = true;
    }
  }

  int pointIndex = 0;
  bosonFacesCount = 0;
  for(int i = 0; i < obj->numfaces; i++)
  {
    Vertex* vertices[3];
    Vertex* vertex;

    if(obj->faces[i].numpoints < 3)
    {
      boError(100) << k_funcinfo << "need at least 3 points - have only " << obj->faces[i].numpoints << " in face " << i << endl;
      return false;
    }

    // TODO: this should be a generic bobmfconverter processor
    // -> we should simply load all vertices into the face and let a
    //    postprocessing phase do this work.
    Face* faces = new Face[obj->faces[i].numpoints - 2];
    if(!convertPolygonToFaces(obj, i, boMesh, faces, &pointIndex))
    {
      boError(100) << k_funcinfo << "converting polygon to faces failed" << endl;
      return false;
    }
    for(int j = 0; j < obj->faces[i].numpoints - 2; j++)
    {
      Face* face = boMesh->face(bosonFacesCount);
      bosonFacesCount++;
      if(!face)
      {
        BO_NULL_ERROR(face);
        delete[] faces;
        return false;
      }
      face->setVertexCount(3);
      face->setVertex(0, faces[j].vertex(0));
      face->setVertex(1, faces[j].vertex(1));
      face->setVertex(2, faces[j].vertex(2));
      face->smoothgroup = faces[j].smoothgroup;
    }
    delete[] faces;
  }

  if(hadDifferentMaterials)
  {
    boWarning(100) << k_funcinfo << "Mesh " << obj->name << " had different materials!" << endl;
  }

  // Add mesh to frame
  f->setMesh(*index, boMesh);
  if(!f->matrix(*index))
  {
    BO_NULL_ERROR(f->matrix(*index));
    return false;
  }
  f->matrix(*index)->translate(obj->loc);
  (*index)++;

  return true;
}

bool LoaderAC::convertPolygonToFaces(ACObject* obj, int face, Mesh* boMesh, Face* boFaces, int* pointIndex)
{
  if(obj->faces[face].numpoints < 3)
  {
    boError(100) << k_funcinfo << "need at least 3 points" << endl;
    return false;
  }
  for(int i = 0; i < obj->faces[face].numpoints - 2; i++)
  {
    boFaces[i].setVertexCount(3);
    boFaces[i].smoothgroup = (obj->faces[face].smooth ? 1 : 0);
  }
  Vertex** vertices = new Vertex*[obj->faces[face].numpoints];

  // create the required vertices and copy the pointer into the vertices array
  for(int i = 0; i < obj->faces[face].numpoints; i++)
  {
    Vertex* vertex = boMesh->vertex(*pointIndex);
    (*pointIndex)++;
    if(!vertex)
    {
      BO_NULL_ERROR(vertex);
      return false;
    }
    vertex->pos = BoVector3Float(obj->vertices[obj->faces[face].vertexindex[i]]);
    vertex->tex = BoVector2Float(obj->faces[face].texu[i], obj->faces[face].texv[i]);
    vertices[i] = vertex;
  }

  for(int i = 0; i < obj->faces[face].numpoints - 2; i++)
  {
    boFaces[i].setVertex(0, vertices[0]);
    boFaces[i].setVertex(1, vertices[i + 1]);
    boFaces[i].setVertex(2, vertices[i + 2]);
  }
  delete[] vertices;
  return true;
}

void LoaderAC::countObjects(ACObject* obj, int* count)
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

void LoaderAC::translateObject(ACObject* obj, const BoVector3Float& trans)
{
  // Add trans to our loc
  obj->loc += trans;

  // Translate children
  for(int i = 0; i < obj->numkids; i++)
  {
    translateObject(&obj->kids[i], obj->loc);
  }
}

/*
 * vim: et sw=2
 */
