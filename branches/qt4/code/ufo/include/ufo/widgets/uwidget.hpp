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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
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
#include "../util/upalette.hpp"

#include "../ui/ustyle.hpp"

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

#define UFO_STYLE_TYPE(name) \
public:                    \
	virtual UStyle::ComponentElement getStyleType() const { \
		return name; \
	}

namespace ufo {

typedef std::map<std::string, UObject*> UPropertiesMap;


// forward declartions
class UColor;
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
class UShortcutEvent;
class UWidgetEvent;

class UStyle;
class UStyleHints;
class UStyleManager;
class UStyleOption;
class UWidgetModel;


/**
 * @internal
 * WARNING: this is a BoUfo extension to libufo. It is NOT part of the
 * libufo API!
 **/
class UFO_EXPORT UBoUfoWidgetDeleter : public UCollectable {
public:
	UBoUfoWidgetDeleter()
	{
	}

	/**
	 * Called right before the widget that this widget deleter belongs to is
	 * painted (see @ref UWidget::paint).
	 *
	 * This may e.g. be used to implement profiling of the paint method.
	 * The default implementation does nothing.
	 *
	 * See also @ref endPaint
	 **/
	virtual void startPaint() {}

	/**
	 * Called right after the widget that this widget deleter belongs to is
	 * painted (see @ref UWidget::paint).
	 *
	 * This may e.g. be used to implement profiling of the paint method.
	 * The default implementation does nothing.
	 *
	 * See also @ref startPaint
	 **/
	virtual void endPaint() {}
};



/** @short The base class for all widgets.
  * @ingroup widgets
  *
  * Every UFO widget is derived from UWidget.
  * All base functionality is provided by this class, i.e.
  * input event processing, sizing, container functionality
  * (adding other widgets), layouting
  * (using layout managers, default is UBoxLayout),
  * several widget attributes, ...
  *
  * @see UBoxLayout
  * @author Johannes Schmidt
  */

class UFO_EXPORT UWidget : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UWidget)
	UFO_UI_CLASS(UWidgetUI)
	UFO_STYLE_TYPE(UStyle::CE_Widget)
private:
	friend class UWidgetUI;

