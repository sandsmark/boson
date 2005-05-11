/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uwidget.hpp
    begin             : Sun May 13 2001
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

#ifndef UWIDGET_HPP
#define UWIDGET_HPP

#include "../uobject.hpp"

#include <map>

// headers in util
#include "../util/ugeom.hpp"
#include "../util/uinsets.hpp"
//#include "../util/uhashmap.hpp"
#include "../util/ustring.hpp"
#include "../util/ucolor.hpp"
#include "../util/ucolorgroup.hpp"
#include "../util/upalette.hpp"


// headers in events
#include "../events/uevent.hpp"

// headers in layouts
#include "../layouts/ulayoutmanager.hpp"

// headers in signals
#include "../signals/ufo_signals.hpp"

//#include "../ukeystroke.hpp"

/** returns the string that defines the ui class. Needed to get proper
  * ui classes from ui manager. Subclasses with own look and feel should
  * override this to get proper ui delegates.
  * @see UUIManager#getUI
  */
#define UFO_UI_CLASS(name) \
public:                    \
	virtual std::string getUIClassID() const { \
		return #name; \
	}

namespace ufo {

typedef std::map<std::string, UObject*> UPropertiesMap;


// forward declartions
class UColor;
class UColorGroup;
class UFont;
class UBorder;
class UUIManager;
class UWidgetUI;
class URootPane;

class UPopupMenu;

class UDrawable;

class UInputMap;
class UKeyStroke;

class UWidgetUI;

class UContext;
class UUIManager;
class UGraphics;

class UEvent;
class UActionEvent;
class UFocusEvent;
class UKeyEvent;
class UMouseEvent;
class UMouseWheelEvent;
class UPropertyChangeEvent;
class UWidgetEvent;

/** The base class for all widgets.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UWidget : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UWidget)
	UFO_UI_CLASS(UWidgetUI)
private:
	friend class UWidgetUI;

public:  // Public types
	// keyboard input conditions
	enum InputCondition {
		WhenFocused = 1,
		WhenAncestorFocused = 2
	};
	// for validation process
	enum Validation {
		ValidationNone = 0,
		ValidationLayout = 1,
		ValidationUI = 2,
		ValidationUIAttributes = 4, // special flag for reinstalling the UI
		ValidationAll = ValidationLayout | ValidationUI | ValidationUIAttributes,
		ValidationLast = 4
	};
	enum EventState {
		NoEvents = 0x0000,
		MouseEvents = 0x0001,
		MouseMotionEvents = 0x0002,
		MouseWheelEvents = 0x0004,
		KeyEvents = 0x0008,
		WidgetEvents = 0x0010,
		FocusEvents = 0x0020
	};
public:
	UWidget();
	UWidget(ULayoutManager * layout);
	virtual ~UWidget();

	/**
	 * Set an object that deletes the corresponding BoUfoWidget of this
	 * UWidget object. The deleter is deleted first in the destructor of
	 * this class.
	 *
	 * WARNING: this is a BoUfo extension to libufo. It is NOT part of the
	 * libufo API!
	 **/
	void setBoUfoWidgetDeleter(UCollectable* deleter);

	/** No descriptions */
	virtual bool isVisible() const;
	/** mark the widget as visible -- this is the default state.
	  * If a wiget is not visible, the widget itsself and all of its children
	  * will not be painted.
	  */
	virtual void setVisible(bool v);

	/** Switches clipping.
	  * If b is true, you can only draw within the rectangle given by
	  * the bounds of this widget.
	  * Default value is no clipping!
	  */
	void setClipping(bool b);
	/** Returns true when clipping is enabled. */
	bool hasClipping() const;

	/** Checks whether this widget is enabled or not */
	bool isEnabled() const;
	/** Enables or disables this widget. A disabled widget does not receive
	  * input events (mouse, keyboard) and is usually drawn with a different
	  * colorgroup (e.g. gray).
	  * Default value is true!
	  */
	void setEnabled(bool b);
	/** @return True if this widget if fully opaque (opacity == 1)
	  * @see getOpacity()
	  */
	bool isOpaque() const;
	/** Sets the background opacity to 1 (fully opaque) if @p o is true
	  * otherwise sets the opacity to 0 which means fully transparent.
	  * The default value is 1.
	  * @see setOpacity
	  */
	virtual void setOpaque(bool o);
	/** Sets the opacity of the widget background.
	  * 1.0 means fully opaque which is the default.
	  * This effect might show unwanted result when used with more complex
	  * widgets or when using a different look and feel.
	  */
	virtual void setOpacity(float f);
	virtual float getOpacity() const;

	/** Returns whether this widget is active.
	  * Mostly this means that the current widget has mouse focus.
	  * But this may differ from widget to widget (e.g. text widget are
	  * active if they have keyboard focus).
	  * This is not really an isolated widget state but the combination of
	  * other states. It is used as style hint for the UI object.
	  */
	virtual bool isActive() const;

