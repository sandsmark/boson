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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgame.h"
#include "kgame.moc"
#include "kgamepropertyhandler.h"
#include "kgameproperty.h"
#include "kplayer.h"
#include "kgameio.h"
#include "kgameerror.h"
#include "kgamesequence.h"

#include "kgamemessage.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <qbuffer.h>
#include <qtimer.h>
#include <qptrqueue.h>
#include <qfile.h>

#include <klocale.h>
#include <krandomsequence.h>
#include <bodebug.h>

#define KGAME_LOAD_COOKIE 4210

// try to place as much as possible here
// many things are *not* possible here as KGame has to use some inline function
class KGamePrivate
{
public:
    KGamePrivate()
    {
        mUniquePlayerNumber = 0;
        mPolicy=KGame::PolicyLocal;
        mGameSequence = 0;
    }

    int mUniquePlayerNumber;
    QPtrQueue<KPlayer> mAddPlayerList;// this is a list of to-be-added players. See addPlayer() docu
    KRandomSequence* mRandom;
    KGame::GamePolicy mPolicy;
    KGameSequence* mGameSequence;


    KGamePropertyHandler* mProperties;

    // player lists
    KGame::KGamePlayerList mPlayerList;
    KGame::KGamePlayerList mInactivePlayerList;

    //KGamePropertys
    KGamePropertyInt mMaxPlayer;
    KGamePropertyUInt mMinPlayer;
    KGamePropertyInt mGameStatus; // Game running?
    QValueList<int> mInactiveIdList;

};

// ------------------- GAME CLASS --------------------------
KGame::KGame(int cookie,QObject* parent) : KGameNetwork(cookie,parent)
{
 boDebug(11001) << k_funcinfo << " - " << this << ", sizeof(KGame)=" << sizeof(KGame) << endl;
 d = new KGamePrivate;

 d->mProperties = new KGamePropertyHandler(this);

 d->mProperties->registerHandler(KGameMessage::IdGameProperty,
                                this,SLOT(sendProperty(int, QDataStream&, bool* )),
                                     SLOT(emitSignal(KGamePropertyBase *)));
 d->mMaxPlayer.registerData(KGamePropertyBase::IdMaxPlayer, this, i18n("MaxPlayers"));
 d->mMaxPlayer.setLocal(-1);  // Infinite
 d->mMinPlayer.registerData(KGamePropertyBase::IdMinPlayer, this, i18n("MinPlayers"));
 d->mMinPlayer.setLocal(0);   // Always ok
 d->mGameStatus.registerData(KGamePropertyBase::IdGameStatus, this, i18n("GameStatus"));
 d->mGameStatus.setLocal(Init);
 // d->mUniquePlayerNumber = 0;
 d->mRandom = new KRandomSequence;
 d->mRandom->setSeed(0);

 connect(this, SIGNAL(signalClientConnected(Q_UINT32)),
                this, SLOT(slotClientConnected(Q_UINT32)));
 connect(this, SIGNAL(signalClientDisconnected(Q_UINT32,bool)),
                this, SLOT(slotClientDisconnected(Q_UINT32,bool)));
 connect(this, SIGNAL(signalConnectionBroken()),
                this, SLOT(slotServerDisconnected()));

 setGameSequence(new KGameSequence());

 // BL: FIXME This signal does no longer exist. When we are merging
 // MH: super....and how do I find out about the lost conenction now?
 // KGame and KGameNetwork, this could be improved!
//  connect(this,SIGNAL(signalConnectionLost(KGameClient *)),
//          this,SLOT(slotConnectionLost(KGameClient *)));
}

KGame::~KGame()
{
 boDebug(11001) << k_funcinfo << endl;
// Debug();
 reset();
 delete d->mGameSequence;
 delete d->mRandom;
 delete d;
 boDebug(11001) << k_funcinfo << " done" << endl;
}

bool KGame::reset()
{
 deletePlayers();
 deleteInactivePlayers();
 return true;
}

void KGame::deletePlayers()
{
// boDebug(11001) << k_funcinfo << endl;
 KGamePlayerList tmp = d->mPlayerList; // in case of PolicyClean player=d->mPlayerList.first() is infinite
 KPlayer *player;
 while((player=tmp.first()))
 {
   delete player; // delete and removes the player
   tmp.removeFirst();
 }
// boDebug(11001) << k_funcinfo << " done" << endl;
}

void KGame::deleteInactivePlayers()
{
 KPlayer *player;
 while((player=d->mInactivePlayerList.first()))
 {
   //player->setGame(0); // prevent call backs
   d->mInactivePlayerList.remove(player);
   delete player;
 }
}

bool KGame::load(QString filename,bool reset)
{
  if (filename.isNull())
  {
    return false;
  }
  QFile f(filename);
  if (!f.open(IO_ReadOnly))
  {
    return false;
  }
  QDataStream s( &f );
  load(s,reset);
  f.close();
  return true;
}

bool KGame::load(QDataStream &stream,bool reset)
{ return loadgame(stream, false,reset); }

