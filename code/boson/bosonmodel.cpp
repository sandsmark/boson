/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonmodel.h"

#include "defines.h"
#include "bosonmodeltextures.h"
#include "bosonprofiling.h"
#include "bo3dtools.h"
#include "bosonglwidget.h" // BoContext
#include "bodebug.h"
#include "bomesh.h"
#include "bomaterial.h"
#include "bosonconfig.h"
#include "bomemorytrace.h"
#include "bomeshrenderer.h"
#include "bomeshrenderermanager.h"
#include "bobmfload.h"

#include <ksimpleconfig.h>
#include <kmdcodec.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include <qstringlist.h>
#include <qvaluelist.h>
#include <qintdict.h>
#include <qptrvector.h>
#include <qfile.h>

#include <math.h>
#if HAVE_GL_GLEXT_H
#include <GL/glext.h>
#endif



BoFrame::BoFrame()
{
 init();
}

BoFrame::BoFrame(const BoFrame& f, unsigned int firstNode, unsigned int nodeCount)
{
 init();
 mDepthMultiplier = f.mDepthMultiplier;
 mRadius = f.mRadius;

 if (nodeCount == 0) {
	boWarning() << k_funcinfo << "no nodes copied" << endl;
	return;
 }
 if (firstNode + nodeCount > f.mNodeCount) {
	boError() << k_funcinfo << "can't copy " << nodeCount
			<< " nodes starting at " << firstNode
			<< ", as there are only " << f.mNodeCount
			<< " nodes!" << endl;
	nodeCount = f.mNodeCount - firstNode;
 }
 unsigned int* nodes = new unsigned int[nodeCount];
 for (unsigned int i = 0; i < nodeCount; i++) {
	// FIXME: is this ok? Shouldn't one 'nodes' be e.g. 'f.nodes' ?
//	nodes[i] = nodes[firstNode + i];
	nodes[i] = firstNode + i;
 }

 copyNodes(f, nodes, nodeCount);
 delete[] nodes;
}

BoFrame::BoFrame(const BoFrame& f, unsigned int* nodes, unsigned int nodeCount)
{
 init();
 mDepthMultiplier = f.mDepthMultiplier;
 mRadius = f.mRadius;
 copyNodes(f, nodes, nodeCount);
}

void BoFrame::init()
{
 mDepthMultiplier = 0.0f;
 mRadius = 0.0f;
 mMatrices = 0;
 mMeshes = 0;
 mNodeCount = 0;
}

BoFrame::~BoFrame()
{
 if (mMatrices) {
	for (unsigned int i = 0; i < mNodeCount; i++) {
		delete mMatrices[i];
	}
 }
 delete[] mMeshes;
 delete[] mMatrices;
}

void BoFrame::copyNodes(const BoFrame& f, unsigned int* nodes, unsigned int count)
{
 if (f.mNodeCount == 0 || !f.mMeshes || !f.mMatrices) {
	boError() << k_funcinfo << "oops - can't copy from invalid frame!" << endl;
	return;
 }
 BO_CHECK_NULL_RET(nodes);
 if (count == 0) {
	boWarning() << k_funcinfo << "no nodes copied" << endl;
	return;
 }
 if (count > f.mNodeCount) {
	boError() << k_funcinfo << "cannot copy " << count
			<< " nodes as frame contains " << f.mNodeCount
			<< " nodes only" << endl;
	return;
 }

 // before allocating anything we first check whether all indices are valid
 for (unsigned int i = 0; i < count; i++) {
	if (nodes[i] >= f.mNodeCount) {
		boError() << k_funcinfo << "index " << nodes[i] << " at " << i
				<< " is not valid! only " << f.mNodeCount
				<< " available in the frame" << endl;
		return;
	}
 }

 allocNodes(count);
 for (unsigned int i = 0; i < count; i++) {
	unsigned int index = nodes[i];
	mMeshes[i] = f.mMeshes[index]; // copy the pointer only
	mMatrices[i]->loadMatrix(*f.mMatrices[index]);
 }
}

void BoFrame::allocNodes(int nodes)
{
 if (mNodeCount != 0) {
	boError() << k_funcinfo << "nodes already loaded" << endl;
	return;
 }
 if (mMatrices) {
	boError() << k_funcinfo << "matrices already allocated??" << endl;
	delete[] mMatrices;
 }
 if (mMeshes) {
	boError() << k_funcinfo << "meshes already allocated??" << endl;
	delete[] mMeshes;
 }
 mMeshes = new BoMesh*[nodes];
 mMatrices = new BoMatrix*[nodes];
 mNodeCount = nodes; // unused?
 for (int i = 0; i < nodes; i++) {
	mMeshes[i] = 0;
	mMatrices[i] = new BoMatrix;
 }
}

