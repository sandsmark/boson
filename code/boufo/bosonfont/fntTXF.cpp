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

#define QT_CLEAN_NAMESPACE

#include "fntLocal.h"
#include "../../bomemory/bodummymemory.h"
#include <bogl.h>
#include <boglx.h>

#include <bodebug.h>

#define DISABLE_MIPMAPS 1

static bool glIsValidContext( void )
{
  return ( glXGetCurrentContext() != NULL );
}

FILE *curr_image_fd;
int isSwapped = 0;

static void tex_make_mip_maps( GLubyte *image, int xsize,
                                     int ysize, int zsize )
{
  GLubyte *texels [ 20 ] ;   /* One element per level of MIPmap */

  for ( int l = 0 ; l < 20 ; l++ )
  {
    texels [ l ] = NULL;
  }

  texels [ 0 ] = image;

  int lev = 0;

#if !DISABLE_MIPMAPS
  for ( lev = 0 ; (( xsize >> (lev+1) ) != 0 ||
                   ( ysize >> (lev+1) ) != 0 ) ; lev++ )
  {
    /* Suffix '1' is the higher level map, suffix '2' is the lower level. */

    int l1 = lev;
    int l2 = lev+1;
    int w1 = xsize >> l1;
    int h1 = ysize >> l1;
    int w2 = xsize >> l2;
    int h2 = ysize >> l2;

    if ( w1 <= 0 ) { w1 = 1; }
    if ( h1 <= 0 ) { h1 = 1; }
    if ( w2 <= 0 ) { w2 = 1; }
    if ( h2 <= 0 ) { h2 = 1; }

    texels [ l2 ] = new GLubyte [ w2 * h2 * zsize ];

    for ( int x2 = 0 ; x2 < w2 ; x2++ )
    {
      for ( int y2 = 0 ; y2 < h2 ; y2++ )
      {
        int x1   = x2 + x2;
        int x1_1 = ( x1 + 1 ) % w1;
        int y1   = y2 + y2;
        int y1_1 = ( y1 + 1 ) % h1;
        for ( int c = 0 ; c < zsize ; c++ )
        {
          int t1 = texels [ l1 ] [ (y1   * w1 + x1  ) * zsize + c ];
          int t2 = texels [ l1 ] [ (y1_1 * w1 + x1  ) * zsize + c ];
          int t3 = texels [ l1 ] [ (y1   * w1 + x1_1) * zsize + c ];
          int t4 = texels [ l1 ] [ (y1_1 * w1 + x1_1) * zsize + c ];

          int t5 = (t1 + t2 + t3 + t4);
          // AB: now t5/=4; would be the correct next statement.
          // but then the mipmaps will look bad with small point sizes if you
          // use white color.
          // this can be solved by multiplying the values by 4, i.e. by not
          // dividing by 4.
#if 0
          // AB: this is an alternative solution. but i think the results are
          // not so good.
          t5 /= 4;
          t5 = qMax(t5, t1);
          t5 = qMax(t5, t2);
          if (t5 > 30) {
            t5 *= 2;
          }
#endif
          t5 = qMin(t5, 255);
          texels [ l2 ] [ (y2 * w2 + x2) * zsize + c ] = t5;
        }
      }
    }
  }
#endif

  texels [ lev+1 ] = NULL;

  if ( ! ((xsize & (xsize-1))==0) ||
       ! ((ysize & (ysize-1))==0) )
  {
    boError() << k_funcinfo << "TXFloader: TXF Map is not a power-of-two in size!" << endl;
    return;
  }

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  int map_level = 0;

#ifdef PROXY_TEXTURES_ARE_NOT_BROKEN
  int ww;

  do
  {
    glTexImage2D( GL_PROXY_TEXTURE_2D,
                     map_level, zsize, xsize, ysize, 0/* Border */,
                            (zsize==1)?GL_LUMINANCE:
                            (zsize==2)?GL_LUMINANCE_ALPHA:
                            (zsize==3)?GL_RGB:
                                       GL_RGBA,
                            GL_UNSIGNED_BYTE, NULL );

    glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_WIDTH, &ww );

    if ( ww == 0 )
    {
      delete[] texels [ 0 ];
      xsize >>= 1;
      ysize >>= 1;

      for ( int l = 0 ; texels [ l ] != NULL ; l++ )
      {
        texels [ l ] = texels [ l+1 ];
      }

      if ( xsize < 64 && ysize < 64 )
      {
        boError() << k_funcinfo << "FNT: OpenGL will not accept a font texture?!?" << endl;
        return;
      }
    }
  } while ( ww == 0 );
#endif

  for ( int i = 0 ; texels [ i ] != NULL ; i++ )
  {
    int w = xsize>>i;
    int h = ysize>>i;

    if ( w <= 0 ) { w = 1; }
    if ( h <= 0 ) { h = 1; }

    glTexImage2D( GL_TEXTURE_2D,
                     map_level, zsize, w, h, 0/* Border */,
                            (zsize==1)?GL_LUMINANCE:
                            (zsize==2)?GL_LUMINANCE_ALPHA:
                            (zsize==3)?GL_RGB:
                                       GL_RGBA,
                            GL_UNSIGNED_BYTE, (GLvoid *) texels[i] );
    map_level++;
    delete [] texels [ i ];
  }
}



