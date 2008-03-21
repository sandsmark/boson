/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/

// AB: this was shamelessy stolen from plib-1.6.0 and converted to be used with
// boson's code instead of plib's.


#ifndef _BO_FNT_H_
#define _BO_FNT_H_

#include "fakedsg.h"

#include <bogl.h>


#define FNTMAX_CHAR  256
#define FNT_TRUE  1
#define FNT_FALSE 0


class BofntFont
{
public:
  BofntFont();

  virtual ~BofntFont();

  virtual void getBBox( const char *s, float pointsize, float italic,
                                  float *left, float *right,
                                  float *bot , float *top  ) = 0;
  virtual void putch( sgVec3 curpos, float pointsize, float italic, char  c ) = 0;
  virtual void puts( sgVec3 curpos, float pointsize, float italic, const char *s ) = 0;
  virtual void begin() = 0;
  virtual void end() = 0;

  virtual int load( const char *fname, GLenum mag = GL_NEAREST,
                                  GLenum min = GL_LINEAR_MIPMAP_LINEAR ) = 0;

  virtual void setFixedPitch( int fix ) = 0;
  virtual int isFixedPitch() const = 0;

  virtual void setWidth( float w ) = 0;
  virtual void setGap( float g ) = 0;

  virtual float getWidth() const = 0;
  virtual float getGap() const = 0;
  virtual float getWidth(char c, float pointSize) const = 0;

  /**
   * Added by AB for boson.
   * @return The width of the widest character in this font.
   **/
  virtual float getWidestChar() const = 0;

  virtual int hasGlyph( char c ) const = 0;
} ;


class BofntTexFont : public BofntFont
{
private:
  GLuint texture;
  int bound;
  int fixed_pitch;

  float width; /* The width of a character in fixed-width mode */
  float gap; /* Set the gap between characters */
  float widestChar; /* The widest char in this font */

  int exists[ FNTMAX_CHAR ]; /* TRUE if character exists in tex-map */

  /*
    The quadrilaterals that describe the characters
    in the font are drawn with the following texture
    and spatial coordinates. The texture coordinates
    are in (S,T) space, with (0,0) at the bottom left
    of the image and (1,1) at the top-right.

    The spatial coordinates are relative to the current
    'cursor' position. They should be scaled such that
    1.0 represent the height of a capital letter. Hence,
    characters like 'y' which have a descender will be
    given a negative v_bot. Most capitals will have
    v_bot==0.0 and v_top==1.0.
  */

  /* Nominal baseline widths */

  float widths[ FNTMAX_CHAR ];

  /* Texture coordinates */

  float t_top[ FNTMAX_CHAR ]; /* Top edge of each character [0..1] */
  float t_bot[ FNTMAX_CHAR ]; /* Bottom edge of each character [0..1] */
  float t_left[ FNTMAX_CHAR ]; /* Left edge of each character [0..1] */
  float t_right[ FNTMAX_CHAR ]; /* Right edge of each character [0..1] */

  /* Vertex coordinates. */

  float v_top[ FNTMAX_CHAR ];
  float v_bot[ FNTMAX_CHAR ];
  float v_left[ FNTMAX_CHAR ];
  float v_right[ FNTMAX_CHAR ];

  void bind_texture ()
  {
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texture );
  }

  float low_putch( sgVec3 curpos, float pointsize, float italic, char c );

  int loadTXF( const char *fname, GLenum mag = GL_NEAREST,
                             GLenum min = GL_LINEAR_MIPMAP_LINEAR );
