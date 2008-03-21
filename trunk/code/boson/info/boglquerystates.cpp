/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann <b_mann@gmx.de>

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

#include "boglquerystates.h"

#include "../../bomemory/bodummymemory.h"
#include "boinfo.h"
#include <bogl.h>
#include <bodebug.h>

#include <klocale.h>

#include <qstringlist.h>
#include <qmap.h>
#include <qlistview.h>

#include <stdlib.h>

/**
 * AB: these defines are evil, but they are very useful to reduce lines of code
 * and probability of typos.
 **/
#define BO_VARNAME(x) m_##x
#define BO_VAR(type, x, size) type m_##x [size];

// AB: we are using int instead of GLboolean, we use (partially) BOboolean to
// make clear that this int describes a boolean.
#define BOboolean GLint

class GLGetValue
{
public:
    // AB: you are meant to use GLint instead of GLboolean!
    static QString get(GLenum e, GLint* v, int size)
    {
        glGetIntegerv(e, v);
        return makeValue(v, size);
    }
    static QString get(GLenum e, GLfloat* v, int size)
    {
        glGetFloatv(e, v);
        return makeValue(v, size);
    }
    static QString get(GLenum e, void** v, int size)
    {
        glGetPointerv(e, v);
        return makeValue(v, size);
    }

    static QString makeValue(GLint* v, int size)
    {
        if (size == 1) {
            return QString::number(*v);
        }
        QString s = "(";
        for (int i = 0; i < size - 1; i++) {
            s += QString::number(v[i]) + ",";
        }
        s += QString::number(v[size - 1]) + ")";
        return s;
    }
    static QString makeValue(GLfloat* v, int size) // size is the size of the array
    {
        if (size == 1) {
            return QString::number(*v);
        }
        QString s = "(";
        for (int i = 0; i < size - 1; i++) {
            s += QString::number(v[i]) + ",";
        }
        s += QString::number(v[size - 1]) + ")";
        return s;
    }
    static QString makeValue(void** v, int size)
    {
        QString s;
        if (size == 1) {
            s.sprintf("%p", *v);
            return s;
        }
        s = "(";
        for (int i = 0; i < size - 1; i++) {
            QString t;
            t.sprintf("%p", v[i]);
            s += t + ",";
        }
        QString t;
        t.sprintf("%p", v[size - 1]);
        s += t + ")";
        return s;
    }
};

/**
 * This class contains all OpenGL values that can be retrieved using
 * glGetIntegerv() and friends and can't be changed on runtime using OpenGL
 * commands.
 **/
class ImplementationValues
{
public:
    ImplementationValues()
    {
        m_GL_EXTENSIONS = (const GLubyte*)"";
        m_GL_RENDERER = (const GLubyte*)"";
        m_GL_VENDOR = (const GLubyte*)"";
        m_GL_VERSION = (const GLubyte*)"";
        m_GL_SHADING_LANGUAGE_VERSION = (const GLubyte*)"";

        mGLVersion = boglGetOpenGLVersion();
        mGLExtensions = boglGetOpenGLExtensions();

        addOpenGL1_1Values();
        addOpenGL1_2Values();
        addOpenGL1_2_1Values();
        addOpenGL1_3Values();
        addOpenGL1_4Values();
        addOpenGL1_5Values();
        addExtensionsValues();
    }

    void getValues()
    {
        getValuesOpenGL1_1();
        getValuesOpenGL1_2();
        getValuesOpenGL1_2_1();
        getValuesOpenGL1_3();
        getValuesOpenGL1_4();
        getValuesOpenGL1_5();
        getValuesOpenGL2_0();
        getValuesExtensions();
    }
    QStringList list() const
    {
        QStringList list;
        QValueList<int> keys = mNameDict.keys();
        for (unsigned int i = 0; i < keys.count(); i++) {
            list.append(QString("%1 = %2").arg(mNameDict[keys[i]]).arg(value(keys[i])));
        }
        return list;
    }

private:
    QString value(GLenum e) const
    {
        return mValues[(int)e];
    }

    // AB: do NOT make public!
    // -> only variables that are actually being updated in a getValues*()
    //    method are allowed to be used.
    //    other variables may be declared, but not provided by the current
    //    OpenGL implementation (e.g. version too old or extension missing).
    //    => if they were used at some point, it would be an invalid read on an
    //    uninitialized variable.
private:
    // OpenGL 1.1
    BO_VAR(GLint, GL_ACCUM_ALPHA_BITS, 1)
    BO_VAR(GLint, GL_ACCUM_BLUE_BITS, 1)
    BO_VAR(GLint, GL_ACCUM_GREEN_BITS, 1)
    BO_VAR(GLint, GL_ACCUM_RED_BITS, 1)
    BO_VAR(GLint, GL_ALPHA_BITS, 1)
    BO_VAR(GLint, GL_AUX_BUFFERS, 1)
    BO_VAR(GLint, GL_BLUE_BITS, 1)
    BO_VAR(GLint, GL_DEPTH_BITS, 1)
    BO_VAR(GLint, GL_DOUBLEBUFFER, 1)
    BO_VAR(GLint, GL_GREEN_BITS, 1)
    BO_VAR(GLint, GL_INDEX_BITS, 1)
    BO_VAR(GLint, GL_INDEX_MODE, 1)
    BO_VAR(GLint, GL_MAX_ATTRIB_STACK_DEPTH, 1)
    BO_VAR(GLint, GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, 1)
    BO_VAR(GLint, GL_MAX_CLIP_PLANES, 1)
    BO_VAR(GLint, GL_MAX_EVAL_ORDER, 1)
    BO_VAR(GLint, GL_MAX_LIGHTS, 1)
    BO_VAR(GLint, GL_MAX_LIST_NESTING, 1)
    BO_VAR(GLint, GL_MAX_MODELVIEW_STACK_DEPTH, 1)
    BO_VAR(GLint, GL_MAX_NAME_STACK_DEPTH, 1)
    BO_VAR(GLint, GL_MAX_PIXEL_MAP_TABLE, 1)
    BO_VAR(GLint, GL_MAX_PROJECTION_STACK_DEPTH, 1)
    BO_VAR(GLint, GL_MAX_TEXTURE_SIZE, 1)
    BO_VAR(GLint, GL_MAX_TEXTURE_STACK_DEPTH, 1)
    BO_VAR(GLint, GL_MAX_VIEWPORT_DIMS, 1)
    BO_VAR(GLint, GL_RED_BITS, 1)
    BO_VAR(GLint, GL_RGBA_MODE, 1)
    BO_VAR(GLint, GL_STENCIL_BITS, 1)
    BO_VAR(GLint, GL_STEREO, 1)
    BO_VAR(GLint, GL_SUBPIXEL_BITS, 1)
//    BO_VAR(GLfloat, GL_LINE_WIDTH_GRANULARITY, 1) // deprecated by OpenGL 1.2
//    BO_VAR(GLfloat, GL_LINE_WIDTH_RANGE, 1) // deprecated by OpenGL 1.2
//    BO_VAR(GLfloat, GL_POINT_SIZE_GRANULARITY, 1) // deprecated by OpenGL 1.2
//    BO_VAR(GLfloat, GL_POINT_SIZE_RANGE, 1) // deprecated by OpenGL 1.2

    // OpenGL 1.2
    BO_VAR(GLint, GL_MAX_COLOR_MATRIX_STACK_DEPTH, 1); // extension "GL_ARB_imaging"
    BO_VAR(GLint, GL_MAX_CONVOLUTION_WIDTH, 3); // extension "GL_ARB_imaging"
    BO_VAR(GLint, GL_MAX_CONVOLUTION_HEIGHT, 2); // extension "GL_ARB_imaging"
    BO_VAR(GLfloat, GL_ALIASED_LINE_WIDTH_RANGE, 2)
    BO_VAR(GLfloat, GL_ALIASED_POINT_SIZE_RANGE, 2)
    BO_VAR(GLint, GL_MAX_3D_TEXTURE_SIZE, 1)
    BO_VAR(GLint, GL_MAX_ELEMENTS_INDICES, 1)
    BO_VAR(GLint, GL_MAX_ELEMENTS_VERTICES, 1)
    BO_VAR(GLfloat, GL_SMOOTH_LINE_WIDTH_GRANULARITY, 1) // replaces GL_LINE_WIDTH_GRANULARITY from OpenGL 1.1
    BO_VAR(GLfloat, GL_SMOOTH_LINE_WIDTH_RANGE, 2) // replaces GL_LINE_WIDTH_RANGE from OpenGL 1.1
    BO_VAR(GLfloat, GL_SMOOTH_POINT_SIZE_GRANULARITY, 1) // replaces GL_POINT_SIZE_GRANULARITY from OpenGL 1.1
    BO_VAR(GLfloat, GL_SMOOTH_POINT_SIZE_RANGE, 2) // replaces GL_POINT_SIZE_RANGE from OpenGL 1.1

    // OpenGL 1.2.1
//    BO_VAR(GLint, GL_MAX_TEXTURE_UNITS_ARB, 1) // "GL_MAX_TEXTURE_UNITS" -> OpenGL 1.3

    // OpenGL 1.3
    BO_VAR(GLint, GL_MAX_CUBE_MAP_TEXTURE_SIZE, 1);
    BO_VAR(GLint, GL_MAX_TEXTURE_UNITS, 1)
    BO_VAR(GLint, GL_SAMPLE_BUFFERS, 1);
    BO_VAR(GLint, GL_SAMPLES, 1);
//    BO_VAR(GL_COMPRESSED_TEXTURE_FORMATS); // TODO
    BO_VAR(GLint, GL_NUM_COMPRESSED_TEXTURE_FORMATS, 1);


    // OpenGL 1.4
    BO_VAR(GLfloat, GL_MAX_TEXTURE_LOD_BIAS, 1);

    // OpenGL 1.5
    BO_VAR(GLint, GL_QUERY_COUNTER_BITS, 1);

    // OpenGL 2.0
    const GLubyte* m_GL_EXTENSIONS;
    BO_VAR(GLint, GL_MAX_DRAW_BUFFERS, 1);
    BO_VAR(GLint, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 1);
    BO_VAR(GLint, GL_MAX_FRAGMENT_COMPONENTS, 1);
    BO_VAR(GLint, GL_MAX_TEXTURE_COORDS, 1);
    BO_VAR(GLint, GL_MAX_VARYING_FLOATS, 1);
    BO_VAR(GLint, GL_MAX_VERTEX_ATTRIBS, 1);
    BO_VAR(GLint, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 1);
    BO_VAR(GLint, GL_MAX_VERTEX_UNIFORM_COMPONENTS, 1);
    const GLubyte* m_GL_RENDERER;
    const GLubyte* m_GL_SHADING_LANGUAGE_VERSION;
    const GLubyte* m_GL_VENDOR;
    const GLubyte* m_GL_VERSION;


