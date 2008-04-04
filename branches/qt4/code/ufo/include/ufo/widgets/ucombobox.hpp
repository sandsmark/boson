/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ucombobox.hpp
    begin             : Sat May 24 2003
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef UCOMBOBOX_HPP
#define UCOMBOBOX_HPP

#include "uwidget.hpp"

namespace ufo {

class UItem;
class UIcon;
class UColor;

class UListBox;
class UTextEdit;

/** @short A button with a popup item list.
  * @ingroup widgets
  *
  * A UComboBox provides a list of options as choice to the user.
  * <p>
  * Currently it is implemented using a text edit and a list box widget.
  * But this might change.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UComboBox : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UComboBox)
	UFO_UI_CLASS(UComboBoxUI)
	UFO_STYLE_TYPE(UStyle::CE_ComboBox)
public: // Public constructors
	UComboBox();
	UComboBox(const std::vector<UString> & listDataA);
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
	const std::vector<UItem*> & getItems() const;

	unsigned int getItemCount() const;

public: // Other methods
	/** Sets the row count which should be visible when the combo box pops up
	  * its menu.
	  */
	void setVisibleRowCount(int visibleRowCountA);
	int getVisibleRowCount() const;

public:

	/** Returns the currently activated item. This item normally shows up
	  * as content of the text field.
	  */
	int getCurrentItem() const;
	void setCurrentItem(int index);

	/** Returns the text representation of the current item.
	  * @see getCurrentItem
	  */
	std::string getCurrentText() const;
	void setCurrentText(const std::string & text);

	/** Pops up the combo box menu */
	virtual void popup();

public: // Public methods
	virtual void setListBox(UListBox * listBox);
	UListBox * getListBox() const;

	virtual void setTextEdit(UTextEdit * textEdit);
	UTextEdit * getTextEdit() const;

public: // Public signal accessors
	typedef USignal2<UComboBox*, int> sig_activated_t;
	typedef USignal2<UComboBox*, int> sig_highlighted_t;
	typedef USignal3<UComboBox*, int, int> sig_selection_change_t;
	sig_activated_t & sigActivated();
	sig_highlighted_t & sigHighlighted();
	sig_selection_change_t & sigSelectionChanged();

protected: // Overrides UWidget
	virtual UDimension getContentsSize(const UDimension & maxSize) const;
	virtual void processMouseEvent(UMouseEvent * e);
	virtual void processKeyEvent(UKeyEvent * e);

protected: // Protected attributes
	// internally used widgets to represent the data
	UListBox * m_listBox;
	UTextEdit * m_textEdit;

private: // Private attributes
	UItem * m_activated;
	std::string m_currentText;

	int m_visRowCount;
private: // Private signals
	sig_activated_t m_sigActivated;
	sig_highlighted_t m_sigHighlighted;
	sig_selection_change_t m_sigSelectionChanged;
};


inline UComboBox::sig_activated_t &
UComboBox::sigActivated() {
	return m_sigActivated;
}

inline UComboBox::sig_highlighted_t &
UComboBox::sigHighlighted() {
	return m_sigHighlighted;
}

inline UComboBox::sig_selection_change_t &
UComboBox::sigSelectionChanged() {
	return m_sigSelectionChanged;
}

} // namespace ufo

#endif // UCOMBOBOX_HPP