bool KGame::loadgame(QDataStream &stream, bool network,bool resetgame)
{
 // Load Game Data

 // internal data
 Q_INT32 c;
 stream >> c; // cookie

 if (c!=cookie())
 {
   boWarning(11001) << "Trying to load different game version we="<<cookie() << " saved=" << c << endl;
   bool result=false;
   emit signalLoadError(stream,network,(int)c,result);
   return result;
 }
 if (resetgame) reset();

 uint i;
 stream >> i;
// setPolicy((GamePolicy)i);

 stream >> d->mUniquePlayerNumber;

 if (gameSequence())
 {
   gameSequence()->setCurrentPlayer(0);  // TODO !!!
 }
 int newseed;
 stream >> newseed;
 d->mRandom->setSeed(newseed);

 // Switch off the direct emitting of signals while
 // loading properties. This can cause inconsistencies
 // otherwise if a property emits and this emit accesses
 // a property not yet loaded
 // Note we habe to have this external locking to prevent the games unlocking
 // to access the players
 dataHandler()->lockDirectEmit();
 KPlayer *player;
 for ( player=playerList()->first(); player != 0; player=playerList()->next() )
 {
   player->dataHandler()->lockDirectEmit();
   // boDebug(11001) << "Player "<<player->kgameId() << " to indirect emit" <<endl;
 }

 // Properties
 dataHandler()->load(stream);

 // If there is additional data to be loaded before players are loaded then do
 // this here.
 emit signalLoadPrePlayers(stream);

 // Load Playerobjects
 uint playercount;
 stream >> playercount;
 boDebug(11001) << "Loading KGame " << playercount << " KPlayer objects " << endl;
 for (i=0;i<playercount;i++)
 {
   KPlayer *newplayer=loadPlayer(stream,network);
   systemAddPlayer(newplayer);
 }

 Q_INT16 cookie;
 stream >> cookie;
 if (cookie==KGAME_LOAD_COOKIE) {
   boDebug(11001) << "   Game loaded propertly"<<endl;
 } else {
   boError(11001) << "   Game loading error. probably format error"<<endl;
 }

 // Switch back on the direct emitting of signals and emit the
 // queued signals.
 // Note we habe to have this external locking to prevent the games unlocking
 // to access the players
 dataHandler()->unlockDirectEmit();
 for ( player=playerList()->first(); player != 0; player=playerList()->next() )
 {
   player->dataHandler()->unlockDirectEmit();
   // boDebug(11001) << "Player "<<player->kgameId() << " to direct emit" <<endl;
 }

 emit signalLoad(stream);
 return true;
}

bool KGame::save(QString filename,bool saveplayers)
{
 if (filename.isNull())
 {
   return false;
 }
 QFile f(filename);
 if (!f.open(IO_WriteOnly))
 {
   return false;
 }
 QDataStream s( &f );
 save(s,saveplayers);
 f.close();
 return true;
}

bool KGame::save(QDataStream &stream,bool saveplayers)
{ return savegame(stream, false,saveplayers); }

bool KGame::savegame(QDataStream &stream,bool /*network*/,bool saveplayers)
{
  // Save Game Data

  // internal variables
  Q_INT32 c=cookie();
  stream << c;

  uint p=(uint)policy();
  stream << p;
  stream << d->mUniquePlayerNumber;
  int newseed=(int)d->mRandom->getLong(65535);
  stream << newseed;
  d->mRandom->setSeed(newseed);

 // Properties
 dataHandler()->save(stream);

 // Save all data that need to be saved *before* the players are saved
 emit signalSavePrePlayers(stream);

 if (saveplayers)
 {
   savePlayers(stream,playerList());
 }
 else
 {
   stream << (uint)0; // no players saved
 }

 stream << (Q_INT16)KGAME_LOAD_COOKIE;

 emit signalSave(stream);
 return true;
}

void KGame::savePlayer(QDataStream &stream,KPlayer* p)
{
// this could be in KGameMessage as well
 stream << (Q_INT32)p->rtti();
 stream << (Q_INT32)p->kgameId();
 stream << (Q_INT32)p->calcIOValue();
 p->save(stream);
}

void KGame::savePlayers(QDataStream &stream, KGamePlayerList *list)
{
 if (!list)
 {
   list=playerList();
 }

 Q_INT32 cnt=list->count();
 boDebug(11001) << "Saving KGame " << cnt << " KPlayer objects " << endl;
 stream << cnt;
 KPlayer *player;
 for ( player=list->first(); player != 0; player=list->next() )
 {
   savePlayer(stream,player);
 }
}

KPlayer *KGame::createPlayer(int /*rtti*/,int /*io*/,bool /*isvirtual*/)
{
  boWarning(11001) << "   No user defined player created. Creating default KPlayer. This crashes if you have overwritten KPlayer!!!! " << endl;
  return new KPlayer;
}
KPlayer *KGame::loadPlayer(QDataStream& stream,bool isvirtual)
{
  Q_INT32 rtti,id,iovalue;
  stream >> rtti >> id >> iovalue;
  KPlayer *newplayer=findPlayerByKGameId(id);
  if (!newplayer)
  {
    boDebug(11001) << k_funcinfo << "Player "<< id << " not found...asking user to create one " << endl;
    newplayer=createPlayer(rtti,iovalue,isvirtual);
    //emit signalCreatePlayer(newplayer,rtti,iovalue,isvirtual,this);
  }
  /*
  if (!newplayer)
  {
    boWarning(11001) << "   No user defined player created. Creating default KPlayer. This crashes if you have overwritten KPlayer!!!! " << endl;
    newplayer=new KPlayer;
  }
  else
  {
    boDebug(11001) << "   USER Player " << newplayer << " done player->rtti=" << newplayer->rtti() << " rtti=" << rtti << endl;
  }
  */
  newplayer->load(stream);
  if (isvirtual)
  {
    newplayer->setVirtual(true);
  }
  return newplayer;
}

// ----------------- Player handling -----------------------

KPlayer* KGame::findPlayerByKGameId(Q_UINT32 id) const
{
 for (QPtrListIterator<KPlayer> it(d->mPlayerList); it.current(); ++it)
 {
   if (it.current()->kgameId() == id)
   {
     return it.current();
   }
 }
 for (QPtrListIterator<KPlayer> it(d->mInactivePlayerList); it.current(); ++it)
 {
   if (it.current()->kgameId() == id)
   {
     return it.current();
   }
 }
 return 0;
}

KPlayer* KGame::findPlayerByUserId(int id) const
{
 for (QPtrListIterator<KPlayer> it(d->mPlayerList); it.current(); ++it)
 {
   if (it.current()->userId() == id)
   {
     return it.current();
   }
 }
 for (QPtrListIterator<KPlayer> it(d->mInactivePlayerList); it.current(); ++it)
 {
   if (it.current()->userId() == id)
   {
     return it.current();
   }
 }
 return 0;
}

