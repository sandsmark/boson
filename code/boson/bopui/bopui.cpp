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

#include "bopui.h"
#include "bopui.moc"

#include "bosonglwidget.h"
#include "bodebug.h"

#include <qintdict.h>

#define PU_USE_NONE // avoid including glut stuff or so.
#include <plib/pu.h>

#include <math.h>
#include <stdlib.h>

int puGetWindowQt()
{
 BoContext* context = BoContext::currentContext();
 if (!context) {
	return 0;
 }
#ifndef Q_WS_X11
#error this assumption is true on X11 only!!
#endif
 // Warning: handle() == winId() is X11 specific.
 // To us that should not matter. On other systems we would need to do a
 // if (paintDevice()->isA("QWidget")) {...} else {error!} here.
 return context->paintDevice()->handle();
}
void puSetWindowQt(int window)
{
 QWidget* w = QWidget::find(window);
 if (!w || !w->inherits("BosonGLWidget")) {
	boError() << k_funcinfo << endl;
	return;
 }
 BosonGLWidget* gl = (BosonGLWidget*)w;
 gl->makeCurrent();
}
void puGetWindowSizeQt(int* w, int* h)
{
 BoContext* context = BoContext::currentContext();
 if (!context) {
	return;
 }
#ifndef Q_WS_X11
#error this assumption is true on X11 only!!
#endif
 int winId = context->paintDevice()->handle();
 QWidget* window = QWidget::find(winId);
 if (!window) {
	BO_NULL_ERROR(window);
	return;
 }
 *w = window->width();
 *h = window->height();
}
void puSetWindowSizeQt(int w, int h)
{
 BoContext* context = BoContext::currentContext();
 if (!context) {
	return;
 }
#ifndef Q_WS_X11
#error this assumption is true on X11 only!!
#endif
 int winId = context->paintDevice()->handle();
 QWidget* window = QWidget::find(winId);
 if (!window) {
	BO_NULL_ERROR(window);
	return;
 }
 if (window->width() != w || window->height() != h) {
	boDebug() << k_funcinfo << w << " " << h << endl;
	window->resize(w, h);
 }
}

void puInitQt()
{
 puSetWindowFuncs(puGetWindowQt,
		puSetWindowQt,
		puGetWindowSizeQt,
		puSetWindowSizeQt);
 puRealInit();
}


class BoPUILayoutItem
{
public:
	BoPUILayoutItem(puObject* o)
	{
		mPUObject = o;
		mWidget = 0;
		mLayout = 0;
		mVSpacing = 0;
		mHSpacing = 0;
	}
	BoPUILayoutItem(BoPUIWidget* w)
	{
		mPUObject = 0;
		mWidget = w;
		mLayout = 0;
		mVSpacing = 0;
		mHSpacing = 0;
	}
	BoPUILayoutItem(BoPUILayout* l)
	{
		mPUObject = 0;
		mWidget = 0;
		mLayout = l;
		mVSpacing = 0;
		mHSpacing = 0;
	}
	BoPUILayoutItem(int vs, int hs)
	{
		mPUObject = 0;
		mWidget = 0;
		mLayout = 0;
		mVSpacing = vs;
		mHSpacing = hs;
	}
	puObject* object() const
	{
		return mPUObject;
	}
	BoPUIWidget* widget() const
	{
		return mWidget;
	}
	BoPUILayout* layout() const
	{
		return mLayout;
	}
	int width() const
	{
		if (object()) {
			if (!object()->isVisible()) {
				return 0;
			}
			puBox* b = object()->getBBox();
			return b->max[0] - b->min[0];
		} else if (widget()) {
			return widget()->width();
		} else if (layout()) {
			return layout()->width();
		}
		return mHSpacing;
	}
	int height() const
	{
		if (object()) {
			if (!object()->isVisible()) {
				return 0;
			}
			puBox* b = object()->getBBox();
			return b->max[1] - b->min[1];
		} else if (widget()) {
			return widget()->height();
		} else if (layout()) {
			return layout()->height();
		}
		return mVSpacing;
	}
	void setPos(int x, int y)
	{
		if (object()) {
			object()->setPosition(x, y);
		} else if (widget()) {
			widget()->setPos(x, y);
		} else if (layout()) {
			layout()->setBasePos(x, y, false);
		}
	}

