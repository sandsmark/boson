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
#ifndef BOUFO_H
#define BOUFO_H

#include <qobject.h>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
template<class T1, class T2> class QMap;
template<class T1> class QValueList;
class QDomElement;

class BoUfoActionCollection;
class BoUfoMenuBar;
class BoUfoWidget;
class BoUfoInternalFrame;
class BoUfoLayeredPane;

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


/**
 * This class is for some experimental projects (which are not yet in cvs).
 **/
class BoUfoFactory
{
public:
	static BoUfoWidget* createWidget(const QString& className);
	static QStringList widgets();
};

class BoUfoManager;

/**
 * @short Class that stores information about a font
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoFontInfo
{
public:
	enum Styles {
		StyleItalic = 1,
		StyleBold = 2,
		StyleUnderline = 4,
		StyleStrikeOut = 8
	};
public:
	BoUfoFontInfo();
	BoUfoFontInfo(const BoUfoFontInfo&);
	BoUfoFontInfo(const QString& fontPlugin, const ufo::UFontInfo&);

	BoUfoFontInfo& operator=(const BoUfoFontInfo&);

	void setFontPlugin(const QString& plugin) { mFontPlugin = plugin; }
	const QString& fontPlugin() const { return mFontPlugin;}

	void setFamily(const QString& family) { mFamily = family; }
	const QString& family() const { return mFamily; }

	void setPointSize(float s) { mPointSize = s; }
	float pointSize() const { return mPointSize; }

	void setItalic(bool e)
	{
		if (e) {
			mStyle |= StyleItalic;
		} else {
			mStyle &= ~StyleItalic;
		}
	}
	bool italic() const { return mStyle & StyleItalic; }
	void setBold(bool e)
	{
		if (e) {
			mStyle |= StyleBold;
		} else {
			mStyle &= ~StyleBold;
		}
	}
	bool bold() const { return mStyle & StyleBold; }
	void setUnderline(bool e)
	{
		if (e) {
			mStyle |= StyleUnderline;
		} else {
			mStyle &= ~StyleUnderline;
		}
	}
	bool underline() const { return mStyle & StyleUnderline; }
	void setStrikeOut(bool e)
	{
		if (e) {
			mStyle |= StyleStrikeOut;
		} else {
			mStyle &= ~StyleStrikeOut;
		}
	}
	bool strikeOut() const { return mStyle & StyleStrikeOut; }

	/**
	 * Set the style directly using values from the @ref Styles enum.
	 * Ususually you should use @ref setItalic, @ref setBold, ... instead
	 **/
	void setStyle(int style) { mStyle = style; }
	int style() const { return mStyle; }

	void setFixedSize(bool e) { mFixedSize = e; }
	bool fixedSize() const { return mFixedSize; }

	/**
	 * This creates a NEW (!!) font according to the properties of this
	 * font. If there is no such font available, libufo will return an
	 * alternative font.
	 *
	 * You should use this font in a ufo widget (using setFont) or delete
	 * it.
	 **/
	ufo::UFont* ufoFont(BoUfoManager* manager) const;

	ufo::UFontInfo ufoFontInfo() const;
private:
	QString mFontPlugin;
	QString mFamily;
	int mStyle;
	float mPointSize;
	bool mFixedSize;
};