	/** Many attributes of a widget can only be stated when it is
	  * within a valid containment hierarchy.
	  * For example, the correct UI object can be determined only if
	  *  the top level widget belongs to a UFO context.
	  * @see sigWidgetAdded
	  * @see sigWidgetRemoved
	  *
	  * @return true if this widget is in a containment hierarchy
	  *  visible on screen.
	  */
	virtual bool isInValidHierarchy() const;
	/** Returns true when this widget has a UI object (responsible for
	  * drawing and event handling) and
	  * a layout manager has laid out all children.
	  */
	bool isValid() const;
	/** relayouts the container and checks whether the ui delegate is valid.
	  * If this widget has never been validated ( e.g. after construction ),
	  *  the ui delegate isn´t valid
	  *
	  * @see updateUI
	  * @see ULayoutManager
	  */
	virtual void validate();
	virtual void validateSelf();

	/** Invalidates the specified attributes of this widget. This is called by
	  * invalidate.
	  * @see invalidate
	  */
	virtual void invalidateSelf();
	/** Calls invalidateSelf and invalidates the parent widget.
	  */
	void invalidate();

	/** invalidates all child widgets and this widget itsself.
	  */
	void invalidateTree();

	///
	/// geometry functions
	///


	/** @return The x value in coordinates of the parent widget */
	int getX() const;
	/** @return The y value in coordinates of the parent widget */
	int getY() const;
	/** @return The width of this widget */
	int getWidth() const;
	/** @return The height of this widget */
	int getHeight() const;
	/** Sets the location within its parent widget. Calls setBounds. */
	void setLocation(int x, int y);
	/** @overload */
	void setLocation(const UPoint & p);
	/** @return The location within its parent widget */
	virtual UPoint getLocation() const;

	/** Sets the size of this widget. Calls setBounds.  */
	void setSize(int w, int h);
	/** @overload */
	void setSize(const UDimension & d);
	/** @return The size of this widget */
	virtual UDimension getSize() const;
	/** @return The size minus insets of this widget */
	UDimension getInnerSize() const;

	/** Sets the bounds of this widget.
	  * May call invalidate and fires widget events.
	  */
	virtual void setBounds(int x, int y, int w, int h);
	/** @overload */
	void setBounds(const URectangle & b);
	/** @return The bounding rectangle in coordinates of the parent widget. */
	virtual URectangle getBounds() const;
	/** This is a convenience method to get the bounding rectangle in
	  * coordinates of this widget minus the insets. That means for example
	  * the rectangle within a custom widget which is not covered by the
	  * border.
	  */
	URectangle getInnerBounds() const;
	/** This is a convenience method. It returns the bounds of this widget
	  * relative to the top-most root pane.
	  * It takes the root location and the size of this widget.
	  */
	URectangle getRootBounds() const;
	/** Returns the bounding rectangle which should be used for clipping.
	  * Takes into account if a parent widget does also clipping.
	  */
	URectangle getClipBounds() const;



	/** @return true if the given point (relative to this widget)
	  * is within this widget. Otherwise false.
	  */
	bool contains(int x, int y) const;
	/** @overload */
	bool contains(const UPoint & p) const;
	/** @return true if the given point (relative to the top level root pane)
	  * is within this widget. Otherwise false.
	  */
	bool containsRootPoint(int x, int y) const;
	/** @overload */
	bool containsRootPoint(const UPoint  & p) const;

	/** Transforms the coordinates of the given point from the coord system
	  * relative to this widget to the coord system
	  * relative to top-level rootpane
	  * @return The given point relative to the top level root pane
	 */
	UPoint pointToRootPoint(int x, int y) const;
	/** @overload */
	UPoint pointToRootPoint(const UPoint & p) const;

	/** Transforms the coordinates of the given point from the coord system
	  * relative to top-level rootpane to the coord system
	  * relative to this widget
	  * @return The given point relative to the top level root pane
	 */
	UPoint rootPointToPoint(int x, int y) const;
	/** @overload */
	UPoint rootPointToPoint(const UPoint & p) const;

	/** @return The location of this widget relative
	  * to the top level root pane.
	  */
	UPoint getRootLocation() const;


	//
	// painting and UI methods
	//


	/** The main paint function.
	  * Calls (in the following order) @ref paintWidget, @ref paintBorder and
	  * @ref paintChildren.
	  * You shouldn't override this methods directly, but @ref paintWidget.
	  * This methods checks for clipping, set the translation and the
	  * font attribute.
	  * If you want to repaint this widget, call @ref repaint.
	  * @see paintWidget
	  */
	virtual void paint(UGraphics * g);

	/** Registers this widget as dirty widget at the repaint manager and
	  * schedules a repaint event.
	  */
	void repaint();


	/** Retrieves a UI object from the UI manager and installs it on this
	  * widget. Called automatically by validate.
	  * @see validate
	  */
	virtual void updateUI();
	/** Sets the UI object for this widget.
	  * @see UWidgetUI
	  */
	virtual void setUI(UWidgetUI * newUI);
	/** @return The UI object. */
	virtual UWidgetUI * getUI() const;

	/** @return The rendering context for this widget,
	  * may be NULL for invalid widgets.
	  */
	UContext * getContext() const;

	/** Sets the context for this widget.
	  * @obsolete
	  *
	  * (This is used by widgets which are not added to a parent
	  * widget(like labels) but which are used to render
	  * some text via its paint method).
	  */
	void setContext(UContext * context);

