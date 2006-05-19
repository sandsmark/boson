/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#include "kgame.h"
#include "kgameio.h"
#include "kplayer.h"
#include "kgamemessage.h"
#include "kgamepropertyhandler.h"

#include <bodebug.h>
#include <klocale.h>

#include <qbuffer.h>
#include <qptrlist.h>

#include <stdio.h>
#include <assert.h>

#define KPLAYER_LOAD_COOKIE 7285

class KPlayerPrivate
{
public:
   KPlayerPrivate()
   {
      mNetworkPlayer = 0;
   }

   Q_UINT32 mId;
   bool mVirtual; // virtual player
   int mPriority; // tag for replacement

   KPlayer* mNetworkPlayer; // replacement player

   KPlayer::KGameIOList mInputList;

   KGamePropertyHandler mProperties;

// Playerdata
   KGamePropertyBool mAsyncInput;  // async input allowed
   KGamePropertyBool mMyTurn;      // Is it my turn to play (only useful if not async)?
   KGamePropertyInt  mUserId;      // a user defined id
   KGamePropertyQString mName;
   KGamePropertyQString mGroup;
};

KPlayer::KPlayer() : QObject(0,0)
{
 init();
}

KPlayer::KPlayer(KGame* game) : QObject(0, 0)
{
 init();
 game->addPlayer(this);
}

void KPlayer::init()
{
// note that NO KGame object exists here! so we cannot use KGameProperty::send!
   boDebug(11001) << k_funcinfo << ": this=" << this << ", sizeof(this)="<<sizeof(KPlayer) << endl;
   boDebug(11001) << "sizeof(m_Group)="<<sizeof(d->mGroup)<<endl;
   d = new KPlayerPrivate;

   d->mProperties.registerHandler(KGameMessage::IdPlayerProperty,
                                  this,SLOT(sendProperty(int, QDataStream&, bool*)),
                                       SLOT(emitSignal(KGamePropertyBase *)));
   d->mVirtual=false;
   mActive=true;
   mGame=0;
   d->mId=0; // "0" is always an invalid ID!
   d->mPriority=0;
   // I guess we cannot translate the group otherwise no
   // international conenctions are possible

   d->mUserId.registerData(KGamePropertyBase::IdUserId, this, i18n("UserId"));
   d->mUserId.setLocal(0);
   d->mGroup.registerData(KGamePropertyBase::IdGroup, this, i18n("Group"));
   d->mGroup.setLocal(i18n("default"));
   d->mName.registerData(KGamePropertyBase::IdName, this, i18n("Name"));
   d->mName.setLocal(i18n("default"));

   d->mAsyncInput.registerData(KGamePropertyBase::IdAsyncInput, this, i18n("AsyncInput"));
   d->mAsyncInput.setLocal(false);
   d->mMyTurn.registerData(KGamePropertyBase::IdTurn, this, i18n("myTurn"));
   d->mMyTurn.setLocal(false);
   d->mMyTurn.setEmittingSignal(true);
   d->mMyTurn.setOptimized(false);
}

KPlayer::~KPlayer()
{
  boDebug(11001) << k_funcinfo << ": this=" << this <<", id=" << this->kgameId() << endl;

  // Delete IODevices
  KGameIO *input;
  while((input=d->mInputList.first()))
  {
    delete input;
  }
  if (game())
  {
    game()->playerDeleted(this);
  }

// note: mProperties does not use autoDelete or so - user must delete objects
// himself
  d->mProperties.clear();
  delete d;
//  boDebug(11001) << k_funcinfo << " done" << endl;
}

bool KPlayer::forwardMessage(QDataStream &msg,int msgid,Q_UINT32 receiver,Q_UINT32 sender)
{
  if (!isActive())
  {
    return false;
  }
  if (!game())
  {
    return false;
  }
  boDebug(11001) << k_funcinfo << ": to game sender="<<sender<<"" << "recv="<<receiver <<"msgid="<<msgid << endl;
  return game()->sendSystemMessage(msg,msgid,receiver,sender);
}