/**
 * This is the primary class for the boson wrapper around libufo (BoUfo). It
 * creates a libufo context and display and provides convenience methods for
 * event handling.
 *
 * Most of this is straight forward, but some important things are to be
 * mentioned:
 *
 * libufo maintains an event queue (just like e.g. Qt). Whenever you push
 * an event to a libufo class, you need to dispatch the events before they take
 * effect. You can do this using @ref dispatchEvent. Since we usually re-render
 * the screen in certain intervals in boson, it may be convenient for you to
 * render the ufo widgets like this:
 * <pre>
 * mUfoManager->dispatchEvents();
 * mUfoManager->render();
 * </pre>
 * This way you can safely forget about dispatching events, because it is done
 * sufficiently often.
 *
 * The @ref menuBar is a very special case, because we use the *ui.rc xml files,
 * just like KDE does. Therefore you do not need to create a menubar yourself -
 * instead use @ref BoUfoAction to declare your action (similar to @ref KAction)
 * and let your menu be generated by @ref BoActionCollection::createGUI.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoManager : public QObject
{
	Q_OBJECT
public:
	/**
	 * @param opaque If FALSE (default) the @ref contentWidget is
	 * transparent.
	 **/
	BoUfoManager(int w, int h, bool opaque = false);
	~BoUfoManager();

	// AB: note that atm we always use video device size == context size
	void resize(int w, int h);

	ufo::UXToolkit* toolkit() const;

	ufo::UXDisplay* display() const
	{
		return mDisplay;
	}

	ufo::UXContext* context() const
	{
		return mContext;
	}
	ufo::URootPane* rootPane() const
	{
		return mRootPane;
	}

	/**
	 * You should prefer @ref contentWidget instead, which returns the same
	 * widget.
	 **/
	ufo::UWidget* contentPane() const
	{
		return mContentPane;
	}

	/**
	 * @return A BoUfo wrapper around @ref contentPane. Use this instead of
	 * @ref contentPane.
	 **/
	BoUfoWidget* contentWidget() const
	{
		return mContentWidget;
	}

	/**
	 * @return The layered pane of the @ref ufo::URootPane. This is the
	 * desktop pane, containing the @ref contentWidget as well as the
	 * menubar
	 **/
	BoUfoLayeredPane* layeredPaneWidget() const
	{
		return mLayeredPaneWidget;
	}

	void addFrame(BoUfoInternalFrame*);
	void removeFrame(BoUfoInternalFrame*);

	/**
	 * This may be required when you have multiple BoUfoManager objects
	 * around (e.g. when you use more than one window). You should always
	 * make the context current before using anything in BoUfo/libufo.
	 *
	 * You can safely ignore this if you have only one BoUfoManager object.
	 **/
	void makeContextCurrent();

	void dispatchEvents();
	void render();


	/**
	 * Use @ref BoUfoActionCollection::initActionCollection to set this
	 **/
	void setActionCollection(BoUfoActionCollection* c)
	{
		mActionCollection = c;
	}
	BoUfoActionCollection* actionCollection() const
	{
		return mActionCollection;
	}
	/**
	 * Called internally. Use @ref BoUfoActionCollection to create menus.
	 **/
	void setMenuBar(BoUfoMenuBar* m);
	BoUfoMenuBar* menuBar() const
	{
		return mMenuBar;
	}

	bool sendResizeEvent(int w, int h);

	// TODO: find out whether the event is taken by UFO and return true if
	// that is the case (other GUI elements can ignore it)
	bool sendMousePressEvent(QMouseEvent* e);
	bool sendMouseReleaseEvent(QMouseEvent* e);
	bool sendMouseMoveEvent(QMouseEvent* e);
	bool sendWheelEvent(QWheelEvent* e);
	bool sendKeyPressEvent(QKeyEvent* e);
	bool sendKeyReleaseEvent(QKeyEvent* e);

	/**
	 * Convenience method that uses the above methods.
	 **/
	bool sendEvent(QEvent* e);

	void setUfoToolkitProperty(const QString& key, const QString& value);
	QString ufoToolkitProperty(const QString& key) const;
	QMap<QString, QString> toolkitProperties() const;

	QValueList<BoUfoFontInfo> listFonts();
	QValueList<BoUfoFontInfo> listFonts(const BoUfoFontInfo&);

	bool focusedWidgetTakesKeyEvents() const;

private:
	ufo::UXDisplay* mDisplay;
	ufo::UXContext* mContext;

	ufo::URootPane* mRootPane;
	ufo::UWidget* mContentPane;
	BoUfoWidget* mContentWidget;
	BoUfoLayeredPane* mLayeredPaneWidget;

	BoUfoActionCollection* mActionCollection;
	BoUfoMenuBar* mMenuBar;
};

/**
 * Frontend to @ref ufo::UDrawable. You can simply subclass this class and
 * totally ignore @ref ufo::UDrawable. If you require a @ref ufo::UDrawable
 * pointer, just use @ref drawable which provides a pointer to the internal
 * drawable (which calls methods in this class only).
 **/
class BoUfoDrawable
{
public:
	BoUfoDrawable();
	virtual ~BoUfoDrawable();

	ufo::UDrawable* drawable() const
	{
		return mDrawable;
	}

	virtual void render(int x, int y, int w, int h) = 0;

	virtual int drawableWidth() const = 0;
	virtual int drawableHeight() const = 0;

private:
	ufo::UDrawable* mDrawable;
};

/**
 * Use @ref BoUfoImage instead.
 **/