	/** @return the UIManager used to draw this widget,
	  * may be NULL for invalid widgets.
	  */
	UUIManager * getUIManager() const;

	/** @obsolete Use instead getContext()->getGraphics()
	  * @return the Graphics object used to draw this widget,
	  * may be NULL for invalid widgets.
	  */
	UGraphics * getGraphics() const;


	//
	// several attribute methods
	//

	//
	// general palette methods
	//
	/** @return The currently used color group. */
	const UColorGroup & getColorGroup() const;
	/** @return the currently used color group type. */
	UPalette::ColorGroupType getColorGroupType() const;

	/** Sets the palette for this widget. */
	void setPalette(const UPalette & palette);
	/** @return The palette for this widget. */
	const UPalette & getPalette() const;

	/** Sets the background color for this widget.
	  * Changes internally the background colors of all color groups.
	  * It is better to use the color groups directly.
	  * @see setPalette
	  */
	void setBackgroundColor(const UColor & col);
	/** @return the backround color */
	const UColor & getBackgroundColor() const;

	/** Sets the foreground color for this widget.
	  * Changes internally the background colors of all color groups.
	  * It is better to use the color groups directly.
	  * @see setPalette
	  */
	void setForegroundColor(const UColor & col);
	/** @return the foreround color */
	const UColor & getForegroundColor() const;

	/** @return True if a background drawable was set.
	  * @see setBackground
	  */
	bool hasBackground() const;
	/** Instead of filling the backgound with the background color, draw
	  * the given drawable.
	  */
	void setBackground(UDrawable * tex);
	/** @return The current back ground or NULL if none was set */
	UDrawable * getBackground() const;

	/** No descriptions */
	void setFont(const UFont * font);
	/** @return The font used for this widget. */
	const UFont * getFont() const;
	/** Sets the border hint for the UI object */
	void setBorder(BorderType borderType);//const UBorder * border);
	/** @return The border type */
	BorderType getBorder() const;


	/** sets an empty margin between widget content and widget border
	  * @see getMargin
	  * @see getInsets
	  */
	void setMargin(const UInsets & margin);
	/** sets an empty margin between widget content and widget border
	  * @see getMargin
	  * @see getInsets
	  */
	void setMargin(int top, int left, int bottom, int right);

	/** returns a possible margin between the widget content and a border
	  * @see setMargin
	  * @see getInsets
	  */
	const UInsets & getMargin() const;

	/** returns all insets, i.e. insets of a possible border and (?)
	  * FIXME now what?
	  */
	virtual UInsets getInsets() const;



	/** @return True if this widget is focused. There can be only one focused
	  * widget for all UFO contexts.
	  */
	virtual bool isFocused() const;
	/** @return True if a child widget is focused. */
	virtual bool isChildFocused() const;
	/** tries to get input focus.
	  * @return The old focused widget or NULL
	  */
	virtual UWidget * requestFocus();
	/** Releases the focus. Returns true on success. */
	virtual bool releaseFocus();

	virtual bool isFocusable() const;
	/** Sets whether this widget is focusable. Default is true. */
	virtual void setFocusable(bool focusable);

	/** @return The widget which has (keyboard) input focus
	  */
	static UWidget * getFocusedWidget();

	/** Returns true when the mouse pointer is over this widget. */
	bool hasMouseFocus() const;

	/** @return The widget which has mouse focus
	  */
	static UWidget * getMouseFocusWidget();

	///
	/// event methods
	///

	/** Dispatches any GUI Event.
	  * This gives a chance to preprocess some events by the widget.
	  * If event e is not enabled for this widget and e is an input event,
	  * event e is propagated to the parent of this widget.
	  * Events are processed by the internal process* functions
	  */
	void dispatchEvent(UEvent * e);




	///
	/// widget functions
	///

	/** Sets the popup menu.
	  * This is used for pull right menus for normal widgets or
	  * for menu bar popups
	  */
	virtual void setPopupMenu(UPopupMenu * popupMenu);
	/** Returns the popup menu.
	  */
	virtual UPopupMenu * getPopupMenu() const;


