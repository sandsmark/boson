/*
    This file is part of the Boson game
    Copyright (C) 2003-2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONCAMPAIGN_H
#define BOSONCAMPAIGN_H

class BosonPlayField;
class BPFPreview;

class QString;
class QStringList;

class BosonCampaignPrivate;
class BosonDataObject;

/**
 * A boson "campaign" is a collection of @ref BosonPlayField objects.
 *
 * At the moment this is nothing more, than simply a collection, in order to
 * group some playfields in the startup widgets. In the future we may add
 * functions to check whether the player has passed a mission and to force the
 * player to pass certain missions before he is allowed to play later missions.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCampaign
{
public:
	BosonCampaign(const QString& identifier, const QString& name);
	~BosonCampaign();

	/**
	 * @return The name of this campaign. i18n'ized.
	 **/
	QString name() const;


	/**
	 * @return A unique identifier for this campaign.
	 **/
	QString identifier() const;

	/**
	 * Add a @ref BosonPlayField object to the campaign. Ownership is not
	 * taken, it will not be deleted in this class.
	 *
	 * Remember that you must remove the playfield, when you delete it
	 * somewhere!
	 **/
	void addPlayField(BPFPreview*);

	/**
	 * Remove the playfield from the campaign. It is not deleted!
	 **/
	void removePlayField(BPFPreview*);

	/**
	 * @return The number of @ref BosonPlayField objects in this campaign
	 **/
	unsigned int playFieldCount() const;

	QStringList playFields() const;

	/**
	 * @return A @ref BosonDataObject for @p campaign, that can be used to
	 * insert this campaign into @ref BosonData.
	 **/
	static BosonDataObject* campaignDataObject(BosonCampaign* campaign);

private:
	BosonCampaignPrivate* d;
};

#endif
