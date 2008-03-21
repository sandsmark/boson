/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

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

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <qstring.h>

class Model;
class LOD;


class Processor
{
  public:
    Processor();
    virtual ~Processor();

    /**
     * Set a name for the processor, to be displayed in debug and error
     * messages.
     **/
    void setName(const QString& name)
    {
      mName = name;
    }
    const QString& name() const
    {
      return mName;
    }

    virtual bool initProcessor(Model* model);

    virtual bool process() = 0;


    Model* model() const  { return mModel; }
    LOD* lod() const  { return mLOD; }

    static void setBaseFrame(unsigned int frame)  { mBaseFrame = frame; }
    static unsigned int baseFrame()  { return mBaseFrame; }

  protected:
    void setLOD(LOD* lod);

  private:
    QString mName;
    Model* mModel;
    LOD* mLOD;
    static unsigned int mBaseFrame;
};


/*
 * vim: et sw=2
 */
#endif //PROCESSOR_H