class BoUfoImageIO
{
public:
	BoUfoImageIO();
	BoUfoImageIO(const QPixmap&);
	BoUfoImageIO(const QImage&);
	~BoUfoImageIO();

	void setPixmap(const QPixmap& p);
	void setImage(const QImage& img);

	ufo::UImageIO* imageIO() const
	{
		return mImageIO;
	}

private:
	void init();

private:
	ufo::UImageIO* mImageIO;
};


/**
 * Frontend to @ref ufo::UImage. Just create an object of this class with the
 * desired @ref QPixmap or @ref QImage and then use @ref image for your ufo
 * object.
 *
 * An object of this class references the @ref ufo::UImage object it holds and
 * unreferences it when it is destroyed. Therefore you can use the @ref image in
 * as many ufo object as you want.
 * Note that when the object of this class is destroyed, the @ref image is only
 * deleted when no ufo object holds a reference anymore, so you can delete this
 * object at any time.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoImage
{
public:
	BoUfoImage();
	BoUfoImage(const QPixmap&); // slowest
	BoUfoImage(const QImage&);
	BoUfoImage(const BoUfoImage&); // fastest
	~BoUfoImage();

	void load(const QPixmap&);
	void load(const QImage&);
	void load(const BoUfoImage&);

	ufo::UImage* image() const
	{
		return mImage;
	}

protected:
	void set(BoUfoImageIO*);
	void set(ufo::UImage*);

private:
	void init();

private:
	ufo::UImage* mImage;
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
		RaisedBevelBorder = 2,
		LoweredBevelBorder = 3,
		TitledBorder = 4,
		UIBorder = 5
	};

	ufo::UWidget* widget() const
	{
		return mWidget;
	}

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

	void setEnabled(bool);
	bool isEnabled() const;

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

	void setVisible(bool);
	void show() { setVisible(true); }
	void hide() { setVisible(false); }
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
	bool hasMouse() const;

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


signals: // TODO: remove ufo::* parameters. use Qt or custom parameters only
	void signalMouseEntered(ufo::UMouseEvent* e);
	void signalMouseExited(ufo::UMouseEvent* e);
	void signalMouseMoved(QMouseEvent* e);
	void signalMouseDragged(QMouseEvent* e);
	void signalMousePressed(QMouseEvent* e);
	void signalMouseReleased(QMouseEvent* e);
	void signalMouseClicked(ufo::UMouseEvent* e);
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
	void uslotWidgetAdded(ufo::UWidget* w);
	void uslotWidgetRemoved(ufo::UWidget* w);
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
};

class BoUfoLabel : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(QString iconFile READ iconFile WRITE setIconFile);
public:
	BoUfoLabel();
	BoUfoLabel(const QString& text);

	ufo::ULabel* label() const
	{
		return mLabel;
	}

	void setText(const QString& text);
	QString text() const;
	void setIcon(const BoUfoImage&);

	/**
	 * Convenience method that calls @ref setIcon.
	 **/
	void setIconFile(const QString&);

	/**
	 * @return The filename used in @ref setIconFile. Note that if you set
	 * an icon through other methods (e.g. by using @ref setIcon directly)
	 * the returned value of this method is undefined.
	 **/
	QString iconFile() const;

	virtual void setOpaque(bool o);

	static void setDefaultForegroundColor(const QColor& color);
	static const QColor& defaultForegroundColor();

	// AB: probably this should be in BoUfoWidget and all derived classes
	// reimplement it for their own widget.
	// however atm we don't really need it
	void setFont(BoUfoManager*, const BoUfoFontInfo& info);

	/**
	 * See @ref BoUfoWidget::setVerticalAlignment
	 **/
	virtual void setVerticalAlignment(VerticalAlignment alignment);

	/**
	 * See @ref BoUfoWidget::setHorizontalAlignment
	 **/
	virtual void setHorizontalAlignment(HorizontalAlignment alignment);

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

private:
	void init();

private:
	ufo::ULabel* mLabel;
	QString mIconFile;

	static QColor mDefaultForegroundColor;
};

