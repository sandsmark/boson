/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOWIDGET_H
#define BOUFOWIDGET_H

#include <qobject.h>
#include <qevent.h>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
template<class T1, class T2> class QMap;
class QDomElement;

class BoUfoImage;
class BoUfoDrawable;
class BoUfoFontInfo;

namespace ufo {
	class UXToolkit;
	class UXDisplay;
	class UXContext;
	class URootPane;
	class UObject;
	class ULayoutManager;
	class UDimension;
	class UDrawable;
	class UImage;
	class UImageIO;
	class UPoint;
	class UFontInfo;
	class UFont;

	class UWidget;
	class UButton;
	class UCheckBox;
	class URadioButton;
	class UTextEdit;
	class UAbstractSlider;
	class USlider;
	class UListBox;
	class ULineEdit;
	class ULabel;
	class UComboBox;
	class UInternalFrame;
	class UBoProgress;
	class ULayeredPane;
	class UWidgetUI;
	class UButtonGroup;


	class UActionEvent;
	class UMouseEvent;
	class UMouseWheelEvent;
	class UWidgetEvent;
	class UKeyEvent;
	class UFocusEvent;
};

class BoUfoManager;

/**
 * @internal
 *
 * Macro to simplify ufo to Qt signal connection.
 *
 * @param className the name of the class you are working in, i.e. where the
 * (Qt) slot is defined in.
 * @param widget Where the signal comes from (i.e. the libufo widget)
 * @param signal The name of the signal. That's the part after the "sig" prefix
 * in libufo. There must be a corresponding "slot" starting with "uslot"
 **/
#define CONNECT_UFO_TO_QT(className, widget, signal) \
		widget->sig##signal().connect(slot(*this, &className::uslot##signal));


/**
 * @short Event class for mouse clicks
 *
 * libufo uses both, mouse push/release and click events. A click event is just
 * like a release event, however it has an additional "click count" parameter
 * that allows to distinguish normal clicks and double/triple/whatever clicks.
 *
 * This class represents this kind of click events.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoMouseEventClick : public QMouseEvent
{
public:
	enum EventType {
		EventClick = QEvent::User + 0
	};
public:
	BoUfoMouseEventClick(Type type, const QPoint& pos, int button, int state, int clickCount = 1)
		: QMouseEvent(type, pos, button, state),
		mClickCount(clickCount)
	{
	}
	BoUfoMouseEventClick(const QPoint& pos, int button, int state, int clickCount = 1)
		: QMouseEvent((QEvent::Type)EventClick, pos, button, state),
		mClickCount(clickCount)
	{
	}

	/**
	 * @return The number of clicks the mouse was clicked within a short
	 * period of time. 1 for normal clicks, 2 for double clicks, ...
	 **/
	int clickCount() const
	{
		return mClickCount;
	}

private:
	int mClickCount;
};


/**
 * This class is an interface between libufo and Qt/boson.
 *
 * Every object of this class holds at least one ufo widget and it is child of
 * that widget (even if it is created in this class!)
 * This interface object will therefore be deleted when the ufo widget
 * is being deleted.
 *
 * You can use the ufo @ref widget directly, however it is often more convenient
 * to just use this interface. You'll also find Qt style signals here.
 *
 * Note that you have to add every BoUfoWidget to a parent (e.g. @ref
 * BoUfoManager::contentWidget), otherwise it will never get deleted.
 *
 * Do <em>NOT</em> delete a BoUfoWidget yourself! It is automatically deleted
 * when the libufo widget that it manages is deleted.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short The boson class for a libufo widget
 **/
