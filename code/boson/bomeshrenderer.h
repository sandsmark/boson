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
#ifndef BOMESHRENDERER_H
#define BOMESHRENDERER_H

#include <qobject.h>

class BosonModel;
class BoMesh;
class BoMeshLOD;
class BoMeshRendererModelData;
class BoMeshRendererMeshData;
class BoMeshRendererMeshLODData;

/**
 * The meshrenderer takes care of rendering all meshes of a model (and therefore
 * basically the whole model). There can be multiple meshrenderer classes
 * available but only a single object can be the current meshrenderer, which is
 * used for all models.
 *
 * Using a meshrenderer is very easy:
 * <pre>
 * renderer->startModelRendering();
 * while (model) {
 *   renderer->setModel(model);
 *   while (mesh) {
 *     renderer->renderMesh(teamColor, mesh, lod_number);
 *   }
 * }
 * renderer->stopModelRendering();
 * </pre>
 *
 * Thats all. @ref startModelRendering sets the necessary OpenGL states, @ref
 * stopModelRendering takes care of un-setting these states again. @ref setModel
 * is used to tell the meshrenderer that we want to start rendering a new model
 * and @ref renderMesh renders it.
 *
 * The interesting part of the meshrenderer comes into the game when you want to
 * write your own renderer - you can change the way models are rendered on the
 * fly, without ending the game then.
 *
 * There are already different meshrenderers implemented - i propose you have a
 * look at these examples.
 * Often you will have to overwrite @ref createModelData, so that your
 * meshrenderer can store it's data in the model. Similar methods exist for @ref
 * BoMesh and @ref BoMeshLOD. @ref initModelData is used to initialize this data
 * object (i.e. fill it with content). But you do <em>not</em> have to use that
 * at all!
 **/
class BoMeshRenderer : public QObject // must be QObject to simplify lib loading
{
	Q_OBJECT
public:
	BoMeshRenderer();
	virtual ~BoMeshRenderer();

	/**
	 * @return The currently set model (NULL is valid!)
	 **/
	inline BosonModel* model() const
	{
		return mModel;
	}

	/**
	 * This is called by @ref BoMeshRendererManager, you should not call
	 * it yourself.
	 * @param model A fully loaded (!) model.
	 **/
	void initializeData(BosonModel* model);

	/**
	 * This is called by @ref BoMeshRendererManager, you should not call
	 * it yourself.
	 **/
	void deinitializeData(BosonModel* model);

	/**
	 * Called (exactly once) before model rendering starts. Here you can
	 * e.g. enable certain OpenGL states, that you require.
	 *
	 * Remember to disable all enabled states in @ref stopModelRendering.
	 * You can use e.g. glPushAttrib() / glPopAttrib() for that!
	 **/
	virtual void startModelRendering() = 0;
	virtual void stopModelRendering() = 0;

	/**
	 * Set the current model. All meshes used in @ref render must belong to
	 * this model.
	 *
	 * A NULL model is valid for resetting data of the renderer.
	 **/
	virtual void setModel(BosonModel* model);

	/**
	 * @param mesh The mesh MUST belong to the model that has been set by
	 * @ref setModel
	 **/
	void renderMesh(const QColor* teamColor, BoMesh* mesh, unsigned int lod);

protected:
	virtual void render(const QColor* teamColor, BoMesh* mesh, BoMeshLOD* lod) = 0;

	/**
	 * Called by @ref initializeData before @ref initModelData is called. The
	 * returned object is assigned to the model that is being initialized
	 * (it also is the current @ref model).
	 *
	 * Note that ownership is taken, i.e. you don't have to delete this!
	 **/
	virtual BoMeshRendererModelData* createModelData() const;
	virtual BoMeshRendererMeshData* createMeshData() const;
	virtual BoMeshRendererMeshLODData* createMeshLODData() const;

	/**
	 * Initialize the meshrenderer data of @p model for use with this
	 * meshrenderer. The meshrenderer data is e.g. @ref
	 * BosonModel::meshRendererModelData.
	 *
	 * That data is specific to the current renderer only (e.g. vertex
	 * buffer objects for a vbo renderer or vertex arrays for a vertex array
	 * renderer).
	 *
	 * Note that at the point of this being called <em>all</em> @ref
	 * BosonModel::meshRendererModelData, @ref BoMesh::meshRendererMeshData
	 * and @ref BoMeshLOD::meshRendererMeshLODData objects of @p model are
	 * already valid!
	 **/
	virtual void initModelData(BosonModel* model);

	/**
	 * Initialize the meshrenderer data for @p mesh. The data is used by
	 * this renderer only and deleted once the renderer is changed.
	 *
	 * See @ref BoMeshRendererMeshData
	 * @param meshIndex The index of the @p mesh in the @ref BosonModel
	 * file. This parameter is for convenience.
	 *
	 * Note that at the point of this being called <em>all</em> @ref
	 * BosonModel::meshRendererModelData, @ref BoMesh::meshRendererMeshData
	 * and @ref BoMeshLOD::meshRendererMeshLODData objects of the model are
	 * already valid!
	 **/
	virtual void initMeshData(BoMesh* mesh, unsigned int meshIndex);

	/**
	 * Initialize the meshrenderer data for @p meshLOD. The data is used by
	 * this renderer only and deleted once the renderer is changed.
	 *
	 * See @ref BoMeshRendererMeshLODData
	 * @param meshIndex The index of the @ref BoMesh the @p lod belongs to 
	 * in the @ref BosonModel file. This parameter is for convenience.
	 * @param lod The level of detail number (i.e. the one given to @ref
	 * BoMesh::levelOfDetail) of the @p meshLOD. This parameter is for
	 * your convenience.
	 *
	 * Note that at the point of this being called <em>all</em> @ref
	 * BosonModel::meshRendererModelData, @ref BoMesh::meshRendererMeshData
	 * and @ref BoMeshLOD::meshRendererMeshLODData objects of the model are
	 * already valid!
	 **/
	virtual void initMeshLODData(BoMeshLOD* meshLOD, unsigned int meshIndex, unsigned int lod);

	/**
	 * Here you should delete all data that you allocated in @ref
	 * initModelData. The @ref BosonModel::setMeshRendererModelData is
	 * deleted automatically, you don't have to care about it.
	 *
	 * So usually this method will do nothing.
	 **/
	virtual void deinitModelData(BosonModel* model);
	virtual void deinitMeshData(BoMesh* mesh);
	virtual void deinitMeshLODData(BoMeshLOD* meshLOD);

	unsigned int lodCount(const BoMesh* mesh) const;
	BoMeshLOD* levelOfDetail(const BoMesh* mesh, unsigned int lod) const;

private:
	BosonModel* mModel;
};

/**
 * A simple class providing information about what classes a plugin provides.
 * You need to derive from this class for your meshrenderer plugin and list all
 * meshrenderer classnames in @ref meshRenderers.
 *
 * Note that the name of your derived class MUST be
 * BoMeshRendererInformation_plugin, where "plugin" is the name of your library
 * file (e.g. "libbomeshrendererplugin")
 **/
class BoMeshRendererInformation : public QObject
{
	Q_OBJECT
public:
	BoMeshRendererInformation() : QObject()
	{
	}
	~BoMeshRendererInformation()
	{
	}
	virtual QStringList meshRenderers() const = 0;
};

#endif