void BoFrame::setMesh(unsigned int index, BoMesh* mesh)
{
 if (index >= mNodeCount) {
	boError() << k_funcinfo << "invalid mesh " << index << " , count=" << mNodeCount << endl;
	return;
 }
 BO_CHECK_NULL_RET(mesh);
 BO_CHECK_NULL_RET(mMeshes);
 // we store the *pointer* only!
 mMeshes[index] = mesh;
}

BoMatrix* BoFrame::matrix(int index) const
{
 return mMatrices[index];
}

void BoFrame::renderFrame(const QColor* teamColor, int mode)
{
 for (unsigned int i = 0; i < mNodeCount; i++) {
	BoMatrix* m = mMatrices[i];
	BoMesh* mesh = mMeshes[i];
	if (!m) {
		boError() << k_funcinfo << "NULL matrix at " << i << endl;
		continue;
	}
	if (!mesh) {
		boError() << k_funcinfo << "NULL mesh at " << i << endl;
		continue;
	}
	if (mode == GL_SELECT) {
		glLoadName(i);
	}
	// TODO: either store (bool flags) or test if mMatrices[i] is identity
	//  matrix. If it is, we can avoid glMultMatrix() call
	mesh->renderMesh(m, teamColor);
 }
}



BoLOD::BoLOD()
{
 mMeshCount = 0;
 mMeshes = 0;
 mFrameCount = 0;
 mFrames = 0;
}

BoLOD::~BoLOD()
{
 for (unsigned int i = 0; i < mMeshCount; i++) {
	delete mMeshes[i];
 }
 for (unsigned int i = 0; i < mFrameCount; i++) {
	delete mFrames[i];
 }
 delete[] mMeshes;
 delete[] mFrames;
}

void BoLOD::allocateMeshes(unsigned int count)
{
 mMeshes = new BoMesh*[count];

 for (unsigned int i = 0; i < count; i++) {
	mMeshes[i] = new BoMesh;
 }

 mMeshCount = count;
}

void BoLOD::allocateFrames(unsigned int count)
{
 mFrames = new BoFrame*[count];

 for (unsigned int i = 0; i < count; i++) {
	mFrames[i] = new BoFrame;
 }

 mFrameCount = count;
}

unsigned int BoLOD::addFrames(unsigned int count)
{
 unsigned int total = mFrameCount + count;

 BoFrame** oldframes = mFrames;
 mFrames = new BoFrame*[total];

 // Copy old frame pointers
 unsigned int i;
 for (i = 0; i < mFrameCount; i++) {
	mFrames[i] = oldframes[i];
 }
 mFrameCount = total;
 delete[] oldframes;

 return i;
}

void BoLOD::setFrame(unsigned int i, BoFrame* f)
{
 mFrames[i] = f;
}

BoMesh* BoLOD::mesh(unsigned int i) const
{
 return mMeshes[i];
}




class BosonModelPrivate
{
public:
	BosonModelPrivate()
	{
		mLODs = 0;
		mLODDistances = 0;
		mLODCount = 0;
		mMaterials = 0;
		mMaterialCount = 0;
		mPointArraySize = 0;
		mPoints = 0;
	}

	BoLOD* mLODs;
	float* mLODDistances;
	unsigned int mLODCount;
	BoMaterial* mMaterials;
	unsigned int mMaterialCount;

	QIntDict<BosonAnimation> mAnimations;
	QString mDirectory;
	QString mFile;

	unsigned int mPointArraySize;
	float* mPoints;

	float mBoundingSphereRadius;

	QCString mMD5;
};

BosonModel::BosonModel(const QString& dir, const QString& file)
{
 init();
 d->mDirectory = dir;
 d->mFile = file;
}

void BosonModel::init()
{
 d = new BosonModelPrivate;
 d->mAnimations.setAutoDelete(true);
 d->mBoundingSphereRadius = 0.0f;
 mMeshRendererModelData = 0;

 // add the default mode 0 (always shows the first frame)
 insertAnimationMode(0, 0, 0, 0.0f, false);
}