class BoUfoPushButton : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(QString iconFile READ iconFile WRITE setIconFile);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoPushButton();
	BoUfoPushButton(const QString& text);

	ufo::UButton* button() const
	{
		return mButton;
	}

	void setText(const QString& text);
	QString text() const;
	void setIcon(const BoUfoImage&);

	/**
	 * Convenience method that calls @ref setIcon.
	 **/
	void setIconFile(const QString&);

	/**
	 * @return The filename used in @ref setIconFile. Note that if you set
	 * an icon through other methods (e.g. by using @ref setIcon directly)
	 * the returned value of this method is undefined.
	 **/
	QString iconFile() const;

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

	/**
	 * See @ref BoUfoWidget::setVerticalAlignment
	 **/
	virtual void setVerticalAlignment(VerticalAlignment alignment);

	/**
	 * See @ref BoUfoWidget::setHorizontalAlignment
	 **/
	virtual void setHorizontalAlignment(HorizontalAlignment alignment);

signals:
	void signalActivated();
	void signalClicked(); // equivalent to signalActivated()
	void signalHighlighted();

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);

private:
	ufo::UButton* mButton;
	QString mIconFile;
};

class BoUfoCheckBox : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(bool checked READ checked WRITE setChecked);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoCheckBox();
	BoUfoCheckBox(const QString& text, bool checked = false);

	ufo::UCheckBox* checkBox() const
	{
		return mCheckBox;
	}

	void setText(const QString& text);
	QString text() const;
	void setChecked(bool);
	bool checked() const;

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

signals:
	void signalActivated();
	void signalHighlighted();
	void signalToggled(bool);

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);

private:
	ufo::UCheckBox* mCheckBox;
};

class BoUfoRadioButton : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(bool selected READ selected WRITE setSelected);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoRadioButton();
	BoUfoRadioButton(const QString& text, bool selected = false);

	ufo::URadioButton* radioButton() const
	{
		return mRadioButton;
	}

	void setText(const QString& text);
	QString text() const;
	void setSelected(bool);
	bool selected() const;

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

signals:
	void signalActivated();
	void signalHighlighted();
	void signalToggled(bool);

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);

private:
	ufo::URadioButton* mRadioButton;
};

/**
 * WARNING: buttons that are deleted are @em not automatically removed from the
 * button group! You should make sure that the button group is always deleted
 * first!
 *
 * Note: An object of this class is not deleted by libufo but by Qt (this is
 * contrary to most other BoUfo classes). It is therefore recommended to use a
 * Qt parent for objects of this class (e.g. a BoUfoWidget).
 **/
class BoUfoButtonGroup : public QObject
{
	Q_OBJECT
public:
	BoUfoButtonGroup(QObject* parent);
	~BoUfoButtonGroup();

	void addButton(BoUfoRadioButton* button);
	void removeButton(BoUfoRadioButton* button);

	BoUfoRadioButton* selectedButton() const;

signals:
	void signalButtonActivated(BoUfoRadioButton* button);

protected slots:
	void slotButtonActivated();

private:
	ufo::UButtonGroup* mButtonGroup;
	QMap<ufo::URadioButton*, BoUfoRadioButton*>* mButtons;
};

/**
 * A normal @ref BoUfoWidget that automatically adds any @ref BoUfoRadioButton
 * that are added to an internal @ref BoUfoButtonGroup.
 **/
class BoUfoButtonGroupWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoButtonGroupWidget();
	~BoUfoButtonGroupWidget();

	virtual void addWidget(BoUfoWidget*);
	virtual void removeWidget(BoUfoWidget*);

	BoUfoRadioButton* selectedButton() const;

signals:
	void signalButtonActivated(BoUfoRadioButton*);

private:
	BoUfoButtonGroup* mButtonGroup;
};

class BoUfoLineEdit : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(bool editable READ isEditable WRITE setEditable);
public:
	BoUfoLineEdit();

	ufo::ULineEdit* lineEdit() const
	{
		return mLineEdit;
	}

	void setText(const QString& text);
	QString text() const;

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

	void setEditable(bool);
	bool isEditable() const;

signals:
	void signalActivated();
	void signalActivated(const QString& text);

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);

signals:
	void signalTextChanged(const QString&);

private:
	ufo::ULineEdit* mLineEdit;
};

class BoUfoTextEdit : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(bool editable READ isEditable WRITE setEditable);
public:
	BoUfoTextEdit();

	ufo::UTextEdit* textEdit() const
	{
		return mTextEdit;
	}

	void setText(const QString& text);
	QString text() const;

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

	void setEditable(bool);
	bool isEditable() const;

private:
	void init();

signals:
	void signalTextChanged(const QString&);