	/** if this is a parent of w, w will be removed of the children list
	  * @param w The widget which should be removed
	  * @return True if w was a child widget and could be removed
	  * @see #remove(unsigned int)
	  */
	virtual bool remove(UWidget * w);
	/** removes the n ´th widget
	  * @param n The index of the widget which should be removed.
	  *  0 is the first widget.
	  * @return True if the n'th widget was removed
	  */
	virtual bool remove(unsigned int n);
	/** removes the n ´th widget
	  * @param n The index of the widget which should be removed.
	  *   0 is the first widget.
	  * @return
	  * 	The n'th widget without decreasing the ref count.
	  * 	The user is responsible for deleting it later.
	  * 	NULL, if there are not n widgets or other errors occured.
	  * @see #remove( UWidget * w )
	  */
	virtual UWidget * removeAndReturn(unsigned int n);
	/** Removes all child widgets.
	  * @return The number of the actual removed widgets.
	  */
	virtual unsigned int removeAll();
	/** Adds a sub widget at the specified index
	* @param w The widget which should be added
	* @param index
	*	if index is -1 or isn´t set, the widget will be added at the "end" of
	*	the container
	*/
	void add(UWidget * w, int index = -1);
	/** add a sub widget at the specified index
	  * @param w The widget which should be added
	  * @param constraints
	  * 	the constraints object is a Object which describes the layout options.
	  * 	Internally, it invokes put("layout", w); to set the property. Currently
	  * 	this is only used by UBorderlayout.
	  * @param index
	  * 	if index is -1 or isn´t set, the widget will be added at the end of
	  * 	the container
	  * @see put
	  * @see UBorderLayout
	  */
	void add(UWidget * w, UObject * constraints, int index = -1);
	/** add a sub widget at the specified index
	  * @param w
	  * 	the widget, which should be added
	  * @param constraints
	  * 	the constraints object is a Object which describes the layout options.
	  * 	Internally, it invokes put("layout", w); to set the property. Currently
	  * 	this is only used by UBorderlayout.
	  * @param index
	  * 	if index is -1 or isn´t set, the widget will be added at the end of
	  * 	the container
	  * @see put
	  * @see UBorderLayout
	  */
	void add(UWidget * w, const UObject * constraints, int index = -1);


	/** Returns the root pane object for this widget. If topmost is true,
	  * this function returns the top most root pane object, which is
	  * normally nested in an UXFrame.
	  * If topmost is false, this function could return the rootpane of
	  * an internal frame.
	  * @param topmost If true, returns the top most root pane.
	  * @return
	  * 	the root pane which has this widget as child or
	  * 	NULL if no top level root pane is owner of this widget
	  */
	virtual URootPane * getRootPane(bool topmost = false) const;

	/** @return The parent widget or NULL if this widget is not nested in
	  * a widget hierarchy.
	  */
	UWidget * getParent() const;


	///
	/// child widget functions
	///

	/** @return The index of the widget within the children vector
	  */
	int getIndexOf(const UWidget * w) const;
	/** This methods moves the widget within the children vector.
	  * The widget @ref w has to be a child of this widget
	  * If index is -1 or greater than the child vector size,
	  * the widget is moved to back.
	  * @param w The child widget to move
	  * @param index The new desired index.
	  */
	void setIndexOf(UWidget * w, int index);
	/** @return the n ´th child widget or NULL n is greater than the count
	  * of the child widgets
	  */
	UWidget * getWidget(unsigned int n) const;
	/** Returns the top most child widget at the given position.
	  * The point should be in the coordinate system of this widget
	  * @return
	  * 	the widget at the specified position or
	  * 	NULL if there is no child widget at this position
	  */
	UWidget * getWidgetAt(int x, int y) const;
	/** @overload */
	UWidget * getWidgetAt(const UPoint & p) const;
	/** Returns the top most visible child widget at the given position.
	  * The point should be in the coordinate system of this widget
	  * @return
	  * 	the widget at the specified position or
	  * 	NULL if there is no widget at this position
	  */
	UWidget * getVisibleWidgetAt(int x, int y) const;
	/** @overload */
	UWidget * getVisibleWidgetAt(const UPoint & p) const;

	std::vector<UWidget*> getVisibleWidgetsAt(int x, int y) const;
	std::vector<UWidget*> getVisibleWidgetsAt(const UPoint & p) const;

	/** @return The number of child widgets this widget has */
	unsigned int getWidgetCount() const;

	/** @return A vector with all child widgets. */
	const std::vector<UWidget*> & getWidgets() const;
	/** @overload */
	std::vector<UWidget*> & getWidgets();


	///
	/// layout functions and size hints
	///

	/** @return The layout manager used by this widget. */
	ULayoutManager * getLayout() const;
	/** Sets the layout manager for this widget. */
	void setLayout(ULayoutManager * layout);

	/** Sets a hint about the minimum size for the layout manager. */
	virtual void setMinimumSize(const UDimension & minimumSize);
	/** @return The minimum size */
	virtual UDimension getMinimumSize() const;

	/** Sets an explicit preferred size hint for the layout manager of
	  * the parent widget and overrides the default calculated one.
	  * If an empty size was given, use the default method to calculate
	  * the preferred size
	  * @param preferredSize The size hint used by the parent's layout manager
	  * @see getPreferredSize
	  */
	virtual void setPreferredSize(const UDimension & preferredSize);
	/** This method returns a size hint used by the layout manager to compute
	  * desired extensions of widget and to layout child widgets.
	  * If an explicit preferred size was set, return that one.
	  * Otherwise the preferred size is calculated via the UI object,
	  * the layout manager or other size resources.
	  * @return The size hint used by the parent's layout manager
	  * @see setPreferredSize
	  * @see setUI
	  */
	virtual UDimension getPreferredSize() const;
	/** In difference to @p getPreferredSize, this method does its
	  * calculations using the given maximal size.
	  * For example, some widgets may expand their height if the desired
	  * width would be bigger than the maximal width.
	  * It is guaranteed that the returned size is smaller than the given
	  * maximal size.
	  * If you pass @p UDimension::maxDimension, you should get the same
	  * return value as with @p getPreferredSize.
	  * @param maxSize The maximal size for the calculated preferred size
	  * @return The size hint used by the parent's layout manager
	  * @see setPreferredSize
	  * @see setUI
	  */
	virtual UDimension getPreferredSize(const UDimension & maxSize) const;