	// FIXME
	// (-> hide matrices in the plain camera widget, change camera
	// widget using tabs, go back to plain => matrices are shown. this
	// is because show() calls show() recursively on all childrens)
	void show()
	{
		if (object()) {
			object()->reveal();
		} else if (widget()) {
			widget()->show();
		} else if (layout()) {
			layout()->show();
		}
	}
	void hide()
	{
		if (object()) {
			object()->hide();
		} else if (widget()) {
			widget()->hide();
		} else if (layout()) {
			layout()->hide();
		}
	}

	// FIXME: we have the same problem as with show()/hide() here, since we
	// call setEnabled() recursively :(
	void setWidgetsEnabled(bool e)
	{
		if (object()) {
			if (e) {
				object()->activate();
			} else {
				object()->greyOut();
			}
		} else if (widget()) {
			widget()->setEnabled(e);
		} else if (layout()) {
			layout()->setWidgetsEnabled(e);
		}
	}
private:
	puObject* mPUObject;
	BoPUIWidget* mWidget;
	BoPUILayout* mLayout;
	int mVSpacing;
	int mHSpacing;
};

class BoPUILayoutPrivate
{
public:
	BoPUILayoutPrivate()
	{
	}
	QPtrList<BoPUILayout> mChildrenLayouts; // get deleted on destruction
	QPtrList<BoPUILayoutItem> mItems;
	bool mDoingLayout;
	bool mDisableLayout;
};

BoPUILayout::BoPUILayout(bool v, QObject* parent, const char* name) : QObject(parent, name)
{
 init();
 mVertical = v;
}

void BoPUILayout::init()
{
 d = new BoPUILayoutPrivate;
 mTopParentLayout = 0;
 mParentLayout = 0;
 d->mItems.setAutoDelete(true);
 d->mChildrenLayouts.setAutoDelete(true);
 d->mDoingLayout = false;
 d->mDisableLayout = false;
 mVertical = true;
 mBaseX = 0;
 mBaseY = 0;

 //TODO: max width/height
}

BoPUILayout::~BoPUILayout()
{
 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 while (it.current()) {
	if (it.current()->layout()) {
		it.current()->layout()->setParentLayout(0);
	} else if (it.current()->widget()) {
		it.current()->widget()->setParentLayout(0);
	}
	++it;
 }

 d->mItems.clear();
 d->mChildrenLayouts.clear();
 if (mParentLayout) {
	mParentLayout->removeLayout(this);
 }
 delete d;
}

void BoPUILayout::setTopParentLayout(BoPUILayout* l)
{
 mTopParentLayout = l;

 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 while (it.current()) {
	if (it.current()->layout()) {
		it.current()->layout()->setTopParentLayout(l);
	} else if (it.current()->widget()) {
		it.current()->widget()->setTopParentLayout(l);
	}
	++it;
 }
}

void BoPUILayout::addObject(puObject* o)
{
 BoPUILayoutItem* i = new BoPUILayoutItem(o);
 d->mItems.append(i);
}

void BoPUILayout::addWidget(BoPUIWidget* w)
{
 if (w->hasTopParentLayout() || w->hasParentLayout()) {
	boError() << k_funcinfo << "widget " << w->name() << " of class " << w->className() << " already belongs to a layout!" << endl;
	return;
 }
 BoPUILayout* topLayout = mTopParentLayout;
 if (!topLayout) {
	topLayout = this;
 }
 w->setTopParentLayout(topLayout);
 w->setParentLayout(this);
 BoPUILayoutItem* i = new BoPUILayoutItem(w);
 d->mItems.append(i);
}