private:
	ufo::UTextEdit* mTextEdit;
};

class BoUfoComboBox : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoComboBox();

	ufo::UComboBox* comboBox() const
	{
		return mComboBox;
	}

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

	int currentItem() const;
	void setCurrentItem(int i);
	QString currentText() const;

	QStringList items() const;
	void setItems(const QStringList& items);
	void clear();
	unsigned int count() const;
	QString itemText(int i) const;
	void setItemText(int i, const QString& text);
	void insertItem(const QString& text, int index = -1);
	void removeItem(int index);


signals:
	void signalActivated(int);
	void signalHighlighted(int);
	void signalSelectionChanged(int, int);

private:
	void init();
	void uslotActivated(ufo::UComboBox*, int);
	void uslotHighlighted(ufo::UComboBox*, int);
	void uslotSelectionChanged(ufo::UComboBox*, int, int);

signals:

private:
	ufo::UComboBox* mComboBox;
};

class BoUfoListBox : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(SelectionMode selection READ selectionMode WRITE setSelectionMode);
	Q_ENUMS(SelectionMode);
public:
	enum SelectionMode {
		NoSelection,
		SingleSelection,
		MultiSelection
	};
public:
	BoUfoListBox();

	ufo::UListBox* listBox() const
	{
		return mListBox;
	}

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

	int selectedItem() const;
	void setItemSelected(int index, bool select);
	QString selectedText() const;
	bool isSelected(int index) const;
	void unselectAll();

	// this exists because of libufo only. use setItemSelected() instead!
	void setSelectedItem(int index);

	/**
	 * @return A list with the indices of all selected items
	 **/
	QValueList<unsigned int> selectedItems() const;

	/**
	 * @return A list with the text of all selected items
	 **/
	QStringList selectedItemsText() const;

	/**
	 * @return TRUE if there is currently one item with @p text selected
	 **/
	bool isTextSelected(const QString& text) const;

	QStringList items() const;
	void setItems(const QStringList& items);
	void clear();
	unsigned int count() const;
	QString itemText(int i) const;
	void setItemText(int i, const QString& text);
	void insertItem(const QString& text, int index = -1);
	void removeItem(int index);


	// note: libufo does not yet implement MultiSelection
	void setSelectionMode(SelectionMode mode);
	SelectionMode selectionMode() const;


signals:
	void signalSelectionChanged(int firstIndex, int lastIndex);

private:
	void init();
	void uslotSelectionChanged(ufo::UListBox*, int, int);

signals:

private:
	ufo::UListBox* mListBox;
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

/**
 * A BoUfo implementation of @ref ufo::USlider.
 *
 * Additionally this class provides support for float values.
 **/
class BoUfoSlider : public BoUfoWidget
{
	Q_OBJECT
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoSlider(Qt::Orientation = Horizontal);

	ufo::USlider* slider() const
	{
		return mSlider;
	}

	int value() const;
	float floatValue() const;
	void setValue(int v);
	void setStepSize(int);
	void setRange(int min, int max);

	void setFloatValue(float v);
	void setFloatRange(float min, float max, float step);

	float minimumValue() const { return mMin; }
	float maximumValue() const { return mMax; }
	float stepSize() const { return mStep; }

	virtual void setOpaque(bool o);

signals:
	void signalValueChanged(int);
	void signalFloatValueChanged(float);

private:
	void uslotValueChanged(ufo::USlider*, int);

private:
	void init(Qt::Orientation);

private:
	float mMin;
	float mMax;
	float mStep;
	ufo::USlider* mSlider;
};

class BoUfoProgress : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(double minimumValue READ minimumValue WRITE setMinimumValue);
	Q_PROPERTY(double maximumValue READ maximumValue WRITE setMaximumValue);
	Q_PROPERTY(double value READ value WRITE setValue);
	Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor);
	Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoProgress(Qt::Orientation = Horizontal);

	void setOrientation(Qt::Orientation o);

	ufo::UBoProgress* progress() const
	{
		return mProgress;
	}

	double value() const;
	void setValue(double v);
	void setRange(double min, double max);

	void setMinimumValue(double min);
	void setMaximumValue(double max);
	double minimumValue() const;
	double maximumValue() const;

	void setStartColor(const QColor& color);
	QColor startColor() const;
	void setEndColor(const QColor& color);
	QColor endColor() const;
	void setColor(const QColor& color);

	virtual void setOpaque(bool o);