BosonModel::~BosonModel()
{
 boDebug(100) << k_funcinfo << endl;
 BoMeshRendererManager::manager()->removeModel(this);
 BosonModelTextures::modelTextures()->removeModel(this);
 if (mMeshRendererModelData) {
	boWarning(100) << k_funcinfo << "meshrenderer forgot to delete model data" << endl;
 }
 delete mMeshRendererModelData;

 boDebug(100) << k_funcinfo << "delete " << d->mLODCount << " lods" << endl;
 delete[] d->mLODs;
 delete[] d->mLODDistances;
 d->mAnimations.clear();
 boDebug(100) << k_funcinfo << "delete " << d->mMaterialCount << " materials" << endl;
 delete[] d->mMaterials;
 delete[] d->mPoints;
 delete d;
 boDebug(100) << k_funcinfo << "done" << endl;
}

BoMaterial* BosonModel::material(unsigned int index) const
{
 return &d->mMaterials[index];
}

unsigned int BosonModel::materialCount() const
{
 return d->mMaterialCount;
}

void BosonModel::allocateMaterials(unsigned int count)
{
 if (d->mMaterials) {
	boWarning() << k_funcinfo << "Materials already allocated!" << endl;
	delete[] d->mMaterials;
 }

 d->mMaterials = new BoMaterial[count];
 d->mMaterialCount = count;
}

const QString& BosonModel::baseDirectory() const
{
 return d->mDirectory;
}

QString BosonModel::file() const
{
 return baseDirectory() + d->mFile;
}
void BosonModel::loadTextures(const QStringList& list)
{
 QStringList::ConstIterator it = list.begin();
 for (; it != list.end(); ++it) {
	BosonModelTextures::modelTextures()->insert(this, *it);
 }
}

void BosonModel::loadModel(const QString& configfilename)
{
 if (d->mFile.isEmpty() || d->mDirectory.isEmpty()) {
	boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
	return;
 }
 BosonProfiler profiler(BosonProfiling::LoadModel);

 // Calculate MD5 hash of the original model file and it's config file
 QString fileName = d->mDirectory + d->mFile;
 QFile modelfile(fileName);
 if (!modelfile.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "could not open model file " << fileName << endl;
	return;
 }
 QFile configfile(configfilename);
 if (!configfile.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "could not open config file " << configfilename << endl;
	return;
 }
 KMD5 md5(modelfile.readAll());
 md5.update(configfile.readAll());
 d->mMD5 = md5.hexDigest();

 // Get filename of the cached model.
 // This converts the original model file if cached model file doesn't exist.
 QString cachedmodel = cachedModelFilename(d->mMD5, fileName, configfilename);
 if (cachedmodel.isNull()) {
	// Conversion failed
	return;
 }


 // Load the model
 BoBMFLoad loader(cachedmodel, this);
 if (!loader.loadModel()) {
	// TODO: add a profiling entry for this
	boError() << k_funcinfo << "error while loading from .bmf file " << cachedmodel << endl;
	return;
 }

 if (lodCount() == 0) {
	boError() << k_funcinfo << "0 lods loaded for model " << cachedmodel << endl;
	return;
 }

 // Load the textures
 QStringList modelTextures;
 for (unsigned int i = 0; i < materialCount(); i++) {
	BoMaterial* mat = material(i);
	if (!mat) {
		BO_NULL_ERROR(mat);
		continue;
	}
	modelTextures.append(mat->textureName());
 }

 boProfiling->start(BosonProfiling::LoadModelTextures);
 loadTextures(modelTextures);
 boProfiling->stop(BosonProfiling::LoadModelTextures);

 if (!BoContext::currentContext()) {
	boError(100) << k_funcinfo << "NULL current context" << endl;
	return;
 }

 // Set texture objects for materials
 for (unsigned int i = 0; i < materialCount(); i++) {
	BoMaterial* mat = material(i);
	if (!mat) {
		BO_NULL_ERROR(mat);
		continue;
	}
	BoTexture* myTex = 0;
	if (!mat->textureName().isEmpty()) {
		myTex = BosonModelTextures::modelTextures()->texture(mat->textureName());
	}
	mat->setTextureObject(myTex);
 }

 // must happen when the model has been loaded completely
 boDebug(100) << k_funcinfo << "adding model to meshrenderer" << endl;
 BoMeshRendererManager::manager()->addModel(this);

 boDebug(100) << k_funcinfo << "loaded from " << fileName << "(cached file: " << cachedmodel << ")" << endl;
}