void BoPUILayout::addLayout(BoPUILayout* l)
{
 if (l->parent()) {
	boError() << k_funcinfo << "layout has already a parent" << endl;
	return;
 }
 if (l->hasTopParentLayout() || l->hasParentLayout()) {
	boError() << k_funcinfo << "layout " << l->name() << " of class " << l->className() << " already belongs to a layout!" << endl;
	return;
 }
 BoPUILayout* topLayout = mTopParentLayout;
 if (!topLayout) {
	topLayout = this;
 }
 l->setTopParentLayout(topLayout);
 l->setParentLayout(this);
 BoPUILayoutItem* i = new BoPUILayoutItem(l);
 d->mItems.append(i);

 d->mChildrenLayouts.append(l); // delete on destruction
}

void BoPUILayout::addSpacing(int s)
{
 int vs = 0;
 int hs = 0;
 if (mVertical) {
	vs = s;
 } else {
	hs = s;
 }
 BoPUILayoutItem* i = new BoPUILayoutItem(vs, hs);
 d->mItems.append(i);
}

void BoPUILayout::removeWidget(BoPUIWidget* w)
{
 BoPUILayoutItem* item = 0;
 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 while (it.current() && !item) {
	if (it.current()->widget() == w) {
		item = it.current();
	}
	++it;
 }
 d->mItems.removeRef(item);
}

void BoPUILayout::removeLayout(BoPUILayout* l)
{
 BoPUILayoutItem* item = 0;
 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 while (it.current() && !item) {
	if (it.current()->layout() == l) {
		item = it.current();
	}
	++it;
 }
 d->mItems.removeRef(item);
}

int BoPUILayout::width() const
{
 int w = 0;
 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 while (it.current()) {
	if (!mVertical) {
		w += it.current()->width();
	} else {
		w = QMAX(w, it.current()->width());
	}
	++it;
 }
 return w;
}

int BoPUILayout::height() const
{
 int h = 0;
 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 while (it.current()) {
	if (mVertical) {
		h += it.current()->height();
	} else {
		h = QMAX(h, it.current()->height());
	}
	++it;
 }
 return h;
}

void BoPUILayout::show()
{
 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 for(; it.current(); ++it) {
	it.current()->show();
 }
 doLayout();
}

void BoPUILayout::hide()
{
 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 for(; it.current(); ++it) {
	it.current()->hide();
 }
 doLayout();
}

void BoPUILayout::setWidgetsEnabled(bool e)
{
 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 for(; it.current(); ++it) {
	it.current()->setWidgetsEnabled(e);
 }
}

void BoPUILayout::disableLayout(bool disable)
{
 if (mTopParentLayout) {
	mTopParentLayout->disableLayout(disable);
	return;
 }
 d->mDisableLayout = disable;
 if (!d->mDisableLayout) {
	doLayout();
 }
}

void BoPUILayout::doLayout()
{
 if (d->mDoingLayout) {
	return;
 }
 if (mTopParentLayout) {
	mTopParentLayout->doLayout();
	return;
 }
 if (d->mDisableLayout) {
	return;
 }
 d->mDoingLayout = true;
 int availableWidth = -1;
 int availableHeight = -1;
 if (parent() && parent()->inherits("BosonGLWidget")) {
	QWidget* w = (QWidget*)parent();
	availableWidth = w->width();
	availableHeight = w->height();
 }
 if (availableWidth < 0 || availableHeight < 0) {
	availableWidth = width();
	availableHeight = height();
 }
 doLayout(availableWidth, availableHeight);
 d->mDoingLayout = false;
}

