/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ubutton.hpp
    begin             : Mon May 28 2001
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

#ifndef UBUTTON_HPP
#define UBUTTON_HPP

#include "ucompound.hpp"
#include "../events/ukeysym.hpp"
#include "../ukeystroke.hpp"

// we need this for proper getUI() overriding
//#include "../ui/ubuttonui.hpp"

namespace ufo {
class UButtonGroup;

/** The base class for all clickable widgets.
  * In its basic version, it is a standard push button.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UButton : public UCompound {
	UFO_DECLARE_DYNAMIC_CLASS(UButton)
	UFO_UI_CLASS(UButtonUI)
public:  // Public types
	enum {
		RolloverEnabled = 1,
		Toggable = 2,
		BorderPainted = 4,
		Rollover = 8,
		Pressed = 16,
		Selected = 32,
		Armed = 64
	};
public:
	UButton();
	UButton(UIcon * icon);
	UButton(const std::string & text, UIcon * icon = NULL);
/*
public: // hides | overrides UWidget
	void setUI(UButtonUI * ui);
	UWidgetUI * getUI() const;
	void updateUI();
*/
public: // Public methods
	/** Emulates a mouse click on that button, i.e. shows a pressed button,
	  * releases it and calls activate.
	  */
	virtual void doClick();

	/** Emulates a mouse click on that button, i.e. shows a pressed button,
	  * releases it and calls activate.
	  * @param millis How long should that button be pressed
	  * @see #doClick
	  */
	virtual void doClick(int millis);

	/** Checks whether the button is selected or not.
	  * Only toggable buttons like check boxes and radio buttons can
	  * be selected.
	  * @see UCheckBox
	  * @see URadioButton
	  * @see UCheckBoxMenuItem
	  * @return true if the button is selected
	  */
	virtual bool isSelected() const ;

	/** Sets whether the button is selected or not.
	  * @param b The new selection state
	  */
	virtual void setSelected(bool b);

	/** Checks whether the button is pressed or not.
	  * This is mainly for visible effects.
	  * @return true if the button is down
	  */
	virtual bool isPressed() const;

	/** Sets whether the button is pressed or not.
	  * A button is pressed normally by mouse or keyboard.
	  * @param b If true, the button is pressed
	  */
	virtual void setPressed(bool b);

	/** checks whether rollover effects are enabled
	  * @return the rollover flag
	  */
	bool isRolloverEnabled() const ;

	/** sets whether rollover effects should be enabled
	  * @param b If true, rollover effects are painted
	  */
	void setRolloverEnabled(bool b);

	/** checks whether rollover effects are taking places, i.e. mouse is over
	  * button
	  * @return the state of rollover activities
	  */
	bool isRollover() const ;

	/** sets whether rollover effects should perform now, i.e. mouse is over
	  * button
	  * @param b
	  * 	if true, the button is painted if the mouse is over the button
	  */
	void setRollover(bool b);

	/** if a button is toggable, you change the state by a mouse click
	  * @return if true, the button is toggable
	  */
	bool isToggable() const ;

	/** if a button is toggable, you change the state by a mouse click
	  * @param b if true, the button is now toggable
	  */
	void setToggable(bool b);

	/** Gets the action command
	  * @see #setActionCommand
	  * @return The string which describes the performed action.
	  */
	virtual std::string getActionCommand() const;

	/** Sets the action command. This string is delivered as action command
	  * to created action events. If no action command is specified,
	  * the current text caption is used as actino command.
	  *
	  * @param actionCommand The action command used for action events
	  */
	virtual void setActionCommand(const std::string & actionCommand);

	//
	// icons
	//
	/**
	  */
	void setPressedIcon(UIcon * icon);
	/** @return The icon used if this button is down. May be NULL.
	  */
	UIcon * getPressedIcon() const;
	/**
	  */
	void setRolloverIcon(UIcon * icon);
	/** @return The icon used if this button has mouse focus. May be NULL.
	  */
	UIcon * getRolloverIcon() const;

	/** Sometimes it is unwanted that the border is painted
	  * (e.g. internal frame closebuttons).
	  * This affects also the insets returned by getInsets();
	  */
	void setBorderPainted(bool b);
	/**
	  * @return true if the border of this button should be painted
	  * @see setBorderPainted
	  */
	bool isBorderPainted() const;

	/** Sets the accelerator. If the given key combination is pressed
	  * and the focused widget does not process it, the button will be
	  * activated.
	  */
	void setAccelerator(const UKeyStroke & stroke);
	UKeyStroke getAccelerator() const;

	/** If the accelerator is Alt+character and the text caption contains that
	  * letter, the index of this character within the caption is returned. */
	int getAcceleratorIndex() const;

	/** Sets the button group. Only one button within a button group
	  * can be selected.
	  */
	void setButtonGroup(UButtonGroup * buttonGroup);
	UButtonGroup * getButtonGroup() const;

	/** Activates the button and fires an action event. */
	virtual void activate();

public: // Overrides UWidget
	virtual UInsets getInsets() const;
	virtual bool isActive() const;

public: // Overrides UCompound
	virtual UIcon * getIcon() const;
	virtual void setText(const std::string & text);

public: // deprecated

	/** Checks whether the button is armed or not
	  * @return true if the button is armed
	  */
	bool isArmed() const ;

	/** Sets whether the button is armed or not
	  * @param b The new "armed"-state
	  */
	void setArmed(bool b);

protected:  // Protected slots
	void fireActionEvent();
	void keybindingSlot(UActionEvent * e);
	/** A slot for the doClick method. */
	void buttonUp();

protected:  // Overrides UWidget
	/** First, check if border should be painted
	  */
	virtual void paintBorder(UGraphics * g);
	virtual void addedToHierarchy();
	virtual void removedFromHierarchy();

private:  // Private attributes
	/**  the icon which is shown when the icon is pressed */
	UIcon * m_pressedIcon;
	/** * the icon which is shown when the mouse is over the button */
	UIcon * m_rolloverIcon;

	/** */
	std::string m_actionCommand;

	/** saves all state flags */
	uint32_t m_flags;
	UButtonGroup * m_buttonGroup;

	UKeyStroke m_accelerator;
	int m_acceleratorIndex;

public: // Public signal methods
	/** deprecated */
	USignal1<UActionEvent*> & sigButtonClicked() { return m_sigActivated; }
	/** deprecated */
	USignal1<UActionEvent*> & sigButtonToggled() { return m_sigActivated; }

	/** This signal is fired when the button has been clicked, toggled etc.
	  */
	USignal1<UActionEvent*> & sigActivated() { return m_sigActivated; }

	/** This signal is fired when the button gets some type of highlight.
	  * This can mean that the button got mouse or keyboard focus.
	  */
	USignal1<UActionEvent*> & sigHighlighted() { return m_sigHighlighted; }

private: // Private signals
	USignal1<UActionEvent*> m_sigActivated;
	USignal1<UActionEvent*> m_sigHighlighted;
};


//
// inline implementation
//

inline UIcon *
UButton::getPressedIcon() const {
	return m_pressedIcon;
}

inline UIcon *
UButton::getRolloverIcon() const {
	return m_rolloverIcon;
}

} // namespace ufo

#endif // UBUTTON_HPP
