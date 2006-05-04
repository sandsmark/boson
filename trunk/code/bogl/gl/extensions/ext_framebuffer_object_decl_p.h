/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks <rivolaks@hot.ee>
    Copyright (C) 2006 Andreas Beckermann <b_mann@gmx.de>

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

#ifndef EXT_FRAMEBUFFER_OBJECT_DECL_P_H
#define EXT_FRAMEBUFFER_OBJECT_DECL_P_H

// note: compliant OpenGL implementations don't have to implement an extension!

extern "C" {
	// GL typedefs
	typedef GLboolean (*_glIsRenderBufferEXT) (GLuint);
	typedef GLvoid (*_glBindRenderbufferEXT) (GLenum, GLuint);
	typedef GLvoid (*_glDeleteRenderbuffersEXT) (GLsizei, const GLuint *);
	typedef GLvoid (*_glGenRenderbuffersEXT) (GLsizei, GLuint *);
	typedef GLvoid (*_glRenderbufferStorageEXT) (GLenum, GLenum, GLsizei, GLsizei);
	typedef GLvoid (*_glGetRenderbufferParameterivEXT) (GLenum, GLenum, GLsizei, GLint*);
	typedef GLboolean (*_glIsFrameBufferEXT) (GLuint);
	typedef GLvoid (*_glBindFramebufferEXT) (GLenum, GLuint);
	typedef GLvoid (*_glDeleteFramebuffersEXT) (GLsizei, const GLuint *);
	typedef GLvoid (*_glGenFramebuffersEXT) (GLsizei, GLuint *);
	typedef GLenum (*_glCheckFramebufferStatusEXT) (GLenum);
	typedef GLvoid (*_glFramebufferTexture1DEXT) (GLenum, GLenum, GLenum, GLuint, GLint);
	typedef GLvoid (*_glFramebufferTexture2DEXT) (GLenum, GLenum, GLenum, GLuint, GLint);
	typedef GLvoid (*_glFramebufferTexture3DEXT) (GLenum, GLenum, GLenum, GLuint, GLint);
	typedef GLvoid (*_glFramebufferRenderbufferEXT) (GLenum, GLenum, GLenum, GLuint);
	typedef GLvoid (*_glFramebufferAttachmentParameterivEXT) (GLenum, GLenum, GLenum, GLint*);
	typedef GLvoid (*_glGenerateMipmapEXT) (GLenum);




	// GL function pointers
	extern _glIsRenderBufferEXT bo_glIsRenderBufferEXT;
	extern _glBindRenderbufferEXT bo_glBindRenderbufferEXT;
	extern _glDeleteRenderbuffersEXT bo_glDeleteRenderbuffersEXT;
	extern _glGenRenderbuffersEXT bo_glGenRenderbuffersEXT;
	extern _glRenderbufferStorageEXT bo_glRenderbufferStorageEXT;
	extern _glGetRenderbufferParameterivEXT bo_glGetRenderbufferParameterivEXT;
	extern _glIsFrameBufferEXT bo_glIsFrameBufferEXT;
	extern _glBindFramebufferEXT bo_glBindFramebufferEXT;
	extern _glDeleteFramebuffersEXT bo_glDeleteFramebuffersEXT;
	extern _glGenFramebuffersEXT bo_glGenFramebuffersEXT;
	extern _glCheckFramebufferStatusEXT bo_glCheckFramebufferStatusEXT;
	extern _glFramebufferTexture1DEXT bo_glFramebufferTexture1DEXT;
	extern _glFramebufferTexture2DEXT bo_glFramebufferTexture2DEXT;
	extern _glFramebufferTexture3DEXT bo_glFramebufferTexture3DEXT;
	extern _glFramebufferRenderbufferEXT bo_glFramebufferRenderbufferEXT;
	extern _glFramebufferAttachmentParameterivEXT bo_glFramebufferAttachmentParameterivEXT;
	extern _glGenerateMipmapEXT bo_glGenerateMipmapEXT;
}; // extern "C"



#define GL_FRAMEBUFFER_EXT                     0x8D40
#define GL_RENDERBUFFER_EXT                    0x8D41
#define GL_STENCIL_INDEX1_EXT                  0x8D46
#define GL_STENCIL_INDEX4_EXT                  0x8D47
#define GL_STENCIL_INDEX8_EXT                  0x8D48
#define GL_STENCIL_INDEX16_EXT                 0x8D49
#define GL_RENDERBUFFER_WIDTH_EXT              0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT             0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT    0x8D44
#define GL_RENDERBUFFER_RED_SIZE_EXT           0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT         0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT          0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT         0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT         0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT       0x8D55
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT            0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT            0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT          0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT  0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT     0x8CD4
#define GL_COLOR_ATTACHMENT0_EXT               0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT               0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT               0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT               0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT               0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT               0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT               0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT               0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT               0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT               0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT              0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT              0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT              0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT              0x8CED
#define GL_COLOR_ATTACHMENT14_EXT              0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT              0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT                0x8D00
#define GL_STENCIL_ATTACHMENT_EXT              0x8D20
#define GL_FRAMEBUFFER_COMPLETE_EXT                          0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT             0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT     0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT             0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT                0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT            0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT            0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                       0x8CDD
#define GL_FRAMEBUFFER_BINDING_EXT             0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT            0x8CA7
#define GL_MAX_COLOR_ATTACHMENTS_EXT           0x8CDF
#define GL_MAX_RENDERBUFFER_SIZE_EXT           0x84E8
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT   0x0506



#endif
