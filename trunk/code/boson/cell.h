#ifndef __CELL_H__
#define __CELL_H__

class UnitProperties;

class Cell
{
public:
	enum GroundType {
		GroundUnknown = 0,
		GroundDeepWater = 1,
		GroundWater = 2,
		GroundGrass = 3,
		GroundDesert = 4,
		
		GroundGrassMineral = 5,
		GroundGrassOil = 6,

		GroundLast = 7
	};
        enum TransType {
		TransGrassWater = 0,
		TransGrassDesert,
		TransDesertWater,
		TransDeepWater,
		TransLast
	};
	enum Transition {
		TransUpLeft = 0,
		TransUpRight,
		TransDownLeft,
		TransDownRight,

		TransUp,
		TransDown,
		TransLeft,
	       	TransRight,
		
		TransUpLeftInverted,
		TransUpRightInverted,
		TransDownLeftInverted,
		TransDownRightInverted
	};


	Cell();
	~Cell();

	void makeCell(int groundType, unsigned char version);

	/**
	 * @return The number of different groundTypes.
	 **/
	static int groundTypeCount()
	{
		return GroundLast;
	}

	/**
	 * This has nothing to do with @ref GroundType! While @ref GroundType
	 * contains the basic plain tiles only this can also be a transtition!!
	 **/
	int groundType() const
	{
		return mType;
	}

	/**
	 * @return The version of the groundType. 
	 **/
	unsigned char version() const
	{
		return mVersion;
	}

	/**
	 * @return The number of the tile in the tile file (earth.png)
	 **/
	static int tile(int groundType, unsigned char version)
	{
		return (groundType << 2 | (version & 0x3) );
	}

	/**
	 * @return Whether the specified unit can go over this ground. Note
	 * that this does <em>not</em> check whether the cell is occupied.
	 **/
	bool canGo(const UnitProperties* unit) const;
	static bool canGo(const UnitProperties* unit, GroundType ground);

	/**
	 * @return The moving cost of this cell. Higher value means the unit
	 * should move slower here.
	 **/
	int moveCost() const; // leave signed, maybe we can use negative values for roads one day?


	/**
	 * @return Whether ground is a plain tile
	 **/
	static bool isPlain(int ground);
	static bool isValidGround(int ground);
	static bool isTrans(int ground);

	/**
	 * @return How many tiles of every transition exist.
	 **/
	static int tilesPerTransition();
	static int groundTilesNumber();
	static int bigTilesPerTransition();
	static int smallTilesPerTransition();

	static int getTransRef(int g); // does this return a @ref TransType ??

	/**
	 * @return The tile number with transRef and transTile
	 **/
	static int getTransNumber(TransType transRef, int transTile);
	
	/**
	 * @return The number of the big tile with transRef and transTile
	 **/
	static int getBigTransNumber(TransType transRef, int transTile);

	static int smallTileNumber(int smallNo, TransType trans, bool inverted);


	static bool isSmallTrans(int g);
	static bool isBigTrans(int g);

	/**
	 * @return With which groundtype this transition starts.
	 **/
	static GroundType from(TransType trans);

	/**
	 * @return With which groundtype this transition ends.
	 **/
	static GroundType to(TransType trans);

protected:
	static int getTransTile(int g);

	void setVersion(unsigned char v)
	{
		mVersion = v;
	}

	void setGroundType(GroundType type);

private:
	GroundType mType;
	unsigned char mVersion;
};

#endif