private:
	void init(Qt::Orientation);

private:
	ufo::UBoProgress* mProgress;
};

class BoUfoNumInput : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString label READ label WRITE setLabel);
	Q_PROPERTY(double minimumValue READ doubleMinimumValue WRITE setDoubleMinimumValue);
	Q_PROPERTY(double maximumValue READ doubleMaximumValue WRITE setDoubleMaximumValue);
	Q_PROPERTY(double stepSize READ doubleStepSize WRITE setDoubleStepSize);
	Q_PROPERTY(double value READ doubleValue WRITE setDoubleValue);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoNumInput();

	BoUfoSlider* slider() const
	{
		return mSlider;
	}
	BoUfoLineEdit* lineEdit() const
	{
		return mLineEdit;
	}

	float value() const;
	void setRange(float min, float max);
	float minimumValue() const;
	float maximumValue() const;
	void setStepSize(float);
	float stepSize() const;

	void setLabel(const QString& label, int a = AlignLeft | AlignTop);
	QString label() const;

	virtual void setOpaque(bool o);

	// AB: these are for Qts property system which doesn't like floats:
	double doubleValue() const { return (double)value(); }
	double doubleMinimumValue() const { return (double)minimumValue(); }
	double doubleMaximumValue() const { return (double)maximumValue(); }
	double doubleStepSize() const { return (double)stepSize(); }
	void setDoubleValue(double v) { setValue((float)v); }
	void setDoubleMinimumValue(double v) { slotSetMinValue((float)v); }
	void setDoubleMaximumValue(double v) { slotSetMaxValue((float)v); }
	void setDoubleStepSize(double v) { setStepSize((float)v); }

public slots:
	void setValue(float);
	void slotSetMaxValue(float);
	void slotSetMinValue(float);

signals:
	void signalValueChanged(float);

private slots:
	void slotSliderChanged(float);
	void slotTextEntered(const QString&);

private:
	void init();

private:
	BoUfoLabel* mLabel;
	BoUfoSlider* mSlider;
	BoUfoLineEdit* mLineEdit;
};

class BoUfoMatrix : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoMatrix();
	~BoUfoMatrix();

	/**
	 * Apply a 4x4 matrix to be displayed
	 **/
	void setMatrix(const float*);

	virtual void setOpaque(bool o);

private:
	void init();

private:
	BoUfoLabel** mMatrix;
};

class BoUfoTabWidgetPrivate;
class BoUfoTabWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoTabWidget();
	~BoUfoTabWidget();

	int addTab(BoUfoWidget* widget, const QString& label);
	void removeTab(BoUfoWidget* widget);

	void setCurrentTab(int tab);
	BoUfoWidget* currentTab() const;

	int findId() const;

	virtual void setOpaque(bool o);

protected slots:
	void slotButtonClicked();

private:
	void init();

private:
	BoUfoTabWidgetPrivate* d;
};

/**
 * This widget uses a modified ufo widget as base. The methods @ref
 * BoUfoWidget::paint, @ref BoUfoWidget::paintWidget and @ref
 * BoUfoWidget::paintBorder are used for rendering.
 *
 * By default, they just call the original implementation of @ref ufo::UWidget.
 **/
class BoUfoCustomWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoCustomWidget();
	~BoUfoCustomWidget();

	virtual void paint();
	virtual void paintWidget();
	virtual void paintBorder();
};

class BoUfoWidgetStack : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoWidgetStack();
	~BoUfoWidgetStack();

	int insertStackWidget(BoUfoWidget*, int id = -1);
	void raiseStackWidget(BoUfoWidget*);
	void raiseStackWidget(int id);
	void removeStackWidget(BoUfoWidget*);
	void removeStackWidget(int id);
	BoUfoWidget* widget(int id) const;
	BoUfoWidget* visibleWidget() const
	{
		return mVisibleWidget;
	}
	int id(BoUfoWidget* widget) const;

private:
	void init();

private:
	QMap<int, BoUfoWidget*>* mId2Widget;
	BoUfoWidget* mVisibleWidget;
};

// A layered pane is comparable to a widget stack, but there can be multiple
// widgets visible at any time. The bottom widgets are drawn first, the top
// widgets are drawn later, so that on the screen they are actually "on top" of
// the other widgets.
class BoUfoLayeredPane : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoLayeredPane();
	BoUfoLayeredPane(ufo::ULayeredPane*, bool provideLayout = true);
	~BoUfoLayeredPane();

	void addLayer(BoUfoWidget* w, int layer = 0, int position = -1);
	void setLayer(BoUfoWidget* w, int layer, int position = -1);
};

