/*
    This file is part of the Boson game
    Copyright (C) 2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "model.h"
#include "loader.h"
#include "debug.h"
#include "lod.h"
#include "frame.h"
#include "texture.h"
#include "material.h"
#include "processors/lodcreator.h"
#include "processors/transformer.h"
#include "processors/vertexoptimizer.h"
#include "processors/frameoptimizer.h"
#include "processors/unuseddataremover.h"
#include "processors/meshoptimizer.h"
#include "processors/textureoptimizer.h"
#include "processors/materialoptimizer.h"

#include <qstring.h>
#include <qfileinfo.h>
#include <qvaluelist.h>
#include <qpair.h>

#include <kinstance.h>
#include <ksimpleconfig.h>


bool processCommandLine(int argc, char** argv);
bool checkConfig();
bool loadConfigFile(Model* m);
void doModelProcessing(Model* m);

// Global variables - used for configuration
QString infilename;
QString outfilename;
QString modelname;
QString modelcomment;
QString modelauthor;
QString configfilename;
unsigned int numlods = 5;
float lod_factor = 0.5;
bool smoothallfaces = false;
bool lod_useerror = false;
bool lod_useboth = false;
float lod_baseerror = 0.075;
float lod_errormod = 4.0;
bool frames_removeall = false;
unsigned int frame_base = 0;
bool frames_keepall = true;
float modelsize = 1.0;
unsigned int tex_size = 512;
QString tex_name;
QString tex_path;
bool tex_converttolowercase = false;
bool model_center = false;


double starttime;
char dbgtimestr[20];
void initdbgtime()
{
  struct timeval tm;
  gettimeofday(&tm, 0);
  starttime = tm.tv_sec + tm.tv_usec / 1000000.0;
}
char* dbgtime()
{
  struct timeval tm;
  gettimeofday(&tm, 0);
  sprintf(dbgtimestr, "%.6f ", (tm.tv_sec + tm.tv_usec / 1000000.0) - starttime);
  return dbgtimestr;
}


int main(int argc, char** argv)
{
  initdbgtime();

  boDebug() << "Creating KInstance object..." << endl;
  KInstance instance("bobmfconverter");

  // Parse cmdline
  boDebug() << "Processing cmdline..." << endl;
  if(!processCommandLine(argc, argv))
  {
    return 1;
  }

  // Verify config
  boDebug() << "Checking config..." << endl;
  if(!checkConfig())
  {
    return 1;
  }


  // Create new model
  boDebug() << "Loading model..." << endl;
  Model* m = new Model();

  // Set model info
  m->setName(modelname);
  m->setComment(modelcomment);
  m->setAuthor(modelauthor);

  // Load the model
  if(!m->load(infilename))
  {
    return 1;
  }

  // Load the config file
  loadConfigFile(m);

  if(smoothallfaces)
  {
    boDebug() << "Smoothing all faces..." << endl;
    m->smoothAllFaces();
  }

  boDebug() << "Completing model loading..." << endl;
  m->loadingCompleted();


  // Do all necessary processing
  doModelProcessing(m);


  // Prepare the model for saving
  boDebug() << "Preparing model for saving..." << endl;
  m->prepareForSaving(frame_base);

  // Save the model
  boDebug() << "Saving model..." << endl;
  m->save(outfilename);

  // Done!
  boDebug() << "All done!" << endl;
  return 0;
}


#define NEXTARG(arg) \
    i++; \
    if(i >= argc) \
    { \
      boError() << k_funcinfo << "No value found for argument " << arg << endl; \
      return false; \
    } \
    arg = argv[i];

bool processCommandLine(int argc, char** argv)
{
  for(int i = 1; i < argc; i++)
  {
    QString arg = QString(argv[i]);
    QString larg = arg.lower();

    if(larg == "-h" || larg == "-help")
    {
      QString usage = QString("Usage: %1 [arguments] <filename>\n").arg(argv[0]);
      usage += "\n";
      usage += "Arguments:\n";
      usage += "  -o -output <file>  Write output to <file>\n";
      usage += "  -c -config <file>  Load config setting from <file>\n";
      usage += "  -name <name>       Name of the model\n";
      usage += "  -comment <text>    Comment for the model\n";
      usage += "  -author <text>     Author(s) of the model\n";
      usage += "  -lods <num>        Create <num> lods for the model\n";
      usage += "  -lodfactor <f>     Every successive lod will have <f> times the faces of it's predecessor\n";
      usage += "  -smoothall         Smooth normals will be used for the whole model\n";
      usage += "  -useerror          Use error-based lod calculation (instead of budget-based)\n";
      usage += "  -useboth           Use combined error- and budget-based lod calculation\n";
      usage += "  -baseerror <num>   Set maximum lod error for the first lod to <num>\n";
      usage += "  -errormod <num>    Every successive lod will have max error which is <num> times bigger than that of it's predecessor\n";
      usage += "  -noframes          Remove all frames (but the first one)\n";
      usage += "  -keepframes        Don't remove duplicate frames\n";
      usage += "  -baseframe <num>   Make frame <num> the base frame (all calculations will be based on it)\n";
      usage += "  -size <num>        Make model's width/height (whichever is bigger) <num> units long (default: 1)\n";
      usage += "  -texsize <num>     Combined texture will be <num>x<num> pixels big\n";
      usage += "  -texname <file>    Combined texture will be written to <file> (this should be without path)\n";
      usage += "  -texpath <dir>     Combined texture file will be put to <dir>\n";
      usage += "  -t <dir>           Add directory <dir> to texture search path\n";
      usage += "  -texnametolower    Convert all texture names to lowercase\n";
      usage += "  -center            Centers the model\n";
      //usage += "  \n";
      cout << usage;
      return false;
    }
    else if(larg == "-o" || larg == "-output")
    {
      NEXTARG(arg);
      outfilename = arg;
    }
    else if(larg == "-c" || larg == "-config")
    {
      NEXTARG(arg);
      configfilename = arg;
    }
    else if(larg == "-name")
    {
      NEXTARG(arg);
      modelname = arg;
    }
    else if(larg == "-comment")
    {
      NEXTARG(arg);
      modelcomment = arg;
    }
    else if(larg == "-author")
    {
      NEXTARG(arg);
      modelauthor = arg;
    }
    else if(larg == "-lods")
    {
      // TODO: error checking for arguments which use int/float/whatever
      //  parameters
      NEXTARG(arg);
      numlods = arg.toUInt();
    }
    else if(larg == "-lodfactor")
    {
      NEXTARG(arg);
      lod_factor = arg.toFloat();
    }
    else if(larg == "-q" || larg == "-quick")
    {
      boError() << "'-quick' argument not yet implemented!" << endl;
    }
    else if(larg == "-smoothall")
    {
      smoothallfaces = true;
    }
    else if(larg == "-useerror")
    {
      lod_useerror = true;
    }
    else if(larg == "-useboth")
    {
      lod_useerror = true;
      lod_useboth = true;
    }
    else if(larg == "-baseerror")
    {
      NEXTARG(arg);
      lod_baseerror = arg.toFloat();
    }
    else if(larg == "-errormod")
    {
      NEXTARG(arg);
      lod_errormod = arg.toFloat();
    }
    else if(larg == "-baseframe")
    {
      NEXTARG(arg);
      frame_base = arg.toUInt();
    }
    else if(larg == "-noframes")
    {
      frames_removeall = true;
    }
    else if(larg == "-keepframes")
    {
      frames_keepall = true;
    }
    else if(larg == "-size")
    {
      NEXTARG(arg);
      modelsize = arg.toFloat();
    }
    else if(larg == "-texsize")
    {
      NEXTARG(arg);
      tex_size = arg.toUInt();
    }
    else if(larg == "-t")
    {
      NEXTARG(arg);
      Texture::addTexturePath(arg);
    }
    else if(larg == "-texname")
    {
      NEXTARG(arg);
      tex_name = arg;
    }
    else if(larg == "-texpath")
    {
      NEXTARG(arg);
      tex_path = arg;
    }
    else if(larg == "-texnametolower")
    {
      tex_converttolowercase = true;
    }
    else if(larg == "-center")
    {
      model_center = true;
    }
    else
    {
      if(arg[0] == '-')
      {
        boError() << "Unrecognized argument " << arg << endl;
        return false;
      }

      infilename = arg;
    }
  }

  return true;
}

bool checkConfig()
{
  // Check input file
  if(infilename.isEmpty())
  {
    boError() << "No input file specified!" << endl;
    return false;
  }
  QFileInfo infileinfo(infilename);
  if(!infileinfo.exists())
  {
    boError() << "Input file '" << infilename << "' doesn't exist!" << endl;
    return false;
  }
  else if(!infileinfo.isReadable())
  {
    boError() << "Input file '" << infilename << "' isn't readable!" << endl;
    return false;
  }

  // Output file
  if(outfilename.isEmpty())
  {
    // Create output filename by replacing input file's extension with '.bmf'
    int i = infilename.findRev('.');
    if(i == -1)
    {
      // Filename didn't have '.' in it. Just append '.bmf'
      outfilename = infilename + ".bmf";
    }
    else
    {
      outfilename = infilename.left(i) + ".bmf";
    }
  }

  // Output texture file
  if(tex_name.isEmpty())
  {
    // Create output texture filename by replacing input file's extension with
    //  '.jpg'
    int i = infilename.findRev('.');
    if(i == -1)
    {
      // Filename didn't have '.' in it. Just append '.jpg'
      tex_name = infilename + ".jpg";
    }
    else
    {
      tex_name = infilename.left(i) + ".jpg";
    }
  }

  // Config file
  if(!configfilename.isEmpty())
  {
    QFileInfo configfileinfo(configfilename);
    if(!configfileinfo.exists())
    {
      boError() << "Config file '" << configfilename << "' doesn't exist!" << endl;
      return false;
    }
    else if(!configfileinfo.isReadable())
    {
      boError() << "Config file '" << configfilename << "' isn't readable!" << endl;
      return false;
    }
    configfilename = configfileinfo.absFilePath();
  }

  return true;
}

bool loadConfigFile(Model* m)
{
  if(configfilename.isEmpty())
  {
    return true;
  }

  KSimpleConfig cfg(configfilename, true);

  // Load model size
  cfg.setGroup("Boson Unit");
  modelsize = (float)cfg.readDoubleNumEntry("UnitWidth", modelsize);

  return true;
}


#include <fstream>
#include "mesh.h"
using namespace std;
void saveLod(Model* m, unsigned int i)
{
  LOD* l = m->lod(i);

  ofstream out(QString("%1-%2.obj").arg(outfilename).arg(i));

  QString vertexstr;
  QString facestr;

  int vertexoffset = 1;
  for(unsigned int j = 0; j < l->meshCount(); j++)
  {
    Mesh* mesh = l->mesh(j);
    for(unsigned int i = 0; i < mesh->vertexCount(); i++)
    {
      BoVector3Float pos = mesh->vertex(i)->pos;
      vertexstr += QString("v %1 %2 %3\n").arg(pos.x()).arg(pos.y()).arg(pos.z());
    }
    for(unsigned int i = 0; i < mesh->faceCount(); i++)
    {
      Face* f = mesh->face(i);
      facestr += QString("f %1 %2 %3\n").arg(f->vertex(0)->id+vertexoffset)
          .arg(f->vertex(1)->id+vertexoffset).arg(f->vertex(2)->id+vertexoffset);
    }
    vertexoffset += mesh->vertexCount();
  }

  out << vertexstr.latin1();
  out << facestr.latin1();

  out.close();
}
#include "bo3dtools.h"
void saveLodFrame(Model* m, unsigned int lodi, unsigned int framei)
{
  LOD* l = m->lod(lodi);
  Frame* f = l->frame(framei);

  ofstream out(QString("%1-%2-%3.obj").arg(outfilename).arg(lodi).arg(framei));

  QString vertexstr;
  QString facestr;

  int vertexoffset = 1;
  for(unsigned int i = 0; i < f->nodeCount(); i++)
  {
    Mesh* mesh = f->mesh(i);
    BoMatrix* matrix = f->matrix(i);
    // Save _tranformed_ vertices
    for(unsigned int j = 0; j < mesh->vertexCount(); j++)
    {
      BoVector3Float pos;
      matrix->transform(&pos, &mesh->vertex(j)->pos);
      vertexstr += QString("v %1 %2 %3\n").arg(pos.x()).arg(pos.y()).arg(pos.z());
    }
    // Save faces
    for(unsigned int j = 0; j < mesh->faceCount(); j++)
    {
      Face* f = mesh->face(j);
      facestr += QString("f %1 %2 %3\n").arg(f->vertex(0)->id+vertexoffset)
          .arg(f->vertex(1)->id+vertexoffset).arg(f->vertex(2)->id+vertexoffset);
    }
    vertexoffset += mesh->vertexCount();
  }

  out << vertexstr.latin1();
  out << facestr.latin1();

  out.close();
}

void saveLodFrameAC(Model* m, unsigned int lodi, unsigned int framei)
{
  LOD* l = m->lod(lodi);
  Frame* f = l->frame(framei);

  ofstream out(QString("%1-%2-%3.ac").arg(outfilename).arg(lodi).arg(framei));

  // AC3D header
  out << "AC3Db" << endl;

  // Materials
  for(unsigned int i = 0; i < m->materialCount(); i++)
  {
    Material* mat = m->material(i);
    out << "MATERIAL \"" << mat->name().latin1() << "\"  ";
    out << "rgb " << mat->diffuse().x() << " " << mat->diffuse().y() << " " << mat->diffuse().z() << "  ";
    out << "amb " << mat->ambient().x() << " " << mat->ambient().y() << " " << mat->ambient().z() << "  ";
    out << "emis " << mat->emissive().x() << " " << mat->emissive().y() << " " << mat->emissive().z() << "  ";
    out << "spec " << mat->specular().x() << " " << mat->specular().y() << " " << mat->specular().z() << "  ";
    out << "shi " << mat->shininess() << "  trans 0" << endl;
  }

  // Meshes (in nodes)
  out << "OBJECT world" << endl;
  out << "kids " << f->nodeCount() << endl;

  for(unsigned int i = 0; i < f->nodeCount(); i++)
  {
    Mesh* mesh = f->mesh(i);
    BoMatrix* matrix = f->matrix(i);

    // Object header
    out << "OBJECT poly" << endl;
    out << "name \"" << mesh->name().latin1() << "\"" << endl;
    if(mesh->material() && mesh->material()->texture())
    {
      out << "texture \"" << mesh->material()->texture()->filename().latin1() << "\"" << endl;
    }

    // Save _tranformed_ vertices
    out << "numvert " << mesh->vertexCount() << endl;
    for(unsigned int j = 0; j < mesh->vertexCount(); j++)
    {
      BoVector3Float pos;
      matrix->transform(&pos, &mesh->vertex(j)->pos);
      out << pos.x() << " " << pos.y() << " " << pos.z() << endl;
    }

    // Save faces
    out << "numsurf " << mesh->faceCount() << endl;
    for(unsigned int j = 0; j < mesh->faceCount(); j++)
    {
      Face* f = mesh->face(j);
      out << "SURF 0x10" << endl;
      if(mesh->material())
      {
        out << "mat " << mesh->material()->id() << endl;
      }
      out << "refs 3" << endl;
      out << f->vertex(0)->id << " " << f->vertex(0)->tex.x() << " " << f->vertex(0)->tex.y() << endl;
      out << f->vertex(1)->id << " " << f->vertex(1)->tex.x() << " " << f->vertex(1)->tex.y() << endl;
      out << f->vertex(2)->id << " " << f->vertex(2)->tex.x() << " " << f->vertex(2)->tex.y() << endl;
    }

    out << "kids 0" << endl;
  }

  out.flush();
  out.close();
}

void doModelProcessing(Model* m)
{
  if(tex_converttolowercase)
  {
    // Convert texture names to lowercase
    QDictIterator<Texture> it(*m->texturesDict());
    while(it.current())
    {
      it.current()->setFilename(it.current()->filename().lower());
      ++it;
    }
  }
  boDebug() << "LOD 0 had " << m->baseLOD()->shortStats() << endl;

  Processor::setBaseFrame(frame_base);

  boDebug() << "Removing unused data..." << endl;
  UnusedDataRemover unuseddataremover(m, m->baseLOD());
  unuseddataremover.process();

  if(!frames_keepall)
  {
    boDebug() << "Removing duplicate frames..." << endl;
    FrameOptimizer frameoptimizer(m, m->baseLOD());
    frameoptimizer.setRemoveAllFrames(frames_removeall);
    frameoptimizer.process();
  }

  boDebug() << "Transforming model..." << endl;
  Transformer transformer(m, m->baseLOD());
  transformer.setModelSize(modelsize);
  transformer.setCenterModel(model_center);
  transformer.process();

  boDebug() << "Optimizing textures..." << endl;
  /*TextureOptimizer textureoptimizer(m, m->baseLOD());
  textureoptimizer.setCombinedTexSize(tex_size);
  textureoptimizer.setCombinedTexFilename(tex_name);
  textureoptimizer.setCombinedTexPath(tex_path);
  textureoptimizer.process();*/

  boDebug() << "Optimizing materials..." << endl;
  MaterialOptimizer materialoptimizer(m, m->baseLOD());
  materialoptimizer.process();

  boDebug() << "Optimizing meshes..." << endl;
  MeshOptimizer meshoptimizer(m, m->baseLOD());
  meshoptimizer.process();

  boDebug() << "Optimizing vertices..." << endl;
  VertexOptimizer vo(m, m->baseLOD());
  vo.process();


  boDebug() << "Calculating model's face normals..." << endl;
  m->calculateFaceNormals();

  boDebug() << "Calculating model's vertex normals..." << endl;
  m->calculateVertexNormals();


  boDebug() << "Creating lods..." << endl;
  m->createLODs(numlods);

  //boDebug() << "Quicksaving base LOD..." << endl;
  //saveLod(m, 0);

  //boDebug() << "Quicksaving base frame in base LOD..." << endl;
  saveLodFrameAC(m, 0, frame_base);
//  saveLodFrameAC(m, 0, 10);

  float targetfactor = 1.0f;
  float loderror = lod_baseerror;
  boDebug() << "LOD 0 has now " << m->baseLOD()->shortStats() << endl;
  boDebug() << "LOD 0 has " << m->baseLOD()->frameCount() << " frames and " <<
      m->baseLOD()->frame(frame_base)->nodeCount() << " nodes in base frame" << endl;
  for(unsigned int i = 1; i < numlods; i++)
  {
    targetfactor *= lod_factor;
    LodCreator lodcreator(m, m->lod(i));
    lodcreator.setFaceTargetFactor(targetfactor);
    lodcreator.setMaxError(loderror);
    lodcreator.setUseError(lod_useerror);
    lodcreator.setUseBoth(lod_useboth);
    lodcreator.process();
    boDebug() << "LOD " << i << " has now " << m->lod(i)->shortStats() << endl;
    //saveLod(m, i);
    saveLodFrameAC(m, i, frame_base);
    //boDebug() << "LOD quicksaved" << endl;

    loderror *= lod_errormod;
  }
}
