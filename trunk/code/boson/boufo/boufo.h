/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
class QDomNamedNodeMap;
template<class T1, class T2> class QMap;

class BoUfoActionCollection;
class BoUfoMenuBar;
class BoUfoWidget;
class BoUfoInternalFrame;

namespace ufo {
	class UXToolkit;
	class UXDisplay;
	class UXContext;
	class URootPane;
	class UObject;
	class ULayoutManager;
	class UDimension;

	class UWidget;
	class UButton;
	class UCheckBox;
	class UTextEdit;
	class USlider;
	class UListBox;
	class ULineEdit;
	class ULabel;
	class UComboBox;
	class UInternalFrame;


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
};


/**
 * This is the primary class for the boson wrapper around libufo (BoUfo). It
 * creates a libufo context and display and provides convenience methods for
 * event handling.
 *
 * Most of this is straight forward, but some important things are to be
 * mentioned:
 *
 * libufo maintains an event queue (just like e.g. Qt). Whenever you post/push
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

	void postResizeEvent(int w, int h);

	// TODO: find out whether the event is taken by UFO and return true if
	// that is the case (other GUI elements can ignore it)
	void postMousePressEvent(QMouseEvent* e);
	void postMouseReleaseEvent(QMouseEvent* e);
	void postMouseMoveEvent(QMouseEvent* e);
	void postWheelEvent(QWheelEvent* e);
	void postKeyPressEvent(QKeyEvent* e);
	void postKeyReleaseEvent(QKeyEvent* e);

private:
	ufo::UXDisplay* mDisplay;
	ufo::UXContext* mContext;

	ufo::URootPane* mRootPane;
	ufo::UWidget* mContentPane;
	BoUfoWidget* mContentWidget;

	BoUfoActionCollection* mActionCollection;
	BoUfoMenuBar* mMenuBar;
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
 **/
class BoUfoWidget : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString constraints READ constraints WRITE setConstraints);
	Q_PROPERTY(bool opaque READ opaque WRITE setOpaque);
	Q_PROPERTY(LayoutClass layout READ layoutClass WRITE setLayoutClass);
	Q_PROPERTY(int minimumWidth READ minimumWidth WRITE setMinimumWidth);
	Q_PROPERTY(int minimumHeight READ minimumHeight WRITE setMinimumHeight);
	Q_PROPERTY(int preferredWidth READ preferredWidth WRITE setPreferredWidth);
	Q_PROPERTY(int preferredHeight READ preferredHeight WRITE setPreferredHeight);
	Q_ENUMS(LayoutClass);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoWidget();
	BoUfoWidget(ufo::UWidget* w);
	~BoUfoWidget();

	enum LayoutClass {
		NoLayout = 0, UFlowLayout, UBoxLayout, UHBoxLayout,
		UVBoxLayout, UBorderLayout
	};

	ufo::UWidget* widget() const
	{
		return mWidget;
	}

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

	virtual void addWidget(BoUfoWidget* w);
	virtual void addSpacing(int spacing);

	virtual void setLayoutClass(LayoutClass);
	LayoutClass layoutClass() const
	{
		return mLayoutClass;
	}

	void setConstraints(const QString&);
	QString constraints() const;

	void setMinimumWidth(int w);
	int minimumWidth() const;

	void setMinimumHeight(int h);
	int minimumHeight() const;

	void setPreferredWidth(int w);
	int preferredWidth() const;

	void setPreferredHeight(int h);
	int preferredHeight() const;

	void show();
	void hide();


	void loadPropertiesFromXML(const QDomNamedNodeMap&);
	void loadProperties(const QMap<QString, QString>&);

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);


signals:
	void signalMouseEntered(ufo::UMouseEvent* e);
	void signalMouseExited(ufo::UMouseEvent* e);
	void signalMouseMoved(ufo::UMouseEvent* e);
	void signalMouseDragged(ufo::UMouseEvent* e);
	void signalMousePressed(ufo::UMouseEvent* e);
	void signalMouseReleased(ufo::UMouseEvent* e);
	void signalMouseClicked(ufo::UMouseEvent* e);
	void signalMouseWheel(ufo::UMouseWheelEvent* e);
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
	void signalWidgetResized(ufo::UWidgetEvent* e);
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

private:
	ufo::UWidget* mWidget;
	LayoutClass mLayoutClass;
};

class BoUfoLabel : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
public:
	BoUfoLabel();
	BoUfoLabel(const QString& text);

	ufo::ULabel* label() const
	{
		return mLabel;
	}

	void setText(const QString& text);
	QString text() const;

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

private:
	void init();

private:
	ufo::ULabel* mLabel;
};

class BoUfoPushButton : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
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

signals:
	void signalActivated();
	void signalClicked(); // equivalent to signalActivated()
	void signalHighlighted();

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);

private:
	ufo::UButton* mButton;
};

class BoUfoCheckBox : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(bool checked READ checked WRITE setChecked);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoCheckBox();
	BoUfoCheckBox(const QString& text, bool checked = false);

	ufo::UCheckBox* checkBOx() const
	{
		return mCheckBox;
	}

	void setText(const QString& text);
	QString text() const;
	void setChecked(bool);
	bool checked() const;

signals:
	void signalActivated();
	void signalHighlighted();
	void signalToggled(bool);

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);

private:
	ufo::UCheckBox* mCheckBox;
};

class BoUfoLineEdit : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
public:
	BoUfoLineEdit();

	ufo::ULineEdit* lineEdit() const
	{
		return mLineEdit;
	}

	void setText(const QString& text);
	QString text() const;

signals:
	void signalActivated();
	void signalActivated(const QString& text);

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

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
public:
	BoUfoTextEdit();

	ufo::UTextEdit* textEdit() const
	{
		return mTextEdit;
	}

	void setText(const QString& text);
	QString text() const;

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

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

signals:
	void signalActivated(int);
	void signalHighlighted(int);
	void signalSelectionChanged(int, int);

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

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
public:
	BoUfoListBox();

	ufo::UListBox* listBox() const
	{
		return mListBox;
	}

signals:
	void signalSelectionChanged(int firstIndex, int lastIndex);

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

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

class BoUfoNumInput : public BoUfoWidget
{
	Q_OBJECT
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
	float minimumValue() const;
	float maximumValue() const;
	float stepSize() const;

	void setLabel(const QString& label, int a = AlignLeft | AlignTop);
	void setStepSize(float);
	void setRange(float min, float max);

public slots:
	void setValue(float);
	void slotSetMaxValue(float);

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

protected slots:
	void slotButtonClicked();

private:
	void init();

private:
	BoUfoTabWidgetPrivate* d;
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

	void setVisible(bool);
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