/**
 * Note: in libufo @ref ufo::UInternalFrame objects have to be added using @ref
 * ufo::URootPane::addFrame, so they are special cases. You cannot add them to
 * other widgets. Therefore you have to add a BoUfoInternalFrame object using
 * @ref BoUfoManager::addFrame. This is done automatically by the constructor.
 **/
class BoUfoInternalFrame : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoInternalFrame(BoUfoManager* manager, const QString& title = QString::null);
	~BoUfoInternalFrame();

	void setBounds(int x, int y, int w, int h);
	void setTitle(const QString& title);
	QString title() const;
	void setResizable(bool r);
	bool resizable() const;
	void setMaximizable(bool m);
	bool maximizable() const;
	void setMinimizable(bool m);
	bool minimizable() const;

	/**
	 * See @ref ufo::UInternalFrame::pack. This adjusts the size of the
	 * widget so that it fits to its contents.
	 **/
	void adjustSize();

	/**
	 * @reimplemented
	 **/
	virtual void setLayoutClass(LayoutClass c)
	{
		contentWidget()->setLayoutClass(c);
	}

	/**
	 * @reimplemented
	 **/
	virtual void addWidget(BoUfoWidget* w)
	{
		contentWidget()->addWidget(w);
	}

	/**
	 * @reimplemented
	 **/
	virtual void addSpacing(int spacing)
	{
		contentWidget()->addSpacing(spacing);
	}

	ufo::UInternalFrame* frame() const
	{
		return (ufo::UInternalFrame*)widget();
	}
	ufo::URootPane* rootPane() const
	{
		return mRootPane;
	}

	/**
	 * You should prefer @ref contentWidget instead, which returns the same
	 * widget.
	 **/
	ufo::UWidget* contentPane() const
	{
		return mContentPane;
	}

	/**
	 * @return A BoUfo wrapper around @ref contentPane. Use this instead of
	 * @ref contentPane.
	 **/
	BoUfoWidget* contentWidget() const
	{
		return mContentWidget;
	}


private:
	void init();

private:
	ufo::URootPane* mRootPane;
	ufo::UWidget* mContentPane;
	BoUfoWidget* mContentWidget;
};


class BoUfoInputDialog : public BoUfoInternalFrame
{
	Q_OBJECT
public:
	BoUfoInputDialog(BoUfoManager* manager, const QString& label, const QString& title = QString::null);
	~BoUfoInputDialog();

	void setLabel(const QString& label);


	// note that due to the design of libufo we cannot make modal dialogs.
	// so we can't return the integer value here.
	// AB: the returned pointer can be discarded usually.
	static BoUfoInputDialog* getIntegerWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title = QString::null, int value = 0, int min = 0, int max = 1000, int step = 1);
	static BoUfoInputDialog* getFloatWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title = QString::null, float value = 0.0, float min = 0.0, float max = 1000.0, float step = 1.0);
	static BoUfoInputDialog* getStringWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title = QString::null, const QString& value = QString::null);

signals:
	/**
	 * Emitted right before the frame is removed from the ufo manager.
	 *
	 * Note that this widget is closed shortly after this and therfore will
	 * get deleted then.
	 **/
	void signalClosed();

	/**
	 * Emitted when Cancel is clicked, right before the widget is being closed.
	 *
	 * Note that this widget is closed shortly after this and therfore will
	 * get deleted then.
	 **/
	void signalCancelled();
	void signalValueEntered(int);
	void signalValueEntered(float);
	void signalValueEntered(const QString&);

protected:
	BoUfoNumInput* numInput();
	BoUfoLineEdit* lineEdit();
	void close();

	static BoUfoInputDialog* createNumDialog(BoUfoManager* manager, const QString& label, const QString& title, float value, float min, float max, float step);

protected slots:
	void slotOkClicked();
	void slotCancelled();

private:
	void init();

private:
	BoUfoManager* mUfoManager;
	BoUfoLabel* mLabel;
	BoUfoWidget* mContents;
	BoUfoPushButton* mOk;
	BoUfoPushButton* mCancel;
	BoUfoNumInput* mNumInput;
	BoUfoLineEdit* mLineEdit;
};

#endif