void BoPUILayout::doLayout(int availableWidth, int availableHeight)
{
 if (availableWidth < 0 && availableHeight < 0) {
	boError() << k_funcinfo << "invalid availableWidth/availableHeight: " << availableWidth << "/" << availableHeight << endl;
	return;
 }

 QPtrListIterator<BoPUILayoutItem> it(d->mItems);
 int x = mBaseX;
 int y = mBaseY;
 while (it.current()) {
	int w = it.current()->width();
	int h = it.current()->height();
	if (mVertical) {
		it.current()->setPos(x, (y + availableHeight) - h);
	} else {
		it.current()->setPos(x, y);
	}

	if (it.current()->layout()) {
		// AB: probably replace availableWidth by w
		it.current()->layout()->doLayout(availableWidth, h);
	} else if (it.current()->widget() && it.current()->widget()->mLayout) {
		// AB: probably replace availableWidth by w
		it.current()->widget()->mLayout->doLayout(availableWidth, h);
	}

	if (mVertical) {
		availableHeight -= h;
	} else {
		x += w;
	}
	++it;
 }
}


BoPUIWidget::BoPUIWidget(QObject* parent, const char* name) : QObject(parent, name)
{
 init();
}

BoPUIWidget::BoPUIWidget(bool vLayout, QObject* parent, const char* name) : QObject(parent, name)
{
 init();
 if (vLayout) {
	mUserLayout = new BoPUIVLayout(0);
 } else {
	mUserLayout = new BoPUIHLayout(0);
 }
 mLayout->addLayout(mUserLayout);
}

void BoPUIWidget::init()
{
 mLayout = new BoPUIHLayout(this, "PUI widget layout");
 mTopParentLayout = 0;
 mParentLayout = 0;
 mUserLayout = 0;
 mLabelWidget = 0;
 mWidget = 0;
 mX = mY = 0;
 mHidden = false;
 mLabel = 0;
 mLegend = 0;
}

BoPUIWidget::~BoPUIWidget()
{
 delete[] mLabel;
 delete[] mLegend;

 delete mLayout;
 delete mLabelWidget;
 if (mParentLayout) {
	mParentLayout->removeWidget(this);
 }
 if (mWidget) {
	// AB: do NOT delete plib widgets directly
	puDeleteObject(mWidget);
 }
}

void BoPUIWidget::setTopParentLayout(BoPUILayout* l)
{
 mTopParentLayout = l;
 if (mLayout) {
	mLayout->setTopParentLayout(l);
 }
}

int BoPUIWidget::width() const
{
 if (mHidden) {
	return 0;
 }
 if (!mLayout) {
	return 0;
 }
 return mLayout->width();
}

int BoPUIWidget::height() const
{
 if (mHidden) {
	return 0;
 }
 if (!mLayout) {
	return 0;
 }
 return mLayout->height();
}

void BoPUIWidget::installCallback(puObject* widget)
{
 BO_CHECK_NULL_RET(widget);
 widget->setUserData(this); // required for callback->signal
 widget->setCallback(myCallback);
}

void BoPUIWidget::myCallback(puObject* o)
{
 BO_CHECK_NULL_RET(o);
 BoPUIWidget* w = (BoPUIWidget*)o->getUserData();
 w->emitSignal();
}

void BoPUIWidget::emitSignal()
{
 emit signalActivated();
}

void BoPUIWidget::setPos(int x, int y)
{
 BO_CHECK_NULL_RET(mLayout);
 mX = x;
 mY = y;
 mLayout->setBasePos(mX, mY);
 mLayout->doLayout();
}

void BoPUIWidget::setSize(int w, int h)
{
 // FIXME
 BO_CHECK_NULL_RET(mWidget);
 BO_CHECK_NULL_RET(mLayout);
 if (mWidget) {
	mWidget->setSize(w, h);
 }
 if (mLayout) {
	// TODO?
 }
}