bool KPlayer::forwardInput(QDataStream &msg,bool transmit,Q_UINT32 sender)
{
  if (!isActive())
  {
    return false;
  }
  if (!game())
  {
    return false;
  }

  boDebug(11001) << k_funcinfo << ": to game playerInput(sender="<<sender<<")" << endl;
  if (!asyncInput() && !myTurn())
  {
    boDebug(11001) << k_funcinfo << ": rejected cause it is not our turn" << endl;
    return false;
  }

  // AB: I hope I remember the usage correctly:
  // this function is called twice (on sender side) - once with transmit = true
  // where it sends the input to the comserver and once with transmit = false
  // where it really looks at the input
  if (transmit)
  {
    boDebug(11001) << "indirect playerInput" << endl;
    return game()->sendPlayerInput(msg,this,sender);
  }
  else
  {
    boDebug(11001) << "direct playerInput" << endl;
    return game()->systemPlayerInput(msg,this,sender);
  }
}

void KPlayer::setKGameId(Q_UINT32 newid)
{
  // Needs to be after the sendProcess
  d->mId=newid;
}


void KPlayer::setGroup(const QString& group)
{ d->mGroup = group; }

const QString& KPlayer::group() const
{ return d->mGroup.value(); }

void KPlayer::setName(const QString& name)
{ d->mName = name; }

const QString& KPlayer::name() const
{ return d->mName.value(); }

Q_UINT32 KPlayer::kgameId() const
{ return d->mId; }

KGamePropertyHandler * KPlayer::dataHandler()
{ return &d->mProperties; }

void KPlayer::setVirtual(bool v)
{ d->mVirtual = v; }

bool KPlayer::isVirtual() const
{ return d->mVirtual;}

void KPlayer::setNetworkPlayer(KPlayer* p)
{ d->mNetworkPlayer = p; }

KPlayer* KPlayer::networkPlayer() const
{ return d->mNetworkPlayer; }

int KPlayer::networkPriority() const
{ return d->mPriority; }

void KPlayer::setNetworkPriority(int p)
{ d->mPriority = p; }

bool KPlayer::addGameIO(KGameIO *input)
{
  if (!input)
  {
    return false;
  }
  d->mInputList.append(input); 
  input->initIO(this); // set player and init device
  return true;
}

// input=0, remove all
bool KPlayer::removeGameIO(KGameIO *targetinput,bool deleteit)
{
  boDebug(11001) << k_funcinfo << ": " << targetinput << " delete=" << deleteit<< endl;
  bool result=true;
  if (!targetinput) // delete all
  {
    KGameIO *input;
    while((input=d->mInputList.first()))
    {
      if (input) removeGameIO(input,deleteit);
    }
  }
  else
  {
//    boDebug(11001) << "remove IO " << targetinput << endl;
    if (deleteit)
    {
      delete targetinput;
    }
    else
    {
      targetinput->setPlayer(0);
      result=d->mInputList.remove(targetinput);
    }
  }
  return result;
}

KGameIO * KPlayer::findRttiIO(int rtti) const
{
  QPtrListIterator<KGameIO> it(d->mInputList);
  while (it.current())
  {
    if (it.current()->rtti() == rtti)
    {
      return it.current();
    }
    ++it;
  }
  return 0;
}

int KPlayer::calcIOValue()
{
  int value=0;
  QPtrListIterator<KGameIO> it(d->mInputList);
  while (it.current())
  {
    value|=it.current()->rtti();
    ++it;
  }
  return value;
}

bool KPlayer::setTurn(bool b, bool exclusive)
{
  boDebug(11001) << k_funcinfo << ": " << kgameId() << " (" << this << ") to " << b << endl;
  if (!isActive())
  {
    return false;
  }

  // if we get to do an exclusive turn all other players are disallowed
  if (exclusive && b && game())
  {
     KPlayer *player;
     KGame::KGamePlayerList *list=game()->playerList();
     for ( player=list->first(); player != 0; player=list->next() )
     {
       if (player==this)
       {
         continue;
       }
       player->setTurn(false,false);
     }
  }

  // Return if nothing changed
  d->mMyTurn = b;

  return true;
}

bool KPlayer::load(QDataStream &stream)
{
  Q_INT32 id,priority;
  stream >> id >> priority;
  setKGameId(id);
  setNetworkPriority(priority);

  // Load Player Data
  //FIXME: maybe set all properties setEmitSignal(false) before?
  d->mProperties.load(stream);

  Q_INT16 cookie;
  stream >> cookie;
  if (cookie==KPLAYER_LOAD_COOKIE)
  {
      boDebug(11001) << "   Player loaded propertly"<<endl;
  }
  else
  {
      boError(11001) << "   Player loading error. probably format error"<<endl;
  }

  // emit signalLoad(stream);
  return true;
}

