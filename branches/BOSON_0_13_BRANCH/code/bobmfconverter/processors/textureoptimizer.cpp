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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "textureoptimizer.h"

#include "debug.h"
#include "lod.h"
#include "frame.h"
#include "mesh.h"
#include "texture.h"
#include "material.h"
#include "model.h"
#include "bo3dtools.h"

#include <math.h>

#include <qdict.h>
#include <qimage.h>
#include <qwmatrix.h>


//#define USE_BP


#ifdef USE_BP
namespace BP
{
  extern "C"
  {
    #include "2dbp/2DBPlib.h"
  };
}
#endif


class TextureOptimizer::TextureInfo
{
  public:
    Texture* tex;
    float score;
    unsigned int blocks;
    float ratio;
    bool rotated;

    // Min/max texels of this texture in combined texture
    float mintexelx;
    float mintexely;
    float maxtexelx;
    float maxtexely;

    bool operator<(const TextureOptimizer::TextureInfo& i)
    {
      return (i.score < score);
    }
};


TextureOptimizer::TextureOptimizer() : Processor()
{
  mTexSize = 512;
  setName("TextureOptimizer");
}

TextureOptimizer::~TextureOptimizer()
{
}

bool TextureOptimizer::process()
{
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }
  if(mTexFilename.isEmpty())
  {
    boError() << k_funcinfo << "Output texture filename not set!" << endl;
    return false;
  }

  // Check for easy case
  if(model()->texturesDict()->count() <= 1)
  {
    return true;
  }

  // We need correct ids
  //model()->updateIds();

  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    normalizeTexCoords(lod()->mesh(i));
  }

  QDictIterator<Texture> it(*model()->texturesDict());
  while(it.current())
  {
    calculateTotalUsedArea(it.current());
    ++it;
  }

  // Combine all the textures
  Texture* c = combineAllTextures();
  if(!c)
  {
    return false;
  }
  // Delete all other textures
  model()->texturesDict()->clear();
  model()->addTexture(c);

  return true;
}

bool TextureOptimizer::normalizeTexCoords(Mesh* m)
{
  bool* vertexprocessed = new bool[m->vertexCount()];
  for(unsigned int i = 0; i < m->vertexCount(); i++)
  {
    vertexprocessed[i] = false;
  }

  for(unsigned int i = 0; i < m->vertexCount(); i++)
  {
    // Find next unprocessed vertex
    if(vertexprocessed[i])
    {
      continue;
    }

    // Create new batch
    QValueList<Vertex*> batch;
    // Find all vertices belonging to that batch using floodfill algorithm
    QValueList<Vertex*> open;
    open.append(m->vertex(i));
    vertexprocessed[i] = true;
    while(!open.isEmpty())
    {
      // Get the first vertex from the open list
      Vertex* v = open.first();
      open.pop_front();
      // Add all vertices sharing common face with v
      for(unsigned int j = 0; j < v->faces.count(); j++)
      {
        Face* f = v->faces[j];
        for(unsigned int k = 0; k < f->vertexCount(); k++)
        {
          Vertex* v2 = f->vertex(k);
          if(vertexprocessed[v2->id])
          {
            continue;
          }
          // Add v2 to open list
          open.append(v2);
          vertexprocessed[v2->id] = true;
        }
      }
      // Add v to batch
      batch.append(v);
    }

    // Batch is now created
    // Find min coords of all vertices in batch
    float minx = batch.first()->tex.x();
    float miny = batch.first()->tex.y();
    QValueList<Vertex*>::Iterator it;
    for(it = batch.begin(); it != batch.end(); ++it)
    {
      minx = QMIN(minx, (*it)->tex.x());
      miny = QMIN(miny, (*it)->tex.y());
    }
    // How much to add to texcoords
    float addx = -floorf(minx);
    float addy = -floorf(miny);
    // Modify texcoords of all vertices in the batch
    for(it = batch.begin(); it != batch.end(); ++it)
    {
      Vertex* v = *it;
      v->tex.setX(v->tex.x() + addx);
      v->tex.setY(v->tex.y() + addy);
    }
  }

  return true;
}