// it is necessary that addPlayer and systemAddPlayer are called in the same
// order. Ie if addPlayer(foo) followed by addPlayer(bar) is called, you must
// not call systemAddPlayer(bar) followed by systemAddPlayer(foo), as the
// mAddPlayerList would get confused. Should be no problem as long as comServer
// and the clients are working correctly.
// BUT: if addPlayer(foo) does not arrive by any reason while addPlayer(bar)
// does, we would be in trouble...
void KGame::addPlayer(KPlayer* newplayer)
{
 boDebug(11001) << k_funcinfo << ":  " << "; maxPlayers=" << maxPlayers() << " playerCount=" << playerCount() << endl;
 if (!newplayer)
 {
  boError(11001) << "trying to add NULL player in KGame::addPlayer()" << endl;
  exit(1);
  return ;
 }

 if (maxPlayers() >= 0 && (int)playerCount() >= maxPlayers())
 {
   boWarning(11001) << "cannot add more than " << maxPlayers() << " players - deleting..." << endl;
   delete newplayer;
   return;
 }

 if (newplayer->kgameId() == 0)
 {
   d->mUniquePlayerNumber++;
   newplayer->setKGameId(KGameMessage::createPlayerId(d->mUniquePlayerNumber, gameId()));
   boDebug(11001) << k_funcinfo << "NEW!!! player " << newplayer << " now has id " << newplayer->kgameId() << endl;
 }
 else
 {
   // this could happen in games which use their own ID management by certain
   // reasons. that is NOT recommended
   boDebug(11001) << k_funcinfo << "player " << newplayer << " already has an id: " << newplayer->kgameId() << endl;
 }

 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 // We distinguis here what policy we have
 if (policy()==PolicyLocal || policy()==PolicyDirty)
 {
   systemAddPlayer(newplayer);
 }
 if (policy()==PolicyClean || policy()==PolicyDirty)
 {
   savePlayer(stream,newplayer);
   // Store the player for delayed clean adding
   if (policy()==PolicyClean)
   {
     d->mAddPlayerList.enqueue(newplayer);
   }
   sendSystemMessage(stream,(int)KGameMessage::IdAddPlayer, 0);
 }
}

void KGame::systemAddPlayer(KPlayer* newplayer)
{
 if (!newplayer)
 {
   boError(11001) << "trying to add NULL player in KGame::systemAddPlayer()" << endl;
   exit(1);
   return ;
 }
 if (newplayer->kgameId() == 0)
 {
   boWarning(11001) << k_funcinfo << "player " << newplayer << " has no ID" << endl;
 }

 if (findPlayerByKGameId(newplayer->kgameId()))
 {
   boError(11001) << "ERROR: Double adding player !!!!! NOT GOOD !!!!!! " << newplayer->kgameId() << "...I delete it again" << endl;
   delete newplayer;
 }
 else
 {
   boDebug(11001) << "Trying to add player " << newplayer <<" maxPlayers="<<maxPlayers()<<" playerCount="<<playerCount() << endl;
   // Add the player to the game
   d->mPlayerList.append(newplayer);
   newplayer->setGame(this);
   boDebug(11001) << "Player: isVirtual=" << newplayer->isVirtual() << endl;
   boDebug(11001) << "        id=" << newplayer->kgameId() << "  #Players="
                  << d->mPlayerList.count() << " added " << newplayer
                  << "  (virtual=" << newplayer->isVirtual() << ")" << endl;
   emit signalPlayerJoinedGame(newplayer);
 }
}

// Called by the KPlayer destructor
void KGame::playerDeleted(KPlayer *player)
{
 boDebug(11001) << k_funcinfo << ": id (" << player->kgameId() << ") to be removed " << player << endl;

 if (policy()==PolicyLocal || policy()==PolicyDirty)
 {
   systemRemovePlayer(player,false);
 }
 if (policy()==PolicyClean || policy()==PolicyDirty)
 {
   if (!player->isVirtual())
   {
     boDebug(11001) << k_funcinfo << ": sending IdRemovePlayer "<<player->kgameId() << endl;
     sendSystemMessage(player->kgameId(), KGameMessage::IdRemovePlayer, 0);
   }
 }
}

bool KGame::removePlayer(KPlayer * player, Q_UINT32 receiver)
{//transmit to all clients, or to receiver only
 if (!player)
 {
   boError(11001) << "trying to remove NULL player in KGame::removePlayer()" << endl;
   exit(1);
   return false;
 }
 boDebug(11001) << k_funcinfo << ": id (" << player->kgameId() << ") to be removed " << player << endl;

 if (policy()==PolicyLocal || policy()==PolicyDirty)
 {
   systemRemovePlayer(player,true);
 }
 if (policy()==PolicyClean || policy()==PolicyDirty)
 {
   boDebug(11001) << k_funcinfo << ": sending IdRemovePlayer "<<player->kgameId() << endl;
   sendSystemMessage(player->kgameId(),KGameMessage::IdRemovePlayer, receiver);
 }
 return true;
 // we will receive the message in networkTransmission()
}

void KGame::systemRemovePlayer(KPlayer* player,bool deleteit)
{
 boDebug(11001) << k_funcinfo << endl;
 if (!player)
 {
   boWarning(11001) << "cannot remove NULL player" << endl;
   return;
 }
 if (!systemRemove(player,deleteit))
 {
   boWarning(11001) << "player " << player << "(" << player->kgameId() << ") Could not be found!" << endl;
 }

 if (gameStatus()==(int)Run && playerCount()<minPlayers())
 {
   boWarning(11001) << k_funcinfo ": not enough players, PAUSING game\n" << endl;
   setGameStatus(Pause);
 }
}