	// deprecated
	/** Sets the preferred size of the widget. */
	virtual void setMaximumSize(const UDimension & maximumSize);
	/**@see setMaximumSize */
	virtual UDimension getMaximumSize() const;

	/** puts a property in the local UProperty object
	  * @param key the string key that should be used to save the value
	  * @param value The object value
	  *
	  * @see get
	  */
	void put(const std::string & key, UObject * value);
	/** const int wrapper for put(const std::string &, UObject *);
	  * @see put
	  */
	//void put(const std::string & key, int value);
	/** const std::string wrapper for put(const std::string &, UObject *);
	  * @see put
	  */
	void put(const std::string & key, const std::string & value);
	/** @return the value object that was set by
	  *	<code>put(const std::string & key, UObject * value);</code> or NULL
	  * @param key The key that belongs to the desired value
	  * @see put
	  */
	UObject * get(const std::string & key) const;
	/** This method is for convenience
	  * @return The string value object that was set by @p put or ""
	  * @param key The key that belongs to the desired value
	  * @see put
	  */
	std::string getString(const std::string & key) const;

	/** */
	void setHorizontalAlignment(Alignment alignX);
	/** */
	Alignment getHorizontalAlignment() const;
	/** */
	void setVerticalAlignment(Alignment alignY);
	/** */
	Alignment getVerticalAlignment() const;

	/** Returns an input map of the given condition.
	  * You can modify ist data directly, so you do not have to set it as
	  * input map again.
	  * An input map registers all actions that should take place if a
	  * keyboard event occured.
	  *
	  * So far, there are two conditions:
	  * WHEN_FOCUSED conditions fires action when the key event occurred
	  * while this widget was focused.
	  * WHEN_ANCESTOR_FOCUSED conditions fires action when the key event occurred
	  * while this widget or an ancestor was focused.
	  */
	UInputMap * getInputMap(InputCondition conditionA = WhenFocused);
	/** @see getInputMap
	  */
	void setInputMap(UInputMap * newInputMapA,
		InputCondition conditionA = WhenFocused);

	/** Sets whether an event is enabled and should be processed
	  * by this widget. If an event is not enabled, it will be propagated
	  * to its parent.
	  */
	void setEventState(UEvent::Type type, bool b);
	/** Returns whether an event is enabled and processed by this widget.
	  */
	bool isEventEnabled(UEvent::Type type) const;

protected: // Protected Types
	/** Flags to determine which attributes were set by the user and
	  * which were set by the UIManager.
	  */
	enum AttribState {
		AttribLayout = 0x0001,
		AttribPalette = 0x0002,
		AttribFont = 0x0004,
		AttribBorder = 0x0008,
		AttribCursor = 0x0010,
		AttribMargin = 0x0020,
		AttribTextIconGap = 0x0040,
		AttribStateLast = 0x0040
	};

protected: // Overrides UObject
	virtual std::ostream & paramString(std::ostream & os) const;

protected:  // Protected methods
	/** Adds child widgets to this widget. This function should be overridden instead of
	  * other add functions
	  * @param w The widget which should be added
	  * @param index If index is -1, the widget will be added at the end of
	  *  the container
	  * @param constraints The constraints object is a Object which describes
	  *  the layout options. Internally, it invokes put("layout", w); to set
	  *  the property. Currently this is only used by UBorderlayout.
	  * @see put
	  * @see UBorderLayout
	  */
	virtual void addImpl(UWidget * w, UObject * constraints, int index);

	/** Notifies recursively all childs that they have been added to
	  * a valid containment hierarchy.
	  */
	virtual void addedToHierarchy();
	/** @see addedToHierarchy */
	virtual void removedFromHierarchy();

	/** Use validate() to layout child widgets.
	  * @obsolete
	  */
	void doLayout();

	/** Paints the widget itsself. Normally, this means drawing the UI.
	  * This is the method which should be overriden if you want to use
	  * your own drawing code (and don't want to create custom UI classes).
	  */
	virtual void paintWidget(UGraphics * g);
	/** Paints the widget's border. */
	virtual void paintBorder(UGraphics * g);
	/** Paints recursively the child widgets. */
	virtual void paintChildren(UGraphics * g);


	/** Process the event by invoking a apropriate process* functions
	  */
	virtual void processEvent(UEvent * e);

	/** Processes focus events. Primarily, it is used to notify listeners.
	  */
	virtual void processMouseEvent(UMouseEvent * e);
	/** Processes focus events. Primarily, it is used to notify listeners.
	  */
	virtual void processMouseWheelEvent(UMouseWheelEvent * e);
	/** Processes key events. Primarily, it is used to notify listeners.
	  */
	virtual void processKeyEvent(UKeyEvent * e);
	/** Processes key events. Primarily, it is used to notify listeners.
	  */
	virtual void processFocusEvent(UFocusEvent * e);
	/** Processes widget events. Primarily, it is used to notify listeners.
	  */
	virtual void processWidgetEvent(UWidgetEvent * e);


