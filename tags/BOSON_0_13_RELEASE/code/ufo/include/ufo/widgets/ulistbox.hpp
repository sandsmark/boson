/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ulistbox.hpp
    begin             : Tue Jun 18 2002
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef ULISTBOX_HPP
#define ULISTBOX_HPP

#include "uscrollablewidget.hpp"

#include "../util/ucolor.hpp"

namespace ufo {

class UItem;
class UIcon;
class UColor;

/** @short A widget with several items
  * @ingroup widgets
  *
  * This widget may be added to a scroll pane.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UListBox : public UScrollableWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UListBox)
	UFO_UI_CLASS(UListBoxUI)
	UFO_STYLE_TYPE(UStyle::CE_ListBox)
public: // Public Types
	/** Allow either single selection or multiple selection.
	  * So far, multiple selection isn't fully implemented
	  * (or should I say not at all).
	  */
	enum SelectionMode {
		SingleSelection,
		MultipleSelection,
		NoSelection
	};
public: // Public constructors
	UListBox();
	UListBox(const std::vector<UString> & listDataA);

public: // overrides UScrollableWidget
	virtual int getUnitIncrement(Orientation orientation = Horizontal) const;
	virtual int getBlockIncrement(Orientation orientation = Horizontal) const;

	virtual UDimension getPreferredViewportSize() const;

public: // Item accessing methods
	/** Appends the strings of list data to the list box.
	  * Using add prefix instead of insert to match UWidget
	  * and other classes.
	  */
	void addList(const std::vector<UString> & listData);
	/** Adds item at index. References item.
	  */
	void addItem(UItem * itemA, int index = -1);
	/** Adds item at index. References item.
	  */
	void addItem(const UString & itemA, int index = -1);
	/** Adds item at index. References item.
	  */
	void addItem(UIcon * itemA, int index = -1);

	/** Removes item at index. */
	void removeItem(unsigned int index);
	void removeAllItems();

	UItem * getItemAt(unsigned int n) const;
	const std::vector<UItem *> & getItems() const;

	unsigned int getItemCount() const;

public: // Selection methods
	SelectionMode getSelectionMode() const;
	void setSelectionMode(SelectionMode modeA);

	int getSelectedIndex() const;
	void setSelectedIndex(int indexA);

	void invertSelection();
	void selectAll();

	std::vector<unsigned int> getSelectedIndices() const;
	void setSelectedIndices(const std::vector<unsigned int> & indicesA);

	bool isSelectedIndex(unsigned int indexA);

	UItem * getSelectedItem();
	std::vector<UItem*> getSelectedItems();

public: // Other methods
	/** Sets the row count which should be visible when this list is shown
	  * within a scrollpane.
	  */
	void setVisibleRowCount(int visibleRowCountA);
	int getVisibleRowCount() const;

	/** @return The top-left corner of the indexed list box item. */
	UPoint indexToLocation(unsigned int indexA);
	/** @return The Index of the item at locationA or -1 when there is no
	  * item under the given point. */
	int locationToIndex(const UPoint & locationA);

protected: // Overrides UWidget
	virtual void paintWidget(UGraphics * g);
	virtual UDimension getContentsSize(const UDimension & maxSize) const;
	virtual void processMouseEvent(UMouseEvent * e);
	virtual void processKeyEvent(UKeyEvent * e);
	virtual void processWidgetEvent(UWidgetEvent * e);

public: // Public signals
	/** UListBox * listBox, int firstIndex, int lastIndex */
	typedef USignal3<UListBox*, int, int> sig_selection_change_t;
	sig_selection_change_t & sigSelectionChanged();

protected: // Protected methods
	void addItemImpl(UItem * itemA, int index);

	void fireSelectionEvent(int firstIndexA, int lastIndexA);

private: // Private attributes
	SelectionMode m_selectionMode;

	std::vector<UItem *> m_items;
	std::vector<unsigned int> m_indices;

	int m_visRowCount;
private: // Private signals
	sig_selection_change_t m_sigSelectionChanged;
};

//
// inline implementation
//
inline UListBox::sig_selection_change_t &
UListBox::sigSelectionChanged() {
	return m_sigSelectionChanged;
}

/** Abstract class for list box items.
  * @author Johannes Schmidt
  */
  /*
class UFO_EXPORT UListBoxItem : public virtual UCollectable {
	UFO_DECLARE_DYNAMIC_CLASS(UListBoxItem)
public:
	virtual void paintItem(UListBox * listBoxA, int x, int y,
		bool isSelectedA, bool hasFocusA) = 0;
	virtual UDimension getItemSize(const UListBox * listBoxA) const = 0;
	*/
	/** FIXME ! not entirely true ...
	  * Notifies the list box item that it was added (b == true) to or
	  * removed from a list box.
	  * May be useful to initialize some context dependent data.
	  */
	  /*
	virtual void notify(UListBox * listBox, bool b) = 0;
};
*/

} // namespace ufo

#endif // ULISTBOX_HPP