bool KGame::systemRemove(KPlayer* p,bool deleteit)
{
 if (!p)
 {
   boWarning(11001) << "cannot remove NULL player" << endl;
   return false;
 }
 bool result;
 boDebug(11001) << k_funcinfo << ": Player (" << p->kgameId() << ") to be removed " << p << endl;

 if (d->mPlayerList.count() == 0)
 {
   result = false;
 }
 else
 {
   result = d->mPlayerList.remove(p);
 }

 emit signalPlayerLeftGame(p);

 p->setGame(0);
 if (deleteit)
 {
   delete p;
 }

 return result;
}

bool KGame::inactivatePlayer(KPlayer* player)
{
 if (!player)
 {
   return false;
 }
 boDebug(11001) << "Inactivate player " << player->kgameId() << endl;

 if (policy()==PolicyLocal || policy()==PolicyDirty)
 {
   systemInactivatePlayer(player);
 }
 if (policy()==PolicyClean || policy()==PolicyDirty)
 {
   sendSystemMessage(player->kgameId(), KGameMessage::IdInactivatePlayer);
 }

 return true;
}

bool KGame::systemInactivatePlayer(KPlayer* player)
{
 if (!player || !player->isActive())
 {
   return false;
 }
 boDebug(11001) << " Inactivate player " << player->kgameId() << endl;

 int pid=player->kgameId();
 // Virtual players cannot be deactivated. They will be removed
 if (player->isVirtual())
 {
   systemRemovePlayer(player,true);
 }
 else
 {
   d->mPlayerList.remove(player);
   d->mInactivePlayerList.prepend(player);
   player->setActive(false);
 }
 emit signalPlayerLeftGame(player);
 if (isAdmin())
 {
   d->mInactiveIdList.prepend(pid);
 }
 return true;
}

bool KGame::activatePlayer(KPlayer * player)
{
  if (!player)
  {
    return false;
  }
  boDebug(11001) << k_funcinfo << ": activate " << player->kgameId() << endl;
  if (policy()==PolicyLocal || policy()==PolicyDirty)
  {
    systemActivatePlayer(player);
  }
  if (policy()==PolicyClean || policy()==PolicyDirty)
  {
    sendSystemMessage(player->kgameId(), KGameMessage::IdActivatePlayer);
  }
 return true;
}

bool KGame::systemActivatePlayer(KPlayer* player)
{
 if (!player || player->isActive())
 {
   return false;
 }
 boDebug(11001) << k_funcinfo << ": activate " << player->kgameId() << endl;

 d->mInactivePlayerList.remove(player);
 player->setActive(true);
 addPlayer(player);
 if (isAdmin())
 {
   d->mInactiveIdList.remove(player->kgameId());
 }
 return true;
}

// -------------------- Properties ---------------------------

void KGame::setMaxPlayers(uint maxnumber)
{ if (isAdmin()) { d->mMaxPlayer.changeValue(maxnumber); } }

void KGame::setMinPlayers(uint minnumber)
{ if (isAdmin()) { d->mMinPlayer.changeValue(minnumber); } }

uint KGame::minPlayers() const
{ return d->mMinPlayer.value(); }

int KGame::maxPlayers() const
{ return d->mMaxPlayer.value(); }

uint KGame::playerCount() const
{ return d->mPlayerList.count(); }

int KGame::gameStatus() const
{ return d->mGameStatus.value(); }

bool KGame::isRunning() const
{ return d->mGameStatus.value() == Run; }

KGamePropertyHandler* KGame::dataHandler() const
{ return d->mProperties; }


KGame::KGamePlayerList* KGame::inactivePlayerList()
{ return &d->mInactivePlayerList; }

const KGame::KGamePlayerList* KGame::inactivePlayerList() const
{ return &d->mInactivePlayerList; }

KGame::KGamePlayerList* KGame::playerList()
{ return &d->mPlayerList; }

const KGame::KGamePlayerList* KGame::playerList() const
{ return &d->mPlayerList; }

KRandomSequence* KGame::random() const
{ return d->mRandom; }


bool KGame::sendPlayerInput(QDataStream &msg, KPlayer *player, Q_UINT32 sender)
{
 if (!player)
 {
   boError(11001) << k_funcinfo << ": NULL player" << endl;
   return false;
 }
 if (!isRunning())
 {
   boError(11001) << k_funcinfo << ": game not running" << endl;
   return false;
 }

 boDebug(11001) << k_funcinfo << ": transmitting playerInput over network" << endl;
 sendSystemMessage(msg, (int)KGameMessage::IdPlayerInput, player->kgameId(), sender);
 return true;
}

bool KGame::systemPlayerInput(QDataStream &msg, KPlayer *player, Q_UINT32 sender)
{
 if (!player)
 {
   boError(11001) << k_funcinfo << ": NULL player" << endl;
   return false;
 }
 if (!isRunning())
 {
   boError(11001) << k_funcinfo << ": game not running" << endl;
   return false;
 }
 boDebug(11001) << "KGame: Got playerInput from messageServer... sender: " << sender << endl;
 if (playerInput(msg,player))
 {
   playerInputFinished(player);
 }
 else
 {
   boDebug(11001) << k_funcinfo<<": switching off player input"<<endl;
   // TODO: (MH 03-2003): We need an return option from playerInput so that
   // the player's is not automatically disabled here 
   if (!player->asyncInput())
   {
     player->setTurn(false); // in turn based games we have to switch off input now
   }
 }
 return true;
}


KPlayer * KGame::playerInputFinished(KPlayer *player)
{
 boDebug(11001) << k_funcinfo<<"player input finished for "<<player->kgameId()<<endl;
 // Check for game over and if not allow the next player to move
 int gameOver = 0;
 if (gameSequence())
 {
   gameSequence()->setCurrentPlayer(player);
 }
 // do not call gameSequence()->checkGameOver() to keep backward compatibility!
 gameOver = checkGameOver(player);
 if (gameOver!=0)
 {
   if (player)
   {
     player->setTurn(false);
   }
   setGameStatus(End);
   emit signalGameOver(gameOver,player,this);
 }
 else if (!player->asyncInput())
 {
   player->setTurn(false); // in turn based games we have to switch off input now
   if (gameSequence())
   {
     QTimer::singleShot(0,this,SLOT(prepareNext()));
   }
 }
 return player;
}