QString BosonModel::cachedModelFilename(const QCString& md5, const QString& originalmodel, const QString& configfile)
{
 QString cachedmodel = KGlobal::dirs()->findResource("data", QString("%1/model-%2.bmf").arg("boson/modelcache").arg(md5));

 if (!cachedmodel) {
	// Cached model wasn't found
	// Convert the model now.
	// Get the path where the cached model can be saved
	cachedmodel = KGlobal::dirs()->saveLocation("data", "boson/modelcache/");
	if (cachedmodel.isNull()) {
		boError() << k_funcinfo << "Failed to get save location for cached model" << endl;
		return QString::null;
	}
	cachedmodel += QString("model-%1.bmf").arg(md5);
	// Find path to bobmfconverter binary
	QString converter = KGlobal::dirs()->findResource("exe", "bobmfconverter");
	if (converter.isNull()) {
		converter = KGlobal::dirs()->findExe("bobmfconverter");
		if (converter.isNull()) {
			boError() << k_funcinfo << "Couldn't find bobmfconverter!" << endl;
			return QString::null;
		}
	}
	// Create KProcess object
	KProcess proc;
	proc << converter;
	// Add default cmdline args
	proc << "-lods" << "5" <<  "-keepframes" <<  "-texnametolower" <<  "-useboth";
	proc << "-o" << cachedmodel;
	if (!configfile.isNull()) {
		proc << "-c" << configfile;
	}
	proc << originalmodel;
	proc << "-comment" << QString("Automatically converted from file '%1'").arg(originalmodel);

	// FIXME: KProcess:Block ain't pretty here...
	if (!proc.start(KProcess::Block)) {
		boError() << k_funcinfo << "Error while trying to convert the model" << endl;
		return QString::null;
	}
 }

 return cachedmodel;
}

void BosonModel::generateConstructionAnimation(unsigned int steps)
{
 if (d->mAnimations[UnitAnimationConstruction]) {
	// Construction animation is already there. Probably it was speicifed in unit
	//  config file.
	return;
 }

 if (steps == 0) {
	// is that ok? maybe create empty animation or...?
	return;
 }

 unsigned int animstart = 0;
 for (unsigned int i = 0; i < lodCount(); i++) {
	BoLOD* l = lod(i);
	// Find the base frame (1st frame)
	BoFrame* base = l->frame(0);
	// Allocate extra frames
	unsigned int offset = l->addFrames(steps);
	if (animstart != 0 && animstart != offset) {
		boError() << k_funcinfo << "Animation start mismatch: animstart: " << animstart <<
				"; offset: " << offset << endl;
	}
	animstart = offset;

	// Make construction animation for that lod
	for (unsigned int j = 0; j < steps; j++) {
		// Create a frame (copy of base frame)
		BoFrame* f = new BoFrame(*base, (unsigned int)0, base->nodeCount());
		// Calculate the dist to move. The first frame will be moved by
		//  2*boundingSphereRadius(), the last one by 0
		float dist = (1.5 * boundingSphereRadius()) * ((steps-j) / (float)steps);
		// Move all nodes in the frame downwards
		BoMatrix m;
		m.translate(0, 0, -dist);
		for (unsigned int n = 0; n < f->nodeCount(); n++)
		{
			BoMatrix m2(m);
			m2.multiply(f->matrix(n));
			f->matrix(n)->loadMatrix(m2);
		}
		// Add the frame to lod
		l->setFrame(offset + j, f);
	}
 }

 // Add construction animation
 insertAnimationMode(UnitAnimationConstruction, animstart, animstart + steps - 1, 1 / 20.0f, true);
}

void BosonModel::insertAnimationMode(int mode, unsigned int start, unsigned int end, float speed, bool loop)
{
 if (mode == 0) {
	// mode == 0 is a special mode. we default to it when everything fails,
	// so this *must* be valid.
	if (end < start || speed < 0.0f) {
		boWarning(100) << k_funcinfo << "invalid values for default mode! start=" << start << ",end=" << end << ",speed=" << speed << endl;
		start = 0;
		end = 0;
		speed = 0.0f;
	}
	if (d->mAnimations[0]) {
		// default mode already there - replace it!
		d->mAnimations.remove(0);
	}
 } else {
	if (end < start || speed < 0.0f) {
		boWarning(100) << k_funcinfo << "invalid values for animation " << mode << "! start=" << start << ",end=" << end << ",speed=" << speed << endl;
		return;
	}
 }
 BosonAnimation* anim = new BosonAnimation(start, end, speed, loop);
 d->mAnimations.insert(mode, anim);
}

