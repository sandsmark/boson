/*
    This file is part of the Boson game
    Copyright (C) 2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef PROCESSOR_H
#define PROCESSOR_H


class Model;
class LOD;


class Processor
{
  public:
    Processor(Model* m, LOD* l = 0);
    virtual ~Processor();

    virtual bool process() = 0;


    Model* model() const  { return mModel; }
    LOD* lod() const  { return mLOD; }

    static void setBaseFrame(unsigned int frame)  { mBaseFrame = frame; }
    static unsigned int baseFrame()  { return mBaseFrame; }


  private:
    Model* mModel;
    LOD* mLOD;
    static unsigned int mBaseFrame;
};


#endif //PROCESSOR_H