	/** notifies all property change listeners that a property change was performed
	  */
	bool firePropertyChangeEvent(const std::string & prop,
			UObject * oldValue, UObject * newValue);

	/** notifies all mouse listeners that a mouse event has occured
	  */
	bool fireMouseEvent(UMouseEvent * e);

	/** notifies all key listeners that a key event has occured
	  */
	bool fireKeyEvent(UKeyEvent * e);

	/** notifies all focus listeners that a focus event  of the specified type has occured
	  */
	bool fireFocusEvent(UFocusEvent * e);
	/** notifies all widget listeners that a widget event of the specified type has occured
	  */
	bool fireWidgetEvent(UWidgetEvent * e);

	bool notifyKeyBindingAction(const UKeyStroke & ks, UKeyEvent * e,
		InputCondition condition);
	/** Processes key bindings in input maps.
	  * @see getInputMap
	  */
	virtual bool processKeyBindings(UKeyEvent * e);

	/** Resets input and mouse focus settings.
	  * Called after hiding or removing widget.
	  */
	void resetFocus();

protected:  // Protected methods

	/** Marks an attribute to be set by the UI. */
	void markUIAttribute(uint32_t state);
	/** marks an attribute not to be set by the UI. */
	void unmarkUIAttribute(uint32_t state);

	uint32_t getUIAttributesState() const;

protected:  // Protected static attributes
	/** The widget with input (keyboard) focus */
	static UWidget * sm_inputFocusWidget;
	/** The widget with mouse focus */
	static UWidget * sm_mouseFocusWidget;
	/** The dragged widget (currentyl not used) */
	static UWidget * sm_dragWidget;


protected:  // Protected attributes
	/** this is the context at which the widget is currently registered */
	UContext * m_context;

	/**  FIXME: does this have to be protected? */
	UWidgetUI * m_ui;

private:  // Private attributes
	//
	// flags
	//

	/** state of visibility */
	uint16_t m_isVisible: 1;
	/** True if clipping is enabled. */
	uint16_t m_hasClipping: 1;
	/** True if this widget is enabled. */
	uint16_t m_isEnabled: 1;
	/** An opaque widget draws its background */
	uint16_t m_isOpaque: 1;
	/** True if this widget is focusable. */
	uint16_t m_isFocusable: 1;
	/** True if the top level parent belongs to a UFO context.
	  * @see isInValidHierarchy()
	  */
	uint16_t m_isInValidHierarchy: 1;

	//
	// flags for caching
	//
	/** An integer flag for all invalid parts of this widget. */
	int m_needsValidation;
	/** A flag for all events which should be processed.
	  * @see EventState
	  */
	uint32_t m_eventState;

	/** Indicates which attributes are set by the user interface classes. */
	uint32_t m_uiAttributes;

	//
	// general variables
	//

	/** the parent widget, may be NULL */
	UWidget * m_parent;

	/** all children are registered here */
	std::vector<UWidget*> m_children;

	/** the layout manager */
	ULayoutManager * m_layout;

	/** The context respectively menu popup menu. */
	UPopupMenu * m_popupMenu;

	//
	// size hints
	//

	/** the bounds of this widget relative to its parent */
	URectangle m_bounds;
	mutable URectangle m_clipBounds;
	/** A cache for the root location relative to the top most root pane. */
	mutable UPoint m_cachedRootLocation;
	/** Cache for the minimum size */
	UDimension m_minimumSize;
	UDimension m_maximumSize;
	/** Cache for the preferred size */
	UDimension m_preferredSize;
	mutable UDimension m_cachedPreferredSize;

	/** the margin between widget content and border */
	UInsets m_margin;

	// alignment
	Alignment m_horizontalAlignment;
	Alignment m_verticalAlignment;

	//
	// further attributes
	//

	/** This attribute contains a drawable background. May be NULL */
	UDrawable * m_bgDrawable;

	/** the font of this widget */
	const UFont * m_font;
	/** the border of this widget */
	BorderType m_border;

	UPalette m_palette;

	float m_opacity;

	/** the background color */
	//const UColor * m_bgColor;
	/** the background color */
	//const UColor * m_fgColor;

	/** specific properties for this widget. */
	UPropertiesMap m_properties;

	UInputMap * m_inputMap;
	UInputMap * m_ancestorInputMap;

	// BoUfo extenstion
	UCollectable* m_boUfoWidgetDeleter;

	// AB: getVisibleWidgetsAt() helper variable
	bool m_seenEvent;

public: // Public Signals
	// Mouse event signals
	/** This signal is fired when the mouse has entered this widget. */
	USignal1<UMouseEvent*> & sigMouseEntered();
	/** This signal is fired when the mouse has left this widget. */
	USignal1<UMouseEvent*> & sigMouseExited();

	/** This signal is fired when the mouse was moved over this widget
	  * and no mouse button was pressed.
	  */
	USignal1<UMouseEvent*> & sigMouseMoved();
	/** This signal is fired when a mouse button was pressed on this button
	  * and - while still pressed - the mouse was moved anywhere on the
	  * context.
	  */
	USignal1<UMouseEvent*> & sigMouseDragged();

