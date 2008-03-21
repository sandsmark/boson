/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

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

#include "model.h"
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
#include "processors/defaultmaterials.h"
#include "processors/normalcalculator.h"
#include "processors/nodeoptimizer.h"

#include <qstring.h>
#include <qfileinfo.h>
#include <qvaluelist.h>
#include <qpair.h>
#include <qregexp.h>

#include <kinstance.h>
#include <ksimpleconfig.h>


bool processCommandLine(int argc, char** argv);
bool checkConfig();
bool loadConfigFile(Model* m);
bool doModelProcessing(Model* m);
bool executeProcessors(Model* model, const QPtrList<Processor>& list);

// Global variables - used for configuration
QString g_inFileName;
QString g_outFileName;
QString g_modelName;
QString g_modelComment;
QString g_modelAuthor;
QString g_configFileName;
unsigned int g_numLods = 5;
float g_lod_factor = 0.5;
bool g_smoothAllFaces = false;
bool g_lod_useError = false;
bool g_lod_useBoth = false;
float g_lod_baseError = 0.075;
float g_lod_errorMod = 4.0;
int g_frames_keepCount = -1;
unsigned int g_frame_base = 0;
bool g_frames_keepAll = true;
float g_modelSize = 1.0;
unsigned int g_tex_size = 512;
QString g_tex_name;
QString g_tex_path;
bool g_tex_convertToLowerCase = false;
bool g_tex_optimize = false;
bool g_model_center = false;
bool g_tex_dontLoad = false;
bool g_meshes_merge = true;
bool g_usenormalcalculator = true;
float g_normalcalculator_threshold = 0.6;
bool g_materials_reset = false;


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
  m->setName(g_modelName);
  m->setComment(g_modelComment);
  m->setAuthor(g_modelAuthor);

  // Load the model
  if(!m->load(g_inFileName))
  {
    return 1;
  }

  // Load the config file
  loadConfigFile(m);

  if(g_smoothAllFaces)
  {
    boDebug() << "Smoothing all faces..." << endl;
    m->smoothAllFaces();
  }

  boDebug() << "Completing model loading..." << endl;
  m->loadingCompleted();

  if(!m->checkLoadedModel())
  {
    boError() << k_funcinfo << "broken model loaded" << endl;
    return 1;
  }


  // Do all necessary processing
  if(!doModelProcessing(m))
  {
    boError() << k_funcinfo << "model processing failed. cannot load model." << endl;
    return 1;
  }

  if(!m->checkLoadedModel())
  {
    boError() << k_funcinfo << "model processing broke model" << endl;
    return 1;
  }

  // Prepare the model for saving
  boDebug() << "Preparing model for saving..." << endl;
  m->prepareForSaving(g_frame_base);

  // Save the model
  boDebug() << "Saving model..." << endl;
  m->save(g_outFileName);

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

    if(larg == "-h" || larg == "-help" || larg == "--help")
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
      usage += "  -texoptimize       Combine all textures into one big texture\n";
      usage += "  -texsize <num>     Combined texture will be <num>x<num> pixels big\n";
      usage += "  -texname <file>    Combined texture will be written to <file> (this should be without path)\n";
      usage += "  -texpath <dir>     Combined texture file will be put to <dir>\n";
      usage += "  -t <dir>           Add directory <dir> to texture search path\n";
      usage += "  -texnametolower    Convert all texture names to lowercase\n";
      usage += "  -center            Centers the model\n";
      usage += "  -dontloadtex       Will not try to load the used textures\n";
      usage += "  -dontmergemeshes   Will not try to merge model's meshes\n";
      usage += "  -resetmaterials    Resets all model's materials to a default one\n";
      //usage += "  \n";
      cout << usage;
      return false;
    }
    else if(larg == "--version" || larg == "-v")
    {
      cerr << "BoBMFConverter version 0.13pre" << endl;
      return false;
    }
    else if(larg == "-o" || larg == "-output")
    {
      NEXTARG(arg);
      g_outFileName = arg;
    }
    else if(larg == "-c" || larg == "-config")
    {
      NEXTARG(arg);
      g_configFileName = arg;
    }
    else if(larg == "-name")
    {
      NEXTARG(arg);
      g_modelName = arg;
    }
    else if(larg == "-comment")
    {
      NEXTARG(arg);
      g_modelComment = arg;
    }
    else if(larg == "-author")
    {
      NEXTARG(arg);
      g_modelAuthor = arg;
    }
    else if(larg == "-lods")
    {
      // TODO: error checking for arguments which use int/float/whatever
      //  parameters
      NEXTARG(arg);
      g_numLods = arg.toUInt();
    }
    else if(larg == "-lodfactor")
    {
      NEXTARG(arg);
      g_lod_factor = arg.toFloat();
    }
    else if(larg == "-q" || larg == "-quick")
    {
      boError() << "'-quick' argument not yet implemented!" << endl;
    }
    else if(larg == "-smoothall")
    {
      g_smoothAllFaces = true;
    }
    else if(larg == "-useerror")
    {
      g_lod_useError = true;
    }
    else if(larg == "-useboth")
    {
      g_lod_useError = true;
      g_lod_useBoth = true;
    }
    else if(larg == "-baseerror")
    {
      NEXTARG(arg);
      g_lod_baseError = arg.toFloat();
    }
    else if(larg == "-errormod")
    {
      NEXTARG(arg);
      g_lod_errorMod = arg.toFloat();
    }
    else if(larg == "-baseframe")
    {
      NEXTARG(arg);
      g_frame_base = arg.toUInt();
    }
    else if(larg == "-noframes")
    {
      g_frames_keepCount = 1;
    }
    else if(larg == "-keepframes")
    {
      g_frames_keepAll = true;
    }
    else if(larg == "-size")
    {
      NEXTARG(arg);
      g_modelSize = arg.toFloat();
    }
    else if(larg == "-texsize")
    {
      NEXTARG(arg);
      g_tex_size = arg.toUInt();
    }
    else if(larg == "-t")
    {
      NEXTARG(arg);
      Texture::addTexturePath(arg);
    }
    else if(larg == "-texoptimize")
    {
      g_tex_optimize = true;
    }
    else if(larg == "-texname")
    {
      NEXTARG(arg);
      g_tex_name = arg;
    }
    else if(larg == "-texpath")
    {
      NEXTARG(arg);
      g_tex_path = arg;
    }
    else if(larg == "-texnametolower")
    {
      g_tex_convertToLowerCase = true;
    }
    else if(larg == "-center")
    {
      g_model_center = true;
    }
    else if(larg == "-dontloadtex")
    {
      g_tex_dontLoad = true;
    }
    else if(larg == "-dontmergemeshes")
    {
      g_meshes_merge = false;
    }
    else if(larg == "-resetmaterials")
    {
      g_materials_reset = true;
    }
    else
    {
      if(arg[0] == '-')
      {
        boError() << "Unrecognized argument " << arg << endl;
        return false;
      }

      g_inFileName = arg;
    }
  }

  return true;
}

