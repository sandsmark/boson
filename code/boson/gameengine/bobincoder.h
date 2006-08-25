/*
 * Copyright (c) 2003 by David Kamphausen
 *
 * bobincoder.h
 * by David Kamphausen (david.kamphausen@web.de)
 *
 * This file is free software; you can redistribute it and/or use it and/or
 * modify it under the terms of the GNU General Public License as published
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 */

#ifndef BOBINCODING_H
#define BOBINCODING_H

class QBitArray;
class QString;

/**
 * @short Encode a bitvector to a string and decode it again.
 *
 * You will need only two methods here: @ref toCoded encodes a bit array into a
 * string and @ref toBinary reverses (i.e. decodes) what @ref toCoded has
 * produced.
 *
 * @author David Kamphausen <david.kamphausen@web.de>
 **/
class BoBinCoder
{
 public:
  /**
   * Decode the string @p coded. @p coded must have been created using @ref
   * toCoded.
   **/
  static QBitArray toBinary(const QString &coded);

  /**
   * Encode the bit array @p binary into a string. The string will contain only
   * usual text characters (such as a-z, A-Z, 0-9 and a few special chars). It
   * can be safely used in text files, such as XML.
   *
   * This class codes 6 bits into a single byte, so the loss is only about 25%,
   * but you gain a huge amount of flexibility, as the binary data cannot safely
   * be embedded into text files - the coded string can be.
   *
   * This function will also add newlines after a certain number of chars, @ref
   * toBinary will automatically remove them.
   **/
  static QString toCoded(const QBitArray &binary);

  static void toCharArray(const QString &coded, char* array, int len);
  static QString toCoded(const char* binary, int len);

 private:
  static QString addNewLines(QString a);
  static QString removeNewLines(QString a);

  static char code64(unsigned int i);
  static unsigned int recode64(unsigned char c);

  static QString BinVector2MYBASE64(const char *array,const size_t len);

  static QString Vector2MYBASE64(const char*array,const size_t len);

  static void MYBASE642BinVector(const QString &s,size_t size,char *buffer);

  static void MYBASE642Vector(const QString &s,size_t size,char *buffer);
  // get real size of coded string
  static size_t getBufferSize(const QString &a);


  static const char*codeBase;
};

#endif