	/** This signal is fired when a mouse button has been pressed
	  * on this widget.
	  */
	USignal1<UMouseEvent*> & sigMousePressed();
	/** This signal is fired when a mouse button has been released
	  * on this widget.
	  */
	USignal1<UMouseEvent*> & sigMouseReleased();
	/** This signal is fired when a mouse button has been pressed and released
	  * on this widget.
	  */
	USignal1<UMouseEvent*> & sigMouseClicked();

	/** This signal is fired when a mouse wheel was used while the mouse
	  * has been over this widget.
	  */
	USignal1<UMouseWheelEvent*> & sigMouseWheel();


	// Key event signals
	/** This signal is fired when a key was pressed
	  * while this widget has been focused.
	  */
	USignal1<UKeyEvent*> & sigKeyPressed();
	/** This signal is fired when a key was released
	  * while this widget has been focused.
	  */
	USignal1<UKeyEvent*> & sigKeyReleased();
	/** This signal is fired when a key was pressed and released
	  * while this widget has been focused.
	  */
	USignal1<UKeyEvent*> & sigKeyTyped();

	// Widget event signals
	/** This signal is fired when this widget has been moved
	  */
	USignal1<UWidgetEvent*> & sigWidgetMoved();
	/** This signal is fired when this widget has been resized
	  */
	USignal1<UWidgetEvent*> & sigWidgetResized();
	/** This signal is fired when this widget has been shown
	  */
	USignal1<UWidgetEvent*> & sigWidgetShown();
	/** This signal is fired when this widget has been hidden
	  */
	USignal1<UWidgetEvent*> & sigWidgetHidden();

	/** This signal is fired when this(!) widget was added to a valid
	  * containment hierarchy, i.e. the top-level parent belongs to a
	  * UFO context.
	  * Furthermore, this means that the UI object is now valid and
	  * you can get serious values by getPreferredSize.
	  * <p>
	  * (Please note that this signal does <b>not</b> mean that another widget
	  * was added to this widget).
	  */
	USignal1<UWidget*> & sigWidgetAdded();
	/** This signal is fired when this widget has been removed from a parent.
	  */
	USignal1<UWidget*> & sigWidgetRemoved();

	// Focus event signals
	/** This signal is fired when this widget gained input (keyboard) focus. */
	USignal1<UFocusEvent*> & sigFocusGained();
	/** This signal is fired when this widget lost input (keyboard) focus. */
	USignal1<UFocusEvent*> & sigFocusLost();

private: // Private Signals
	// mouse signals
	USignal1<UMouseEvent*> m_sigMouseEntered;
	USignal1<UMouseEvent*> m_sigMouseExited;

	USignal1<UMouseEvent*> m_sigMouseMoved;
	USignal1<UMouseEvent*> m_sigMouseDragged;

	USignal1<UMouseEvent*> m_sigMousePressed;
	USignal1<UMouseEvent*> m_sigMouseReleased;
	USignal1<UMouseEvent*> m_sigMouseClicked;

	USignal1<UMouseWheelEvent*> m_sigMouseWheel;

	// key signals
	USignal1<UKeyEvent*> m_sigKeyPressed;
	USignal1<UKeyEvent*> m_sigKeyReleased;
	USignal1<UKeyEvent*> m_sigKeyTyped;

	// widget signals
	USignal1<UWidgetEvent*> m_sigWidgetMoved;
	USignal1<UWidgetEvent*> m_sigWidgetResized;
	USignal1<UWidgetEvent*> m_sigWidgetShown;
	USignal1<UWidgetEvent*> m_sigWidgetHidden;

	USignal1<UWidget*> m_sigWidgetAdded;
	USignal1<UWidget*> m_sigWidgetRemoved;

	// focus signals
	USignal1<UFocusEvent*> m_sigFocusGained;
	USignal1<UFocusEvent*> m_sigFocusLost;
};


//
// inline implementation
//

// Mouse Motion event signals
inline USignal1<UMouseEvent*> &
UWidget::sigMouseEntered() {
	setEventState(UEvent::MouseEntered, true);
	return m_sigMouseEntered;
}
inline USignal1<UMouseEvent*> &
UWidget::sigMouseExited() {
	setEventState(UEvent::MouseExited, true);
	return m_sigMouseExited;
}
inline USignal1<UMouseEvent*> &
UWidget::sigMouseMoved() {
	setEventState(UEvent::MouseMoved, true);
	return m_sigMouseMoved;
}
inline USignal1<UMouseEvent*> &
UWidget::sigMouseDragged() {
	setEventState(UEvent::MouseDragged, true);
	return m_sigMouseDragged;
}

// Mouse event signals
inline USignal1<UMouseEvent*> &
UWidget::sigMousePressed() {
	setEventState(UEvent::MousePressed, true);
	return m_sigMousePressed;
}
inline USignal1<UMouseEvent*> &
UWidget::sigMouseReleased() {
	setEventState(UEvent::MouseReleased, true);
	return m_sigMouseReleased;
}
inline USignal1<UMouseEvent*> &
UWidget::sigMouseClicked() {
	setEventState(UEvent::MouseClicked, true);
	return m_sigMouseClicked;
}