bool checkConfig()
{
  // Check input file
  if(g_inFileName.isEmpty())
  {
    boError() << "No input file specified!" << endl;
    return false;
  }
  QFileInfo inFileinfo(g_inFileName);
  if(!inFileinfo.exists())
  {
    boError() << "Input file '" << g_inFileName << "' doesn't exist!" << endl;
    return false;
  }
  else if(!inFileinfo.isReadable())
  {
    boError() << "Input file '" << g_inFileName << "' isn't readable!" << endl;
    return false;
  }

  // Output file
  if(g_outFileName.isEmpty())
  {
    // Create output filename by replacing input file's extension with '.bmf'
    int i = g_inFileName.findRev('.');
    if(i == -1)
    {
      // Filename didn't have '.' in it. Just append '.bmf'
      g_outFileName = g_inFileName + ".bmf";
    }
    else
    {
      g_outFileName = g_inFileName.left(i) + ".bmf";
    }
  }

  // Output texture file
  if(g_tex_name.isEmpty())
  {
    // Create output texture filename by replacing input file's extension with
    //  '.jpg'
    int i = g_inFileName.findRev('.');
    if(i == -1)
    {
      // Filename didn't have '.' in it. Just append '.jpg'
      g_tex_name = g_inFileName + ".jpg";
    }
    else
    {
      g_tex_name = g_inFileName.left(i) + ".jpg";
    }
  }

  // Config file
  if(!g_configFileName.isEmpty())
  {
    QFileInfo configFileInfo(g_configFileName);
    if(!configFileInfo.exists())
    {
      boError() << "Config file '" << g_configFileName << "' doesn't exist!" << endl;
      return false;
    }
    else if(!configFileInfo.isReadable())
    {
      boError() << "Config file '" << g_configFileName << "' isn't readable!" << endl;
      return false;
    }
    g_configFileName = configFileInfo.absFilePath();
  }

  return true;
}