class BoUfoWidget : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString constraints READ constraints WRITE setConstraints);
	Q_PROPERTY(int gridLayoutColumns READ gridLayoutColumns WRITE setGridLayoutColumns);
	Q_PROPERTY(int gridLayoutRows READ gridLayoutRows WRITE setGridLayoutRows);
	Q_PROPERTY(bool opaque READ opaque WRITE setOpaque);
	Q_PROPERTY(LayoutClass layout READ layoutClass WRITE setLayoutClass);
	Q_PROPERTY(int minimumWidth READ minimumWidth WRITE setMinimumWidth);
	Q_PROPERTY(int minimumHeight READ minimumHeight WRITE setMinimumHeight);
	Q_PROPERTY(int preferredWidth READ preferredWidth WRITE setPreferredWidth);
	Q_PROPERTY(int preferredHeight READ preferredHeight WRITE setPreferredHeight);
	Q_PROPERTY(QString backgroundImageFile READ backgroundImageFile WRITE setBackgroundImageFile);
	Q_PROPERTY(VerticalAlignment verticalAlignment READ verticalAlignment WRITE setVerticalAlignment);
	Q_PROPERTY(HorizontalAlignment horizontalAlignment READ horizontalAlignment WRITE setHorizontalAlignment);
	Q_PROPERTY(BorderType borderType READ borderType WRITE setBorderType);
	Q_PROPERTY(int stretch READ stretch WRITE setStretch);
	Q_PROPERTY(bool takesKeyboardFocus READ takesKeyboardFocus WRITE setTakesKeyboardFocus);
	Q_ENUMS(LayoutClass);
	Q_ENUMS(HorizontalAlignment);
	Q_ENUMS(VerticalAlignment);
	Q_ENUMS(BorderType);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoWidget();
	BoUfoWidget(ufo::UWidget* w);
	~BoUfoWidget();

	enum LayoutClass {
		NoLayout = 0,
		UHBoxLayout,
		UVBoxLayout,
		UGridLayout,
		UBorderLayout,
		UFullLayout
	};
	enum HorizontalAlignment {
		AlignLeft = 0,
		AlignHCenter = 1,
		AlignRight = 2
	};
	enum VerticalAlignment {
		AlignTop = 0,
		AlignVCenter = 1,
		AlignBottom = 2
	};
	enum BorderType {
		NoBorder = 0,
		LineBorder = 1,
		BottomLineBorder = 2,
		RaisedBevelBorder = 3,
		LoweredBevelBorder = 4,
		StyleBorder = 100,
		CssBorder = 101
	};

	ufo::UWidget* ufoWidget() const
	{
		return mWidget;
	}

	/**
	 * Reimplemented from @ref QObject::setName
	 **/
	virtual void setName(const char* name);

	/**
	 * Set the vertical alignment, see @ref
	 * ufo::UWidget::setVerticalAlignment. @p alignment should be a value
	 * from @ref Qt::AlignmentFlags that describes a vertical alignment
	 *
	 * It seems that these alignment flags are hints to the layout manager
	 * in libufo.
	 **/
	virtual void setVerticalAlignment(VerticalAlignment alignment);
	virtual void setHorizontalAlignment(HorizontalAlignment alignment);
	VerticalAlignment verticalAlignment() const;
	HorizontalAlignment horizontalAlignment() const;

	void setBorderType(BorderType type);
	BorderType borderType() const;

	/**
	 * Convenience method for @ref setBackground with a drawable that paints
	 * @p img.
	 **/
	void setBackgroundImage(const BoUfoImage& img);
	void setBackgroundImageFile(const QString&);

	/**
	 * @return The file set by @ref setBackgroundImageFile, @ref
	 * QString::null by default. Undefined if you changed the background
	 * with methods other than @ref setBackgroundImageFile.
	 **/
	QString backgroundImageFile() const;

	/**
	 * @param drawable The object used for rendering the background. Note
	 * that this is ignored if @ref opaque is FALSE. Also note that you have
	 * to take care about deleting this yourself!
	 *
	 * WARNING: this has no effect, unless @ref opaque is TRUE !
	 **/
	void setBackground(BoUfoDrawable* drawable);

	/**
	 * Equivalent to widget()->setLayout(layout);
	 **/
	void setLayout(ufo::ULayoutManager* layout);

	/**
	 * Causes to redo the layout. See also @ref ufo::UWidget::invalidate
	 **/
	void invalidate();

	virtual void setOpaque(bool);
	virtual bool opaque() const;

	bool isEnabled() const;

	void setTakesKeyboardFocus(bool);
	bool takesKeyboardFocus() const;
	bool hasFocus() const;

	void setFocus();
	void clearFocus();

	/**
	 * Add @p w as child to this widget. This is also adds @p w to the
	 * layout of this widget, as BoUfo layout classes work on the child
	 * widgets.
	 *
	 * Note that actually @ref BoUfoWidget::widget of @p w is added to @ref
	 * BoUfoWidget::widget of this object.
	 **/
	virtual void addWidget(BoUfoWidget* w);

	/**
	 * Insert a spacing (to the layout) of @p spacing. This adds a dummy
	 * widget that takes exactly @p spacing size in both directions.
	 **/
	virtual void addSpacing(int spacing);

	/**
	 * Remove the child widget @p w and delete it.
	 *
	 * If @p w is not a child of this widget, this method does nothing.
	 *
	 * See also @ref addWidget
	 * @return TRUE if the widget was removed, otherwise FALSE.
	 **/
	bool removeWidget(BoUfoWidget* w);

	/**
	 * Remove all child widgets.
	 * @return The number of returned widgets
	 **/
	unsigned int removeAllWidgets();

	unsigned int widgetCount() const;

	/**
	 * Set the foreground color of the @ref widget and all of it's children
	 * (!!) to @p c
	 **/
	virtual void setForegroundColor(const QColor& c);

	/**
	 * WARNING: this has no effect, unless @ref opaque is TRUE !
	 **/
	void setBackgroundColor(const QColor& c);

	virtual void setLayoutClass(LayoutClass);
	LayoutClass layoutClass() const
	{
		return mLayoutClass;
	}

	/**
	 * A hint to the layout manager. See libufo layout documentation.
	 *
	 * The constraints property is currently used by the @ref UBorderLayout
	 * only, where it specifies the position of the widget inside the border
	 * layout. Its value may be "north", "south", "east", "west" or
	 * "center".
	 **/
	void setConstraints(const QString&);
	QString constraints() const;

	/**
	 * A hint to the layout manager, if this widget uses a @ref
	 * UGridLayout layout.
	 *
	 * This properties lets you set the number of columns the widgets will
	 * be organized in. Note that the columns do @em not necessarily have
	 * the same size! (However all widgets in the same column do have the same
	 * width)
	 *
	 * By default every widget uses a value of -1. See also @ref
	 * setGridLayoutRows, but note that it makes no sense to set both,
	 * columns and rows to values other than -1.
	 **/
	void setGridLayoutColumns(int columns);
	int gridLayoutColumns() const;

	/**
	 * A hint to the layout manager, if this widget uses a @ref
	 * UGridLayout layout.
	 *
	 * This properties lets you set the number of rows the widgets will
	 * be organized in. Note that the rows do @em not necessarily have
	 * the same size! (However all widgets in the same row do have the same
	 * height)
	 *
	 * By default every widget uses a value of -1. See also @ref
	 * setGridLayoutColumns, but note that it makes no sense to set both,
	 * columns and rows to values other than -1.
	 **/
	void setGridLayoutRows(int rows);
	int gridLayoutRows() const;

	/**
	 * Set a stretch factor that can be used by the layout class of the
	 * _parent_ widget.
	 *
	 * By default every widget has a stretchFactor of 0.
	 **/
	void setStretch(int stretchFactor);
	int stretch() const;

	void setMinimumWidth(int w);
	int minimumWidth() const;

	void setMinimumHeight(int h);
	int minimumHeight() const;

	void setPreferredWidth(int w);
	int preferredWidth() const;

	void setPreferredHeight(int h);
	int preferredHeight() const;

	bool isVisible() const;

	/**
	 * Call @ref ufo::UWidget::paint for this widget.
	 *
	 * You normally don't need this, as the paint of the parent widget
	 * should call this automatically.
	 *
	 * Calling this makes sense for widgets that have not been added to
	 * other widgets only.
	 **/
	void render(BoUfoManager*);


	void loadPropertiesFromXML(const QDomElement&);
	void loadProperties(const QMap<QString, QString>&);

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);
	virtual void setSize(int w, int h);
	virtual void setPos(int x, int y);
	int width() const;
	int height() const;
	int x() const;
	int y() const;

	void setMouseEventsEnabled(bool enabled, bool moveEnabled);
	void setKeyEventsEnabled(bool enabled);
	void setFocusEventsEnabled(bool enabled);
	void setWidgetEventsEnabled(bool enabled);
	bool isMouseEventsEnabled() const;
	bool isMouseMoveEventsEnabled() const;
	bool isKeyEventsEnabled() const;
	bool isFocusEventsEnabled() const;
	bool isWidgetEventsEnabled() const;
	bool hasMouse() const;

	void setFont(const BoUfoFontInfo& info);
	void unsetFont();

	/**
	 * @return TRUE if @ref setFont was called without calling @ref
	 * unsetFont afterwards. Otherwise FALSE.
	 **/
	bool providesOwnFont() const
	{
		return mUsesOwnFont;
	}

	/**
	 * @param rootPos A position relative to the @ref BoUfoManager::rootPane
	 * @return @p rootPos in coordinates relative to this widget.
	 **/
	QPoint mapFromRoot(const QPoint& rootPos) const;

	/**
	 * @param pos A position relative to this widget.
	 * @return @p pos in coordinates relative to the @ref
	 * BoUfoManager::rootPane
	 **/
	QPoint mapToRoot(const QPoint& pos) const;


	/**
	 * @return The location of this BoUfo widget in the root pane, i.e.
	 * @ref BoUfoManager::rootPane.
	 **/
	QPoint rootLocation() const;

	/**
	 * @return A rectangle that describes the widget in normal OpenGL space
	 * (that is with (0,0) at bottom-left). This rect can be used to define
	 * a viewport for the widget (using glViewport()).
	 **/
	QRect widgetViewportRect() const;

	/**
	 * Used by @ref BoUfoCustomWidget only
	 **/
	virtual void paint() {}
	/**
	 * Used by @ref BoUfoCustomWidget only
	 **/
	virtual void paintWidget() {}
	/**
	 * Used by @ref BoUfoCustomWidget only
	 **/
	virtual void paintBorder() {}

	/**
	 * Used by @ref BoUfoCustomWidget only
	 **/
	virtual QSize preferredSize(const QSize& maxSize = QSize(QCOORD_MAX, QCOORD_MAX)) const
	{
		Q_UNUSED(maxSize);
		return QSize(0, 0);
	}