// Per default we do not do anything
int KGame::checkGameOver(KPlayer *player)
{
 if (gameSequence())
 {
   return gameSequence()->checkGameOver(player);
 }
 return 0;
}

void KGame::setGameSequence(KGameSequence* sequence)
{
 delete d->mGameSequence;
 d->mGameSequence = sequence;
 if (d->mGameSequence)
 {
   d->mGameSequence->setGame(this);
 }
}

KGameSequence* KGame::gameSequence() const
{
  return d->mGameSequence;
}

void KGame::prepareNext()
{
 if (gameSequence())
 {
   // we don't call gameSequence->nextPlayer() to keep old code working
   nextPlayer(gameSequence()->currentPlayer());
 }
}

KPlayer *KGame::nextPlayer(KPlayer *last,bool exclusive)
{
 if (gameSequence())
 {
   return gameSequence()->nextPlayer(last, exclusive);
 }
 return 0;
}

void KGame::setGameStatus(int status)
{
 boDebug(11001) << k_funcinfo << ": GAMESTATUS CHANGED  to" << status << endl;
 if (status==(int)Run && playerCount()<minPlayers()) 
 {
   boDebug(11001) << k_funcinfo << ": not enough players, pausing game\n" << endl;
   status=Pause;
 }
 d->mGameStatus = status;
}

void KGame::networkTransmission(QDataStream &stream, int msgid, Q_UINT32 receiver, Q_UINT32 sender, Q_UINT32 /*clientID*/)
{//clientID is unused
 // message targets a playerobject. If we find it we forward the message to the
 // player. Otherwise we proceed here and hope the best that the user processes
 // the message

//  boDebug(11001) << k_funcinfo << ": we="<<(int)gameId()<<" id="<<msgid<<" recv=" << receiver << " sender=" << sender << endl;

 
 // *first* notice the game that something has changed - so no return prevents
 // this
 emit signalMessageUpdate(msgid, receiver, sender);
 if (KGameMessage::isPlayer(receiver))
 {
   //boDebug(11001) << "message id " << msgid << " seems to be for a player ("<<active=p->isActive()<<" recv="<< receiver << endl;
   KPlayer *p=findPlayerByKGameId(receiver);
   if (p && p->isActive())
   {
     p->networkTransmission(stream,msgid,sender);
     return;
   }
   if (p)
   {
      boDebug(11001) << "player is here but not active" << endl;
   }
   else
   {
      boDebug(11001) << "no player found" << endl;
   }
 }
 // If it is not for a player it is meant for us!!!! Otherwise the
 // gamenetwork would not have passed the message to us!

 // GameProperties processed
 if (d->mProperties->processMessage(stream, msgid, sender == gameId())) 
 {
//   boDebug(11001 ) << "KGame: message taken by property - returning" << endl;
   return ;
 }

 switch(msgid)
 {
   case KGameMessage::IdSetupGame:  // Client: First step in setup game
   {
     Q_INT16 v;
     Q_INT32 c;
     stream >> v >> c;
     boDebug(11001) << " ===================> (Client) " << k_funcinfo << ": Got IdSetupGame ================== " << endl;
     boDebug(11001) << "our game id is " << gameId() << " Lib version=" << v << " App Cookie=" << c << endl; 
     // Verify identity of the network partners
     if (c!=cookie())
     {
       boError(11001) << "IdGameSetup: Negotiate Game: cookie mismatch I'am="<<cookie()<<" master="<<c<<endl;
       sendError(KGameError::Cookie, KGameError::errCookie(cookie(), c));
       disconnect(); // disconnect from master
     }
     else if (v!=KGameMessage::version())
     {
       sendError(KGameError::Version, KGameError::errVersion(v));
       disconnect(); // disconnect from master
     }
     else
     {
       setupGame(sender);
     }
     boDebug(11001) << "========== (Client) Setup game done\n";
   }
   break;
   case KGameMessage::IdSetupGameContinue:  // Master: second step in game setup
   {
     boDebug(11001) << "=====>(Master) " << k_funcinfo << " - IdSetupGameContinue" << endl;
     setupGameContinue(stream, sender);
   }
   break;
   case KGameMessage::IdActivatePlayer:  // Activate Player
   {
     int id;
     stream >> id;
     boDebug(11001) << "Got IdActivatePlayer id=" << id << endl;
     if (sender!=gameId()  || policy()!=PolicyDirty)
     {
       systemActivatePlayer(findPlayerByKGameId(id));
     }
   }
   break;
   case KGameMessage::IdInactivatePlayer:  // Inactivate Player
   {
     int id;
     stream >> id;
     boDebug(11001) << "Got IdInactivatePlayer id=" << id << endl;
     if (sender!=gameId()  || policy()!=PolicyDirty)
     {
       systemInactivatePlayer(findPlayerByKGameId(id));
     }
   }
   break;
   case KGameMessage::IdAddPlayer:
   {
     boDebug(11001) << k_funcinfo << ": Got IdAddPlayer" << endl;
     if (sender!=gameId()  || policy()!=PolicyDirty)
     {
       KPlayer *newplayer=0;
       // We sent the message so the player is already available
       if (sender==gameId())
       {
          boDebug(11001) << "dequeue previously added player" << endl;
          newplayer = d->mAddPlayerList.dequeue();
       }
       else
       {
         newplayer=loadPlayer(stream,true);
       }
       systemAddPlayer(newplayer);// the final, local, adding
       //systemAddPlayer(stream);
     }
   }
   break;
   case KGameMessage::IdRemovePlayer: // Client should delete player id
   {
     int id;
     stream >> id;
     boDebug(11001) << k_funcinfo << ": Got IdRemovePlayer " << id << endl;
     KPlayer *p=findPlayerByKGameId(id);
     if (p)
     {
       // Otherwise the player is already removed
       if (sender!=gameId()  || policy()!=PolicyDirty)
       {
         systemRemovePlayer(p,true);
       }
     }
     else
     {
       boWarning(11001) << k_funcinfo << "Cannot find player " << id << endl;
     }
   }
   break;
   case KGameMessage::IdGameLoad:
   {
     boDebug(11001) << "====> (Client) " << k_funcinfo << ": Got IdGameLoad" << endl;
     loadgame(stream,true,false);
   }
   break;
   case KGameMessage::IdGameSetupDone:
   {
     int cid;
     stream >> cid;
     boDebug(11001) << "====> (CLIENT) " << k_funcinfo << ": Got IdGameSetupDone for client "
             << cid << " we are =" << gameId() << endl;
     sendSystemMessage(gameId(), KGameMessage::IdGameConnected, 0);
   }
   break;
   case KGameMessage::IdGameConnected:
   {
     int cid;
     stream >> cid;
     boDebug(11001) << "====> (ALL) " << k_funcinfo << ": Got IdGameConnected for client "<< cid << " we are =" << gameId() << endl;
     emit signalClientJoinedGame(cid,this);
   }
   break;

   case KGameMessage::IdSyncRandom:  // Master forces a new random seed on us
   {
     int newseed;
     stream >> newseed;
     boDebug(11001) << "CLIENT: setting random seed to " << newseed << endl;
     d->mRandom->setSeed(newseed);
   }
   break;
   case KGameMessage::IdDisconnect:
   {
   // if we disconnect we *always* start a local game. 
   // this could lead into problems if we just change the message server
     if (sender != gameId())
     {
         boDebug(11001) << "client " << sender << " leaves game" << endl;
         return;
     }
     boDebug(11001) << "leaving the game" << endl;
     // start a new local game
     // no other client is by default connected to this so this call should be
     // enough
     setMaster();
   }
   break;
   default:
    {
     if (msgid < KGameMessage::IdUser)
     {
       boError(11001) << "incorrect message id " << msgid << " - emit anyway"
                      << endl;
     }
     boDebug(11001) << k_funcinfo << ": User data msgid " << msgid << endl;
     emit signalNetworkData(msgid - KGameMessage::IdUser,((QBuffer*)stream.device())->readAll(),receiver,sender);
   }
   break;
 }

}