inline USignal1<UMouseWheelEvent*> &
UWidget::sigMouseWheel() {
	setEventState(UEvent::MouseWheel, true);
	return m_sigMouseWheel;
}

// Key event signals
inline USignal1<UKeyEvent*> &
UWidget::sigKeyPressed() {
	setEventState(UEvent::KeyPressed, true);
	return m_sigKeyPressed;
}
inline USignal1<UKeyEvent*> &
UWidget::sigKeyReleased() {
	setEventState(UEvent::KeyReleased, true);
	return m_sigKeyReleased;
}
inline USignal1<UKeyEvent*> &
UWidget::sigKeyTyped() {
	setEventState(UEvent::KeyTyped, true);
	return m_sigKeyTyped;
}

// Widget event signals
inline USignal1<UWidgetEvent*> &
UWidget::sigWidgetMoved() {
	setEventState(UEvent::WidgetMoved, true);
	return m_sigWidgetMoved;
}
inline USignal1<UWidgetEvent*> &
UWidget::sigWidgetResized() {
	setEventState(UEvent::WidgetResized, true);
	return m_sigWidgetResized;
}
inline USignal1<UWidgetEvent*> &
UWidget::sigWidgetShown() {
	setEventState(UEvent::WidgetShown, true);
	return m_sigWidgetShown;
}
inline USignal1<UWidgetEvent*> &
UWidget::sigWidgetHidden() {
	setEventState(UEvent::WidgetHidden, true);
	return m_sigWidgetHidden;
}
inline USignal1<UWidget*> &
UWidget::sigWidgetAdded() {
	return m_sigWidgetAdded;
}
inline USignal1<UWidget*> &
UWidget::sigWidgetRemoved() {
	return m_sigWidgetRemoved;
}

// Focus event signals
inline USignal1<UFocusEvent*> &
UWidget::sigFocusGained() {
	setEventState(UEvent::FocusGained, true);
	return m_sigFocusGained;
}
inline USignal1<UFocusEvent*> &
UWidget::sigFocusLost() {
	setEventState(UEvent::FocusLost, true);
	return m_sigFocusLost;
}



//
// inline position
//

inline int
UWidget::getX() const {
	return m_bounds.x;
}
inline int
UWidget::getY() const {
	return m_bounds.y;
}
inline int
UWidget::getWidth() const {
	return m_bounds.w;
}
inline int
UWidget::getHeight() const {
	return m_bounds.h;
}

inline void
UWidget::setLocation(int x, int y) {
	setBounds(x, y, m_bounds.w, m_bounds.h);
}
inline void
UWidget::setLocation(const UPoint & p) {
	setLocation(p.x, p.y);
}
inline UPoint
UWidget::getLocation() const {
	return UPoint(m_bounds.x, m_bounds.y);
}

inline void
UWidget::setSize(int w, int h) {
	setBounds(m_bounds.x, m_bounds.y, w, h);
}
inline void
UWidget::setSize(const UDimension & d) {
	setSize(d.w, d.h);
}
inline UDimension
UWidget::getSize() const {
	return UDimension(m_bounds.w, m_bounds.h);
}
inline UDimension
UWidget::getInnerSize() const {
	UDimension ret(getSize());
	const UInsets & in = getInsets();
	ret.w -= in.getHorizontal();
	ret.h -= in.getVertical();
	return ret;
}

inline void
UWidget::setBounds(const URectangle & b) {
	setBounds(b.x, b.y, b.w, b.h);
}
inline URectangle
UWidget::getInnerBounds() const {
	URectangle ret(getBounds());
	const UInsets & in = getInsets();
	ret.x = in.left;
	ret.y = in.top;
	ret.w -= in.getHorizontal();
	ret.h -= in.getVertical();
	return ret;
}
inline URectangle
UWidget::getRootBounds() const {
	return URectangle(getRootLocation(), getSize());
}


inline bool
UWidget::contains(int x, int y) const {
	return (x >= 0 && x < getWidth() && y >= 0 && y < getHeight());
}

inline bool
UWidget::contains(const UPoint & p) const {
	return (p.x >= 0 && p.x < getWidth() && p.y >= 0 && p.y < getHeight());
}

inline bool
UWidget::containsRootPoint(int x, int y) const {
	return contains(rootPointToPoint(x, y));
}

inline bool
UWidget::containsRootPoint(const UPoint & p) const {
	return contains(rootPointToPoint(p));
}

inline UPoint
UWidget::pointToRootPoint(int x, int y) const {
	return pointToRootPoint(UPoint(x, y));
}

inline UPoint
UWidget::pointToRootPoint(const UPoint & p) const {
	return (p + getRootLocation());
}

inline UPoint
UWidget::rootPointToPoint(int x, int y) const {
	return rootPointToPoint(UPoint(x, y));
}

inline UPoint
UWidget::rootPointToPoint(const UPoint & p) const {
	return (p - getRootLocation());
}


inline void
UWidget::add(UWidget * w, int index) {
	addImpl(w, NULL, index);
}
inline void
UWidget::add(UWidget * w, UObject * constraints, int index) {
	addImpl(w, constraints, index);
}

} // namespace ufo

#endif // UWIDGET_HPP
