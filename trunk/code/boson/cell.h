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
	 * @return Guess what?
	 **/
	GroundType groundType() const
	{
		return mType;
	}

	/**
	 * @return The version of the groundType. Something like "grass->desert" or
	 * "only grass" or "another only grass" or so. At least I think so...
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

protected:
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