// called by the IdSetupGameContinue Message - MASTER SIDE
// Here the master needs to decide which players can take part at the game
// and which will be deactivated
void KGame::setupGameContinue(QDataStream& stream, Q_UINT32 sender)
{
  KPlayer *player;
  Q_INT32 cnt;
  int i;
  stream >> cnt;

  QValueList<int> inactivateIds;

  KGamePlayerList newPlayerList;

#warning why do we do this?
  newPlayerList.setAutoDelete(true);

  for (i=0;i<cnt;i++)
  {
    player=loadPlayer(stream,true);
    boDebug(11001) << " Master got player " << player->kgameId() <<" rawgame=" << KGameMessage::rawGameId(player->kgameId())  << " from sender " << sender << endl;
    if (KGameMessage::rawGameId(player->kgameId()) != sender)
    {
      boError(11001) << "Client tries to add player with wrong game id - cheat possible" << endl;
    }
    else
    {
      newPlayerList.append(player);
      boDebug(11001) << " newplayerlist appended " << player->kgameId() << endl;
    }
  }

#warning this call is useless, as newPlayerList players is are deleted! (auto deletion)
  newPlayersJoin(playerList(),&newPlayerList,inactivateIds);


  boDebug(11001) << " Master calculates how many players to activate client has cnt=" << cnt << endl;
  boDebug(11001) << " The game has " << playerCount() << " active players" << endl;
  boDebug(11001) << " The user deactivated "<< inactivateIds.count() << " player already " << endl;
  boDebug(11001) << " MaxPlayers for this game is " << maxPlayers() << endl;

  // Do we have too many players? (After the programmer disabled some?)
  // MH: We cannot use have player here as it CHANGES in the loop
  // int havePlayers = cnt+playerCount()-inactivateIds.count();
  boDebug(11001) << " havePlayers " << cnt+playerCount()-inactivateIds.count() << endl;
  while (maxPlayers() > 0 && maxPlayers() < (int)(cnt+playerCount() - inactivateIds.count()))
  {
    boDebug(11001) << "  Still to deacticvate "
            << (int)(cnt+playerCount()-inactivateIds.count())-(int)maxPlayers() 
            << endl;
    KPlayer *currentPlayer=0;
    int currentPriority=0x7fff; // MAX_UINT (16bit?) to get the maximum of the list
    // find lowest network priority which is not yet in the newPlayerList
    // do this for the new players
    for ( player=newPlayerList.first(); player != 0; player=newPlayerList.next() ) 
    {
      // Already in the list
      if (inactivateIds.find(player->kgameId())!=inactivateIds.end()) 
      {
        continue;
      }
      if (player->networkPriority()<currentPriority)
      {
        currentPriority=player->networkPriority();
        currentPlayer=player;
      }
    }

    // find lowest network priority which is not yet in the newPlayerList
    // Do this for the network players
    for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) 
    {
      // Already in the list
      if (inactivateIds.find(player->kgameId())!=inactivateIds.end()) 
      {
        continue;
      }
      if (player->networkPriority()<currentPriority)
      {
        currentPriority=player->networkPriority();
        currentPlayer=player;
      }
    }

    // add it to inactivateIds
    if (currentPlayer)
    {
      boDebug(11001) << "Marking player " << currentPlayer->kgameId() << " for inactivation" << endl;
      inactivateIds.append(currentPlayer->kgameId());
    }
    else
    {
      boError(11001) << "Couldn't find a player to dectivate..That is not so good..." << endl;
      break;
    }
  }

  boDebug(11001) << "Alltogether deactivated " << inactivateIds.count() << " players" << endl;

  QValueList<int>::Iterator it;
  for ( it = inactivateIds.begin(); it != inactivateIds.end(); ++it )
  {
    int pid=*it;
    boDebug(11001) << " pid=" << pid << endl;
  }

  // Now deactivate the network players from the inactivateId list
  //QValueList<int>::Iterator it;
  for ( it = inactivateIds.begin(); it != inactivateIds.end(); ++it )
  {
    int pid=*it;
    if (KGameMessage::rawGameId(pid) == sender)
    {
      continue; // client's player
    }
    boDebug(11001) << " -> the network needs to deactivate " << pid <<endl;
    player=findPlayerByKGameId(pid);
    if (player)
    {
      // We have to make REALLY sure that the player is gone. With any policy
      systemInactivatePlayer(player);
      if (policy()!=PolicyLocal)
      {
        sendSystemMessage(player->kgameId(), KGameMessage::IdInactivatePlayer);
      }
    }
    else
    {
      boError(11001) << " We should deactivate a player, but cannot find it...not good." << endl;
    }
  }

  // Now send out the player list which the client can activate
  for ( player=newPlayerList.first(); player != 0; player=newPlayerList.next() )
  {
    boDebug(11001) << " newplayerlist contains " << player->kgameId() << endl;
    // Only activate what is not in the list
    if (inactivateIds.find(player->kgameId())!=inactivateIds.end())
    {
      continue;
    }
    boDebug(11001) << " -> the client can ******** reactivate ********  " << player->kgameId() << endl;
    sendSystemMessage(player->kgameId(), KGameMessage::IdActivatePlayer, sender);
  }

  // Save the game over the network
  QByteArray bufferS;
  QDataStream streamS(bufferS,IO_WriteOnly);
  // Save game over netowrk and save players
  savegame(streamS,true,true);
  sendSystemMessage(streamS,KGameMessage::IdGameLoad,sender);


  // Only to the client first , as the client will add players
  sendSystemMessage(sender, KGameMessage::IdGameSetupDone, sender);
}

