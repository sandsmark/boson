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


#include "materialoptimizer.h"

#include "debug.h"
#include "model.h"
#include "lod.h"
#include "mesh.h"
#include "material.h"

#include <q3valuevector.h>

#define MAX_MATERIAL_DIFF 0.1f


MaterialOptimizer::MaterialOptimizer() : Processor()
{
  setName("MaterialOptimizer");
  mReset = false;
}

MaterialOptimizer::~MaterialOptimizer()
{
}

bool MaterialOptimizer::process()
{
  /*if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }*/

  if(mReset)
  {
    resetMaterials();
  }

  Q3ValueVector<Material*> validmaterials;
  for(unsigned int i = 0; i < model()->materialCount(); i++)
  {
    Material* mat = model()->material(i);
    bool valid = true;
    for(unsigned int j = 0; j < validmaterials.count(); j++)
    {
      Material* validmat = validmaterials[j];
      if(mat->texture() == validmat->texture())
      {
        if(materialDifference(mat, validmat) <= MAX_MATERIAL_DIFF)
        {
          // Merge mat and validmat
          mergeMaterials(mat, validmat);
          valid = false;
          break;
        }
      }
    }
    if(valid)
    {
      validmaterials.append(mat);
    }
    else
    {
      delete mat;
    }
  }

  if(validmaterials.count() != model()->materialCount())
  {
    unsigned int oldcount = model()->materialCount();
    model()->setMaterials(validmaterials);
    boDebug() << k_funcinfo << "Merged " << oldcount << " materials into " << validmaterials.count() << endl;
  }

  return true;
}

float MaterialOptimizer::materialDifference(Material* m1, Material* m2)
{
  float diff = 0.0f;
  // Ambient color
  diff += qAbs(m1->ambient().x() - m2->ambient().x());
  diff += qAbs(m1->ambient().y() - m2->ambient().y());
  diff += qAbs(m1->ambient().z() - m2->ambient().z());
  // Diffuse color
  diff += qAbs(m1->diffuse().x() - m2->diffuse().x());
  diff += qAbs(m1->diffuse().y() - m2->diffuse().y());
  diff += qAbs(m1->diffuse().z() - m2->diffuse().z());
  // Specular color
  diff += qAbs(m1->specular().x() - m2->specular().x());
  diff += qAbs(m1->specular().y() - m2->specular().y());
  diff += qAbs(m1->specular().z() - m2->specular().z());
  // Shininess
  diff += qAbs(m1->shininess() - m2->shininess()) * 0.33;

  return diff;
}

void MaterialOptimizer::mergeMaterials(Material* merge, Material* valid)
{
  // Replace merge with valid in all meshes
  for(unsigned int i = 0; i < model()->lodCount(); i++)
  {
    LOD* l = model()->lod(i);
    for(unsigned int j = 0; j < l->meshCount(); j++)
    {
      if(l->mesh(j)->material() == merge)
      {
        l->mesh(j)->setMaterial(valid);
      }
    }
  }
}

void MaterialOptimizer::resetMaterials()
{
  for(unsigned int i = 0; i < model()->materialCount(); i++)
  {
    Material* mat = model()->material(i);
    mat->setAmbient(BoVector4Float(0.8, 0.8, 0.8, 1.0));
    mat->setDiffuse(BoVector4Float(1.0, 1.0, 1.0, 1.0));
    mat->setSpecular(BoVector4Float(0.0, 0.0, 0.0, 1.0));
    mat->setEmissive(BoVector4Float(0.0, 0.0, 0.0, 1.0));
    mat->setShininess(0.0f);
  }
}