bool loadConfigFile(Model* m)
{
  if(g_configFileName.isEmpty())
  {
    return true;
  }

  KSimpleConfig cfg(g_configFileName, true);

  // Load model size
  cfg.setGroup("Boson Unit");
  g_modelSize = (float)cfg.readDoubleNumEntry("UnitWidth", g_modelSize);

  // Load entries from "Model" config group.
  // Note that size entry here takes preference over the one in "Boson Unit"
  //  group.
  cfg.setGroup("Model");
  g_modelSize = (float)cfg.readDoubleNumEntry("Size", g_modelSize);
  g_meshes_merge = cfg.readBoolEntry("MergeMeshes", g_meshes_merge);
  g_usenormalcalculator = cfg.readBoolEntry("UseNormalCalculator", g_usenormalcalculator);
  g_normalcalculator_threshold = (float)cfg.readDoubleNumEntry("NormalCalculatorThreshold", g_normalcalculator_threshold);
  g_frames_keepCount = cfg.readNumEntry("KeepFramesCount", g_frames_keepCount);
  g_numLods = cfg.readNumEntry("LODs", g_numLods);

  if(g_frames_keepCount == -1)
  {
    g_frames_keepCount = 1;
    QMap<QString, QString> entries = cfg.entryMap("Model");
    QRegExp animationend("Animation-[A-Za-z]+-End");
    for(QMap<QString, QString>::Iterator it = entries.begin(); it != entries.end(); ++it)
    {
      if(animationend.exactMatch(it.key()))
      {
        g_frames_keepCount = QMAX(g_frames_keepCount, cfg.readNumEntry(it.key(), 0)+1);
      }
    }
    boDebug() << k_funcinfo << "Automatically set number of kept frames to " << g_frames_keepCount << endl;
  }

  return true;
}


#include <fstream>
#include "mesh.h"
using namespace std;
void saveLod(Model* m, unsigned int i)
{
  LOD* l = m->lod(i);

  ofstream out(QString("%1-%2.obj").arg(g_outFileName).arg(i));

  QString vertexStr;
  QString faceStr;

  int vertexOffset = 1;
  for(unsigned int j = 0; j < l->meshCount(); j++)
  {
    Mesh* mesh = l->mesh(j);
    for(unsigned int i = 0; i < mesh->vertexCount(); i++)
    {
      BoVector3Float pos = mesh->vertex(i)->pos;
      vertexStr += QString("v %1 %2 %3\n").arg(pos.x()).arg(pos.y()).arg(pos.z());
    }
    for(unsigned int i = 0; i < mesh->faceCount(); i++)
    {
      Face* f = mesh->face(i);
      faceStr += QString("f %1 %2 %3\n").arg(f->vertex(0)->id+vertexOffset)
          .arg(f->vertex(1)->id+vertexOffset).arg(f->vertex(2)->id+vertexOffset);
    }
    vertexOffset += mesh->vertexCount();
  }

  out << vertexStr.latin1();
  out << faceStr.latin1();

  out.close();
}
#include "bo3dtools.h"
void saveLodFrame(Model* m, unsigned int lodi, unsigned int framei)
{
  LOD* l = m->lod(lodi);
  Frame* f = l->frame(framei);

  ofstream out(QString("%1-%2-%3.obj").arg(g_outFileName).arg(lodi).arg(framei));

  QString vertexStr;
  QString faceStr;

  int vertexOffset = 1;
  for(unsigned int i = 0; i < f->nodeCount(); i++)
  {
    Mesh* mesh = f->mesh(i);
    BoMatrix* matrix = f->matrix(i);
    // Save _tranformed_ vertices
    for(unsigned int j = 0; j < mesh->vertexCount(); j++)
    {
      BoVector3Float pos;
      matrix->transform(&pos, &mesh->vertex(j)->pos);
      vertexStr += QString("v %1 %2 %3\n").arg(pos.x()).arg(pos.y()).arg(pos.z());
    }
    // Save faces
    for(unsigned int j = 0; j < mesh->faceCount(); j++)
    {
      Face* f = mesh->face(j);
      faceStr += QString("f %1 %2 %3\n").arg(f->vertex(0)->id+vertexOffset)
          .arg(f->vertex(1)->id+vertexOffset).arg(f->vertex(2)->id+vertexOffset);
    }
    vertexOffset += mesh->vertexCount();
  }

  out << vertexStr.latin1();
  out << faceStr.latin1();

  out.close();
}