void TextureOptimizer::calculateTotalUsedArea(Texture* t)
{
  float totalfacearea = 0.0f;
  float totaltexarea = 0.0f;
  unsigned int totalmeshes = 0;
  // Min/max used texels
  BoVector2Float min, max;
  bool minmaxtexelsset = false;

  Frame* f = lod()->frame(baseFrame());
  for(unsigned int i = 0; i < f->nodeCount(); i++)
  {
    Mesh* mesh = f->mesh(i);
    if(!mesh->material())
    {
      //boDebug() << "    " << "Mesh " << i << "('" << mesh->name() << "') has no material" << endl;
      continue;
    }
    else if(mesh->material()->texture() != t)
    {
      //boDebug() << "    " << "Mesh " << i << "('" << mesh->name() << "') has texture " <<
      //    mesh->material()->texture()->filename() << endl;
      continue;
    }
    //boDebug() << "    " << "Mesh " << i << "('" << mesh->name() << "') has given tex, processing..." << endl;
    BoMatrix* matrix = f->matrix(i);
    bool identitymatrix = matrix->isIdentity();

    // Min/max used texels
    if(!minmaxtexelsset)
    {
      min = mesh->face(0)->vertex(0)->tex;
      max = min;
      minmaxtexelsset = true;
    }

    for(unsigned int j = 0; j < mesh->faceCount(); j++)
    {
      Face* face = mesh->face(j);
      // Calculate face area first
      BoVector3Float v0 = face->vertex(0)->pos;
      BoVector3Float v1 = face->vertex(1)->pos;
      BoVector3Float v2 = face->vertex(2)->pos;
      if(!identitymatrix)
      {
        matrix->transform(&v0, &face->vertex(0)->pos);
        matrix->transform(&v1, &face->vertex(1)->pos);
        matrix->transform(&v2, &face->vertex(2)->pos);
      }
      float facearea = calculateArea(v0, v1, v2);
      // Then calculate texture area (area of the face in texture space)
      BoVector2Float t0 = face->vertex(0)->tex;
      BoVector2Float t1 = face->vertex(1)->tex;
      BoVector2Float t2 = face->vertex(2)->tex;
      float texarea = calculateArea(t0, t1, t2);
      // Find min/max texels
      min.setX(QMIN(min.x(), QMIN(t0.x(), QMIN(t1.x(), t2.x()))));
      min.setY(QMIN(min.y(), QMIN(t0.y(), QMIN(t1.y(), t2.y()))));
      max.setX(QMAX(max.x(), QMAX(t0.x(), QMAX(t1.x(), t2.x()))));
      max.setY(QMAX(max.y(), QMAX(t0.y(), QMAX(t1.y(), t2.y()))));
      // Add the areas to total area sizes
      if(finite(facearea) && finite(texarea))
      {
        totalfacearea += facearea;
        totaltexarea += texarea;
      }
    }
    totalmeshes++;
  }


//  t->setTotalUsedArea(area);
//  boDebug() << k_funcinfo << "Face/tex area ratio for texture '" << t->filename() <<
//      "' is " << totalfacearea << "/" << totaltexarea << "(" <<
//      totalfacearea / totaltexarea << ":1) in " << totalmeshes << " meshes" << endl;
  boDebug() << k_funcinfo << "Tex/face area ratio for texture '" << t->filename() <<
      "' is " << totaltexarea << "/" << totalfacearea << "(" <<
      totaltexarea / totalfacearea << ":1) in " << totalmeshes << " meshes" << endl;
  boDebug() << "    " << "Texture's min/max used texels: (" <<
      min.x() << "; " << min.y() << ")-(" << max.x() << "; " << max.y() << ")" << endl;

  t->setTotalUsedFaceArea(totalfacearea);
  t->setTotalUsedTexArea(totaltexarea);
  t->setUsedArea(min.x(), min.y(), max.x(), max.y());
}

float TextureOptimizer::calculateArea(const BoVector3Float& v0, const BoVector3Float& v1, const BoVector3Float& v2)
{
  // Find surface area of the face (triangle) using Heron's quotation

  float xdiff01 = v0.x() - v1.x();
  float xdiff12 = v1.x() - v2.x();
  float xdiff20 = v2.x() - v0.x();
  float ydiff01 = v0.y() - v1.y();
  float ydiff12 = v1.y() - v2.y();
  float ydiff20 = v2.y() - v0.y();
  float zdiff01 = v0.z() - v1.z();
  float zdiff12 = v1.z() - v2.z();
  float zdiff20 = v2.z() - v0.z();

  // Find lengths of all sides
  float a = sqrt(xdiff01 * xdiff01 + ydiff01 * ydiff01 + zdiff01 * zdiff01);
  float b = sqrt(xdiff12 * xdiff12 + ydiff12 * ydiff12 + zdiff12 * zdiff12);
  float c = sqrt(xdiff20 * xdiff20 + ydiff20 * ydiff20 + zdiff20 * zdiff20);

  float p = 0.5 * (a + b + c);

  return sqrt(p * (p - a) * (p - b) * (p - c));
}