    // Extensions
    BO_VAR(GLint, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 1); // GL_EXT_texture_filter_anisotropic

private:
    void addOpenGL1_1Values()
    {
#define ADD(a) mNameDict.insert(a, #a);
        ADD(GL_ACCUM_ALPHA_BITS);
        ADD(GL_ACCUM_BLUE_BITS);
        ADD(GL_ACCUM_GREEN_BITS);
        ADD(GL_ACCUM_RED_BITS);
        ADD(GL_ALPHA_BITS);
        ADD(GL_AUX_BUFFERS);
        ADD(GL_BLUE_BITS);
        ADD(GL_DEPTH_BITS);
        ADD(GL_DOUBLEBUFFER);
        ADD(GL_GREEN_BITS);
        ADD(GL_INDEX_BITS);
        ADD(GL_INDEX_MODE);
        ADD(GL_MAX_ATTRIB_STACK_DEPTH);
        ADD(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
        ADD(GL_MAX_CLIP_PLANES);
        ADD(GL_MAX_EVAL_ORDER);
        ADD(GL_MAX_LIGHTS);
        ADD(GL_MAX_LIST_NESTING);
        ADD(GL_MAX_MODELVIEW_STACK_DEPTH);
        ADD(GL_MAX_NAME_STACK_DEPTH);
        ADD(GL_MAX_PIXEL_MAP_TABLE);
        ADD(GL_MAX_PROJECTION_STACK_DEPTH);
        ADD(GL_MAX_TEXTURE_SIZE);
        ADD(GL_MAX_TEXTURE_STACK_DEPTH);
        ADD(GL_MAX_VIEWPORT_DIMS);
        ADD(GL_RED_BITS);
        ADD(GL_RGBA_MODE);
        ADD(GL_STENCIL_BITS);
        ADD(GL_STEREO);
        ADD(GL_SUBPIXEL_BITS);
//        ADD(GL_POINT_SIZE_RANGE); // deprecated by OpenGL 1.2
//        ADD(GL_POINT_SIZE_GRANULARITY); // deprecated by OpenGL 1.2
//        ADD(GL_LINE_WIDTH_RANGE); // deprecated by OpenGL 1.2
//        ADD(GL_LINE_WIDTH_GRANULARITY); // deprecated by OpenGL 1.2
#undef ADD
    }
    void addOpenGL1_2Values()
    {
#define ADD(a) mNameDict.insert(a, #a);
        ADD(GL_MAX_COLOR_MATRIX_STACK_DEPTH); // extension "GL_ARB_imaging"
        ADD(GL_MAX_CONVOLUTION_WIDTH); // extension "GL_ARB_imaging"
        ADD(GL_MAX_CONVOLUTION_HEIGHT); // extension "GL_ARB_imaging"

        ADD(GL_ALIASED_LINE_WIDTH_RANGE);
        ADD(GL_ALIASED_POINT_SIZE_RANGE);
        ADD(GL_MAX_3D_TEXTURE_SIZE);
        ADD(GL_MAX_ELEMENTS_INDICES);
        ADD(GL_MAX_ELEMENTS_VERTICES);
        ADD(GL_SMOOTH_LINE_WIDTH_GRANULARITY); // replaces GL_LINE_WIDTH_GRANULARITY from OpenGL 1.1
        ADD(GL_SMOOTH_LINE_WIDTH_RANGE); // replaces GL_LINE_WIDTH_RANGE from OpenGL 1.1
        ADD(GL_SMOOTH_POINT_SIZE_GRANULARITY); // replaces GL_POINT_SIZE_GRANULARITY from OpenGL 1.1
        ADD(GL_SMOOTH_POINT_SIZE_RANGE); // replaces GL_POINT_SIZE_RANGE from OpenGL 1.1
#undef ADD
    }
    void addOpenGL1_2_1Values()
    {
#define ADD(a) mNameDict.insert(a, #a);
        ADD(GL_MAX_TEXTURE_UNITS_ARB); // extension "GL_ARB_multitexture"
#undef ADD
    }
    void addOpenGL1_3Values()
    {
#define ADD(a) mNameDict.insert(a, #a);
        ADD(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
        ADD(GL_MAX_TEXTURE_UNITS);
        ADD(GL_SAMPLE_BUFFERS);
        ADD(GL_SAMPLES);
        ADD(GL_COMPRESSED_TEXTURE_FORMATS);
        ADD(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
#undef ADD

    }

    void addOpenGL1_4Values()
    {
#ifdef GL_VERSION_1_4
#define ADD(a) mNameDict.insert(a, #a);
        ADD(GL_MAX_TEXTURE_LOD_BIAS);
#undef ADD
#endif
    }
    void addOpenGL1_5Values()
    {
#ifdef GL_VERSION_1_5
#define ADD(a) mNameDict.insert(a, #a);
        ADD(GL_QUERY_COUNTER_BITS);
#undef ADD
#endif
    }
    void addOpenGL2_0Values()
    {
#ifdef GL_VERSION_2_0
#define ADD(a) mNameDict.insert(a, #a);
        ADD(GL_EXTENSIONS);
        ADD(GL_RENDERER);
        ADD(GL_SHADING_LANGUAGE_VERSION);
        ADD(GL_VENDOR);
        ADD(GL_VERSION);
        ADD(GL_MAX_VERTEX_ATTRIBS);
        ADD(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
        ADD(GL_MAX_VARYING_FLOATS);
        ADD(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
        ADD(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
        ADD(GL_MAX_TEXTURE_COORDS);
        ADD(GL_MAX_FRAGMENT_COMPONENTS);
        ADD(GL_MAX_DRAW_BUFFERS);
#undef ADD
#endif
    }

    void addExtensionsValues()
    {
#define ADD(a) mNameDict.insert(a, #a);
        ADD(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
#undef ADD
    }


    void getValuesOpenGL1_1()
    {
        if (mGLVersion < MAKE_VERSION(1, 1, 0)) {
            return;
        }
#define UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        UPDATE(GL_ACCUM_ALPHA_BITS);
        UPDATE(GL_ACCUM_BLUE_BITS);
        UPDATE(GL_ACCUM_GREEN_BITS);
        UPDATE(GL_ACCUM_RED_BITS);
        UPDATE(GL_ALPHA_BITS);
        UPDATE(GL_AUX_BUFFERS);
        UPDATE(GL_BLUE_BITS);
        UPDATE(GL_DEPTH_BITS);
        UPDATE(GL_DOUBLEBUFFER);
        UPDATE(GL_GREEN_BITS);
        UPDATE(GL_INDEX_BITS);
        UPDATE(GL_INDEX_MODE);
        UPDATE(GL_MAX_ATTRIB_STACK_DEPTH);
        UPDATE(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
        UPDATE(GL_MAX_CLIP_PLANES);
        UPDATE(GL_MAX_EVAL_ORDER);
        UPDATE(GL_MAX_LIGHTS);
        UPDATE(GL_MAX_LIST_NESTING);
        UPDATE(GL_MAX_MODELVIEW_STACK_DEPTH);
        UPDATE(GL_MAX_NAME_STACK_DEPTH);
        UPDATE(GL_MAX_PIXEL_MAP_TABLE);
        UPDATE(GL_MAX_PROJECTION_STACK_DEPTH);
        UPDATE(GL_MAX_TEXTURE_SIZE);
        UPDATE(GL_MAX_TEXTURE_STACK_DEPTH);
        UPDATE(GL_MAX_VIEWPORT_DIMS);
        UPDATE(GL_RED_BITS);
        UPDATE(GL_RGBA_MODE);
        UPDATE(GL_STENCIL_BITS);
        UPDATE(GL_STEREO);
        UPDATE(GL_SUBPIXEL_BITS);

        // AB: we do not update them here, because Boson requires OpenGL > 1.1,
        //     i.e. we do have 1.2 anyway.
//        UPDATE(GL_LINE_WIDTH_GRANULARITY) // deprecated by OpenGL 1.2
//        UPDATE(GL_LINE_WIDTH_RANGE) // deprecated by OpenGL 1.2
//        UPDATE(GL_POINT_SIZE_GRANULARITY) // deprecated by OpenGL 1.2
//        UPDATE(GL_POINT_SIZE_RANGE) // deprecated by OpenGL 1.2
#undef UPDATE

        // AB: these are special cases:
        //     only in OpenGL 2.0 they are listed in the "implementation
        //     dependent" section, but they already exist before.
        m_GL_EXTENSIONS = glGetString(GL_EXTENSIONS);
        m_GL_RENDERER = glGetString(GL_RENDERER);
        m_GL_VENDOR = glGetString(GL_VENDOR);
        m_GL_VERSION = glGetString(GL_VERSION);
    }
    void getValuesOpenGL1_2()
    {
        if (mGLVersion < MAKE_VERSION(1, 2, 0)) {
            return;
        }
#define UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        if (mGLExtensions.contains("GL_ARB_imaging")) {
                UPDATE(GL_MAX_COLOR_MATRIX_STACK_DEPTH);
#if 0
                // TODO
                glGetConvolutionParameteriv(..., m_GL_MAX_CONVOLUTION_WIDTH);
                glGetConvolutionParameteriv(..., m_GL_MAX_CONVOLUTION_HEIGHT);
#endif
        }
        UPDATE(GL_ALIASED_LINE_WIDTH_RANGE);
        UPDATE(GL_ALIASED_POINT_SIZE_RANGE);
        UPDATE(GL_MAX_3D_TEXTURE_SIZE);
        UPDATE(GL_MAX_ELEMENTS_INDICES);
        UPDATE(GL_MAX_ELEMENTS_VERTICES);
        UPDATE(GL_SMOOTH_LINE_WIDTH_GRANULARITY);
        UPDATE(GL_SMOOTH_LINE_WIDTH_RANGE);
        UPDATE(GL_SMOOTH_POINT_SIZE_GRANULARITY);
        UPDATE(GL_SMOOTH_POINT_SIZE_RANGE);
#undef UPDATE
    }
    void getValuesOpenGL1_2_1()
    {
        if (mGLVersion < MAKE_VERSION(1, 2, 1)) {
            return;
        }
#define UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        // AB: actually the enum in 1.2.1 is GL_MAX_TEXTURE_UNITS_ARB, but since
        // both enums are defined to the same number, we can use the one from GL
        // 1.3, too.
        if (mGLExtensions.contains("GL_ARB_multitexture")) {
            UPDATE(GL_MAX_TEXTURE_UNITS);
        }
#undef UPDATE
    }
    void getValuesOpenGL1_3()
    {
        if (mGLVersion < MAKE_VERSION(1, 3, 0)) {
            return;
        }
#define UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        UPDATE(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
        UPDATE(GL_MAX_TEXTURE_UNITS);
        UPDATE(GL_SAMPLE_BUFFERS);
        UPDATE(GL_SAMPLES);
        UPDATE(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
//        UPDATE(GL_COMPRESSED_TEXTURE_FORMATS);
#undef UPDATE
    }

    void getValuesOpenGL1_4()
    {
        if (mGLVersion < MAKE_VERSION(1, 4, 0)) {
            return;
        }
#ifdef GL_VERSION_1_4
#define UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        UPDATE(GL_MAX_TEXTURE_LOD_BIAS);
#undef UPDATE
#endif
    }
    void getValuesOpenGL1_5()
    {
        if (mGLVersion < MAKE_VERSION(1, 5, 0)) {
            return;
        }
#ifdef GL_VERSION_1_5
#define UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        UPDATE(GL_QUERY_COUNTER_BITS);
#undef UPDATE
#endif
    }
    void getValuesOpenGL2_0()
    {
        if (mGLVersion < MAKE_VERSION(2, 0, 0)) {
            return;
        }
#ifdef GL_VERSION_2_0
#define UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        m_GL_EXTENSIONS = glGetString(GL_EXTENSIONS);
        m_GL_RENDERER = glGetString(GL_RENDERER);
        m_GL_SHADING_LANGUAGE_VERSION = glGetString(GL_SHADING_LANGUAGE_VERSION);
        m_GL_VENDOR = glGetString(GL_VENDOR);
        m_GL_VERSION = glGetString(GL_VERSION);
#undef UPDATE
#endif
    }
    void getValuesExtensions()
    {
#define UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        if (mGLExtensions.contains("GL_EXT_texture_filter_anisotropic")) {
            UPDATE(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
        }

#undef UPDATE
    }

private:
    QMap<int, QString> mNameDict;
    QMap<int, QString> mValues;

    unsigned int mGLVersion;
    QStringList mGLExtensions;
};

// class like below, but contains only glGetMaterialfv() calls
class MaterialStates
{
public:
    MaterialStates()
    {
    }

    void getMaterialStates(GLenum face)
    {
        mFace = face;

        glGetMaterialfv(face, GL_AMBIENT, mAmbient);
        glGetMaterialfv(face, GL_DIFFUSE, mDiffuse);
        glGetMaterialfv(face, GL_SPECULAR, mSpecular);
        glGetMaterialfv(face, GL_EMISSION, mEmission);
        glGetMaterialfv(face, GL_SHININESS, &mShininess);

        glGetMaterialfv(face, GL_COLOR_INDEXES, mColorIndexes);
    }
    GLenum mFace;

    GLfloat mAmbient[4];
    GLfloat mDiffuse[4];
    GLfloat mSpecular[4];
    GLfloat mEmission[4];
    GLfloat mShininess;

    GLfloat mColorIndexes[3];
};

// class like below, but contains only glGetLightfv() calls
class LightStates
{
public:
    LightStates()
    {
    }
    void getLightStates(GLint light)
    {
        if (light < 0) {
            boError() << k_funcinfo << light << endl;
            return;
        }
        mLight = light;
        glGetIntegerv(GL_LIGHT0 + mLight, &mLightEnabled);
        glGetLightfv(GL_LIGHT0 + mLight, GL_AMBIENT, mAmbient);
        glGetLightfv(GL_LIGHT0 + mLight, GL_DIFFUSE, mDiffuse);
        glGetLightfv(GL_LIGHT0 + mLight, GL_SPECULAR, mSpecular);
        glGetLightfv(GL_LIGHT0 + mLight, GL_POSITION, mPosition);
        glGetLightfv(GL_LIGHT0 + mLight, GL_SPOT_DIRECTION, mSpotDirection);
        glGetLightfv(GL_LIGHT0 + mLight, GL_SPOT_EXPONENT, &mSpotExponent);
        glGetLightfv(GL_LIGHT0 + mLight, GL_SPOT_CUTOFF, &mSpotCutoff);
        glGetLightfv(GL_LIGHT0 + mLight, GL_CONSTANT_ATTENUATION, &mConstantAttenuation);
        glGetLightfv(GL_LIGHT0 + mLight, GL_LINEAR_ATTENUATION, &mLinearAttenuation);
        glGetLightfv(GL_LIGHT0 + mLight, GL_QUADRATIC_ATTENUATION, &mQuadraticAttenuation);
    }
    GLint mLight;
    BOboolean mLightEnabled;
    GLfloat mAmbient[4];
    GLfloat mDiffuse[4];
    GLfloat mSpecular[4];
    GLfloat mPosition[4];
    GLfloat mSpotDirection[3];
    GLfloat mSpotExponent;
    GLfloat mSpotCutoff;
    GLfloat mConstantAttenuation;
    GLfloat mLinearAttenuation;
    GLfloat mQuadraticAttenuation;
};


// this class contains the GL states only, i.e. those values that can change on
// runtime. GL_MAX_TEXTURE_SIZE (for example) does not belong to these, while
// GL_TEXTURE_2D can be enabled on runtime and therefore does.
// TODO: can we integrate glAreTexturesResidient() into this somehow? we'd need
// to know which textures exist...
class GLStates
{
public:
    GLStates()
    {
        glGetIntegerv(GL_MAX_LIGHTS, &mMaxLights);
        if (mMaxLights < 8) {
            boError() << k_funcinfo << "OpenGL must provide at least 8 lights, your implementation has " << mMaxLights << " only. aborting now" << endl;
            exit(1);
            return;
        }
        mLightStates = new LightStates[mMaxLights];

#define ADD(a, b) mNameDict.insert(a, b);
        ADD(GL_ACCUM_CLEAR_VALUE, "GL_ACCUM_CLEAR_VALUE");
        ADD(GL_ACTIVE_TEXTURE, "GL_ACTIVE_TEXTURE");
        ADD(GL_ALPHA_BIAS, "GL_ALPHA_BIAS");
        ADD(GL_ALPHA_SCALE, "GL_ALPHA_SCALE");
        ADD(GL_ALPHA_TEST, "GL_ALPHA_TEST");
        ADD(GL_ALPHA_TEST_FUNC, "GL_ALPHA_TEST_FUNC");
        ADD(GL_ALPHA_TEST_REF, "GL_ALPHA_TEST_REF");
        ADD(GL_ATTRIB_STACK_DEPTH, "GL_ATTRIB_STACK_DEPTH");
        ADD(GL_AUTO_NORMAL, "GL_AUTO_NORMAL");

        ADD(GL_BLEND, "GL_BLEND");
        ADD(GL_BLEND_COLOR, "GL_BLEND_COLOR");
        ADD(GL_BLEND_DST, "GL_BLEND_DST");
        ADD(GL_BLEND_EQUATION, "GL_BLEND_EQUATION");
        ADD(GL_BLEND_SRC, "GL_BLEND_SRC");
        ADD(GL_BLUE_BIAS, "GL_BLUE_BIAS");
        ADD(GL_BLUE_SCALE, "GL_BLUE_SCALE");

        ADD(GL_CLIENT_ACTIVE_TEXTURE, "GL_CLIENT_ACTIVE_TEXTURE");
        ADD(GL_CLIENT_ATTRIB_STACK_DEPTH, "GL_CLIENT_ATTRIB_STACK_DEPTH");
        ADD(GL_COLOR_ARRAY, "GL_COLOR_ARRAY");
        ADD(GL_COLOR_ARRAY_POINTER, "GL_COLOR_ARRAY_POINTER"); // pointer
        ADD(GL_COLOR_ARRAY_SIZE, "GL_COLOR_ARRAY_SIZE");
        ADD(GL_COLOR_ARRAY_STRIDE, "GL_COLOR_ARRAY_STRIDE");
        ADD(GL_COLOR_ARRAY_TYPE, "GL_COLOR_ARRAY_TYPE");
        ADD(GL_COLOR_CLEAR_VALUE, "GL_COLOR_CLEAR_VALUE");
        ADD(GL_COLOR_LOGIC_OP, "GL_COLOR_LOGIC_OP");
        ADD(GL_COLOR_MATERIAL, "GL_COLOR_MATERIAL");
        ADD(GL_COLOR_MATERIAL_FACE, "GL_COLOR_MATERIAL_FACE");
        ADD(GL_COLOR_MATERIAL_PARAMETER, "GL_COLOR_MATERIAL_PARAMETER");
        ADD(GL_COLOR_MATRIX, "GL_COLOR_MATRIX");
        ADD(GL_COLOR_TABLE, "GL_COLOR_TABLE");
        ADD(GL_COLOR_WRITEMASK, "GL_COLOR_WRITEMASK");
        ADD(GL_CONVOLUTION_1D, "GL_CONVOLUTION_1D");
        ADD(GL_CONVOLUTION_2D, "GL_CONVOLUTION_2D");
        ADD(GL_CULL_FACE, "GL_CULL_FACE");
        ADD(GL_CULL_FACE_MODE, "GL_CULL_FACE_MODE");
        ADD(GL_CURRENT_COLOR, "GL_CURRENT_COLOR");
        ADD(GL_CURRENT_INDEX, "GL_CURRENT_INDEX");
        ADD(GL_CURRENT_NORMAL, "GL_CURRENT_NORMAL");
        ADD(GL_CURRENT_RASTER_COLOR, "GL_CURRENT_RASTER_COLOR");
        ADD(GL_CURRENT_RASTER_DISTANCE, "GL_CURRENT_RASTER_DISTANCE");
        ADD(GL_CURRENT_RASTER_INDEX, "GL_CURRENT_RASTER_INDEX");
        ADD(GL_CURRENT_RASTER_POSITION, "GL_CURRENT_RASTER_POSITION");
        ADD(GL_CURRENT_RASTER_POSITION_VALID, "GL_CURRENT_RASTER_POSITION_VALID");
        ADD(GL_CURRENT_RASTER_TEXTURE_COORDS, "GL_CURRENT_RASTER_TEXTURE_COORDS");
        ADD(GL_CURRENT_TEXTURE_COORDS, "GL_CURRENT_TEXTURE_COORDS");

        ADD(GL_DEPTH_BIAS, "GL_DEPTH_BIAS");
        ADD(GL_DEPTH_CLEAR_VALUE, "GL_DEPTH_CLEAR_VALUE");
        ADD(GL_DEPTH_FUNC, "GL_DEPTH_FUNC");
        ADD(GL_DEPTH_RANGE, "GL_DEPTH_RANGE");
        ADD(GL_DEPTH_SCALE, "GL_DEPTH_SCALE");
        ADD(GL_DEPTH_TEST, "GL_DEPTH_TEST");
        ADD(GL_DEPTH_WRITEMASK, "GL_DEPTH_WRITEMASK");
        ADD(GL_DITHER, "GL_DITHER");
        ADD(GL_DRAW_BUFFER, "GL_DRAW_BUFFER");

        ADD(GL_EDGE_FLAG, "GL_EDGE_FLAG");
        ADD(GL_EDGE_FLAG_ARRAY, "GL_EDGE_FLAG_ARRAY");
        ADD(GL_EDGE_FLAG_ARRAY_POINTER, "GL_EDGE_FLAG_ARRAY_POINTER"); // pointer
        ADD(GL_EDGE_FLAG_ARRAY_STRIDE, "GL_EDGE_FLAG_ARRAY_STRIDE");

        ADD(GL_FEEDBACK_BUFFER_POINTER, "GL_FEEDBACK_BUFFER_POINTER"); // pointer
        ADD(GL_FEEDBACK_BUFFER_SIZE, "GL_FEEDBACK_BUFFER_SIZE");
        ADD(GL_FEEDBACK_BUFFER_TYPE, "GL_FEEDBACK_BUFFER_TYPE");
        ADD(GL_FOG, "GL_FOG");
        ADD(GL_FOG_COLOR, "GL_FOG_COLOR");
        ADD(GL_FOG_DENSITY, "GL_FOG_DENSITY");
        ADD(GL_FOG_END, "GL_FOG_END");
        ADD(GL_FOG_HINT, "GL_FOG_HINT");
        ADD(GL_FOG_INDEX, "GL_FOG_INDEX");
        ADD(GL_FOG_MODE, "GL_FOG_MODE");
        ADD(GL_FOG_START, "GL_FOG_START");
        ADD(GL_FRONT_FACE, "GL_FRONT_FACE");

        ADD(GL_GREEN_BIAS, "GL_GREEN_BIAS");
        ADD(GL_GREEN_SCALE, "GL_GREEN_SCALE");

        ADD(GL_HISTOGRAM, "GL_HISTOGRAM");

        ADD(GL_INDEX_ARRAY, "GL_INDEX_ARRAY");
        ADD(GL_INDEX_ARRAY_POINTER, "GL_INDEX_ARRAY_POINTER"); // pointer
        ADD(GL_INDEX_ARRAY_STRIDE, "GL_INDEX_ARRAY_STRIDE");
        ADD(GL_INDEX_ARRAY_TYPE, "GL_INDEX_ARRAY_TYPE");
        ADD(GL_INDEX_CLEAR_VALUE, "GL_INDEX_CLEAR_VALUE");
        ADD(GL_INDEX_LOGIC_OP, "GL_INDEX_LOGIC_OP");
        ADD(GL_INDEX_OFFSET, "GL_INDEX_OFFSET");
        ADD(GL_INDEX_SHIFT, "GL_INDEX_SHIFT");
        ADD(GL_INDEX_WRITEMASK, "GL_INDEX_WRITEMASK");

        ADD(GL_LIGHTING, "GL_LIGHTING");
        ADD(GL_LIGHT_MODEL_AMBIENT, "GL_LIGHT_MODEL_AMBIENT");
        ADD(GL_LIGHT_MODEL_COLOR_CONTROL, "GL_LIGHT_MODEL_COLOR_CONTROL");
        ADD(GL_LIGHT_MODEL_LOCAL_VIEWER, "GL_LIGHT_MODEL_LOCAL_VIEWER");
        ADD(GL_LIGHT_MODEL_TWO_SIDE, "GL_LIGHT_MODEL_TWO_SIDE");
        ADD(GL_LINE_SMOOTH, "GL_LINE_SMOOTH");
        ADD(GL_LINE_SMOOTH_HINT, "GL_LINE_SMOOTH_HINT");
        ADD(GL_LINE_STIPPLE, "GL_LINE_STIPPLE");
        ADD(GL_LINE_STIPPLE_PATTERN, "GL_LINE_STIPPLE_PATTERN");
        ADD(GL_LINE_STIPPLE_REPEAT, "GL_LINE_STIPPLE_REPEAT");
        ADD(GL_LINE_WIDTH, "GL_LINE_WIDTH");
        ADD(GL_LIST_BASE, "GL_LIST_BASE");
        ADD(GL_LIST_INDEX, "GL_LIST_INDEX");
        ADD(GL_LIST_MODE, "GL_LIST_MODE");
        ADD(GL_LOGIC_OP_MODE, "GL_LOGIC_OP_MODE");

        ADD(GL_MAP_COLOR, "GL_MAP_COLOR");
        ADD(GL_MAP_STENCIL, "GL_MAP_STENCIL");
        ADD(GL_MAP1_COLOR_4, "GL_MAP1_COLOR_4");
        ADD(GL_MAP1_GRID_SEGMENTS, "GL_MAP1_GRID_SEGMENTS");
        ADD(GL_MAP1_INDEX, "GL_MAP1_INDEX");
        ADD(GL_MAP1_NORMAL, "GL_MAP1_NORMAL");
        ADD(GL_MAP1_TEXTURE_COORD_1, "GL_MAP1_TEXTURE_COORD_1");
        ADD(GL_MAP1_TEXTURE_COORD_2, "GL_MAP1_TEXTURE_COORD_2");
        ADD(GL_MAP1_TEXTURE_COORD_3, "GL_MAP1_TEXTURE_COORD_3");
        ADD(GL_MAP1_TEXTURE_COORD_4, "GL_MAP1_TEXTURE_COORD_4");
        ADD(GL_MAP1_VERTEX_3, "GL_MAP1_VERTEX_3");
        ADD(GL_MAP1_VERTEX_4, "GL_MAP1_VERTEX_4");
        ADD(GL_MAP2_COLOR_4, "GL_MAP2_COLOR_4");
        ADD(GL_MAP2_GRID_SEGMENTS, "GL_MAP2_GRID_SEGMENTS");
        ADD(GL_MAP2_INDEX, "GL_MAP2_INDEX");
        ADD(GL_MAP2_NORMAL, "GL_MAP2_NORMAL");
        ADD(GL_MAP2_TEXTURE_COORD_1, "GL_MAP2_TEXTURE_COORD_1");
        ADD(GL_MAP2_TEXTURE_COORD_2, "GL_MAP2_TEXTURE_COORD_2");
        ADD(GL_MAP2_TEXTURE_COORD_3, "GL_MAP2_TEXTURE_COORD_3");
        ADD(GL_MAP2_TEXTURE_COORD_4, "GL_MAP2_TEXTURE_COORD_4");
        ADD(GL_MAP2_VERTEX_3, "GL_MAP2_VERTEX_3");
        ADD(GL_MAP2_VERTEX_4, "GL_MAP2_VERTEX_4");
        ADD(GL_MATRIX_MODE, "GL_MATRIX_MODE");
        ADD(GL_MINMAX, "GL_MINMAX");
        ADD(GL_MODELVIEW_MATRIX, "GL_MODELVIEW_MATRIX");
        ADD(GL_MODELVIEW_STACK_DEPTH, "GL_MODELVIEW_STACK_DEPTH");

        ADD(GL_NAME_STACK_DEPTH, "GL_NAME_STACK_DEPTH");
        ADD(GL_NORMAL_ARRAY, "GL_NORMAL_ARRAY");
        ADD(GL_NORMAL_ARRAY_POINTER, "GL_NORMAL_ARRAY_POINTER"); // pointer
        ADD(GL_NORMAL_ARRAY_STRIDE, "GL_NORMAL_ARRAY_STRIDE");
        ADD(GL_NORMAL_ARRAY_TYPE, "GL_NORMAL_ARRAY_TYPE");
        ADD(GL_NORMALIZE, "GL_NORMALIZE");

        ADD(GL_PACK_ALIGNMENT, "GL_PACK_ALIGNMENT");
        ADD(GL_PACK_IMAGE_HEIGHT, "GL_PACK_IMAGE_HEIGHT");
        ADD(GL_PACK_LSB_FIRST, "GL_PACK_LSB_FIRST");
        ADD(GL_PACK_ROW_LENGTH, "GL_PACK_ROW_LENGTH");
        ADD(GL_PACK_SKIP_IMAGES, "GL_PACK_SKIP_IMAGES");
        ADD(GL_PACK_SKIP_PIXELS, "GL_PACK_SKIP_PIXELS");
        ADD(GL_PACK_SKIP_ROWS, "GL_PACK_SKIP_ROWS");
        ADD(GL_PACK_SWAP_BYTES, "GL_PACK_SWAP_BYTES");
        ADD(GL_PERSPECTIVE_CORRECTION_HINT, "GL_PERSPECTIVE_CORRECTION_HINT");
        ADD(GL_POINT_SIZE, "GL_POINT_SIZE");
        ADD(GL_POINT_SMOOTH, "GL_POINT_SMOOTH");
        ADD(GL_POINT_SMOOTH_HINT, "GL_POINT_SMOOTH_HINT");
        ADD(GL_POLYGON_SMOOTH, "GL_POLYGON_SMOOTH");
        ADD(GL_POLYGON_SMOOTH_HINT, "GL_POLYGON_SMOOTH_HINT");
        ADD(GL_POLYGON_MODE, "GL_POLYGON_MODE");
        ADD(GL_POLYGON_OFFSET_FACTOR, "GL_POLYGON_OFFSET_FACTOR");
        ADD(GL_POLYGON_OFFSET_FILL, "GL_POLYGON_OFFSET_FILL");
        ADD(GL_POLYGON_OFFSET_LINE, "GL_POLYGON_OFFSET_LINE");
        ADD(GL_POLYGON_OFFSET_POINT, "GL_POLYGON_OFFSET_POINT");
        ADD(GL_POLYGON_STIPPLE, "GL_POLYGON_STIPPLE");
        ADD(GL_POST_COLOR_MATRIX_ALPHA_BIAS, "GL_POST_COLOR_MATRIX_ALPHA_BIAS");
        ADD(GL_POST_COLOR_MATRIX_ALPHA_SCALE, "GL_POST_COLOR_MATRIX_ALPHA_SCALE");
        ADD(GL_POST_COLOR_MATRIX_BLUE_BIAS, "GL_POST_COLOR_MATRIX_BLUE_BIAS");
        ADD(GL_POST_COLOR_MATRIX_BLUE_SCALE, "GL_POST_COLOR_MATRIX_BLUE_SCALE");
        ADD(GL_POST_COLOR_MATRIX_COLOR_TABLE, "GL_POST_COLOR_MATRIX_COLOR_TABLE");
        ADD(GL_POST_COLOR_MATRIX_GREEN_BIAS, "GL_POST_COLOR_MATRIX_GREEN_BIAS");
        ADD(GL_POST_COLOR_MATRIX_GREEN_SCALE, "GL_POST_COLOR_MATRIX_GREEN_SCALE");
        ADD(GL_POST_COLOR_MATRIX_RED_BIAS, "GL_POST_COLOR_MATRIX_RED_BIAS");
        ADD(GL_POST_COLOR_MATRIX_RED_SCALE, "GL_POST_COLOR_MATRIX_RED_SCALE");
        ADD(GL_POST_CONVOLUTION_ALPHA_BIAS, "GL_POST_CONVOLUTION_ALPHA_BIAS");
        ADD(GL_POST_CONVOLUTION_ALPHA_SCALE, "GL_POST_CONVOLUTION_ALPHA_SCALE");
        ADD(GL_POST_CONVOLUTION_BLUE_BIAS, "GL_POST_CONVOLUTION_BLUE_BIAS");
        ADD(GL_POST_CONVOLUTION_BLUE_SCALE, "GL_POST_CONVOLUTION_BLUE_SCALE");
        ADD(GL_POST_CONVOLUTION_COLOR_TABLE, "GL_POST_CONVOLUTION_COLOR_TABLE");
        ADD(GL_POST_CONVOLUTION_GREEN_BIAS, "GL_POST_CONVOLUTION_GREEN_BIAS");
        ADD(GL_POST_CONVOLUTION_GREEN_SCALE, "GL_POST_CONVOLUTION_GREEN_SCALE");
        ADD(GL_POST_CONVOLUTION_RED_BIAS, "GL_POST_CONVOLUTION_RED_BIAS");
        ADD(GL_POST_CONVOLUTION_RED_SCALE, "GL_POST_CONVOLUTION_RED_SCALE");
        ADD(GL_PROJECTION_MATRIX, "GL_PROJECTION_MATRIX");
        ADD(GL_PROJECTION_STACK_DEPTH, "GL_PROJECTION_STACK_DEPTH");

        ADD(GL_READ_BUFFER, "GL_READ_BUFFER");
        ADD(GL_RENDER_MODE, "GL_RENDER_MODE");
        ADD(GL_RESCALE_NORMAL, "GL_RESCALE_NORMAL");
        ADD(GL_RED_BIAS, "GL_RED_BIAS");
        ADD(GL_RED_SCALE, "GL_RED_SCALE");

        ADD(GL_SCISSOR_BOX, "GL_SCISSOR_BOX");
        ADD(GL_SCISSOR_TEST, "GL_SCISSOR_TEST");
        ADD(GL_SELECTION_BUFFER_POINTER, "GL_SELECTION_BUFFER_POINTER"); // pointer
        ADD(GL_SELECTION_BUFFER_SIZE, "GL_SELECTION_BUFFER_SIZE");
        ADD(GL_SEPARABLE_2D, "GL_SEPARABLE_2D");
        ADD(GL_SHADE_MODEL, "GL_SHADE_MODEL");
        ADD(GL_STENCIL_CLEAR_VALUE, "GL_STENCIL_CLEAR_VALUE");
        ADD(GL_STENCIL_FAIL, "GL_STENCIL_FAIL");
        ADD(GL_STENCIL_FUNC, "GL_STENCIL_FUNC");
        ADD(GL_STENCIL_PASS_DEPTH_FAIL, "GL_STENCIL_PASS_DEPTH_FAIL");
        ADD(GL_STENCIL_PASS_DEPTH_PASS, "GL_STENCIL_PASS_DEPTH_PASS");
        ADD(GL_STENCIL_REF, "GL_STENCIL_REF");
        ADD(GL_STENCIL_TEST, "GL_STENCIL_TEST");
        ADD(GL_STENCIL_VALUE_MASK, "GL_STENCIL_VALUE_MASK");
        ADD(GL_STENCIL_WRITEMASK, "GL_STENCIL_WRITEMASK");

        ADD(GL_TEXTURE_1D, "GL_TEXTURE_1D");
        ADD(GL_TEXTURE_2D, "GL_TEXTURE_2D");
        ADD(GL_TEXTURE_3D, "GL_TEXTURE_3D");
        ADD(GL_TEXTURE_BINDING_1D, "GL_TEXTURE_BINDING_1D");
        ADD(GL_TEXTURE_BINDING_2D, "GL_TEXTURE_BINDING_2D");
        ADD(GL_TEXTURE_BINDING_3D, "GL_TEXTURE_BINDING_3D");
        ADD(GL_TEXTURE_COORD_ARRAY, "GL_TEXTURE_COORD_ARRAY");
        ADD(GL_TEXTURE_COORD_ARRAY_POINTER, "GL_TEXTURE_COORD_ARRAY_POINTER");
        ADD(GL_TEXTURE_COORD_ARRAY_SIZE, "GL_TEXTURE_COORD_ARRAY_SIZE");
        ADD(GL_TEXTURE_COORD_ARRAY_STRIDE, "GL_TEXTURE_COORD_ARRAY_STRIDE");
        ADD(GL_TEXTURE_COORD_ARRAY_TYPE, "GL_TEXTURE_COORD_ARRAY_TYPE");
        ADD(GL_TEXTURE_MATRIX, "GL_TEXTURE_MATRIX");
        ADD(GL_TEXTURE_STACK_DEPTH, "GL_TEXTURE_STACK_DEPTH");
        ADD(GL_TEXTURE_GEN_Q, "GL_TEXTURE_GEN_Q");
        ADD(GL_TEXTURE_GEN_R, "GL_TEXTURE_GEN_R");
        ADD(GL_TEXTURE_GEN_S, "GL_TEXTURE_GEN_S");
        ADD(GL_TEXTURE_GEN_T, "GL_TEXTURE_GEN_T");

        ADD(GL_UNPACK_ALIGNMENT, "GL_UNPACK_ALIGNMENT");
        ADD(GL_UNPACK_IMAGE_HEIGHT, "GL_UNPACK_IMAGE_HEIGHT");
        ADD(GL_UNPACK_LSB_FIRST, "GL_UNPACK_LSB_FIRST");
        ADD(GL_UNPACK_ROW_LENGTH, "GL_UNPACK_ROW_LENGTH");
        ADD(GL_UNPACK_SKIP_IMAGES, "GL_UNPACK_SKIP_IMAGES");
        ADD(GL_UNPACK_SKIP_PIXELS, "GL_UNPACK_SKIP_PIXELS");
        ADD(GL_UNPACK_SKIP_ROWS, "GL_UNPACK_SKIP_ROWS");
        ADD(GL_UNPACK_SWAP_BYTES, "GL_UNPACK_SWAP_BYTES");

        ADD(GL_VERTEX_ARRAY, "GL_VERTEX_ARRAY");
        ADD(GL_VERTEX_ARRAY_SIZE, "GL_VERTEX_ARRAY_SIZE");
        ADD(GL_VERTEX_ARRAY_TYPE, "GL_VERTEX_ARRAY_TYPE");
        ADD(GL_VERTEX_ARRAY_STRIDE, "GL_VERTEX_ARRAY_STRIDE");
        ADD(GL_VERTEX_ARRAY_POINTER, "GL_VERTEX_ARRAY_POINTER"); // pointer
        ADD(GL_VIEWPORT, "GL_VIEWPORT");

        ADD(GL_ZOOM_X, "GL_ZOOM_X");
        ADD(GL_ZOOM_Y, "GL_ZOOM_Y");

        // TODO GL_PIXELMAP_x_SIZE
        // TODO: GL_LIGHT_i
        // TODO: GL_CLIP_PLANE_i

#undef ADD

    }

    ~GLStates()
    {
        delete[] mLightStates;
    }

    void getStates()
    {
        // FIXME: AB: is sizeof(array) == arrayizse portable?
#define BO_UPDATE(x) mValues.insert((int)x, GLGetValue::get(x, m_##x, sizeof(m_##x) / sizeof(m_##x[0])));
        BO_UPDATE(GL_ACCUM_CLEAR_VALUE);
        BO_UPDATE(GL_ACTIVE_TEXTURE);
        BO_UPDATE(GL_ALPHA_BIAS);
        BO_UPDATE(GL_ALPHA_SCALE);
        BO_UPDATE(GL_ALPHA_TEST);
        BO_UPDATE(GL_ALPHA_TEST_FUNC);
        BO_UPDATE(GL_ALPHA_TEST_REF);
        BO_UPDATE(GL_AUTO_NORMAL);
        BO_UPDATE(GL_ATTRIB_STACK_DEPTH);

        BO_UPDATE(GL_BLEND);
        BO_UPDATE(GL_BLEND_COLOR);
        BO_UPDATE(GL_BLEND_DST);
        BO_UPDATE(GL_BLEND_EQUATION);
        BO_UPDATE(GL_BLEND_SRC);
        BO_UPDATE(GL_BLUE_BIAS);
        BO_UPDATE(GL_BLUE_SCALE);

        BO_UPDATE(GL_CLIENT_ACTIVE_TEXTURE);
        BO_UPDATE(GL_CLIENT_ATTRIB_STACK_DEPTH);
        BO_UPDATE(GL_COLOR_ARRAY);
        BO_UPDATE(GL_COLOR_ARRAY_POINTER); // pointer
        BO_UPDATE(GL_COLOR_ARRAY_SIZE);
        BO_UPDATE(GL_COLOR_ARRAY_STRIDE);
        BO_UPDATE(GL_COLOR_ARRAY_TYPE);
        BO_UPDATE(GL_COLOR_CLEAR_VALUE);
        BO_UPDATE(GL_COLOR_LOGIC_OP);
        BO_UPDATE(GL_COLOR_MATERIAL);
        BO_UPDATE(GL_COLOR_MATERIAL_PARAMETER);
        BO_UPDATE(GL_COLOR_MATERIAL_FACE);
        BO_UPDATE(GL_COLOR_MATRIX);
        BO_UPDATE(GL_COLOR_TABLE);
        BO_UPDATE(GL_COLOR_WRITEMASK);
        BO_UPDATE(GL_CONVOLUTION_1D);
        BO_UPDATE(GL_CONVOLUTION_2D);
        BO_UPDATE(GL_CULL_FACE);
        BO_UPDATE(GL_CULL_FACE_MODE);
        BO_UPDATE(GL_CURRENT_COLOR);
        BO_UPDATE(GL_CURRENT_INDEX);
        BO_UPDATE(GL_CURRENT_NORMAL);
        BO_UPDATE(GL_CURRENT_RASTER_COLOR);
        BO_UPDATE(GL_CURRENT_RASTER_DISTANCE);
        BO_UPDATE(GL_CURRENT_RASTER_INDEX);
        BO_UPDATE(GL_CURRENT_RASTER_POSITION);
        BO_UPDATE(GL_CURRENT_RASTER_POSITION_VALID);
        BO_UPDATE(GL_CURRENT_RASTER_TEXTURE_COORDS);
        BO_UPDATE(GL_CURRENT_TEXTURE_COORDS);

        BO_UPDATE(GL_DEPTH_BIAS);
        BO_UPDATE(GL_DEPTH_CLEAR_VALUE);
        BO_UPDATE(GL_DEPTH_FUNC);
        BO_UPDATE(GL_DEPTH_RANGE);
        BO_UPDATE(GL_DEPTH_SCALE);
        BO_UPDATE(GL_DEPTH_TEST);
        BO_UPDATE(GL_DEPTH_WRITEMASK);
        BO_UPDATE(GL_DITHER);
        BO_UPDATE(GL_DRAW_BUFFER);

        BO_UPDATE(GL_EDGE_FLAG);
        BO_UPDATE(GL_EDGE_FLAG_ARRAY);
        BO_UPDATE(GL_EDGE_FLAG_ARRAY_POINTER); // pointer
        BO_UPDATE(GL_EDGE_FLAG_ARRAY_STRIDE);

        BO_UPDATE(GL_FEEDBACK_BUFFER_POINTER); // pointer
        BO_UPDATE(GL_FEEDBACK_BUFFER_SIZE);
        BO_UPDATE(GL_FEEDBACK_BUFFER_TYPE);
        BO_UPDATE(GL_FOG);
        BO_UPDATE(GL_FOG_COLOR);
        BO_UPDATE(GL_FOG_DENSITY);
        BO_UPDATE(GL_FOG_END);
        BO_UPDATE(GL_FOG_HINT);
        BO_UPDATE(GL_FOG_INDEX);
        BO_UPDATE(GL_FOG_MODE);
        BO_UPDATE(GL_FOG_START);
        BO_UPDATE(GL_FRONT_FACE);

        BO_UPDATE(GL_GREEN_BIAS);
        BO_UPDATE(GL_GREEN_SCALE);

        BO_UPDATE(GL_HISTOGRAM);

        BO_UPDATE(GL_INDEX_ARRAY);
        BO_UPDATE(GL_INDEX_ARRAY_POINTER); // pointer
        BO_UPDATE(GL_INDEX_ARRAY_STRIDE);
        BO_UPDATE(GL_INDEX_ARRAY_TYPE);
        BO_UPDATE(GL_INDEX_CLEAR_VALUE);
        BO_UPDATE(GL_INDEX_LOGIC_OP);
        BO_UPDATE(GL_INDEX_SHIFT);
        BO_UPDATE(GL_INDEX_OFFSET);
        BO_UPDATE(GL_INDEX_WRITEMASK);

        BO_UPDATE(GL_LIGHTING);
        BO_UPDATE(GL_LIGHT_MODEL_AMBIENT);
        BO_UPDATE(GL_LIGHT_MODEL_COLOR_CONTROL);
        BO_UPDATE(GL_LIGHT_MODEL_LOCAL_VIEWER);
        BO_UPDATE(GL_LIGHT_MODEL_TWO_SIDE);
        BO_UPDATE(GL_LINE_SMOOTH);
        BO_UPDATE(GL_LINE_SMOOTH_HINT);
        BO_UPDATE(GL_LINE_STIPPLE);
        BO_UPDATE(GL_LINE_STIPPLE_PATTERN);
        BO_UPDATE(GL_LINE_STIPPLE_REPEAT);
        BO_UPDATE(GL_LINE_WIDTH);
        BO_UPDATE(GL_LIST_BASE);
        BO_UPDATE(GL_LIST_INDEX);
        BO_UPDATE(GL_LIST_MODE);
        BO_UPDATE(GL_LOGIC_OP_MODE);

        BO_UPDATE(GL_MAP_COLOR);
        BO_UPDATE(GL_MAP_STENCIL);
        BO_UPDATE(GL_MAP1_COLOR_4);
        BO_UPDATE(GL_MAP1_GRID_SEGMENTS);
        BO_UPDATE(GL_MAP1_INDEX);
        BO_UPDATE(GL_MAP1_NORMAL);
        BO_UPDATE(GL_MAP1_TEXTURE_COORD_1);
        BO_UPDATE(GL_MAP1_TEXTURE_COORD_2);
        BO_UPDATE(GL_MAP1_TEXTURE_COORD_3);
        BO_UPDATE(GL_MAP1_TEXTURE_COORD_4);
        BO_UPDATE(GL_MAP1_VERTEX_3);
        BO_UPDATE(GL_MAP1_VERTEX_4);
        BO_UPDATE(GL_MAP2_COLOR_4);
        BO_UPDATE(GL_MAP2_GRID_SEGMENTS);
        BO_UPDATE(GL_MAP2_INDEX);
        BO_UPDATE(GL_MAP2_NORMAL);
        BO_UPDATE(GL_MAP2_TEXTURE_COORD_1);
        BO_UPDATE(GL_MAP2_TEXTURE_COORD_2);
        BO_UPDATE(GL_MAP2_TEXTURE_COORD_3);
        BO_UPDATE(GL_MAP2_TEXTURE_COORD_4);
        BO_UPDATE(GL_MAP2_VERTEX_3);
        BO_UPDATE(GL_MAP2_VERTEX_4);
        BO_UPDATE(GL_MATRIX_MODE);
        BO_UPDATE(GL_MINMAX);
        BO_UPDATE(GL_MODELVIEW_MATRIX);
        BO_UPDATE(GL_MODELVIEW_STACK_DEPTH);

        BO_UPDATE(GL_NAME_STACK_DEPTH);
        BO_UPDATE(GL_NORMAL_ARRAY);
        BO_UPDATE(GL_NORMAL_ARRAY_POINTER); // pointer
        BO_UPDATE(GL_NORMAL_ARRAY_STRIDE);
        BO_UPDATE(GL_NORMAL_ARRAY_TYPE);
        BO_UPDATE(GL_NORMALIZE);

        BO_UPDATE(GL_PACK_ALIGNMENT);
        BO_UPDATE(GL_PACK_LSB_FIRST);
        BO_UPDATE(GL_PACK_IMAGE_HEIGHT);
        BO_UPDATE(GL_PACK_ROW_LENGTH);
        BO_UPDATE(GL_PACK_SKIP_IMAGES);
        BO_UPDATE(GL_PACK_SKIP_PIXELS);
        BO_UPDATE(GL_PACK_SKIP_ROWS);
        BO_UPDATE(GL_PACK_SWAP_BYTES);
        BO_UPDATE(GL_PERSPECTIVE_CORRECTION_HINT);
        BO_UPDATE(GL_POINT_SIZE);
        BO_UPDATE(GL_POINT_SMOOTH);
        BO_UPDATE(GL_POINT_SMOOTH_HINT);
        BO_UPDATE(GL_POLYGON_MODE);
        BO_UPDATE(GL_POLYGON_OFFSET_FACTOR);
        BO_UPDATE(GL_POLYGON_OFFSET_FILL);
        BO_UPDATE(GL_POLYGON_OFFSET_LINE);
        BO_UPDATE(GL_POLYGON_OFFSET_POINT);
        BO_UPDATE(GL_POLYGON_SMOOTH);
        BO_UPDATE(GL_POLYGON_SMOOTH_HINT);
        BO_UPDATE(GL_POLYGON_STIPPLE);
        BO_UPDATE(GL_POST_COLOR_MATRIX_ALPHA_BIAS);
        BO_UPDATE(GL_POST_COLOR_MATRIX_ALPHA_SCALE);
        BO_UPDATE(GL_POST_COLOR_MATRIX_BLUE_BIAS);
        BO_UPDATE(GL_POST_COLOR_MATRIX_BLUE_SCALE);
        BO_UPDATE(GL_POST_COLOR_MATRIX_COLOR_TABLE);
        BO_UPDATE(GL_POST_COLOR_MATRIX_GREEN_BIAS);
        BO_UPDATE(GL_POST_COLOR_MATRIX_GREEN_SCALE);
        BO_UPDATE(GL_POST_COLOR_MATRIX_RED_BIAS);
        BO_UPDATE(GL_POST_COLOR_MATRIX_RED_SCALE);
        BO_UPDATE(GL_POST_CONVOLUTION_ALPHA_BIAS);
        BO_UPDATE(GL_POST_CONVOLUTION_ALPHA_SCALE);
        BO_UPDATE(GL_POST_CONVOLUTION_BLUE_BIAS);
        BO_UPDATE(GL_POST_CONVOLUTION_BLUE_SCALE);
        BO_UPDATE(GL_POST_CONVOLUTION_COLOR_TABLE);
        BO_UPDATE(GL_POST_CONVOLUTION_GREEN_BIAS);
        BO_UPDATE(GL_POST_CONVOLUTION_GREEN_SCALE);
        BO_UPDATE(GL_POST_CONVOLUTION_RED_BIAS);
        BO_UPDATE(GL_POST_CONVOLUTION_RED_SCALE);
        BO_UPDATE(GL_PROJECTION_MATRIX);
        BO_UPDATE(GL_PROJECTION_STACK_DEPTH);

        BO_UPDATE(GL_READ_BUFFER);
        BO_UPDATE(GL_RED_SCALE);
        BO_UPDATE(GL_RED_BIAS);
        BO_UPDATE(GL_RENDER_MODE);
        BO_UPDATE(GL_RESCALE_NORMAL);

        BO_UPDATE(GL_SCISSOR_BOX);
        BO_UPDATE(GL_SCISSOR_TEST);
        BO_UPDATE(GL_SELECTION_BUFFER_POINTER); // pointer
        BO_UPDATE(GL_SELECTION_BUFFER_SIZE);
        BO_UPDATE(GL_SEPARABLE_2D);
        BO_UPDATE(GL_SHADE_MODEL);
        BO_UPDATE(GL_STENCIL_CLEAR_VALUE);
        BO_UPDATE(GL_STENCIL_FAIL);
        BO_UPDATE(GL_STENCIL_FUNC);
        BO_UPDATE(GL_STENCIL_PASS_DEPTH_FAIL);
        BO_UPDATE(GL_STENCIL_PASS_DEPTH_PASS);
        BO_UPDATE(GL_STENCIL_REF);
        BO_UPDATE(GL_STENCIL_TEST);
        BO_UPDATE(GL_STENCIL_VALUE_MASK);
        BO_UPDATE(GL_STENCIL_WRITEMASK);

        BO_UPDATE(GL_TEXTURE_1D);
        BO_UPDATE(GL_TEXTURE_2D);
        BO_UPDATE(GL_TEXTURE_3D);
        BO_UPDATE(GL_TEXTURE_BINDING_1D);
        BO_UPDATE(GL_TEXTURE_BINDING_2D);
        BO_UPDATE(GL_TEXTURE_BINDING_3D);
        BO_UPDATE(GL_TEXTURE_COORD_ARRAY);
        BO_UPDATE(GL_TEXTURE_COORD_ARRAY_POINTER);
        BO_UPDATE(GL_TEXTURE_COORD_ARRAY_SIZE);
        BO_UPDATE(GL_TEXTURE_COORD_ARRAY_STRIDE);
        BO_UPDATE(GL_TEXTURE_COORD_ARRAY_TYPE);
        BO_UPDATE(GL_TEXTURE_GEN_Q);
        BO_UPDATE(GL_TEXTURE_GEN_R);
        BO_UPDATE(GL_TEXTURE_GEN_S);
        BO_UPDATE(GL_TEXTURE_GEN_T);
        BO_UPDATE(GL_TEXTURE_MATRIX);
        BO_UPDATE(GL_TEXTURE_STACK_DEPTH);

        BO_UPDATE(GL_UNPACK_ALIGNMENT);
        BO_UPDATE(GL_UNPACK_IMAGE_HEIGHT);
        BO_UPDATE(GL_UNPACK_LSB_FIRST);
        BO_UPDATE(GL_UNPACK_ROW_LENGTH);
        BO_UPDATE(GL_UNPACK_SKIP_IMAGES);
        BO_UPDATE(GL_UNPACK_SKIP_PIXELS);
        BO_UPDATE(GL_UNPACK_SKIP_ROWS);
        BO_UPDATE(GL_UNPACK_SWAP_BYTES);

        BO_UPDATE(GL_VERTEX_ARRAY);
        BO_UPDATE(GL_VERTEX_ARRAY_POINTER); // pointer
        BO_UPDATE(GL_VERTEX_ARRAY_SIZE);
        BO_UPDATE(GL_VERTEX_ARRAY_STRIDE);
        BO_UPDATE(GL_VERTEX_ARRAY_TYPE);
        BO_UPDATE(GL_VIEWPORT);

        BO_UPDATE(GL_ZOOM_X);
        BO_UPDATE(GL_ZOOM_Y);

        // TODO GL_PIXELMAP_x_SIZE
        // TODO: GL_CLIP_PLANE_i
        // TODO: GL_LIGHT_i

#undef BO_UPDATE


        mMaterialStates[0].getMaterialStates(GL_FRONT);
        mMaterialStates[1].getMaterialStates(GL_BACK);
        for (GLint i = 0; i < mMaxLights; i++) {
            mLightStates[i].getLightStates(i);
        }
    }


    QStringList list() const
    {
        QStringList list;
        QValueList<int> keys = mNameDict.keys();
        QValueList<int>::Iterator it;
        for (it = keys.begin(); it != keys.end(); ++it) {
            list.append(listItem((GLenum)*it));
        }
        return list;
    }

    QStringList listEnabled() const
    {
        QStringList list;
        // AB: see man glIsEnabled() for a list
        list.append(listItem(GL_ALPHA_TEST));
        list.append(listItem(GL_AUTO_NORMAL));
        list.append(listItem(GL_BLEND));
        list.append(listItem(GL_COLOR_ARRAY));
        list.append(listItem(GL_COLOR_LOGIC_OP));
        list.append(listItem(GL_COLOR_MATERIAL));
        list.append(listItem(GL_COLOR_TABLE));
        list.append(listItem(GL_CONVOLUTION_1D));
        list.append(listItem(GL_CONVOLUTION_2D));
        list.append(listItem(GL_CULL_FACE));
        list.append(listItem(GL_DEPTH_TEST));
        list.append(listItem(GL_DITHER));
        list.append(listItem(GL_EDGE_FLAG_ARRAY));
        list.append(listItem(GL_FOG));
        list.append(listItem(GL_HISTOGRAM));
        list.append(listItem(GL_INDEX_ARRAY));
        list.append(listItem(GL_LIGHTING));
        list.append(listItem(GL_LINE_SMOOTH));
        list.append(listItem(GL_LINE_STIPPLE));
        list.append(listItem(GL_MAP1_COLOR_4));
        list.append(listItem(GL_MAP1_INDEX));
        list.append(listItem(GL_MAP1_NORMAL));
        list.append(listItem(GL_MAP1_TEXTURE_COORD_1));
        list.append(listItem(GL_MAP1_TEXTURE_COORD_2));
        list.append(listItem(GL_MAP1_TEXTURE_COORD_3));
        list.append(listItem(GL_MAP1_TEXTURE_COORD_4));
        list.append(listItem(GL_MAP1_VERTEX_3));
        list.append(listItem(GL_MAP1_VERTEX_4));
        list.append(listItem(GL_MAP2_COLOR_4));
        list.append(listItem(GL_MAP2_INDEX));
        list.append(listItem(GL_MAP2_NORMAL));
        list.append(listItem(GL_MAP2_TEXTURE_COORD_1));
        list.append(listItem(GL_MAP2_TEXTURE_COORD_2));
        list.append(listItem(GL_MAP2_TEXTURE_COORD_3));
        list.append(listItem(GL_MAP2_TEXTURE_COORD_4));
        list.append(listItem(GL_MAP2_VERTEX_3));
        list.append(listItem(GL_MAP2_VERTEX_4));
        list.append(listItem(GL_MINMAX));
        list.append(listItem(GL_NORMAL_ARRAY));
        list.append(listItem(GL_NORMALIZE));
        list.append(listItem(GL_POINT_SMOOTH));
        list.append(listItem(GL_POLYGON_SMOOTH));
        list.append(listItem(GL_POLYGON_OFFSET_FILL));
        list.append(listItem(GL_POLYGON_OFFSET_LINE));
        list.append(listItem(GL_POLYGON_OFFSET_POINT));
        list.append(listItem(GL_POLYGON_STIPPLE));
        list.append(listItem(GL_POST_COLOR_MATRIX_COLOR_TABLE));
        list.append(listItem(GL_POST_CONVOLUTION_COLOR_TABLE));
        list.append(listItem(GL_RESCALE_NORMAL));
        list.append(listItem(GL_SCISSOR_TEST));
        list.append(listItem(GL_SEPARABLE_2D));
        list.append(listItem(GL_STENCIL_TEST));
        list.append(listItem(GL_TEXTURE_1D));
        list.append(listItem(GL_TEXTURE_2D));
        list.append(listItem(GL_TEXTURE_3D));
        list.append(listItem(GL_TEXTURE_COORD_ARRAY));
        list.append(listItem(GL_TEXTURE_GEN_Q));
        list.append(listItem(GL_TEXTURE_GEN_R));
        list.append(listItem(GL_TEXTURE_GEN_S));
        list.append(listItem(GL_TEXTURE_GEN_T));
        list.append(listItem(GL_VERTEX_ARRAY));

        // TODO: GL_CLIP_PLANEi
        // TODO: GL_LIGHTi

        return list;
    }
    QString value(GLenum e) const
    {
        return mValues[(int)e];
    }

    BO_VAR(GLint,     GL_ACCUM_CLEAR_VALUE, 1)
    BO_VAR(GLint,     GL_ACTIVE_TEXTURE, 1)
    BO_VAR(GLfloat,   GL_ALPHA_BIAS, 1)
    BO_VAR(GLfloat,   GL_ALPHA_SCALE, 1)
    BO_VAR(BOboolean, GL_ALPHA_TEST, 1)
    BO_VAR(GLint,     GL_ALPHA_TEST_FUNC, 1)
    BO_VAR(GLint,     GL_ALPHA_TEST_REF, 1)
    BO_VAR(GLint,     GL_ATTRIB_STACK_DEPTH, 1)
    BO_VAR(BOboolean, GL_AUTO_NORMAL, 1)

    BO_VAR(BOboolean, GL_BLEND, 1)
    BO_VAR(GLfloat,   GL_BLEND_COLOR, 4)
    BO_VAR(GLint,     GL_BLEND_DST, 1)
    BO_VAR(GLint,     GL_BLEND_EQUATION, 1)
    BO_VAR(GLint,     GL_BLEND_SRC, 1)
    BO_VAR(GLfloat,   GL_BLUE_BIAS, 1)
    BO_VAR(GLfloat,   GL_BLUE_SCALE, 1)

    BO_VAR(GLint,     GL_CLIENT_ACTIVE_TEXTURE, 1)
    BO_VAR(GLint,     GL_CLIENT_ATTRIB_STACK_DEPTH, 1)
    BO_VAR(BOboolean, GL_COLOR_ARRAY, 1)
    BO_VAR(void*,     GL_COLOR_ARRAY_POINTER, 1)
    BO_VAR(GLint,     GL_COLOR_ARRAY_SIZE, 1)
    BO_VAR(GLint,     GL_COLOR_ARRAY_STRIDE, 1)
    BO_VAR(GLint,     GL_COLOR_ARRAY_TYPE, 1)
    BO_VAR(GLfloat,   GL_COLOR_CLEAR_VALUE, 4)
    BO_VAR(BOboolean, GL_COLOR_LOGIC_OP, 1)
    BO_VAR(BOboolean, GL_COLOR_MATERIAL, 1)
    BO_VAR(GLint,     GL_COLOR_MATERIAL_FACE, 1)
    BO_VAR(GLint,     GL_COLOR_MATERIAL_PARAMETER, 1)
    BO_VAR(GLfloat,   GL_COLOR_MATRIX, 16)
    BO_VAR(BOboolean, GL_COLOR_TABLE, 1)
    BO_VAR(BOboolean, GL_COLOR_WRITEMASK, 1)
    BO_VAR(BOboolean, GL_CONVOLUTION_1D, 1)
    BO_VAR(BOboolean, GL_CONVOLUTION_2D, 1)
    BO_VAR(BOboolean, GL_CULL_FACE, 1)
    BO_VAR(GLint,     GL_CULL_FACE_MODE, 1)
    BO_VAR(GLfloat,   GL_CURRENT_COLOR, 4)
    BO_VAR(GLint,     GL_CURRENT_INDEX, 1)
    BO_VAR(GLfloat,   GL_CURRENT_NORMAL, 3)
    BO_VAR(GLfloat,   GL_CURRENT_RASTER_COLOR, 4)
    BO_VAR(GLfloat,   GL_CURRENT_RASTER_DISTANCE, 1)
    BO_VAR(GLint,     GL_CURRENT_RASTER_INDEX, 1)
    BO_VAR(GLfloat,   GL_CURRENT_RASTER_POSITION, 4)
    BO_VAR(BOboolean, GL_CURRENT_RASTER_POSITION_VALID, 1)
    BO_VAR(GLfloat,   GL_CURRENT_RASTER_TEXTURE_COORDS, 4)
    BO_VAR(GLfloat,   GL_CURRENT_TEXTURE_COORDS, 4)

    BO_VAR(GLfloat,   GL_DEPTH_BIAS, 1)
    BO_VAR(GLfloat,   GL_DEPTH_CLEAR_VALUE, 1)
    BO_VAR(GLint,     GL_DEPTH_FUNC, 1)
    BO_VAR(GLfloat,   GL_DEPTH_RANGE, 2)
    BO_VAR(GLfloat,   GL_DEPTH_SCALE, 1)
    BO_VAR(GLint,     GL_DEPTH_TEST, 1)
    BO_VAR(BOboolean, GL_DEPTH_WRITEMASK, 1)
    BO_VAR(BOboolean, GL_DITHER, 1)
    BO_VAR(GLint,     GL_DRAW_BUFFER, 1)

    BO_VAR(BOboolean, GL_EDGE_FLAG, 1)
    BO_VAR(BOboolean, GL_EDGE_FLAG_ARRAY, 1)
    BO_VAR(void*,     GL_EDGE_FLAG_ARRAY_POINTER, 1)
    BO_VAR(GLint,     GL_EDGE_FLAG_ARRAY_STRIDE, 1)

    BO_VAR(void*,     GL_FEEDBACK_BUFFER_POINTER, 1)
    BO_VAR(GLint,     GL_FEEDBACK_BUFFER_SIZE, 1)
    BO_VAR(GLint,     GL_FEEDBACK_BUFFER_TYPE, 1)
    BO_VAR(BOboolean, GL_FOG, 1)
    BO_VAR(GLfloat,   GL_FOG_COLOR, 4)
    BO_VAR(GLfloat,   GL_FOG_DENSITY, 1)
    BO_VAR(GLfloat,   GL_FOG_END, 1)
    BO_VAR(GLint,     GL_FOG_HINT, 1)
    BO_VAR(GLint,     GL_FOG_INDEX, 1)
    BO_VAR(GLint,     GL_FOG_MODE, 1)
    BO_VAR(GLfloat,   GL_FOG_START, 1)
    BO_VAR(GLint,     GL_FRONT_FACE, 1)

    BO_VAR(GLfloat,   GL_GREEN_BIAS, 1)
    BO_VAR(GLfloat,   GL_GREEN_SCALE, 1)

    BO_VAR(BOboolean, GL_HISTOGRAM, 1)

    BO_VAR(BOboolean, GL_INDEX_ARRAY, 1)
    BO_VAR(void*,     GL_INDEX_ARRAY_POINTER, 1)
    BO_VAR(GLint,     GL_INDEX_ARRAY_STRIDE, 1)
    BO_VAR(GLint,     GL_INDEX_ARRAY_TYPE, 1)
    BO_VAR(GLfloat,   GL_INDEX_CLEAR_VALUE, 1)
    BO_VAR(BOboolean, GL_INDEX_LOGIC_OP, 1)
    BO_VAR(GLint,     GL_INDEX_OFFSET, 1)
    BO_VAR(GLint,     GL_INDEX_SHIFT, 1)
    BO_VAR(GLint,     GL_INDEX_WRITEMASK, 1)

    BO_VAR(BOboolean, GL_LIGHTING, 1)
    BO_VAR(GLfloat,   GL_LIGHT_MODEL_AMBIENT, 4)
    BO_VAR(BOboolean, GL_LIGHT_MODEL_LOCAL_VIEWER, 1)
    BO_VAR(BOboolean, GL_LIGHT_MODEL_TWO_SIDE, 1)
    BO_VAR(GLint,     GL_LIGHT_MODEL_COLOR_CONTROL, 1)
    BO_VAR(BOboolean, GL_LINE_SMOOTH, 1)
    BO_VAR(GLint,     GL_LINE_SMOOTH_HINT, 1)
    BO_VAR(GLint,     GL_LINE_STIPPLE_PATTERN, 1)
    BO_VAR(GLint,     GL_LINE_STIPPLE_REPEAT, 1)
    BO_VAR(BOboolean, GL_LINE_STIPPLE, 1)
    BO_VAR(GLfloat,   GL_LINE_WIDTH, 1)
    BO_VAR(GLint,     GL_LIST_BASE, 1)
    BO_VAR(GLint,     GL_LIST_INDEX, 1)
    BO_VAR(GLint,     GL_LIST_MODE, 1)
    BO_VAR(GLint,     GL_LOGIC_OP_MODE, 1)

    BO_VAR(BOboolean, GL_MAP_COLOR, 1)
    BO_VAR(BOboolean, GL_MAP_STENCIL, 1)
    BO_VAR(BOboolean, GL_MAP1_COLOR_4, 1)
    BO_VAR(GLfloat,   GL_MAP1_GRID_SEGMENTS, 1)
    BO_VAR(BOboolean, GL_MAP1_INDEX, 1)
    BO_VAR(BOboolean, GL_MAP1_NORMAL, 1)
    BO_VAR(BOboolean, GL_MAP1_TEXTURE_COORD_1, 1)
    BO_VAR(BOboolean, GL_MAP1_TEXTURE_COORD_2, 1)
    BO_VAR(BOboolean, GL_MAP1_TEXTURE_COORD_3, 1)
    BO_VAR(BOboolean, GL_MAP1_TEXTURE_COORD_4, 1)
    BO_VAR(BOboolean, GL_MAP1_VERTEX_3, 1)
    BO_VAR(BOboolean, GL_MAP1_VERTEX_4, 1)
    BO_VAR(BOboolean, GL_MAP2_COLOR_4, 1)
    BO_VAR(GLfloat,   GL_MAP2_GRID_SEGMENTS, 2)
    BO_VAR(BOboolean, GL_MAP2_INDEX, 1)
    BO_VAR(BOboolean, GL_MAP2_NORMAL, 1)
    BO_VAR(BOboolean, GL_MAP2_TEXTURE_COORD_1, 1)
    BO_VAR(BOboolean, GL_MAP2_TEXTURE_COORD_2, 1)
    BO_VAR(BOboolean, GL_MAP2_TEXTURE_COORD_3, 1)
    BO_VAR(BOboolean, GL_MAP2_TEXTURE_COORD_4, 1)
    BO_VAR(BOboolean, GL_MAP2_VERTEX_3, 1)
    BO_VAR(BOboolean, GL_MAP2_VERTEX_4, 1)
    BO_VAR(GLint,     GL_MATRIX_MODE, 1)
    BO_VAR(BOboolean, GL_MINMAX, 1)
    BO_VAR(GLfloat,   GL_MODELVIEW_MATRIX, 16)
    BO_VAR(GLint,     GL_MODELVIEW_STACK_DEPTH, 1)

    BO_VAR(GLint,     GL_NAME_STACK_DEPTH, 1)
    BO_VAR(BOboolean, GL_NORMAL_ARRAY, 1)
    BO_VAR(GLint,     GL_NORMAL_ARRAY_TYPE, 1)
    BO_VAR(GLint,     GL_NORMAL_ARRAY_STRIDE, 1)
    BO_VAR(void*,     GL_NORMAL_ARRAY_POINTER, 1)
    BO_VAR(BOboolean, GL_NORMALIZE, 1)

    BO_VAR(GLint,     GL_PACK_ALIGNMENT, 1)
    BO_VAR(GLint,     GL_PACK_IMAGE_HEIGHT, 1)
    BO_VAR(BOboolean, GL_PACK_LSB_FIRST, 1)
    BO_VAR(GLint,     GL_PACK_ROW_LENGTH, 1)
    BO_VAR(GLint,     GL_PACK_SKIP_IMAGES, 1)
    BO_VAR(GLint,     GL_PACK_SKIP_PIXELS, 1)
    BO_VAR(GLint,     GL_PACK_SKIP_ROWS, 1)
    BO_VAR(BOboolean, GL_PACK_SWAP_BYTES, 1)
    BO_VAR(GLint,     GL_PERSPECTIVE_CORRECTION_HINT, 1)
    BO_VAR(GLfloat,   GL_POINT_SIZE, 1)
    BO_VAR(BOboolean, GL_POINT_SMOOTH, 1)
    BO_VAR(GLint,     GL_POINT_SMOOTH_HINT, 1)
    BO_VAR(GLint,     GL_POLYGON_MODE, 1)
    BO_VAR(GLfloat,   GL_POLYGON_OFFSET_FACTOR, 1)
    BO_VAR(BOboolean, GL_POLYGON_OFFSET_FILL, 1)
    BO_VAR(BOboolean, GL_POLYGON_OFFSET_LINE, 1)
    BO_VAR(BOboolean, GL_POLYGON_OFFSET_POINT, 1)
    BO_VAR(BOboolean, GL_POLYGON_SMOOTH, 1)
    BO_VAR(GLint,     GL_POLYGON_SMOOTH_HINT, 1)
    BO_VAR(BOboolean, GL_POLYGON_STIPPLE, 1)
    BO_VAR(GLfloat,   GL_POST_COLOR_MATRIX_ALPHA_BIAS, 1)
    BO_VAR(GLfloat,   GL_POST_COLOR_MATRIX_ALPHA_SCALE, 1)
    BO_VAR(GLfloat,   GL_POST_COLOR_MATRIX_BLUE_BIAS, 1)
    BO_VAR(GLfloat,   GL_POST_COLOR_MATRIX_BLUE_SCALE, 1)
    BO_VAR(BOboolean, GL_POST_COLOR_MATRIX_COLOR_TABLE, 1)
    BO_VAR(GLfloat,   GL_POST_COLOR_MATRIX_GREEN_BIAS, 1)
    BO_VAR(GLfloat,   GL_POST_COLOR_MATRIX_GREEN_SCALE, 1)
    BO_VAR(GLfloat,   GL_POST_COLOR_MATRIX_RED_BIAS, 1)
    BO_VAR(GLfloat,   GL_POST_COLOR_MATRIX_RED_SCALE, 1)
    BO_VAR(BOboolean, GL_POST_CONVOLUTION_COLOR_TABLE, 1)
    BO_VAR(GLfloat,   GL_POST_CONVOLUTION_ALPHA_BIAS, 1)
    BO_VAR(GLfloat,   GL_POST_CONVOLUTION_ALPHA_SCALE, 1)
    BO_VAR(GLfloat,   GL_POST_CONVOLUTION_BLUE_BIAS, 1)
    BO_VAR(GLfloat,   GL_POST_CONVOLUTION_BLUE_SCALE, 1)
    BO_VAR(GLfloat,   GL_POST_CONVOLUTION_GREEN_BIAS, 1)
    BO_VAR(GLfloat,   GL_POST_CONVOLUTION_GREEN_SCALE, 1)
    BO_VAR(GLfloat,   GL_POST_CONVOLUTION_RED_BIAS, 1)
    BO_VAR(GLfloat,   GL_POST_CONVOLUTION_RED_SCALE, 1)
    BO_VAR(GLfloat,   GL_PROJECTION_MATRIX, 16)
    BO_VAR(GLint,     GL_PROJECTION_STACK_DEPTH, 1)

    BO_VAR(GLint,     GL_READ_BUFFER, 1)
    BO_VAR(GLfloat,   GL_RED_BIAS, 1)
    BO_VAR(GLfloat,   GL_RED_SCALE, 1)
    BO_VAR(GLint,     GL_RENDER_MODE, 1)
    BO_VAR(BOboolean, GL_RESCALE_NORMAL, 1)

    BO_VAR(BOboolean, GL_SCISSOR_TEST, 1)
    BO_VAR(GLint,     GL_SCISSOR_BOX, 4)
    BO_VAR(void*,     GL_SELECTION_BUFFER_POINTER, 1)
    BO_VAR(GLint,     GL_SELECTION_BUFFER_SIZE, 1)
    BO_VAR(BOboolean, GL_SEPARABLE_2D, 1)
    BO_VAR(GLint,     GL_SHADE_MODEL, 1)
    BO_VAR(GLint,     GL_STENCIL_CLEAR_VALUE, 1)
    BO_VAR(GLint,     GL_STENCIL_FAIL, 1)
    BO_VAR(GLint,     GL_STENCIL_FUNC, 1)
    BO_VAR(GLint,     GL_STENCIL_PASS_DEPTH_FAIL, 1)
    BO_VAR(GLint,     GL_STENCIL_PASS_DEPTH_PASS, 1)
    BO_VAR(GLint,     GL_STENCIL_REF, 1)
    BO_VAR(BOboolean, GL_STENCIL_TEST, 1)
    BO_VAR(GLint,     GL_STENCIL_VALUE_MASK, 1)
    BO_VAR(GLint,     GL_STENCIL_WRITEMASK, 1)

    BO_VAR(BOboolean, GL_TEXTURE_1D, 1)
    BO_VAR(BOboolean, GL_TEXTURE_2D, 1)
    BO_VAR(BOboolean, GL_TEXTURE_3D, 1)
    BO_VAR(GLint,     GL_TEXTURE_BINDING_1D, 1)
    BO_VAR(GLint,     GL_TEXTURE_BINDING_2D, 1)
    BO_VAR(GLint,     GL_TEXTURE_BINDING_3D, 1)
    BO_VAR(BOboolean, GL_TEXTURE_COORD_ARRAY, 1)
    BO_VAR(void*,     GL_TEXTURE_COORD_ARRAY_POINTER, 1)
    BO_VAR(GLint,     GL_TEXTURE_COORD_ARRAY_SIZE, 1)
    BO_VAR(GLint,     GL_TEXTURE_COORD_ARRAY_STRIDE, 1)
    BO_VAR(GLint,     GL_TEXTURE_COORD_ARRAY_TYPE, 1)
    BO_VAR(BOboolean, GL_TEXTURE_GEN_Q, 1)
    BO_VAR(BOboolean, GL_TEXTURE_GEN_R, 1)
    BO_VAR(BOboolean, GL_TEXTURE_GEN_S, 1)
    BO_VAR(BOboolean, GL_TEXTURE_GEN_T, 1)
    BO_VAR(GLfloat,   GL_TEXTURE_MATRIX, 16)
    BO_VAR(GLint,     GL_TEXTURE_STACK_DEPTH, 1)

    BO_VAR(GLint,     GL_UNPACK_ALIGNMENT, 1)
    BO_VAR(GLint,     GL_UNPACK_IMAGE_HEIGHT, 1)
    BO_VAR(BOboolean, GL_UNPACK_LSB_FIRST, 1)
    BO_VAR(GLint,     GL_UNPACK_ROW_LENGTH, 1)
    BO_VAR(GLint,     GL_UNPACK_SKIP_IMAGES, 1)
    BO_VAR(GLint,     GL_UNPACK_SKIP_PIXELS, 1)
    BO_VAR(GLint,     GL_UNPACK_SKIP_ROWS, 1)
    BO_VAR(BOboolean, GL_UNPACK_SWAP_BYTES, 1)

    BO_VAR(BOboolean, GL_VERTEX_ARRAY, 1)
    BO_VAR(void*,     GL_VERTEX_ARRAY_POINTER, 1)
    BO_VAR(GLint,     GL_VERTEX_ARRAY_SIZE, 1)
    BO_VAR(GLint,     GL_VERTEX_ARRAY_STRIDE, 1)
    BO_VAR(GLint,     GL_VERTEX_ARRAY_TYPE, 1)
    BO_VAR(GLint,     GL_VIEWPORT, 4)

    BO_VAR(GLfloat,   GL_ZOOM_X, 1)
    BO_VAR(GLfloat,   GL_ZOOM_Y, 1)


    BOboolean mClipPlane_i; // FIXME: i is implementation dependant. we might require a list here
    bool mLight_i; // FIXME: i is implementation dependant (at least 8 - see GL_MAX_LIGHTS)
    int mPixelMap_x_Size; // FIXME what is x?



    // values outside of glGet()...
    MaterialStates mMaterialStates[2]; // front and back
//  int mTextureEnvMode; // glGetTexEnviv()
//  float mTextureEnvColor; // glGetTextureEnvColor()
    int mMaxLights;
    LightStates* mLightStates;

protected:
    QString listItem(GLenum key) const
    {
        QString k = mNameDict[key];
        if (k.isEmpty()) {
            boWarning() << k_funcinfo << "don't have key " << (int)key << endl;
            return QString(" = "); // AB: some methods require this format
        }
        return QString::fromLatin1("%1 = %2").arg(k).arg(value(key));
    }
private:
    QMap<int, QString> mValues;
    QMap<int, QString> mNameDict;
};

class BoGLQueryStatesPrivate
{
public:
    BoGLQueryStatesPrivate()
    {
    }
    ImplementationValues mImplementationValues;
    GLStates mGLStates;
};


BoGLQueryStates::BoGLQueryStates()
{
 d = new BoGLQueryStatesPrivate;
}

BoGLQueryStates::~BoGLQueryStates()
{
 delete d;
}


void BoGLQueryStates::init()
{
 d->mImplementationValues.getValues();
}

void BoGLQueryStates::getStates()
{
 d->mGLStates.getStates();
}

QStringList BoGLQueryStates::implementationValueList() const
{
 return d->mImplementationValues.list();
}

QStringList BoGLQueryStates::stateList()
{
 d->mGLStates.getStates();
 return d->mGLStates.list();
}

QStringList BoGLQueryStates::oldStateList() const
{
 return d->mGLStates.list();
}

QStringList BoGLQueryStates::oldStateEnabledList() const
{
 return d->mGLStates.listEnabled();
}

QStringList BoGLQueryStates::getDifferences(const QStringList& _l1, const QStringList& _l2)
{
 QStringList list;
 if (_l1.isEmpty() || _l1.isEmpty()) {
    return list;
 }
 QMap<QString, QString> m1;
 QMap<QString, QString> m2;
 QStringList::ConstIterator it;
 for (it = _l1.begin(); it != _l1.end(); ++it) {
    QString s = *it;
    QStringList l = QStringList::split(" = ", s);
    if (l.count() != 2) {
        boWarning() << k_funcinfo << "string " << s << " is not valid" << endl;
        continue;
    }
    m1.insert(l[0], l[1]);
 }
 for (it = _l2.begin(); it != _l2.end(); ++it) {
    QString s = *it;
    QStringList l = QStringList::split(" = ", s);
    if (l.count() != 2) {
        boWarning() << k_funcinfo << "string " << s << " is not valid" << endl;
        continue;
    }
    m2.insert(l[0], l[1]);
 }

 QMap<QString, QString>::Iterator i;
 for (i = m1.begin(); i != m1.end(); i++) {
    if (!m2.contains(*i)) {
        continue;
    }
    QString s1 = i.data();
    QString s2 = m2[i.key()];
    if (s1 == s2) {
        continue;
    }
    list.append(QString::fromLatin1("%1 = %2").arg(i.key()).arg(s1));
    list.append(QString::fromLatin1("%1 = %2").arg(i.key()).arg(s2));
 }
 return list;
}

/*
 * vim: et sw=4
 */