void BosonModel::loadAnimationMode(int mode, KSimpleConfig* conf, const QString& name)
{
 unsigned int start = 0;
 unsigned int end = 0;
 float speed = 1.0f;
 bool loop = true;

 // Base name of the keys of this animation
 QString basekey = QString("Animation-%1-").arg(name);

 // Check if animation with this name is in config file. There has to be at
 //  least -Start key
 if (!conf->hasKey(basekey + "Start")) {
	return;
 }

 start = conf->readUnsignedNumEntry(basekey + "Start", start);
 end = conf->readUnsignedNumEntry(basekey + "End", end);
 speed = (float)(conf->readDoubleNumEntry(basekey + "Speed", speed));
 loop = conf->readBoolEntry(basekey + "Loop", loop);

 insertAnimationMode(mode, start, end, speed, loop);
}

BosonAnimation* BosonModel::animation(int mode) const
{
 return d->mAnimations[mode];
}

float* BosonModel::pointArray() const
{
 return d->mPoints;
}

unsigned int BosonModel::pointArraySize() const
{
 return d->mPointArraySize;
}

void BosonModel::allocatePointArray(unsigned int size)
{
 if (d->mPoints) {
	boWarning() << k_funcinfo << "Point array already allocated!" << endl;
	delete[] d->mPoints;
 }
 d->mPoints = new float[size * BoMesh::pointSize()];
 d->mPointArraySize = size;

 static int usedpoints = 0;

 usedpoints += size;
 boDebug() << k_funcinfo << usedpoints << " points are used now, taking " <<
		(usedpoints * BoMesh::pointSize() * sizeof(float)) / 1024 << "kb" << endl;
}

void BosonModel::prepareRendering()
{
 BoMeshRendererManager* manager = BoMeshRendererManager::manager();
 if (!manager->checkCurrentRenderer()) {
	boError() << k_funcinfo << "unable to load a renderer" << endl;
	return;
 }
 if (manager->currentRenderer()) {
	// AB: maybe we add a setModel() to the manager, then we can avoid
	// #including the renderer file.
	manager->currentRenderer()->setModel(this);
 }
}

void BosonModel::allocateLODs(unsigned int count)
{
 if (d->mLODs) {
	boWarning() << k_funcinfo << "LODs already allocated!" << endl;
	delete[] d->mLODs;
	delete[] d->mLODDistances;
 }

 d->mLODs = new BoLOD[count];
 d->mLODDistances = new float[count];
 d->mLODCount = count;

 // Calculate initial distances for lods
 float dist = 0.0f;
 for (unsigned int i = 0; i < count; i++) {
	setLodDistance(i, dist);
	// dists will be 0, 10, 25, 45, 70, ...
	dist = dist + (10 * (0.5 + i * 0.5));
 }
}

unsigned int BosonModel::lodCount() const
{
 return d->mLODCount;
}

BoLOD* BosonModel::lod(unsigned int index) const
{
 return &d->mLODs[index];
}

float BosonModel::lodDistance(unsigned int index) const
{
 return d->mLODDistances[index];
}

void BosonModel::setLodDistance(unsigned int index, float distance) const
{
 d->mLODDistances[index] = distance;
}


unsigned int BosonModel::preferredLod(float dist) const
{
 // For each lod, minimum distance of the model from the camera is stored
 for (unsigned int lod = lodCount() - 1; lod > 0; lod--) {
	if (dist >= lodDistance(lod)) {
		// This is the correct lod
		return lod;
	}
 }

 // None of the reduce-detail lods were fitting. Use the full-detail one
 return 0;
}

void BosonModel::setMeshRendererModelData(BoMeshRendererModelData* data)
{
 delete mMeshRendererModelData;
 mMeshRendererModelData = data;
}

void BosonModel::startModelRendering()
{
 if (!BoMeshRendererManager::checkCurrentRenderer()) {
	boError() << k_funcinfo << "unable to load a renderer" << endl;
	return;
 }
 BoMeshRenderer* renderer = BoMeshRendererManager::manager()->currentRenderer();
 BO_CHECK_NULL_RET(renderer);
 renderer->startModelRendering();
}

void BosonModel::stopModelRendering()
{
 BoMeshRenderer* renderer = BoMeshRendererManager::manager()->currentRenderer();
 BO_CHECK_NULL_RET(renderer);
 renderer->stopModelRendering();
}

float BosonModel::boundingSphereRadius() const
{
 return d->mBoundingSphereRadius;
}

void BosonModel::setBoundingSphereRadius(float r)
{
 d->mBoundingSphereRadius = r;
}