float TextureOptimizer::calculateArea(const BoVector2Float& v0, const BoVector2Float& v1, const BoVector2Float& v2)
{
  // Find surface area of the face (triangle) using Heron's quotation

  float xdiff01 = v0.x() - v1.x();
  float xdiff12 = v1.x() - v2.x();
  float xdiff20 = v2.x() - v0.x();
  float ydiff01 = v0.y() - v1.y();
  float ydiff12 = v1.y() - v2.y();
  float ydiff20 = v2.y() - v0.y();

  // Find lengths of all sides
  float a = sqrt(xdiff01 * xdiff01 + ydiff01 * ydiff01);
  float b = sqrt(xdiff12 * xdiff12 + ydiff12 * ydiff12);
  float c = sqrt(xdiff20 * xdiff20 + ydiff20 * ydiff20);

  float p = 0.5 * (a + b + c);

  return sqrt(p * (p - a) * (p - b) * (p - c));
}

Texture* TextureOptimizer::combineAllTextures()
{
  // Create TextureInfo objects for each texture
  // Also gather some statistics (e.g. total face/tex area)
  QValueVector<TextureInfo> textures(model()->texturesDict()->count());
  float totaltexarea = 0.0f;
  float totalfacearea = 0.0f;
  QDictIterator<Texture> it(*model()->texturesDict());
  for(unsigned int i = 0; it.current(); ++it, i++)
  {
    Texture* t = it.current();
    textures[i].tex = t;

    totaltexarea += t->totalUsedTexArea();
    totalfacearea += t->totalUsedFaceArea();
  }
  float avgtotalresolution = totaltexarea / totalfacearea;

  boDebug() << "  " << "There are " << textures.count() << " textures, total used tex/face areas: " <<
      totaltexarea << "/" << totalfacearea << ";  average ratio: " <<
      avgtotalresolution << ":1" << endl;

  // Calculate "scores" for the textures.
  //  The bigger the score is, the more important (and more-seen) the texture
  //  is.
  //  To have big score, texture should have:
  //  * High (visible) resolution (tex/face area ratio)
  //  * High total face area (used on many faces)
  //  * High used texture area
  float totalscore = 0.0f;
  for(unsigned int i = 0; i < textures.count(); i++)
  {
    Texture* t = textures[i].tex;
    float avgresolution = t->totalUsedTexArea() / t->totalUsedFaceArea();
    float texarea = (t->usedAreaMaxX() - t->usedAreaMinX()) * (t->usedAreaMaxY() - t->usedAreaMinY());
    float score = sqrt(1 + t->totalUsedTexArea() / totaltexarea) * avgresolution * (0.8 + 0.2 * sqrt(texarea));

    textures[i].score = score;
    totalscore += score;
  }

  // Debug
  for(unsigned int i = 0; i < textures.count(); i++)
  {
    TextureInfo tinfo = textures[i];
    float avgresolution = tinfo.tex->totalUsedTexArea() / tinfo.tex->totalUsedFaceArea();
    int pix = (int)sqrt((tinfo.score / totalscore) * (mTexSize * mTexSize));
    boDebug() << "    " << "Texture '" << tinfo.tex->filename() << "' would get " <<
        tinfo.score / totalscore * 100 << "% of combined texture (about " <<
        pix << "x" << pix << " pixels of " << mTexSize << "x" << mTexSize << ")  (score: " <<
        tinfo.score << "; total: " << totalscore << ";  reso: " <<
        avgresolution << "; avg: " << avgtotalresolution << ")" << endl;
  }


  // Load the textures
  boDebug() << k_funcinfo << "Loading textures..." << endl;
  for(unsigned int i = 0; i < textures.count(); i++)
  {
    if(!textures[i].tex->load())
    {
      return 0;
    }
  }

  boDebug() << k_funcinfo << "Creating combined texture..." << endl;
  Texture* combinedtex = new Texture(mTexFilename);
  // Create combined texture image
  QImage img(mTexSize, mTexSize, 32);
  // Fill the image with 50% gray
  img.fill(qRgb(128, 128, 128));
//  img.fill(qRgb(255, 255, 96));


  // Sort the list of textures by amount of space they'll get in the combined
  //  texture
  qHeapSort(textures);

  // We divide the combined texture into 16x16 pixel blocks, every block is
  //  used by (at most) one texture.
  // Calculate how many such blocks textures can use
  unsigned int totalblocks = (mTexSize / 16) * (mTexSize / 16);
  unsigned int blocksside = (mTexSize / 16);
  for(unsigned int i = 0; i < textures.count(); i++)
  {
    textures[i].blocks = (unsigned int)(textures[i].score / totalscore * totalblocks);

    // Calculate optimal ratio of the texture
    Texture* t = textures[i].tex;
    float usedtexw = t->usedAreaMaxX() - t->usedAreaMinX();
    float usedtexh = t->usedAreaMaxY() - t->usedAreaMinY();
    textures[i].ratio = (usedtexw * t->width()) / (usedtexh * t->height());
    boDebug() << "    " << "Texture '" << textures[i].tex->filename() <<
        "' has w/h ratio " << textures[i].ratio << endl;
//    boDebug() << "    " << "Texture '" << textures[i].tex->filename() <<
//        "' will get " << textures[i].blocks << " 16x16 blocks" << endl;
  }


  // List of used blocks
  bool* usedblocks = new bool[totalblocks];
#define BLOCKUSED(x, y)  usedblocks[y * blocksside + x]
  for(unsigned int i = 0; i < totalblocks; i++)
  {
    usedblocks[i] = false;
  }

#ifdef USE_BP
  boDebug() << k_funcinfo << "Initing 2DBP..." << endl;
  int* boxw = new int[textures.count()];
  int* boxh = new int[textures.count()];
  for(unsigned int i = 0; i < textures.count(); i++)
  {
    float wantedarea = (textures[i].score / totalscore) * (mTexSize * mTexSize);
    float scale = sqrt(wantedarea / textures[i].ratio);
    boxw[i] = QMIN((int)mTexSize, (int)(scale * textures[i].ratio));
    boxh[i] = QMIN((int)mTexSize, (int)(scale));
  }
  // Pass 1
  BP::init_all(mTexSize, textures.count(), 200, 1500);
  for(unsigned int i = 0; i < textures.count(); i++)
  {
    boDebug() << "2DBP src:  BP::put_box(" << i << ", " << boxw[i] << ", " << boxh[i] << ");" << endl;
//    BP::put_box(i, boxw[i], boxh[i]);
    BP::put_box(i, boxh[i], boxw[i]);
  }
  boDebug() << k_funcinfo << "Computing 2DBP..." << endl;
  BP::compute_GA(0);
  if(!BP::isValid())
  {
    boError() << k_funcinfo << "2DBP pass 1 failed!" << endl;
    return false;
  }
  boDebug() << k_funcinfo << "Changing coords..." << endl;
  // Find max Y coord of the solution
  int maxY = 0;
  for(unsigned int i = 0; i < textures.count(); i++)
  {
    maxY = QMAX(maxY, BP::posY(i) + BP::posH(i));
  }
  float scale = mTexSize / (float)maxY;
  boDebug() << "    " << "Textures will be scaled vertically by " << scale << endl;
  int usedarea = 0;
  for(unsigned int i = 0; i < textures.count(); i++)
  {
    bool rot = !BP::isRotated(i);
    int w = BP::posW(i);
    int h = (int)(BP::posH(i) * scale);
    if(rot)
    {
      w = (int)(BP::posH(i) * scale);
      h = BP::posW(i);
    }
    boDebug() << "2DBP result " << i << ": pos: (" << BP::posX(i) << "; " << (int)(BP::posY(i) * scale) <<
        "),  size: " << w << "x" << h << ", rotated: " << rot << endl;
    textures[i].rotated = rot;
    copyTextureToCombinedTexture(&textures[i], &img,
        BP::posX(i), (int)(BP::posY(i) * scale), w, h,
        mTexSize);
    /*copyTextureToCombinedTexture(&textures[i], &img,
        BP::posX(i), BP::posY(i), BP::posW(i), BP::posH(i),
        mTexSize);*/
    modifyTexCoordsForCombinedTexture(&textures[i]);
    replaceTextureInMaterials(textures[i].tex, combinedtex);
    usedarea += w*h;
  }
  boDebug() << "  " << (1 - usedarea / (float)(mTexSize*mTexSize)) * 100 <<
      "% of combined texture was left unused" << endl;
  BP::clean_all();
#else
  // Find best positions for the textures
  // Dummy algorithm
  int hdivisions = (int)floorf(sqrtf((float)textures.count()));
  int vdivisions = (int)ceilf(textures.count() / (float)hdivisions);
  int hdivsize = (int)(mTexSize / (float)hdivisions);
  int vdivsize = (int)(mTexSize / (float)vdivisions);

  boDebug() << "    " << k_funcinfo << "Combined image is divided into " <<
      hdivisions << "x" << vdivisions << " blocks, " <<
      hdivsize << "x" << vdivsize << " each (" << textures.count() << " images)" << endl;

  for(int vdiv = 0; vdiv < vdivisions; vdiv++)
  {
    for(int hdiv = 0; hdiv < hdivisions; hdiv++)
    {
      unsigned int i = vdiv * hdivisions + hdiv;
      if(i >= textures.count())
      {
        continue;
      }
      // Put texture i to this "slot"
      // Calculate pos where the tex will be put
      int left = hdivsize * hdiv;
      int top = vdivsize * vdiv;
      // Copy
      copyTextureToCombinedTexture(&textures[i], &img, left, top, hdivsize, vdivsize, mTexSize);
      modifyTexCoordsForCombinedTexture(&textures[i]);
      replaceTextureInMaterials(textures[i].tex, combinedtex);
    }
  }
#endif

  boDebug() << k_funcinfo << "Saving combined texture..." << endl;
  // Find out format of the combined texture
  const char* format;
  if(mTexFilename.endsWith(".jpg", false) || mTexFilename.endsWith(".jpeg", false))
  {
    format = "JPEG";
  }
  else if(mTexFilename.endsWith(".png", false))
  {
    format = "PNG";
  }
  else
  {
    boWarning() << k_funcinfo << "Couldn't guess format of the filename ('" << mTexFilename <<
        "'), defaulting to JPEG" << endl;
    format = "JPEG";
  }
  // Save the image
  img.save(mTexPath + mTexFilename, format);

  return combinedtex;
}