public slots:
	void setVisible(bool);
	void show() { setVisible(true); }
	void hide() { setVisible(false); }
	void setEnabled(bool);

signals: // TODO: remove ufo::* parameters. use Qt or custom parameters only
	void signalMouseEntered(ufo::UMouseEvent* e);
	void signalMouseExited(ufo::UMouseEvent* e);
	void signalMouseMoved(QMouseEvent* e);
	void signalMouseDragged(QMouseEvent* e);
	void signalMousePressed(QMouseEvent* e);
	void signalMouseReleased(QMouseEvent* e);

	/**
	 * Emitted on "clicks", i.e. when the mouse is released with barely (or
	 * not at all) being moved away from the point where it was pushed down.
	 *
	 * The provided parameter is actually a @ref BoUfoMouseEventClick with a
	 * type of @ref BoUfoMouseEventClick::EventClick. In addition to a
	 * normal @ref QMouseEvent you can use it to retrieve the @ref
	 * BoUfoMouseEventClick::clickCount which allows to distinguish between
	 * simple clicks and e.g. double clicks.
	 **/
	void signalMouseClicked(QMouseEvent* e);
	void signalMouseWheel(QWheelEvent* e);
	void signalKeyPressed(ufo::UKeyEvent* e);
	void signalKeyReleased(ufo::UKeyEvent* e);
	void signalKeyTyped(ufo::UKeyEvent* e);
	/**
	 * Emitted when this widget is added to another widget.
	 * See @ref ufo::UWidget::sigWidgetAdded
	 **/
	void signalWidgetAdded();
	void signalWidgetRemoved();
	void signalWidgetMoved(ufo::UWidgetEvent* e);
	void signalWidgetResized();
	void signalWidgetShown(ufo::UWidgetEvent* e);
	void signalWidgetHidden(ufo::UWidgetEvent* e);
	void signalFocusGained(ufo::UFocusEvent* e);
	void signalFocusLost(ufo::UFocusEvent* e);