void BoPUIWidget::setLabel(const QString& text, int alignment)
{
 if (isA("BoPUILabel")) {
	boError() << k_funcinfo << "use setText() for BoPUILabel" << endl;
	return;
 }
 setLabelText(text);

 if (mLabelWidget) {
	mLabelWidget->setText(text);
 } else {
	mLabelWidget = new BoPUILabel(text, this);
 }
 mLayout->removeWidget(mLabelWidget);

#warning todo
 // TODO: take alignment into account
 mLayout->addWidget(mLabelWidget);
}

void BoPUIWidget::setLabelText(const QString& text)
{
 // puObject just copies the pointer, so we get garbage if we use text.latin1()
 // directly.
 delete[] mLabel;
 mLabel = new char[text.length() + 1];
 strncpy(mLabel, text.latin1(), text.length() + 1);
}

void BoPUIWidget::setLegend(const QString& text)
{
 BO_CHECK_NULL_RET(mWidget);
 // puObject just copies the pointer, so we get garbage if we use text.latin1()
 // directly.
 delete[] mLegend;
 mLegend = new char[text.length() + 1];
 strncpy(mLegend, text.latin1(), text.length() + 1);

 if (mWidget) {
	mWidget->setLegend(mLegend);
 }
}

void BoPUIWidget::show()
{
 mLayout->show();
 mHidden = false;
}

void BoPUIWidget::hide()
{
 mLayout->hide();
 mHidden = true;
}

void BoPUIWidget::setEnabled(bool e)
{
 mLayout->setWidgetsEnabled(e);
 mIsEnabled = e;
}


BoPUILabel::BoPUILabel(QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
}

BoPUILabel::BoPUILabel(const QString& text, QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
 setText(text);
}

void BoPUILabel::init()
{
 puText* text = new puText(0, 0);

 // AB: plib ignores the label in height() when it uses the default place. so we
 // use one where it is used in width() as well as height().
 // it should not matter concerning anywhere else, as there is nothing but the
 // label in this widget.
 // AB: but keep in mind that things like _LEFT will change the appearence when
 // rendering!
 text->setLabelPlace(PUPLACE_ABOVE_RIGHT);

 mLayout->addObject(text);
 mWidget = text;
}

void BoPUILabel::setText(const QString& text)
{
 setLabelText(text);
 mWidget->setLabel(mLabel);

 mLayout->doLayout(); // in particular important if there was no text previously!
}

BoPUIPushButton::BoPUIPushButton(QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
}

BoPUIPushButton::BoPUIPushButton(const QString& text, QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
 setLegend(text);

 // AB: this should be included in setLegend(). but calling setLegend() twiice
 // would cause problems then
 int w = puGetDefaultLegendFont().getStringWidth(text.latin1()) + PUSTR_LGAP + PUSTR_RGAP;
 int h = puGetDefaultLegendFont().getStringHeight(text.latin1()) + puGetDefaultLegendFont().getStringDescender () + PUSTR_TGAP + PUSTR_BGAP;
 setSize(w, h);
}

void BoPUIPushButton::init()
{
 puOneShot* button = new puOneShot(x(), y(), x() + width(), y() + height());
 mLayout->addObject(button);
 mWidget = button;
 setPos(0, 0);
 setSize(20, 20);

 installCallback(mWidget);
}

void BoPUIPushButton::emitSignal()
{
 BoPUIWidget::emitSignal();
 emit signalClicked();
}


BoPUICheckBox::BoPUICheckBox(bool checked, QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
 setChecked(checked);
}

BoPUICheckBox::BoPUICheckBox(const QString& text, bool checked, QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
 setChecked(checked);
 setLabel(text);
}

BoPUICheckBox::BoPUICheckBox(const QString& text, QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
 setLabel(text);
}

void BoPUICheckBox::init()
{
 puButton* button = new puButton(x(), y(), x() + width(), y() + height());
 button->setButtonType(PUBUTTON_XCHECK);
 mLayout->addObject(button);
 mWidget = button;
 setPos(0, 0);
 setSize(20, 20);

 installCallback(mWidget);
}