// called by the IdSetupGame Message - CLIENT SIDE
// Client needs to prepare for network transfer
void KGame::setupGame(Q_UINT32 sender)
{
  QByteArray bufferS;
  QDataStream streamS(bufferS,IO_WriteOnly);

  // Deactivate all players
  KGamePlayerList mTmpList(d->mPlayerList); // we need copy otherwise the removal crashes
  Q_INT32 cnt=mTmpList.count();
  boDebug(11001) << "Client: playerlistcount=" << d->mPlayerList.count() << " tmplistcout=" << cnt << endl;

  streamS << cnt;

  QPtrListIterator<KPlayer> it(mTmpList);
  KPlayer *player;
  while (it.current())
  {
    player=it.current();
    systemInactivatePlayer(player);
    // Give the new game id to all players (which are inactivated now)
    player->setKGameId(KGameMessage::createPlayerId(player->kgameId(),gameId()));

    // Save it for the master to decide what to do
    savePlayer(streamS,player);

    ++it;
    --cnt;
  }
  if (d->mPlayerList.count() > 0 || cnt!=0)
  {
    boError(11001) << "KGame::setupGame(): Player list is not empty! or cnt!=0=" <<cnt << endl;
    exit(1);
  }

  sendSystemMessage(streamS,KGameMessage::IdSetupGameContinue,sender);
}

// unused by KGame
void KGame::syncRandom()
{
 int newseed=(int)d->mRandom->getLong(65535);
 sendSystemMessage(newseed,KGameMessage::IdSyncRandom); // Broadcast

#warning FIXME: do not do this for clean policy!
 d->mRandom->setSeed(newseed);
}

void KGame::Debug()
{
 KGameNetwork::Debug();
 boDebug(11001) << "------------------- KGAME -------------------------" << endl;
 boDebug(11001) << "this:          " << this << endl;
 boDebug(11001) << "uniquePlayer   " << d->mUniquePlayerNumber << endl;
 boDebug(11001) << "gameStatus     " << gameStatus() << endl;
 boDebug(11001) << "MaxPlayers :   " << maxPlayers() << endl;
 boDebug(11001) << "NoOfPlayers :  " << playerCount() << endl;
 boDebug(11001) << "NoOfInactive:  " << d->mInactivePlayerList.count() << endl;
 boDebug(11001) << "---------------------------------------------------" << endl;
}

void KGame::slotClientConnected(Q_UINT32 clientID)
{
 if (isAdmin())
 {
   negotiateNetworkGame(clientID);
 }
}

void KGame::slotServerDisconnected() // Client side
{
  boDebug(11001) << "======= SERVER DISCONNECT ======="<<endl;
  boDebug(11001) << "+++ (CLIENT)++++++++" << k_funcinfo << ": our GameID="<<gameId() << endl;

  int oldgamestatus=gameStatus();

  KPlayer *player;
  KGamePlayerList removeList;
  boDebug(11001) << "Playerlist of client=" << d->mPlayerList.count() << " count" << endl;
  boDebug(11001) << "Inactive Playerlist of client=" << d->mInactivePlayerList.count() << " count" << endl;
  for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) 
  {
    // TODO: CHECK: id=0, could not connect to server in the first place??
    if (KGameMessage::rawGameId(player->kgameId()) != gameId() && gameId()!=0)
    {
      boDebug(11001) << "Player " << player->kgameId() << " belongs to a removed game" << endl;
      removeList.append(player);
    }
  }


  // AB: this might be better after the setMaster() call, as we could make
  // signalReplacePlayerIO() work only with isAdmin()==true
  for ( player=removeList.first(); player != 0; player=removeList.next() )
  {
    bool remove = true;
    emit signalReplacePlayerIO(player, &remove);
    if (remove)
    {
      boDebug(11001) << " ---> Removing player " << player->kgameId() <<  endl;
      systemRemovePlayer(player,true); // no network necessary
    }
  }

  setMaster();
  boDebug(11001) << " our game id is after setMaster " << gameId() << endl;

  KGamePlayerList mReList(d->mInactivePlayerList);
  for ( player=mReList.first(); player != 0; player=mReList.next() )
  {
    // TODO ?check for priority? Sequence should be ok
    if ((int)playerCount()<maxPlayers() || maxPlayers()<0)
    {
      systemActivatePlayer(player);
    }
  }
  boDebug(11001) << " Players activated player-cnt=" << playerCount() << endl;

  for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) 
  {
    int oldid=player->kgameId();
    d->mUniquePlayerNumber++;
    player->setKGameId(KGameMessage::createPlayerId(d->mUniquePlayerNumber,gameId()));
    boDebug(11001) << "Player id " << oldid <<" changed to " << player->kgameId() << " as we are now local" << endl;
  }
  // TODO clear inactive lists ?
  Debug();
  for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) 
  {
    player->Debug();
  }
  boDebug(11001) << "+++++++++++" << k_funcinfo << " DONE=" << endl;
  emit signalClientLeftGame(0,oldgamestatus,this);
}