void TextureOptimizer::copyTextureToCombinedTexture(TextureInfo* src, QImage* dst, int dstx, int dsty, int w, int h, int dstsize)
{
  Texture* t = src->tex;
  // The texture may repeat, so calculate the size of the "base" (that is,
  //  non-repeated) texture image.
  // Width/height of used texture area
  float usedtexw = t->usedAreaMaxX() - t->usedAreaMinX();
  float usedtexh = t->usedAreaMaxY() - t->usedAreaMinY();
  // Size of the base image (in pixels)
  float basew = w / usedtexw;
  float baseh = h / usedtexh;
  // Resize texture image
  boDebug() << "    " << "Copying texture '" << t->filename() << "', used area: (" <<
      t->usedAreaMinX() << "; " << t->usedAreaMinY() << ")-(" <<
      t->usedAreaMaxX() << "; " << t->usedAreaMaxY() << ")  base tex size: " <<
      lround(basew) << "x" << lround(baseh) << endl;
  boDebug() << "    " << "Texture '" << t->filename() << "' will have " << w << "x" << h <<
      " pixels at (" << dstx << "; " << dsty << ")" << endl;
  int startpixx = (int)floor(t->usedAreaMinX() * basew);
  int startpixy = (int)floor(t->usedAreaMinY() * baseh);
  int endpixx = startpixx + w;
  int endpixy = startpixy + h;
  int intbasew = (int)basew;
  int intbaseh = (int)baseh;
  QImage baseimage;
  if(src->rotated)
  {
    // Scale the image first. Note the swapped w/h
    baseimage = t->image()->smoothScale(intbaseh, intbasew);
    // Now transform the image: rotate it 90 degrees around lower-left corner
    QWMatrix wm;
    wm.translate(0, -intbaseh);
    wm.rotate(90);
    baseimage = baseimage.xForm(wm);
    // Finally mirror the image vertically
    baseimage = baseimage.mirror();
    boDebug() << "      " << "baseimage size after rotation: " <<
        baseimage.width() << "x" << baseimage.height() << " (intbasesize: " <<
        intbasew << "x" << intbaseh << ")" << endl;
  }
  else
  {
    baseimage = t->image()->smoothScale(intbasew, intbaseh);
  }
  // Tile the base texture
  QImage tiledimage(w, h, 32);
  //tiledimage.fill(qRgb(255, 255, 96));
  for(int x = 0; x < endpixx; x += intbasew)
  {
    for(int y = 0; y < endpixy; y += intbaseh)
    {
      int copyx = QMAX(x, startpixx) % intbasew;
      int copyy = QMAX(y, startpixy) % intbaseh;
      bitBlt(&tiledimage, QMAX(x, startpixx) - startpixx, QMAX(y, startpixy) - startpixy,
          &baseimage, copyx, copyy, intbasew, intbaseh,  0);
    }
  }
  bitBlt(dst, dstx, dsty,  &tiledimage, 0, 0, w, h, 0);

  // Calculate how much we have to translate/scale texcoords
  // Min/max texel coords of the texture in combined texture
  src->mintexelx = dstx / (float)dstsize;
  src->mintexely = (dstsize - (dsty + h)) / (float)dstsize;
  src->maxtexelx = (dstx + w) / (float)dstsize;
  src->maxtexely = (dstsize - dsty) / (float)dstsize;
  boDebug() << "        " << "Texel rect is (" << src->mintexelx << "; " << src->mintexely << ")-(" <<
      src->maxtexelx << "; " << src->maxtexely << ")  dst: (" << dstx << "; " << dsty << "); size: " <<
      w << "x" << h << ";  dstsize: " << dstsize << endl;

}