BoPUICheckBox::~BoPUICheckBox()
{
}

void BoPUICheckBox::emitSignal()
{
 BoPUIWidget::emitSignal();
 puButton* button = (puButton*)mWidget;
 int checked = button->getIntegerValue();
 emit signalToggled(checked);
}

bool BoPUICheckBox::isChecked() const
{
 int checked = mWidget->getIntegerValue();
 return (bool)checked;
}

void BoPUICheckBox::setChecked(bool c)
{
 mWidget->setValue(c ? 1 : 0);
}

BoPUISlider::BoPUISlider(QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
}

void BoPUISlider::init()
{
 puSlider* slider = new puSlider(0, 0, 25, false);
 mLayout->addObject(slider);
 mWidget = slider;
 setPos(0, 0);
 setSize(100, 25); // ATM we always use horizontal sliders

 installCallback(mWidget);
}

void BoPUISlider::setStepSize(float s)
{
 puSlider* slider = (puSlider*)mWidget;
 slider->setStepSize(s);
}

void BoPUISlider::setRange(float min, float max)
{
 puSlider* slider = (puSlider*)mWidget;
 slider->setMinValue(min);
 slider->setMaxValue(max);
}

float BoPUISlider::minValue() const
{
 puSlider* slider = (puSlider*)mWidget;
 return slider->getMinValue();
}

float BoPUISlider::maxValue() const
{
 puSlider* slider = (puSlider*)mWidget;
 return slider->getMaxValue();
}

void BoPUISlider::setValue(float v)
{
 puSlider* slider = (puSlider*)mWidget;
 slider->setValue(v);
}

float BoPUISlider::value() const
{
 puSlider* slider = (puSlider*)mWidget;
 return slider->getFloatValue();
}

void BoPUISlider::emitSignal()
{
 puSlider* slider = (puSlider*)mWidget;
 emit signalValueChanged(slider->getFloatValue());
}

BoPUIInput::BoPUIInput(QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
}

void BoPUIInput::init()
{
 puInput* input = new puInput(0, 0, 0, 0);
 mLayout->addObject(input);
 mWidget = input;
 setPos(0, 0);
 setPos(50, 20);

 installCallback(input);
}

void BoPUIInput::emitSignal()
{
 BoPUIWidget::emitSignal();
 emit signalTextChanged(text());
}

void BoPUIInput::setText(const QString& text)
{
#warning FIXME: width
 mWidget->setValue(text.latin1());
 // TODO: setSize(QMAX(new_width, width()), height());
}

QString BoPUIInput::text() const
{
 QString text = QString::fromLatin1(mWidget->getStringValue());
 return text;
}

BoPUINumInput::BoPUINumInput(QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
}

void BoPUINumInput::init()
{
 mSlider = new BoPUISlider(this);
 mInput = new BoPUIInput(this);
 mLayout->addWidget(mSlider);
 mLayout->addWidget(mInput);

 connect(mSlider, SIGNAL(signalValueChanged(float)), this, SLOT(slotSliderChanged(float)));
 connect(mInput, SIGNAL(signalTextChanged(QString)), this, SLOT(slotInputChanged(QString)));

 setValue(10.0f);
 mInput->setSize(50, 25);
}

void BoPUINumInput::emitSignal()
{
 BoPUIWidget::emitSignal();
}

void BoPUINumInput::setStepSize(float s)
{
 mSlider->setStepSize(s);
}

void BoPUINumInput::setRange(float min, float max)
{
 mSlider->setRange(min, max);
 if (value() < min || value() > max) {
	setValue(min);
	emit signalValueChanged(value());
 }
}

float BoPUINumInput::minValue() const
{
 return mSlider->minValue();
}

float BoPUINumInput::maxValue() const
{
 return mSlider->maxValue();
}

void BoPUINumInput::setValue(float v)
{
 mSlider->setValue(v);
 mInput->setText(QString::number(v));
}

