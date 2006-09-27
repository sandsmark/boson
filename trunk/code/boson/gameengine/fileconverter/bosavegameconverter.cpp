/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosavegameconverter.h"

#include "bosonfileconverter.h"
#include "../boversion.h"
#include "bodebug.h"

#include <qmap.h>

// AB: must be the LAST include
#include "no_game_code.h"


BoSaveGameConverter::BoSaveGameConverter()
{
}

BoSaveGameConverter::~BoSaveGameConverter()
{
}

bool BoSaveGameConverter::convertFiles(QMap<QString, QByteArray>& destFiles)
{
 boDebug() << k_funcinfo << "converting from " << handlesBosonVersionString() << " format" << endl;
 bool ret = convert(destFiles);
 if (!ret) {
	boError() << k_funcinfo << "could not convert from boson " << handlesBosonVersionString() << " file format" << endl;
 }
 return ret;
}


class BoSaveGameConverter_09 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_SAVEGAME_FORMAT_VERSION_0_9;
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.9";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_09::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_9_To_0_9_1(destFiles);
}

class BoSaveGameConverter_091 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_SAVEGAME_FORMAT_VERSION_0_9_1;
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.9.1";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_091::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_9_1_To_0_10(destFiles);
}

class BoSaveGameConverter_010 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_SAVEGAME_FORMAT_VERSION_0_10;
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.10";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_010::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_10_To_0_10_80(destFiles);
}

class BoSaveGameConverter_01080 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x04);
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.10.80";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_01080::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_10_80_To_0_10_81(destFiles);
}

class BoSaveGameConverter_01081 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x05);
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.10.81";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_01081::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_10_81_To_0_10_82(destFiles);
}

class BoSaveGameConverter_01082 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x06);
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.10.82";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_01082::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_10_82_To_0_10_83(destFiles);
}

class BoSaveGameConverter_01083 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x07);
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.10.83";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_01083::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_10_83_To_0_10_84(destFiles);
}

class BoSaveGameConverter_01084 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x08);
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.10.84";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_01084::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_10_84_To_0_10_85(destFiles);
}

class BoSaveGameConverter_01085 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x02, 0x09);
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.10.85";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_01085::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_10_85_To_0_11(destFiles);
}

class BoSaveGameConverter_011 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_SAVEGAME_FORMAT_VERSION_0_11;
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.11";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_011::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_11_To_0_11_80(destFiles);
}

class BoSaveGameConverter_01180 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x03, 0x00);
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.11.80";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_01180::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_11_80_To_0_11_81(destFiles);
}

class BoSaveGameConverter_01181 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_MAKE_SAVEGAME_FORMAT_VERSION(0x00, 0x03, 0x01);
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.11.81";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_01181::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_11_81_To_0_12(destFiles);
}

class BoSaveGameConverter_012 : public BoSaveGameConverter
{
public:
	virtual int handlesVersion() const
	{
		return BOSON_SAVEGAME_FORMAT_VERSION_0_12;
	}
	virtual QString handlesBosonVersionString() const
	{
		return "0.12";
	}
	virtual bool convert(QMap<QString, QByteArray>& destFiles);
};

bool BoSaveGameConverter_012::convert(QMap<QString, QByteArray>& destFiles)
{
 BosonFileConverter converter;
 return converter.convertPlayField_From_0_12_To_0_13(destFiles);
}


QMap<int, BoSaveGameConverter*> BoSaveGameConverter::createConverters()
{
 QMap<int, BoSaveGameConverter*> map;

 insertNewConverterToMap(map, new BoSaveGameConverter_09());
 insertNewConverterToMap(map, new BoSaveGameConverter_091());
 insertNewConverterToMap(map, new BoSaveGameConverter_010());
 insertNewConverterToMap(map, new BoSaveGameConverter_01080());
 insertNewConverterToMap(map, new BoSaveGameConverter_01081());
 insertNewConverterToMap(map, new BoSaveGameConverter_01082());
 insertNewConverterToMap(map, new BoSaveGameConverter_01083());
 insertNewConverterToMap(map, new BoSaveGameConverter_01084());
 insertNewConverterToMap(map, new BoSaveGameConverter_01085());
 insertNewConverterToMap(map, new BoSaveGameConverter_011());
 insertNewConverterToMap(map, new BoSaveGameConverter_012());

 return map;
}