void KGame::slotClientDisconnected(Q_UINT32 clientID,bool /*broken*/) // server side
{
 boDebug(11001) << "++++(SERVER)+++++++" << k_funcinfo << " clientId=" << clientID << endl;

 int oldgamestatus=gameStatus();

 KPlayer *player;
 KGamePlayerList removeList;
 boDebug(11001) << "Playerlist of client=" << d->mPlayerList.count() << " count" << endl;
 for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() )
 {
   if (KGameMessage::rawGameId(player->kgameId())==clientID)
   {
     boDebug(11001) << "Player " << player->kgameId() << " belongs to the removed game" << endl;
     removeList.append(player);
   }
 }

 for ( player=removeList.first(); player != 0; player=removeList.next() )
 {
   // try to replace the KGameIO first
   bool remove = true;
   emit signalReplacePlayerIO(player, &remove);
   if (remove) {
     // otherwise (no new KGameIO) remove the player
     boDebug(11001) << " ---> Removing player " << player->kgameId() <<  endl;
     removePlayer(player,0);
   }
 }

 // Now add inactive players - sequence should be ok
 // TODO remove players from removed game
 for (unsigned int idx=0;idx<d->mInactiveIdList.count();idx++)
 {
   QValueList<int>::Iterator it1 = d->mInactiveIdList.at(idx);
   player = findPlayerByKGameId(*it1);
   if (((int)playerCount() < maxPlayers() || maxPlayers() < 0) && player && KGameMessage::rawGameId(*it1) != clientID)
   {
     activatePlayer(player);
   }
 }
  emit signalClientLeftGame(clientID,oldgamestatus,this);
}


// -------------------- Synchronisation -----------------------

// this initializes a newly connected client.
// we send the number of players (including type) as well as game status and
// properties to the client. After the initialization has been completed both
// clients should have the same status (ie players, properties, etc)
void KGame::negotiateNetworkGame(Q_UINT32 clientID)
{
 boDebug(11001) << "===========================" << k_funcinfo << ": clientID=" << clientID << " =========================== "<< endl;
 if (!isAdmin())
 {
   boError(11001) << k_funcinfo << ": Serious WARNING..only gameAdmin should call this" << endl;
   return ;
 }

 QByteArray buffer;
 QDataStream streamGS(buffer,IO_WriteOnly);

 // write Game setup specific data
 //streamGS << (Q_INT32)maxPlayers();
 //streamGS << (Q_INT32)minPlayers();

 // send to the newly connected client *only*
 Q_INT16 v=KGameMessage::version();
 Q_INT32 c=cookie();
 streamGS << v << c;
 sendSystemMessage(streamGS, KGameMessage::IdSetupGame, clientID);
}

bool KGame::sendGroupMessage(const QByteArray &msg, int msgid, Q_UINT32 sender, const QString& group)
{
// AB: group must not be i18n'ed!! we should better use an id for group and use
// a groupName() for the name // FIXME
 KPlayer *player;
 for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() )
 {
   if (player && player->group()==group)
   {
     sendMessage(msg,msgid,player->kgameId(), sender);
   }
 }
 return true;
}

bool KGame::sendGroupMessage(const QDataStream &msg, int msgid, Q_UINT32 sender, const QString& group)
{ return sendGroupMessage(((QBuffer*)msg.device())->buffer(), msgid, sender, group); }

bool KGame::sendGroupMessage(const QString& msg, int msgid, Q_UINT32 sender, const QString& group)
{
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << msg;
 return sendGroupMessage(stream, msgid, sender, group);
}

bool KGame::addProperty(KGamePropertyBase* data)
{ return dataHandler()->addProperty(data); }

bool KGame::sendPlayerProperty(int msgid, QDataStream& s, Q_UINT32 playerId)
{ return sendSystemMessage(s, msgid, playerId); }

void KGame::sendProperty(int msgid, QDataStream& stream, bool* sent)
{
  bool s = sendSystemMessage(stream, msgid);
  if (s)
  {
    *sent = true;
  }
}

void KGame::emitSignal(KGamePropertyBase *me)
{
 emit signalPropertyChanged(me,this);
}

KGamePropertyBase* KGame::findProperty(int id) const
{ return d->mProperties->find(id); }

KGame::GamePolicy KGame::policy() const
{
 return d->mPolicy;
}
void KGame::setPolicy(GamePolicy p,bool recursive)
{
 // Set KGame policy
 d->mPolicy=p;
 if (recursive)
 {
   // Set all KGame property policy
   dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p,false);

   // Set all KPLayer (active or inactive) property policy
   for (QPtrListIterator<KPlayer> it(d->mPlayerList); it.current(); ++it)
   {
     it.current()->dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p,false);
   }
   for (QPtrListIterator<KPlayer> it(d->mInactivePlayerList); it.current(); ++it)
   {
     it.current()->dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p,false);
   }
 }
}

/*
 * vim: et sw=2
 */