void BoPUINumInput::slotSliderChanged(float v)
{
 mInput->setText(QString::number(v));
 emit signalValueChanged(v);
}

void BoPUINumInput::slotInputChanged(QString text)
{
 bool ok;
 float v = text.toFloat(&ok);
 if (!ok) {
	mInput->setText(QString::number(mSlider->value()));
	return;
 }
 // TODO:
 // if (!isValidValue(v)) { return; }
 mSlider->setValue(v);
 emit signalValueChanged(v);
}

float BoPUINumInput::value() const
{
 return mSlider->value();
}


class BoPUITabWidgetPrivate
{
public:
	BoPUITabWidgetPrivate()
	{
		mButtonsLayout = 0;
		mTabLayout = 0;
	}
	BoPUIHLayout* mButtonsLayout;
	BoPUIVLayout* mTabLayout;

	QPtrList<BoPUIPushButton> mButtons;
	QIntDict<BoPUIWidget> mTabs; // ownership is NOT taken
	int mCurrentTab;
};


BoPUITabWidget::BoPUITabWidget(QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 init();
}

BoPUITabWidget::~BoPUITabWidget()
{
 d->mButtons.clear();
 delete d;
}

void BoPUITabWidget::init()
{
 d = new BoPUITabWidgetPrivate;
 d->mButtons.setAutoDelete(true);
 BoPUIVLayout* topLayout = new BoPUIVLayout(0);
 d->mButtonsLayout = new BoPUIHLayout(0);
 d->mTabLayout = new BoPUIVLayout(0);

 topLayout->addLayout(d->mButtonsLayout);
 topLayout->addLayout(d->mTabLayout);
 mLayout->addLayout(topLayout);
 d->mCurrentTab = -1;
}

void BoPUITabWidget::addTab(BoPUIWidget* widget, const QString& label)
{
 BO_CHECK_NULL_RET(widget);
 BoPUIPushButton* button = new BoPUIPushButton(label, this, "tabbutton");
 d->mButtons.append(button);
 d->mTabs.insert(d->mTabs.count(), widget);
 connect(button, SIGNAL(signalClicked()), this, SLOT(slotButtonClicked()));

 d->mButtonsLayout->addWidget(button);
 d->mTabLayout->addWidget(widget);

 widget->hide();
 if (!currentTab()) {
	setCurrentTab(0);
 }
}

void BoPUITabWidget::removeTab(BoPUIWidget* widget)
{
 QIntDictIterator<BoPUIWidget> it(d->mTabs);
 int page = 0;
 while (it.current() && it.current() != widget) {
	++it;
	page++;
 }

 d->mTabs.remove(page);
 d->mButtons.remove(page);
}

void BoPUITabWidget::slotButtonClicked()
{
 BO_CHECK_NULL_RET(sender());
 if (!sender()->inherits("BoPUIPushButton")) {
	boError() << k_funcinfo << "sender() is not a BoPUIPushButton" << endl;
	return;
 }
 BoPUIPushButton* senderButton = (BoPUIPushButton*)sender();
 QPtrListIterator<BoPUIPushButton> it(d->mButtons);
 int i = 0;
 while (it.current()) {
	if (it.current() == senderButton) {
		setCurrentTab(i);
		return;
	}
	++it;
	i++;
 }
}

void BoPUITabWidget::setCurrentTab(int i)
{
 BoPUIWidget* w = currentTab();
 mLayout->disableLayout(true);
 if (w) {
	w->hide();
 }
 d->mCurrentTab = i;
 w = currentTab();
 if (!w) {
	if (d->mTabs.count() > 0) {
		setCurrentTab(0);
	} else {
		mLayout->disableLayout(false);
	}
	return;
 }
 w->show();
 mLayout->disableLayout(false);
}

BoPUIWidget* BoPUITabWidget::currentTab() const
{
 return d->mTabs[d->mCurrentTab];
}

