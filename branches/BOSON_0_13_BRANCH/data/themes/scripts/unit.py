from utils import boprint

try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")





class Unit:
  """
  A wrapper for BoScript which handles everything about a single unit.
  @see UnitGroup

  Member variables:
  * mPlayer   = the player that own this unit
  * mID       = the ID of the unit
  """

  #
  #----------------------  public  ----------------------
  #
  def __init__( self, player, id ):
    # all these variables are private - use them read only!
    self.mPlayer = player
    self.mID     = id

  def id( self ):
    return self.mID

  def attack( self, targetID ):
    """
    Unit will attack the unit with the given target ID. In case the
    target is not in weapon range, the unit will move until the target
    is close enough to be attacked."""
    BoScript.attack( self.mID, targetID )

  def canMineMinerals( self ):
    """@return @c true if the unit can mine minerals, otherwise @c false."""
    return BoScript.canUnitMineMinerals( self.mID )

  def canMineOil( self ):
    """@return @c true if the unit can mine oil, otherwise @c false."""
    return BoScript.canUnitMineOil( self.mID )

  def canProduce( self ):
    """
    @return @c true if the unit can produce something (units or upgrades),
    otherwise @c false."""
    return BoScript.canUnitProduce( self.mID )

  def canShoot( self ):
    """@return @c true if the unit can shoot, otherwise @c false."""
    return BoScript.canUnitShoot( self.mID )

  def dropBomb( self, weapon, x, y ):
    """Drops a bomb by using the specified weapon at the given position."""
    BoScript.dropBomb( self.mID, weapon, x, y )

  def isAircraft( self ):
    """@return @c true if the unit is an aircraft, otherwise @c false."""
    return BoScript.isUnitAircraft( self.mID )

  def isAlive( self ):
    """@return @c true if the unit is alive, otherwise @c false."""
    return BoScript.isUnitAlive( self.mID )

  def isMobile( self ):
    """@return @c true if the unit is mobile, otherwise @c false."""
    return BoScript.isUnitMobile( self.mID )

  def mine( self, x, y ):
    """Sends the unit to the mine at (x,y)."""
    BoScript.mineUnit( self.mID, x, y )

  def move( self, x, y ):
    """
    Move unit to position x, y. The unit will go to the given poistion without
    attacking other units on its way."""
    BoScript.moveUnit( self.mID, x, y )

  def moveWithAttacking( self, x, y ):
    """
    The same as in @ref moveUnit, but the unit will attack enemy units
    on its way."""
    BoScript.moveUnitWithAttacking( self.mID, x, y )

  def owner( self ):
    """@return The owner of the unit."""
    return BoScript.unitOwner( self.mID )

  def position( self ):
    """@return The unit position."""
    return BoScript.unitPosition( self.mID )

  def produceUnit( self, productID ):
    """Produces a unit of the given product ID."""
    BoScript.produceUnit( self.mID, productID )

  def setRotation( self, rotation ):
    """Sets the unit's rotation to the given value."""
    BoScript.setUnitRotation( self.mID, rotation )

  def stop( self ):
    """Stops unit from doing anything and becomes idle. Note that even
    idle units shoot at any enemy units in range. If stop was called while
    the unit was attacking, then after calling stop() it may start to
    shoot at another unit."""
    BoScript.stopUnit( self.mID )

  def type( self ):
    """@return The type of the unit."""
    return BoScript.unitType( self.mID )

  def teleport( self, x, y ):
    """Immediately moves the unit to position (x,y)."""
    BoScript.teleportUnit( self.mID, x, y )

  def work( self ):
    """
    Current work the unit is involved in, e.g., attacking, moving, ...
    For a complete list of possible return values see unitbase.h .
    @return the ID of the current work."""
    return BoScript.unitAdvanceWork( self.mID )
  def sightRange( self ):
    """
    @return how many cells away the unit can see."""
    return BoScript.unitSightRange( self.mID )