void saveLodFrameAC(Model* m, unsigned int lodi, unsigned int framei)
{
  LOD* l = m->lod(lodi);
  Frame* f = l->frame(framei);

  ofstream out(QString("%1-%2-%3.ac").arg(g_outFileName).arg(lodi).arg(framei));

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

bool doModelProcessing(Model* m)
{
  if(!g_tex_dontLoad)
  {
    // Load textures
    m->loadTextures();
  }
  else
  {
    boDebug() << "not loading textures due to request." << endl;
  }

  if(g_tex_convertToLowerCase)
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

  Processor::setBaseFrame(g_frame_base);

  if(!m->checkLoadedModel())
  {
    boError() << k_funcinfo << "cannot process broken model" << endl;
    return false;
  }

  QPtrList<Processor> processorList;
  processorList.setAutoDelete(true);

  processorList.append(new DefaultMaterials);
  processorList.append(new UnusedDataRemover);
  if(!g_frames_keepAll || g_frames_keepCount > 0)
  {
    FrameOptimizer* frameOptimizer = new FrameOptimizer();
    frameOptimizer->setName("DuplicateFrameRemover");
    frameOptimizer->setKeepFramesCount(g_frames_keepCount);
    processorList.append(frameOptimizer);
  }

  Transformer* transformer = new Transformer();
  transformer->setModelSize(g_modelSize);
  transformer->setCenterModel(g_model_center);
  processorList.append(transformer);
  transformer = 0;

  if(g_tex_optimize)
  {
    TextureOptimizer* textureOptimizer = new TextureOptimizer();
    textureOptimizer->setCombinedTexSize(g_tex_size);
    textureOptimizer->setCombinedTexFilename(g_tex_name);
    textureOptimizer->setCombinedTexPath(g_tex_path);
    processorList.append(textureOptimizer);
  }

  processorList.append(new NodeOptimizer());

  if(g_usenormalcalculator)
  {
    processorList.append(new NormalCalculator(g_normalcalculator_threshold));
  }
  MaterialOptimizer* materialOptimizer = new MaterialOptimizer();
  materialOptimizer->setResetMaterials(g_materials_reset);
  processorList.append(materialOptimizer);
  if(g_meshes_merge)
  {
    processorList.append(new MeshOptimizer());
  }
  processorList.append(new VertexOptimizer());


  if(!executeProcessors(m, processorList))
  {
    return false;
  }
  processorList.setAutoDelete(true);
  processorList.clear();


  if(!g_usenormalcalculator)
  {
    boDebug() << "Calculating model's face normals..." << endl;
    m->calculateFaceNormals();

    boDebug() << "Calculating model's vertex normals..." << endl;
    m->calculateVertexNormals();
  }

  boDebug() << "Creating lods..." << endl;
  m->createLODs(g_numLods);

  if(!m->checkLoadedModel())
  {
    boError() << k_funcinfo << "broken model after initial LOD creation" << endl;
    return false;
  }

  float targetFactor = 1.0f;
  float lodError = g_lod_baseError;
  for(unsigned int i = 1; i < g_numLods; i++)
  {
    targetFactor *= g_lod_factor;
    LodCreator* lodCreator = new LodCreator(i);
    lodCreator->setFaceTargetFactor(targetFactor);
    lodCreator->setMaxError(lodError);
    lodCreator->setUseError(g_lod_useError);
    lodCreator->setUseBoth(g_lod_useBoth);
    processorList.append(lodCreator);

    lodError *= g_lod_errorMod;
  }



  if(!executeProcessors(m, processorList))
  {
    return false;
  }
  processorList.setAutoDelete(true);
  processorList.clear();

  return true;
}


bool executeProcessors(Model* model, const QPtrList<Processor>& list)
{
  if(!model)
  {
    BO_NULL_ERROR(model);
    return false;
  }
  if(!model->checkLoadedModel())
  {
    boError() << k_funcinfo << "cannot process broken model" << endl;
    return false;
  }
  for(QPtrListIterator<Processor> it(list); it.current(); ++it)
  {
    QString name = it.current()->name();
    if(name.isEmpty())
    {
      name = "Unnamed";
    }
    boDebug() << k_funcinfo << "starting " << name << endl;
    if(!it.current()->initProcessor(model))
    {
      boError() << k_funcinfo << "initializing of processor " << name << " failed" << endl;
      return false;
    }
    if(!it.current()->process())
    {
      boError() << k_funcinfo << "processor " << name << " failed" << endl;
      return false;
    }
    if(!model->checkLoadedModel())
    {
      boError() << k_funcinfo << "model broken after executing processor " << name << ". Fix that processor!" << endl;
      return false;
    }
    boDebug() << k_funcinfo << name << " succeeded" << endl;
  }
  return true;
}

/*
 * vim: et sw=2
 */