public:

  BofntTexFont()
  {
    bound = FNT_FALSE;
    fixed_pitch = FNT_TRUE;
    texture =   0;
    width =  1.0f;
    gap =  0.1f;
    widestChar = 0.0f;

    memset( exists, FNT_FALSE, FNTMAX_CHAR * sizeof(int) );
  }

  BofntTexFont( const char *fname, GLenum mag = GL_NEAREST, 
                            GLenum min = GL_LINEAR_MIPMAP_LINEAR ) : BofntFont()
  {
    bound = FNT_FALSE;
    fixed_pitch = FNT_TRUE;
    texture =   0;
    width =  1.0f;
    gap =  0.1f;

    memset( exists, FNT_FALSE, FNTMAX_CHAR * sizeof(int) );
    load( fname, mag, min );
  }

  ~BofntTexFont()
  {
    if ( texture != 0 )
    {
      glDeleteTextures( 1, &texture );
    }
  }

  int load( const char *fname, GLenum mag = GL_NEAREST,
                          GLenum min = GL_LINEAR_MIPMAP_LINEAR );

  void setFixedPitch( int fix ) { fixed_pitch = fix;  }
  int isFixedPitch() const    { return fixed_pitch; }

  void setWidth( float w ) { width = w; }
  void setGap( float g ) { gap = g; }

  float getWidth() const { return width; }
  float getGap() const { return gap; }

  /**
   * Added by AB for boson.
   * @return The width of the character c. '\n' and unknown chars are 0.0f
   **/
  float getWidth(char c, float pointSize) const
  {
    if (fixed_pitch) {
      return width;
    }
    if (c == ' ') {
      return pointSize / 2.0f;
    } else if (c == '\n') {
      return 0.0f;
    } else if (!exists[(int)c]) {
      if (c >= 'A' && c <= 'Z') {
        c = c - 'A' + 'a';
      } else if (c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
      }
      if (!exists[(int)c]) {
        return 0.0f;
      }
    }
    return widths[(int)c] * pointSize;
  }

  /**
   * See @ref BofntFont::getWidestChar. Only valid once a font is loaded.
   **/
  virtual float getWidestChar() const { return isFixedPitch() ? width : widestChar; }


  void setGlyph( char c, float wid,
                  float tex_left, float tex_right,
                  float tex_bot , float tex_top  ,
                  float vtx_left, float vtx_right,
                  float vtx_bot , float vtx_top  );

  int getGlyph( char c, float* wid,
                  float *tex_left = NULL, float *tex_right = NULL,
                  float *tex_bot  = NULL, float *tex_top   = NULL,
                  float *vtx_left = NULL, float *vtx_right = NULL,
                  float *vtx_bot  = NULL, float *vtx_top   = NULL) ;

  int hasGlyph( char c ) const { return exists[ (int)c ]; }

  void getBBox ( const char *s, float pointsize, float italic,
                 float *left, float *right,
                 float *bot , float *top  ) ;

  void begin()
  {
    bind_texture();
    bound = FNT_TRUE;
  }

  void end()
  {
    bound = FNT_FALSE;
  }

  void puts( sgVec3 curpos, float pointsize, float italic, const char *s );

  void putch( sgVec3 curpos, float pointsize, float italic, char c )
  {
    if ( ! bound )
    {
      bind_texture();
    }

    low_putch( curpos, pointsize, italic, c );
  }

};


class BofntRenderer
{
  BofntFont *font;

  sgVec3 curpos;

  float pointsize;
  float italic;

public:
  BofntRenderer()
  {
    start2f ( 0.0f, 0.0f );
    font = NULL;
    pointsize = 10;
    italic = 0;
  }

  void start3fv( sgVec3 pos ) { sgCopyVec3( curpos, pos ); }
  void start2fv( sgVec2 pos ) { sgCopyVec2( curpos, pos ); curpos[2]=0.0f; }
  void start2f( float x, float y ) { sgSetVec3( curpos, x, y, 0.0f ); }
  void start3f( float x, float y, float z ) { sgSetVec3( curpos, x, y, z ); }

  void getCursor( float *x, float *y, float *z ) const
  {
    if ( x != NULL ) *x = curpos [ 0 ] ;
    if ( y != NULL ) *y = curpos [ 1 ] ;
    if ( z != NULL ) *z = curpos [ 2 ] ;
  }

  void setFont( BofntFont *f ) { font = f; }
  BofntFont *getFont() const { return font; }

  void setSlant( float i ) { italic    = i; }
  void setPointSize( float p ) { pointsize = p; }

  float getSlant() const { return italic; }
  float getPointSize() const { return pointsize; }

  void begin() { font->begin(); }
  void end() { font->end(); }

  void putch( char  c ) { font->putch( curpos, pointsize, italic, c ); }
  void puts( const char *s ) { font->puts( curpos, pointsize, italic, s ); }
} ;


#endif

/*
 * vim: et sw=2
 */