bool KPlayer::save(QDataStream &stream)
{
  stream << (Q_INT32)kgameId() << (Q_INT32)networkPriority();

  d->mProperties.save(stream);

  stream << (Q_INT16)KPLAYER_LOAD_COOKIE;

  //emit signalSave(stream);
  return true;
}


void KPlayer::networkTransmission(QDataStream &stream,int msgid,Q_UINT32 sender)
{
  //boDebug(11001) << k_funcinfo ": msgid=" << msgid << " sender=" << sender << " we are=" << id() << endl;
  // PlayerProperties processed
  bool issender;
  if (game())
  {
    issender=sender==game()->gameId();
  }
  else
  {
    issender=true;
  }
  if (d->mProperties.processMessage(stream,msgid,issender))
  {
	return ;
  }
  switch(msgid)
  {
    case KGameMessage::IdPlayerInput:
      {
        boDebug(11001) << k_funcinfo << ": Got player move "
	        << "KPlayer (virtual) forwards it to the game object" << endl;
        forwardInput(stream,false);
      }
    break;
    default:
        emit signalNetworkData(msgid - KGameMessage::IdUser,
	        ((QBuffer*)stream.device())->readAll(),sender,this);
        boDebug(11001) << k_funcinfo << ": "
	        << "User data msgid " << msgid << endl;
    break;
  }

}

KGamePropertyBase* KPlayer::findProperty(int id) const
{
  return d->mProperties.find(id);
}

bool KPlayer::addProperty(KGamePropertyBase* data)
{
  return d->mProperties.addProperty(data);
}

void KPlayer::sendProperty(int msgid, QDataStream& stream, bool* sent)
{
  if (game())
  {
    bool s = game()->sendPlayerProperty(msgid, stream, kgameId());
    if (s)
    {
      *sent = true;
    }
  }
}

void KPlayer::emitSignal(KGamePropertyBase *me)
{
  // Notify KGameIO (Process) for a new turn
  if (me->id()==KGamePropertyBase::IdTurn)
  {
    //boDebug(11001) << k_funcinfo << ": for KGamePropertyBase::IdTurn " << endl;
    QPtrListIterator<KGameIO> it(d->mInputList);
    while (it.current())
    {
      it.current()->notifyTurn(d->mMyTurn.value());
      ++it;
    }
  }
  emit signalPropertyChanged(me,this);
}

// --------------------- DEBUG --------------------
void KPlayer::Debug()
{
   boDebug(11001) << "------------------- KPLAYER -----------------------" << endl;
   boDebug(11001) << "this:    " << this << endl;
   boDebug(11001) << "rtti:    " << rtti() << endl;
   boDebug(11001) << "id  :    " << kgameId() << endl;
   boDebug(11001) << "Name :   " << name() << endl;
   boDebug(11001) << "Group:   " << group() << endl;
   boDebug(11001) << "Async:   " << asyncInput() << endl;
   boDebug(11001) << "myTurn:  " << myTurn() << endl;
   boDebug(11001) << "Virtual: " << isVirtual() << endl;
   boDebug(11001) << "Active:  " << isActive() << endl;
   boDebug(11001) << "Priority:" << networkPriority() << endl;
   boDebug(11001) << "Game   : " << game() << endl;
   boDebug(11001) << "#IOs:    " << d->mInputList.count() << endl;
   boDebug(11001) << "---------------------------------------------------" << endl;
}

bool KPlayer::myTurn() const
{
   return d->mMyTurn.value();
}

int KPlayer::userId() const
{
   return d->mUserId.value();
}

void KPlayer::setUserId(int i)
{
   d->mUserId = i;
}

void KPlayer::setAsyncInput(bool a)
{
   d->mAsyncInput = a;
}
bool KPlayer::asyncInput() const
{
   return d->mAsyncInput.value();
}

KPlayer::KGameIOList *KPlayer::ioList()
{
   return &d->mInputList;
}

#include "kplayer.moc"