public:  // Public types
	// keyboard input conditions
	enum InputCondition {
		WhenFocused = 1,
		WhenAncestorFocused = 2
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
	void setBoUfoWidgetDeleter(UBoUfoWidgetDeleter* deleter);



	/** @return True if this widget is currently visible on screen
	  *  (mapped to screen). This means also, that all parent widgets are
	  *  visible on screen.
	  */
	virtual bool isVisible() const;
	/** <p>If @p v is true and the parent widget returns true on isVisible,
	  * maps this widget all child widget to screen.
	  * If a child widget was hidden with setVisible(false), it remains
	  * hidden.
	  * </p><p>
	  * If @p v is false, this widget and all child widget will disappear
	  * from screen.</p>
	  */
	virtual void setVisible(bool v);

	/** Switches clipping.
	  * If b is true, you can only draw within the rectangle given by
	  * the bounds of this widget.
	  * Default value is true for clipping.
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
	  * Calling setEnabled(false) on a widget implicetly disables all
	  * child widgets. They are reenabled on calling setEnabled(true) on the
	  * same widget.
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

	/** Returns whether this widget is active.
	  * Mostly this means that the current widget has mouse focus.
	  * But this may differ from widget to widget (e.g. text widget are
	  * active if they have keyboard focus).
	  * This is not really an isolated widget state but the combination of
	  * other states. It is used as style hint for the UI object.
	  */
	virtual bool isActive() const;

	/** Many attributes of a widget can only be stated when it is
	  * within a valid containment hierarchy (and mapped to screen).
	  * For example, the correct UI object can be determined only if
	  *  the top level widget belongs to a UFO context.
	  * @see sigWidgetAdded
	  * @see sigWidgetRemoved
	  *
	  * @return true if this widget is in a containment hierarchy
	  *  visible on screen.
	  */
	virtual bool isInValidHierarchy() const;
	/** Returns true if the layout manager has laid out all children.
	  */
	bool isValid() const;
	/** Relayouts the container.
	  *
	  * @see ULayoutManager
	  */
	virtual void validate();
	virtual void validateSelf();

	/** Invalidates the specified attributes of this widget and forces
	  * a relayout of child widgets.
	  * This is called by invalidate.
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
	/** Returns the bounding rectangle in root coordinates
	  * which should be used for clipping.
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


	/** @return The style manager used to inquire style and style hints. */
	virtual UStyleManager * getStyleManager() const;

	/** @return The style object used to paint this widget. */
	UStyle * getStyle() const;
	/** Explicetly sets the style for this widget.
	  * It is not recommend to
	  * use this method directly and to set specific styles for certain
	  * widgets.
	  */
	void setStyle(UStyle * style);
	/** @return The style hints object used to paint this widget. */
	const UStyleHints * getStyleHints() const;
	/** Explicetly sets the style hints object for this widget. */
	void setStyleHints(UStyleHints * hints);

	/** Sets the cascading style sheet (CSS) type for this widget.
	  * This affects only XUL interpreted GUIs-
	  */
	void setCssType(const std::string & type);
	/** @return The cascading style sheet (CSS) type for this widget. */
	std::string getCssType() const;
	/** Sets the cascading style sheet (CSS) class for this widget.
	  * This affects GUI with custom style sheets.
	  * @see UStyleManager::loadStyleSheet
	  */
	void setCssClass(const std::string & cssClass);
	/** @return The cascading style sheet (CSS) class for this widget. */
	std::string getCssClass() const;

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

	/** @deprecated */
	UUIManager * getUIManager() const;

	/** @deprecated */
	UGraphics * getGraphics() const;


	//
	// several attribute methods
	//

	//
	// style hints
	//
	float getOpacity() const;
	/** Sets the opacity of the widget background.
	  * 1.0 means fully opaque which is the default.
	  * This effect might show unwanted result when used with more complex
	  * widgets or when using a different look and feel.
	  */
	virtual void setOpacity(float f);

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

	/** Sets the font style hint. */
	void setFont(const UFont & font);
	/** @return The font style hint used for this widget. */
	const UFont & getFont() const;
	/** Sets the border hint for the UI object */
	void setBorder(BorderType borderType);
	/** @return The border type */
	BorderType getBorder() const;


	/** Sets an insets style hint used as empty margin between widget content
	  * and widget border
	  * @see getMargin
	  * @see getInsets
	  */
	void setMargin(const UInsets & margin);
	/** Sets an insets style hint used as empty margin between widget content
	  * and widget border
	  * @see getMargin
	  * @see getInsets
	  */
	void setMargin(int top, int left, int bottom, int right);

	/** returns a possible margin between the widget content and a border
	  * @see setMargin
	  * @see getInsets
	  */
	const UInsets & getMargin() const;

	/** returns all insets, i.e. insets of a possible border and margin
	  * @see setMargin
	  */
	virtual UInsets getInsets() const;


	/** Sets a hint about the minimum size for the layout manager. */
	void setMinimumSize(const UDimension & minimumSize);
	/** @return The minimum size */
	UDimension getMinimumSize() const;

	/** Sets an explicit preferred size hint for this widget.
	  * This size hint is used by the parent's layout manager. If you do
	  * not use layout managers, use @p setSize or @p setBounds
	  * A @p preferredSize is equal to @p UDimenion::invalid, this size
	  * hint is ignored.
	  * @param preferredSize The size hint used by the parent's layout manager
	  * @see getPreferredSize
	  */
	void setPreferredSize(const UDimension & preferredSize);

	/** Sets the preferred size of the widget. */
	void setMaximumSize(const UDimension & maximumSize);
	/**@see setMaximumSize */
	UDimension getMaximumSize() const;

	/** Sets the style hint for horizontal layouts.
	  * This influences the lay out of child widgets or visible parts like
	  * icons and text.
	  */
	void setHorizontalAlignment(Alignment alignX);
	/** @see setHorizontalAlignment */
	Alignment getHorizontalAlignment() const;
	/** Sets the vertical alignment of this widget.
	  * This influences the lay out of child widgets or visible parts like
	  * icons and text.
	  */
	void setVerticalAlignment(Alignment alignY);
	/** @see setVerticalAlignment */
	Alignment getVerticalAlignment() const;

	/** Sets the Direction of this widget.
	  * There are two valid direction: @p LeftToRight and @p RightToLeft.
	  * This influence the order of child widgets.
	  * Default value is @p LeftToRight
	  */
	void setDirection(Direction dir);
	/** @see setDirection */
	Direction getDirection() const;
	/** Sets the Orientation of this widget.
	  * @arg Horizontal (default)
	  * @arg Vetical
	  */
	void setOrientation(Orientation orientation);
	/** @see setOrientation */
	Orientation getOrientation() const;

	//
	// generic methods
	//

	/** @return True if this widget is focused. There can be only one focused
	  * widget for all UFO contexts.
	  */
	virtual bool isFocused() const;
	/** @return True if a child widget is focused. */
	virtual bool isChildFocused() const;
	/** @return True if a parent widget is focused. */
	virtual bool isAncestorFocused() const;
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
	void setPopupMenu(UPopupMenu * popupMenu);
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
	virtual UWidget * getWidgetAt(const UPoint & p) const;
	/** Returns the top most visible child widget at the given position.
	  * The point should be in the coordinate system of this widget
	  * @return
	  * 	the widget at the specified position or
	  * 	NULL if there is no widget at this position
	  */
	UWidget * getVisibleWidgetAt(int x, int y) const;
	/** @overload */
	virtual UWidget * getVisibleWidgetAt(const UPoint & p) const;

	/** @return The number of child widgets this widget has */
	unsigned int getWidgetCount() const;

	/** @return A vector with all child widgets. */
	const std::vector<UWidget*> & getWidgets() const;
	/** @overload */
	std::vector<UWidget*> & getWidgets();


	///
	/// layout methods and size methods
	///

	/** @return The layout manager used by this widget. */
	ULayoutManager * getLayout() const;
	/** Sets the layout manager for this widget.
	  * The default layout manager is UBoxLayout
	  * @see UBoxLayout
	  */
	void setLayout(ULayoutManager * layout);

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

	///
	/// property methods
	///

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

	/** You shouldn't need to call this method!
	  * It's supposed to be UFO internal.
	  * @return The widget model.
	  */
	const UWidgetModel * getModel() const;

	/** As soon as a widget is visible and focused, all key strokes which are
	  * equal to the given one fire a shortcut event to this widget.
	  * It can be processed via processShortcutEvent.
	  * Every key stroke which match a shortcut fires exactly one shortcut
	  * event to one widget.
	  * If more than one widget registered for the same shortcut,
	  * UShortcutEvent::isAmbiguous() returns true.
	  * Pressing the stroke again dispatches a newly created event
	  * to the next listener.
	  * @see UShortcutEvent::isAmbiguous
	  * @see processShortcutEvent
	  */
	void grabShortcut(const UKeyStroke & stroke);
	/** Releases a previously grabbed shortcut.
	  * @see grabShortcut
	  */
	void releaseShortcut(const UKeyStroke & stroke);
	/** Releases all previously grabbed shortcuts.
	  * @see grabShortcut
	  */
	void releaseAllShortcuts();

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
	/** Removes the widget at the given index.
	  * @return True if succesfull
	  */
	virtual bool removeImpl(int index);

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


	/** Process the event by invoking a appropriate process* functions
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
	virtual void processShortcutEvent(UShortcutEvent * e);
	/** Processes key events. Primarily, it is used to notify listeners.
	  */
	virtual void processFocusEvent(UFocusEvent * e);
	/** Processes widget events. Primarily, it is used to notify listeners.
	  */
	virtual void processWidgetEvent(UWidgetEvent * e);

	virtual void processStyleHintChange(uint32_t styleHint);
	virtual void processStateChangeEvent(uint32_t state);


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

	virtual UDimension getContentsSize(const UDimension & maxSize) const;
	void detachStyleHints();

public:  // Protected methods
	bool testState(uint32_t state) const;
	void setState(uint32_t state, bool b = true);
	void setStates(uint32_t states);
	uint32_t getStates() const;

private:
	/** Recursively shows/hides this and its child widgets if not
	  * invisibilty forced.
	  */
	void setChildrenVisible(bool b);
	/** Recursively enables/disables this and its child widgets if not
	  * a disabled state is forced.
	  */
	void setChildrenEnabled(bool b);

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

	UWidgetModel * m_model;

private:  // Private attributes
	//
	// flags
	//

	/** state of visibility */
	uint32_t m_isVisible: 1;
	/** True if clipping is enabled. */
	uint32_t m_hasClipping: 1;
	/** True if this widget is enabled. */
	uint32_t m_isEnabled: 1;
	/** An opaque widget draws its background */
	//uint16_t m_isOpaque: 1;
	/** True if this widget is focusable. */
	uint32_t m_isFocusable: 1;
	/** True if the top level parent belongs to a UFO context.
	  * @see isInValidHierarchy()
	  */
	uint32_t m_isInValidHierarchy: 1;
	/**  */
	uint32_t m_hasInvalidLayout : 1;
	/** Does own an own copy of style hints */
	uint32_t m_styleHintsDetached : 1;

	//
	// flags for caching
	//
	/** A flag for all events which should be processed.
	  * @see EventState
	  */
	uint32_t m_eventState;

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
	// style hints
	//
	std::string m_cssType;
	std::string m_cssClass;
	mutable UStyle * m_style;
	mutable UStyleHints * m_styleHints;

	//
	// size hints
	//

	/** the bounds of this widget relative to its parent */
	URectangle m_bounds;
	mutable URectangle m_clipBounds;
	/** A cache for the root location relative to the top most root pane. */
	mutable UPoint m_cachedRootLocation;
	mutable UDimension m_cachedPreferredSize;

	/** the background color */
	//const UColor * m_bgColor;
	/** the background color */
	//const UColor * m_fgColor;

	/** specific properties for this widget. */
	UPropertiesMap m_properties;

	UInputMap * m_inputMap;
	UInputMap * m_ancestorInputMap;

	// BoUfo extenstion
	UBoUfoWidgetDeleter* m_boUfoWidgetDeleter;

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
	USignal1<UWidgetEvent*> & sigWidgetAdded();
	/** This signal is fired when this widget has been removed from a parent.
	  */
	USignal1<UWidgetEvent*> & sigWidgetRemoved();

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

	USignal1<UWidgetEvent*> m_sigWidgetAdded;
	USignal1<UWidgetEvent*> m_sigWidgetRemoved;

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
inline USignal1<UWidgetEvent*> &
UWidget::sigWidgetAdded() {
	return m_sigWidgetAdded;
}
inline USignal1<UWidgetEvent*> &
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
	return ret - getInsets();
}

inline void
UWidget::setBounds(const URectangle & b) {
	setBounds(b.x, b.y, b.w, b.h);
}
inline URectangle
UWidget::getInnerBounds() const {
	URectangle ret(0, 0, getWidth(), getHeight());
	return ret - getInsets();
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

inline UPoint
UWidget::pointToRootPoint(const UPoint & p) const {
	return (p + getRootLocation());
}

inline UPoint
UWidget::pointToRootPoint(int x, int y) const {
	return pointToRootPoint(UPoint(x, y));
}

inline UPoint
UWidget::rootPointToPoint(const UPoint & p) const {
	return (p - getRootLocation());
}

inline UPoint
UWidget::rootPointToPoint(int x, int y) const {
	return rootPointToPoint(UPoint(x, y));
}

inline bool
UWidget::containsRootPoint(int x, int y) const {
	return contains(rootPointToPoint(x, y));
}

inline bool
UWidget::containsRootPoint(const UPoint & p) const {
	return contains(rootPointToPoint(p));
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
