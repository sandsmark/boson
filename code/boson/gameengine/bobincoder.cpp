/*
 * Copyright (c) 2003 by David Kamphausen
 *
 * bobincoder.cc
 * by David Kamphausen (david.kamphausen@web.de)
 *
 * This file is free software; you can redistribute it and/or use it and/or
 * modify it under the terms of the GNU General Public License as published
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

#include <qbitarray.h>
#include <qstring.h>

#include <assert.h> // FIXME: remove! no abort() calls on errors!

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"

using namespace std;

#include "bobincoder.h"

const char*BoBinCoder::codeBase="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_:";

char BoBinCoder::code64(unsigned int i)
{
  assert(i<64);
  return codeBase[i];
}

unsigned int BoBinCoder::recode64(unsigned char c)
{
  unsigned int i;
  for(i=0;i<64;i++)
    if(codeBase[i]==c)
      return i;

  assert(i<64);
  return i;
}

// Code 6-bits in one byte
// input is an array on length len, which contains only 0s and 1s (!!!)
// This function isn't used at the moment - so you can discard it, if
// you want
QString BoBinCoder::BinVector2MYBASE64(const char *array,const size_t len)
{
  size_t i;
  unsigned int value=0;
  size_t bitcount=0;
  QString a;

  for(i=0;i!=len;i++)
    {
      unsigned int actValue=array[i];
      assert(actValue==1 || actValue==0);
      actValue<<=bitcount;
      value|=actValue;
      bitcount++;
      if(bitcount==6)
	{
	  a+=char(code64(value));
	  bitcount=0;
	  value=0;
	}
    }
  a+=char(code64(value));

  return a;
}

// code 6 bits in one byte
// len*8 bits are encoded - so the output string has length len*8/6
QString BoBinCoder::Vector2MYBASE64(const char*array,const size_t len)
{
  // buffer
  size_t actBits=0;
  // buffer len
  size_t actBitsLen=0;

  // input ptr
  size_t pos=0;


  QString ostr;

  while(pos<len)
    {
      if(actBitsLen<6)
	{
	  // read next byte from input
	  size_t actValue=array[pos++];
	  actValue&=0xFF;

	  actValue<<=actBitsLen;

	  actBits|=actValue;
	  actBitsLen+=8;
	}
      // take lower 6 bits
      size_t output=actBits&0x3F;
      // shift buffer
      actBits>>=6;
      // dec buffer len
      actBitsLen-=6;

      ostr+=char(code64(output));
    }

  if(actBitsLen>0)
    {
      // take lower 6 bits - simply take the rest
      size_t output=actBits&0x3F;

      ostr+=char(code64(output));
    }
  return ostr;
}

// note that the buffer must have a length of 'size'!
// This function isn't used at the moment - so you can discard it, if
// you want
void BoBinCoder::MYBASE642BinVector(const QString &s,size_t size,char *buffer)
{
  size_t pos=0;

  QByteArray b = s.toUtf8();
  for(int i=0;i<b.length();i++)
    {
      unsigned int value=recode64(b[i]);
      for(size_t bitcount=0;bitcount<6 && pos<size ;bitcount++)
	{
	  unsigned actValue=value>>bitcount;
	  actValue&=1;
	  buffer[pos++]=actValue;
	}
    }
}

// note that the buffer must have at least a size of 'size'
void BoBinCoder::MYBASE642Vector(const QString &s,size_t size,char *buffer)
{
  // buffer
  size_t actBits=0;
  // buffer len
  size_t actBitsLen=0;
  size_t pos=0;
  size_t inputpos=0;

  QByteArray b = s.toUtf8();

  while(pos<size)
    {
      // if there are too few bits left and there is something left in the
      // input-queue
      while(actBitsLen<8 && inputpos<(size_t)b.length())
	{
	  // get next input byte (6 bits)
	  assert(inputpos<(size_t)b.length());

	  size_t val=recode64(b[inputpos++]);
	  val&=0x3F;
	  val<<=actBitsLen;
	  actBitsLen+=6;
	  actBits|=val;
	}
      size_t actByte=actBits&0xFF;
      buffer[pos++]=actByte;
      actBits>>=8;
      actBitsLen-=8;
    }

}

QString BoBinCoder::addNewLines(QString a)
{
  for(int i=64;i<a.length();i+=65)
    {
      a.insert(i,"\n");
    }

  return a;
}

QString BoBinCoder::removeNewLines(QString a)
{
  return a.replace("\n","").replace(" ","");
}


QString BoBinCoder::toCoded(const QBitArray &bit)
{
  QString a;
  size_t len=bit.size();
  a=Vector2MYBASE64((const char*)&len,sizeof(size_t));
  char* buffer = new char[bit.size()];
  for (int i = 0; i < bit.size(); i++)
  {
	if (bit.testBit(i))
	{
		buffer[i] = 1;
	}
	else
	{
		buffer[i] = 0;
	}
  }
  a+=Vector2MYBASE64(buffer,bit.size());
  delete[] buffer;
  a=addNewLines(a);
  return a;
}

// get real size of coded string
size_t BoBinCoder::getBufferSize(const QString &a)
{
  // +10 - some security added ;-)
  return a.length()*6/8+10;
}

QBitArray BoBinCoder::toBinary(const QString &b)
{
  QString a = removeNewLines(b);
  // size of coded real-size - should be optimized by compiler
  // WARNING : This could lead to problems, if
  size_t csize=sizeof(size_t)*8/6+1;
  size_t reallen;
  MYBASE642Vector(a,sizeof(size_t),(char*)&reallen);
  boDebug() << k_funcinfo << reallen << endl;
  char *buffer=new char[reallen];
  MYBASE642Vector(a.mid(csize,a.length()-csize),reallen,buffer);
  QBitArray bit;
  boDebug() << k_funcinfo << reallen << endl;
  bit.resize(reallen);
  for (unsigned int i = 0; i < reallen; i++)
  {
	if (buffer[i] == 0)
	{
		bit.clearBit(i);
	}
	else
	{
		bit.setBit(i);
	}
  }
  delete[] buffer;
  return bit;
}

void BoBinCoder::toCharArray(const QString &coded, char* array, int len)
{
  QString a = removeNewLines(coded);
  MYBASE642Vector(a,len,array);
}

QString BoBinCoder::toCoded(const char* buffer, int len)
{
  QString a=Vector2MYBASE64(buffer,len);
  a=addNewLines(a);
  return a;
}

