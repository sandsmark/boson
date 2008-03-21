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


#include "lodcreator.h"

#include "debug.h"
#include "model.h"
#include "lod.h"
#include "mesh.h"

#include <mixkit/stdmix.h>
#include <mixkit/MxStdModel.h>
#include <mixkit/MxQSlim.h>


LodCreator::LodCreator(int lodIndex) : Processor()
{
  mLODIndex = lodIndex;
  mMxModel = 0;
  mTargetFactor = -1;
  mMaxError = -1;
  mUseError = false;
  mUseBoth = false;
  setName(QString("LOD Creator (LOD %1)").arg(lodIndex));
}

LodCreator::~LodCreator()
{
}

bool LodCreator::initProcessor(Model* model)
{
  if(!Processor::initProcessor(model))
  {
    return false;
  }
  if(mLODIndex < 0)
  {
    return false;
  }
  if((unsigned int)mLODIndex >= model->lodCount())
  {
    boError() << k_funcinfo << "LOD index out of bounds: " << mLODIndex << " >= " << model->lodCount() << endl;
    return false;
  }
  LOD* lod = model->lod(mLODIndex);
  if(!lod)
  {
    BO_NULL_ERROR(lod);
    return false;
  }
  setLOD(lod);
  return true;
}

bool LodCreator::process()
{
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }
  if(useError())
  {
    if(maxError() == -1)
    {
      boError() << k_funcinfo << "Max error not set!" << endl;
      return false;
    }
  }
  else
  {
    if(mTargetFactor == -1)
    {
      boError() << k_funcinfo << "Face target not set!" << endl;
      return false;
    }
  }

  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    if(!processMesh(lod()->mesh(i)))
    {
      return false;
    }
  }

  return true;
}

bool LodCreator::processMesh(Mesh* mesh)
{
  // Load the mesh into MxStdModel
  mMxModel = loadMeshIntoMxModel(mesh);
  if(!mMxModel)
  {
    return false;
  }

  // Init slimming process
  MxQSlim* slim = initSlim(mMxModel);

  // Decimate
  /*float maxerror = 0.0;
  float minerror = 0.0;
  float totalerror = 0.0;
  float lasterror = 0.0;
  float error = ((MxEdgeQSlim*)slim)->rdecimate((int)MAX(mTargetFactor * mesh->faceCount(), 4), maxerror, minerror, totalerror, lasterror);

  boDebug() << k_funcinfo << "Reduced to " << (int)MAX(mTargetFactor * mesh->faceCount(), 4) <<
      " of " << mesh->faceCount() << "; error: " << error << "; maxerror: " << maxerror <<
      "; minerror: " << minerror << "; totalerror: " << totalerror <<
      "; lasterror: " << lasterror << endl;*/

  int facetarget = (int)(mTargetFactor * mesh->faceCount());
  if(useError() || useBoth())
  {
    ((MxEdgeQSlim*)slim)->decimate_until_error(maxError(), useBoth() ? facetarget : 4);
  }
  else
  {
    slim->decimate(MAX(facetarget, 4));
  }

  cleanupModel(mMxModel);

  // Put MxStdModel data back to the mesh
  if(!updateMeshFromMxModel(mesh, mMxModel))
  {
    delete mMxModel;
    mMxModel = 0;
    delete slim;
    return false;
  }

  // Delete data structures
  delete mMxModel;
  mMxModel = 0;
  delete slim;

  return true;
}


MxStdModel* LodCreator::loadMeshIntoMxModel(Mesh* mesh)
{
  MxStdModel* m = new MxStdModel(mesh->vertexCount(), mesh->faceCount());

  m->normal_binding(MX_PERVERTEX);
  m->texcoord_binding(MX_PERVERTEX);

  // Add vertices
  for(unsigned int i = 0; i < mesh->vertexCount(); i++)
  {
    Vertex* v = mesh->vertex(i);
    m->add_vertex(v->pos.x(), v->pos.y(), v->pos.z());
    m->add_normal(v->normal.x(), v->normal.y(), v->normal.z());
    m->add_texcoord(v->tex.x(), v->tex.y());
  }

  // Add faces
  for(unsigned int i = 0; i < mesh->faceCount(); i++)
  {
    Face* f = mesh->face(i);
    if(f->vertexCount() != 3)
    {
      boError() << k_funcinfo << "Mesh '" << mesh->name() << "' has face with " <<
          f->vertexCount() << " vertices!" << endl;
      return 0;
    }
    m->add_face(f->vertex(0)->id, f->vertex(1)->id, f->vertex(2)->id);
  }

  return m;
}

MxQSlim* LodCreator::initSlim(MxStdModel* m)
{
  MxQSlim* slim = new MxEdgeQSlim(*m);

  slim->placement_policy = MX_PLACE_OPTIMAL;
  slim->boundary_weight = 1000.0;
  slim->weighting_policy = MX_WEIGHT_ANGLE;
  slim->compactness_ratio = 0.0;
  slim->meshing_penalty = 1.0;
  slim->will_join_only = false;

  slim->initialize();

  return slim;
}

void LodCreator::cleanupModel(MxStdModel* m)
{
  // Mark stray vertices for removal
  for(unsigned int i = 0; i < m->vert_count(); i++)
  {
    if(m->vertex_is_valid(i) && (m->neighbors(i).length() == 0))
    {
      m->vertex_mark_invalid(i);
    }
  }

  // Compact vertices so that only the valid ones remain
  m->compact_vertices();
}

bool LodCreator::updateMeshFromMxModel(Mesh* mesh, MxStdModel* model)
{
  // Re-allocate vertex and face arrays. This deletes the previous arrays
  // There may be invalid faces in the model, so we need to count the valid
  //  faces first
  int validfaces = 0;
  for(unsigned int i = 0; i < model->face_count(); i++)
  {
    if(model->face_is_valid(i))
    {
      validfaces++;
    }
  }

  mesh->allocateVertices(model->vert_count());
  mesh->allocateFaces(validfaces);

  // Load vertices
  for(unsigned int i = 0; i < model->vert_count(); i++)
  {
    if(!model->vertex_is_valid(i))
    {
      boError() << k_funcinfo << "Invalid vertex " << i << " is still there!" << endl;
      return false;
    }
    Vertex* v = mesh->vertex(i);
    v->pos.set(model->vertex(i)[0], model->vertex(i)[1], model->vertex(i)[2]);
    v->normal.set(model->normal(i)[0], model->normal(i)[1], model->normal(i)[2]);
    v->tex.set(model->texcoord(i)[0], model->texcoord(i)[1]);
  }

  // Load faces
  int meshi = 0;
  for(unsigned int i = 0; i < model->face_count(); i++)
  {
    if(model->face_is_valid(i))
    {
      Face* f = mesh->face(meshi);
      f->setVertexCount(3);
      f->setVertex(0, mesh->vertex(model->face(i)[0]));
      f->setVertex(1, mesh->vertex(model->face(i)[1]));
      f->setVertex(2, mesh->vertex(model->face(i)[2]));
      meshi++;
    }
  }

  return true;
}

/*
 * vim: et sw=2
 */