private:
	void init(ufo::UWidget* w);

	void uslotHierarchy(ufo::UWidget*, bool added);
	void uslotMouseEntered(ufo::UMouseEvent* e);
	void uslotMouseExited(ufo::UMouseEvent* e);
	void uslotMouseMoved(ufo::UMouseEvent* e);
	void uslotMouseDragged(ufo::UMouseEvent* e);
	void uslotMousePressed(ufo::UMouseEvent* e);
	void uslotMouseReleased(ufo::UMouseEvent* e);
	void uslotMouseClicked(ufo::UMouseEvent* e);
	void uslotMouseWheel(ufo::UMouseWheelEvent* e);
	void uslotKeyPressed(ufo::UKeyEvent* e);
	void uslotKeyReleased(ufo::UKeyEvent* e);
	void uslotKeyTyped(ufo::UKeyEvent* e);
	void uslotWidgetAdded(ufo::UWidgetEvent* w);
	void uslotWidgetRemoved(ufo::UWidgetEvent* w);
	void uslotWidgetMoved(ufo::UWidgetEvent* e);
	void uslotWidgetResized(ufo::UWidgetEvent* e);
	void uslotWidgetShown(ufo::UWidgetEvent* e);
	void uslotWidgetHidden(ufo::UWidgetEvent* e);
	void uslotFocusGained(ufo::UFocusEvent* e);
	void uslotFocusLost(ufo::UFocusEvent* e);

protected:
	/**
	 * @overload
	 * Use the public version instead.
	 **/
	// AB: this one is useful when you want to use a ufo::UImage. however
	// you still need to take care of deletion yourself and if we would make
	// this public, we'd forget that.
	void setBackground(ufo::UDrawable* drawable);

private:
	ufo::UWidget* mWidget;
	LayoutClass mLayoutClass;
	QString mBackgroundImageFile;
	ufo::UDrawable* mBackgroundImageDrawable;
	bool mUsesOwnFont;
};

class BoUfoVBox : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoVBox();

private:
	void init();
};

class BoUfoHBox : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoHBox();

private:
	void init();
};

#endif