int BofntTexFont::loadTXF( const char *fname, GLenum mag, GLenum min )
{
  if ( ! glIsValidContext() )
  {
    boError() << k_funcinfo << "FNT font loader called without a valid OpenGL context." << endl;
    return FNT_FALSE;
  }

  FILE *fd;

  if ( (fd = fopen ( fname, "rb" )) == NULL )
  {
    boError() << k_funcinfo << "fntLoadTXF: Failed to open '" << fname << "' for reading." << endl;
    return FNT_FALSE;
  }

  curr_image_fd = fd;

  unsigned char magic [ 4 ];

  if ( (int)fread( &magic, sizeof (unsigned int), 1, fd ) != 1 )
  {
    boError() << k_funcinfo << "fntLoadTXF: '" << fname << "' an empty file!" << endl;
    return FNT_FALSE;
  }

  if ( magic [ 0 ] != 0xFF || magic [ 1 ] != 't' ||
       magic [ 2 ] != 'x'  || magic [ 3 ] != 'f' )
  {
    boError() << k_funcinfo << "fntLoadTXF: '" << fname << "' is not a 'txf' font file." << endl;
    return FNT_FALSE;
  }

  isSwapped  = 0;
  int endianness = _fnt_readInt();

  isSwapped = ( endianness != 0x12345678 );

  int format = _fnt_readInt();
  int tex_width = _fnt_readInt();
  int tex_height = _fnt_readInt();
  int max_height = _fnt_readInt();
                    _fnt_readInt();
  int num_glyphs = _fnt_readInt();

  int w = tex_width;
  int h = tex_height;

  float xstep = 0.5f / (float) w;
  float ystep = 0.5f / (float) h;

  int i, j;

  /*
    Load the TXF_Glyph array
  */

  TXF_Glyph glyph;

  for ( i = 0 ; i < num_glyphs ; i++ )
  {
    glyph.ch      = _fnt_readShort();

    glyph.w       = _fnt_readByte();
    glyph.h       = _fnt_readByte();
    glyph.x_off   = _fnt_readByte();
    glyph.y_off   = _fnt_readByte();
    glyph.step    = _fnt_readByte();
    glyph.unknown = _fnt_readByte();
    glyph.x       = _fnt_readShort();
    glyph.y       = _fnt_readShort();

    setGlyph( (char) glyph.ch,
          (float)  glyph.step              / (float) max_height,
          (float)  glyph.x                 / (float) w + xstep,
          (float)( glyph.x + glyph.w )     / (float) w + xstep,
          (float)  glyph.y                 / (float) h + ystep,
          (float)( glyph.y + glyph.h )     / (float) h + ystep,
          (float)  glyph.x_off             / (float) max_height,
          (float)( glyph.x_off + glyph.w ) / (float) max_height,
          (float)  glyph.y_off             / (float) max_height,
          (float)( glyph.y_off + glyph.h ) / (float) max_height );
  }

  exists [ (int)' ' ] = 0;

  /*
    Load the image part of the file
  */

  int ntexels = w * h;

  unsigned char *teximage;
  unsigned char *texbitmap;

  switch ( format )
  {
    case FNT_BYTE_FORMAT:
      {
        unsigned char *orig = new unsigned char [ ntexels ];

        if ( (int)fread ( orig, 1, ntexels, fd ) != ntexels )
        {
          boError() << k_funcinfo << "fntLoadTXF: Premature EOF in " << fname << endl;
          return FNT_FALSE ;
        }

        teximage = new unsigned char [ 2 * ntexels ] ;

        for ( i = 0 ; i < ntexels ; i++ )
        {
          teximage [ i*2     ] = orig [ i ];
          teximage [ i*2 + 1 ] = orig [ i ];
        }

        delete [] orig;
      }
      break;
 
    case FNT_BITMAP_FORMAT:
      {
        int stride = (w + 7) >> 3;

        texbitmap = new unsigned char [ stride * h ];

        if ( (int)fread ( texbitmap, 1, stride * h, fd ) != stride * h )
        {
          delete [] texbitmap;
          boError() << k_funcinfo << "fntLoadTXF: Premature EOF in " << fname << endl;
          return FNT_FALSE;
        }

        teximage = new unsigned char [ 2 * ntexels ] ;

        for ( i = 0 ; i < 2 * ntexels ; i++ )
        {
          teximage [ i ] = 0 ;
        }

        for (i = 0; i < h; i++)
        {
          for (j = 0; j < w; j++)
          {
            if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7)))
            {
              teximage[(i * w + j) * 2    ] = 255;
              teximage[(i * w + j) * 2 + 1] = 255;
            }
          }
        }

        delete [] texbitmap;
      }
      break;

    default:
      boError() << k_funcinfo << "fntLoadTXF: Unrecognised format type in " << fname << endl;
      return FNT_FALSE;
  }

  fclose( fd );

  fixed_pitch = 0;

  glGenTextures( 1, & texture );
  glBindTexture( GL_TEXTURE_2D, texture );

  tex_make_mip_maps( teximage, w, h, 2 );

  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
#if DISABLE_MIPMAPS
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
#endif
  glBindTexture( GL_TEXTURE_2D, 0 );

  return FNT_TRUE;
}

/*
 * vim: et sw=2
 */
