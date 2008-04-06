/* This file is part of the KDE project
   Copyright (C) 2002-2003 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "pcx.h"

#include <qimage.h>

#include <kdebug.h>

static QDataStream &operator>>( QDataStream &s, RGB &rgb )
{
  s >> rgb.r >> rgb.g >> rgb.b;

  return s;
}

static QDataStream &operator>>( QDataStream &s, Palette &pal )
{
  for ( int i=0; i<16; ++i )
    s >> pal.rgb[ i ];

  return s;
}

static QDataStream &operator>>( QDataStream &s, PCXHEADER &ph )
{
  s >> ph.Manufacturer;
  s >> ph.Version;
  s >> ph.Encoding;
  s >> ph.Bpp;
  s >> ph.XMin >> ph.YMin >> ph.XMax >> ph.YMax;
  s >> ph.HDpi >> ph.YDpi;
  s >> ph.ColorMap;
  s >> ph.Reserved;
  s >> ph.NPlanes;
  s >> ph.BytesPerLine;
  s >> ph.PaletteInfo;
  s >> ph.HScreenSize;
  s >> ph.VScreenSize;

  return s;
}

static QDataStream &operator<<( QDataStream &s, const RGB &rgb )
{
  s << rgb.r << rgb.g << rgb.b;

  return s;
}

static QDataStream &operator<<( QDataStream &s, const Palette &pal )
{
  for ( int i=0; i<16; ++i )
    s << pal.rgb[ i ];

  return s;
}

static QDataStream &operator<<( QDataStream &s, const PCXHEADER &ph )
{
  s << ph.Manufacturer;
  s << ph.Version;
  s << ph.Encoding;
  s << ph.Bpp;
  s << ph.XMin << ph.YMin << ph.XMax << ph.YMax;
  s << ph.HDpi << ph.YDpi;
  s << ph.ColorMap;
  s << ph.Reserved;
  s << ph.NPlanes;
  s << ph.BytesPerLine;
  s << ph.PaletteInfo;
  s << ph.HScreenSize;
  s << ph.VScreenSize;

  quint8 byte = 0;
  for ( int i=0; i<54; ++i )
    s << byte;

  return s;
}

static PCXHEADER header;
static QImage img;
static quint16 w, h;

void PCXHEADER::reset()
{
  QByteArray dummy( 128 );
  dummy.fill( 0 );
  QDataStream s( dummy, QIODevice::ReadOnly );
  s >> *this;
}

static void readLine( QDataStream &s, QByteArray &buf )
{
  quint32 i=0;
  quint32 size = buf.size();
  quint8 byte, count;

  if ( header.Encoding == 1 )
  {
    // Uncompress the image data
    while ( i < size )
    {
      count = 1;
      s >> byte;
      if ( byte > 0xc0 )
      {
        count = byte - 0xc0;
        s >> byte;
      }
      while ( count-- && i < size )
        buf[ i++ ] = byte;
    }
  }
  else
  {
    // Image is not compressed (possible?)
    while ( i < size )
    {
      s >> byte;
      buf[ i++ ] = byte;
    }
  }
}

static void readImage1( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine );

  img.create( w, h, 1, 2, QImage::BigEndian );

  for ( int y=0; y<h; ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, buf );

    for ( int x=0; x<header.BytesPerLine; ++x )
      *( img.scanLine( y )+x ) = buf[ x ];
  }

  // Set the color palette
  img.setColor( 0, qRgb( 0, 0, 0 ) );
  img.setColor( 1, qRgb( 255, 255, 255 ) );
}

static void readImage4( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine*4 );
  QByteArray pixbuf( w );

  img.create( w, h, 8, 16, QImage::IgnoreEndian );

  for ( int y=0; y<h; ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    pixbuf.fill( 0 );
    readLine( s, buf );

    for ( int i=0; i<4; i++ )
    {
      quint32 offset = i*header.BytesPerLine;
      for ( int x=0; x<w; ++x )
        if ( buf[ offset + ( x/8 ) ] & ( 128 >> ( x%8 ) ) )
          pixbuf[ x ] += ( 1 << i );
    }

    uchar *p = img.scanLine( y );

    for ( int x=0; x<w; ++x )
      *p++ = pixbuf[ x ];
  }

  // Read the palette
  for ( int i=0; i<16; ++i )
    img.setColor( i, header.ColorMap.color( i ) );
}

static void readImage8( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine );

  img.create( w, h, 8, 256, QImage::IgnoreEndian );

  for ( int y=0; y<h; ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, buf );

    uchar *p = img.scanLine( y );

    for ( int x=0; x<header.BytesPerLine; ++x )
      *p++ = buf[ x ];
  }

  quint8 flag;
  s >> flag;
  kDebug() << "Flag: " << flag << endl;

  if ( flag == 12 && header.Version == 5 )
  {
    // Read the palette
    quint8 r, g, b;
    for ( int i=0; i<256; ++i )
    {
      s >> r >> g >> b;
      img.setColor( i, qRgb( r, g, b ) );
    }
  }
}

static void readImage24( QDataStream &s )
{
  QByteArray r_buf( header.BytesPerLine );
  QByteArray g_buf( header.BytesPerLine );
  QByteArray b_buf( header.BytesPerLine );

  img.create( w, h, 32 );

  for ( int y=0; y<h; ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, r_buf );
    readLine( s, g_buf );
    readLine( s, b_buf );

    uint *p = ( uint * )img.scanLine( y );

    for ( int x=0; x<header.BytesPerLine; ++x )
      *p++ = qRgb( r_buf[ x ], g_buf[ x ], b_buf[ x ] );
  }
}

void kimgio_pcx_read( QImageIO *io )
{
  QDataStream s( io->ioDevice() );
  s.setByteOrder( QDataStream::LittleEndian );

  if ( s.device()->size() < 128 )
  {
    io->setStatus( -1 );
    return;
  }

  s >> header;

  if ( header.Manufacturer != 10 || s.atEnd())
  {
    io->setStatus( -1 );
    return;
  }

  w = ( header.XMax-header.XMin ) + 1;
  h = ( header.YMax-header.YMin ) + 1;

  img.reset();

  kDebug( 399 ) << "Manufacturer: " << header.Manufacturer << endl;
  kDebug( 399 ) << "Version: " << header.Version << endl;
  kDebug( 399 ) << "Encoding: " << header.Encoding << endl;
  kDebug( 399 ) << "Bpp: " << header.Bpp << endl;
  kDebug( 399 ) << "Width: " << w << endl;
  kDebug( 399 ) << "Height: " << h << endl;
  kDebug( 399 ) << "Window: " << header.XMin << "," << header.XMax << "," 
            << header.YMin << "," << header.YMax << endl;
  kDebug( 399 ) << "BytesPerLine: " << header.BytesPerLine << endl;
  kDebug( 399 ) << "NPlanes: " << header.NPlanes << endl;

  // Skip the rest of the header
  quint8 byte;
  while ( s.device()->at() < 128 )
    s >> byte;

  if ( header.Bpp == 1 && header.NPlanes == 1 )
  {
    readImage1( s );
  }
  else if ( header.Bpp == 1 && header.NPlanes == 4 )
  {
    readImage4( s );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 1 )
  {
    readImage8( s );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 3 )
  {
    readImage24( s );
  }

  kDebug( 399 ) << "Image Bytes: " << img.numBytes() << endl;
  kDebug( 399 ) << "Image Bytes Per Line: " << img.bytesPerLine() << endl;
  kDebug( 399 ) << "Image Depth: " << img.depth() << endl;

  if ( !img.isNull() )
  {
    io->setImage( img );
    io->setStatus( 0 );
  }
  else
  {
    io->setStatus( -1 );
  }
}

static void writeLine( QDataStream &s, QByteArray &buf )
{
  quint32 i = 0;
  quint32 size = buf.size();
  quint8 count, data;
  char byte;

  while ( i < size )
  {
    count = 1;
    byte = buf[ i++ ];

    while ( ( i < size ) && ( byte == buf[ i ] ) && ( count < 63 ) )
    {
      ++i;
      ++count;
    }

    data = byte;

    if ( count > 1 || data >= 0xc0 )
    {
      count |= 0xc0;
      s << count;
    }

    s << data;
  }
}

static void writeImage1( QDataStream &s )
{
  img = img.convertBitOrder( QImage::BigEndian );

  header.Bpp = 1;
  header.NPlanes = 1;
  header.BytesPerLine = img.bytesPerLine();

  header.ColorMap.setColor( 0, qRgb( 0, 0, 0 ) );
  header.ColorMap.setColor( 1, qRgb( 255, 255, 255 ) );

  s << header;

  QByteArray buf( header.BytesPerLine );

  for ( int y=0; y<h; ++y )
  {
    quint8 *p = img.scanLine( y );

    for ( int i=0; i<header.BytesPerLine; ++i )
      buf[ i ] = p[ i ];

    writeLine( s, buf );
  }
}

static void writeImage4( QDataStream &s )
{
  header.Bpp = 1;
  header.NPlanes = 4;
  header.BytesPerLine = w/8;

  for ( int i=0; i<16; ++i )
    header.ColorMap.setColor( i, img.color( i ) );

  s << header;

  QByteArray buf[ 4 ];

  for ( int i=0; i<4; ++i )
      buf[ i ].resize( header.BytesPerLine );

  for ( int y=0; y<h; ++y )
  {
    quint8 *p = img.scanLine( y );

    for ( int i=0; i<4; ++i )
      buf[ i ].fill( 0 );

    for ( int x=0; x<w; ++x )
    {
      for ( int i=0; i<4; ++i )
        if ( *( p+x ) & ( 1 << i ) )
          buf[ i ][ x/8 ] |= 1 << ( 7-x%8 );
    }

    for ( int i=0; i<4; ++i )
      writeLine( s, buf[ i ] );
  }
}

static void writeImage8( QDataStream &s )
{
  header.Bpp = 8;
  header.NPlanes = 1;
  header.BytesPerLine = img.bytesPerLine();

  s << header;

  QByteArray buf( header.BytesPerLine );

  for ( int y=0; y<h; ++y )
  {
    quint8 *p = img.scanLine( y );

    for ( int i=0; i<header.BytesPerLine; ++i )
      buf[ i ] = p[ i ];

    writeLine( s, buf );
  }

  // Write palette flag
  quint8 byte = 12;
  s << byte;

  // Write palette
  for ( int i=0; i<256; ++i )
    s << RGB( img.color( i ) );
}

static void writeImage24( QDataStream &s )
{
  header.Bpp = 8;
  header.NPlanes = 3;
  header.BytesPerLine = w;

  s << header;

  QByteArray r_buf( w );
  QByteArray g_buf( w );
  QByteArray b_buf( w );

  for ( int y=0; y<h; ++y )
  {
    uint *p = ( uint * )img.scanLine( y );

    for ( int x=0; x<w; ++x )
    {
      QRgb rgb = *p++;
      r_buf[ x ] = qRed( rgb );
      g_buf[ x ] = qGreen( rgb );
      b_buf[ x ] = qBlue( rgb );
    }

    writeLine( s, r_buf );
    writeLine( s, g_buf );
    writeLine( s, b_buf );
  }
}

void kimgio_pcx_write( QImageIO *io )
{
  QDataStream s( io->ioDevice() );
  s.setByteOrder( QDataStream::LittleEndian );

  img = io->image();

  w = img.width();
  h = img.height();

  kDebug( 399 ) << "Width: " << w << endl;
  kDebug( 399 ) << "Height: " << h << endl;
  kDebug( 399 ) << "Depth: " << img.depth() << endl;
  kDebug( 399 ) << "BytesPerLine: " << img.bytesPerLine() << endl;
  kDebug( 399 ) << "Num Colors: " << img.numColors() << endl;

  header.Manufacturer = 10;
  header.Version = 5;
  header.Encoding = 1;
  header.XMin = 0;
  header.YMin = 0;
  header.XMax = w-1;
  header.YMax = h-1;
  header.HDpi = 300;
  header.YDpi = 300;
  header.Reserved = 0;
  header.PaletteInfo =1;

  if ( img.depth() == 1 )
  {
    writeImage1( s );
  }
  else if ( img.depth() == 8 && img.numColors() <= 16 )
  {
    writeImage4( s );
  }
  else if ( img.depth() == 8 )
  {
    writeImage8( s );
  }
  else if ( img.depth() == 32 )
  {
    writeImage24( s );
  }

  io->setStatus( 0 );
}

/* vim: et sw=2 ts=2
*/