void TextureOptimizer::modifyTexCoordsForCombinedTexture(TextureOptimizer::TextureInfo* tinfo)
{
  Texture* t = tinfo->tex;
  float usedtexw = t->usedAreaMaxX() - t->usedAreaMinX();
  float usedtexh = t->usedAreaMaxY() - t->usedAreaMinY();
  float texelw = tinfo->maxtexelx - tinfo->mintexelx;
  float texelh = tinfo->maxtexely - tinfo->mintexely;

//  boDebug() << "        " << "usedtex size: " << usedtexw << "x" << usedtexh <<
//      "; usedarea min: (" << t->usedAreaMinX() << "; " << t->usedAreaMinY() << ")" << endl;

  float minx = 10000.0f;
  float miny = 10000.0f;
  float maxx = -10000.0f;
  float maxy = -10000.0f;

  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    Mesh* m = lod()->mesh(i);
    if(!m->material())
    {
      continue;
    }
    else if(m->material()->texture() != tinfo->tex)
    {
      continue;
    }
    //boDebug() << "        " << "processing mesh " << m->name() << endl;

    for(unsigned int j = 0; j < m->vertexCount(); j++)
    {
      Vertex* v = m->vertex(j);
      BoVector2Float tex = v->tex;
        minx = QMIN(minx, tex.x());
        miny = QMIN(miny, tex.y());
        maxx = QMAX(maxx, tex.x());
        maxy = QMAX(maxy, tex.y());
      if(tinfo->rotated)
      {
        float tmpx = (tex.y() - t->usedAreaMinY()) * (texelw / usedtexh) + tinfo->mintexelx;
        float tmpy = (tex.x() - t->usedAreaMinX()) * (texelh / usedtexw) + tinfo->mintexely;
        tex.set(tmpx, tmpy);
      }
      else
      {
        tex.setX((tex.x() - t->usedAreaMinX()) * (texelw / usedtexw) + tinfo->mintexelx);
        tex.setY((tex.y() - t->usedAreaMinY()) * (texelh / usedtexh) + tinfo->mintexely);
      }
      v->tex = tex;
    }

  }

//  boDebug() << "        " << "Min/max seen texcoords were: (" <<
//      minx << "; " << miny << ") and (" << maxx << "; " << maxy << ")" << endl;
}

void TextureOptimizer::replaceTextureInMaterials(Texture* replace, Texture* with)
{
  for(unsigned int i = 0; i < model()->materialCount(); i++)
  {
    Material* mat = model()->material(i);
    if(mat->texture() == replace)
    {
      mat->setTexture(with);
    }
  }
}

