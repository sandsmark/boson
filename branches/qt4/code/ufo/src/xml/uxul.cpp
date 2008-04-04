/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/xml/uxul.cpp
    begin             : Sat Feb 27 2005
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

#include "ufo/xml/uxul.hpp"

#include "ufo/ui/ucss.hpp"

#include "ufo/ui/ustylehints.hpp"
#include "ufo/ui/ustylemanager.hpp"

#include "ufo/widgets/uwidget.hpp"
#include "ufo/widgets/ubutton.hpp"
#include "ufo/widgets/ucombobox.hpp"
#include "ufo/widgets/ucheckbox.hpp"
#include "ufo/widgets/ugroupbox.hpp"
#include "ufo/widgets/ustackwidget.hpp"
#include "ufo/widgets/uradiobutton.hpp"
#include "ufo/widgets/ulabel.hpp"
#include "ufo/widgets/ulistbox.hpp"
#include "ufo/widgets/uscrollpane.hpp"
#include "ufo/widgets/umenu.hpp"
#include "ufo/widgets/umenuitem.hpp"
#include "ufo/widgets/ucheckboxmenuitem.hpp"
#include "ufo/widgets/umenubar.hpp"
#include "ufo/widgets/utabwidget.hpp"
#include "ufo/widgets/utextedit.hpp"
#include "ufo/widgets/ulineedit.hpp"
#include "ufo/widgets/uslider.hpp"
#include "ufo/widgets/uprogressbar.hpp"
#include "ufo/widgets/urootpane.hpp"

#include "ufo/ubuttongroup.hpp"

#include "ufo/image/uimageicon.hpp"

#include "ufo/layouts/uboxlayout.hpp"
#include "ufo/util/udimension.hpp"
#include "ufo/util/uinteger.hpp"
#include "ufo/util/ufilearchive.hpp"

// for frame creation
#include "ufo/ux/uxdisplay.hpp"
#include "ufo/ux/uxframe.hpp"
#include "ufo/ux/uxcontext.hpp"

// the xml parser
#define TIXML_USE_STL
#include "ufo/xml/tinyxml.h"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UXul, UObject)

UXul::UXul()
	: m_doc(NULL)
	, m_root(NULL)
	, m_title()
	, m_map()
	, m_actionMap()
{}

UXul::UXul(const std::string & guiFile)
	: m_doc(NULL)
	, m_root(NULL)
	, m_title()
	, m_map()
	, m_actionMap()
{
	load(guiFile);
}
UXul::UXul(const std::string & guiFile, std::map<std::string, UActionSlot> map)
	: m_doc(NULL)
	, m_root(NULL)
	, m_title()
	, m_map()
	, m_actionMap(map)
{
	load(guiFile);
}

UXul::~UXul()
{
	if (m_doc) {
		delete (m_doc);
	}
	// FIXME: delete root pane?
}

bool
isTrue(const char * str) {
	static std::string trueString("true");
	if (str) {
		return trueString == str;
	}
	return false;
}

bool
checkAttribute(TiXmlElement * element, const char * key, const char * value) {
	if (element->Attribute(key)) {
		if (std::string(value) == element->Attribute(key)) {
			return true;
		}
	}
	return false;
}

void
createLayout(TiXmlElement * element, UWidget * widget) {
	std::string value = UString(element->Value()).lowerCase();
	const char * vertical_align;
	const char * horizontal_align;

	if (value == "vbox" || checkAttribute(element, "orient", "vertical")) {
		// we have vertical layout
		widget->setLayout(new UBoxLayout(Vertical));
		vertical_align = "pack";
		horizontal_align = "align";
	} else {
		// we have horizontal layout
		widget->setLayout(new UBoxLayout(Horizontal));
		vertical_align = "align";
		horizontal_align = "pack";
	}
	widget->setHorizontalAlignment(AlignLeft);
	widget->setVerticalAlignment(AlignTop);

	if (checkAttribute(element, horizontal_align, "center")) {
		widget->setHorizontalAlignment(AlignCenter);
	} else if (checkAttribute(element, horizontal_align, "start")) {
		widget->setHorizontalAlignment(AlignLeft);
	} else if (checkAttribute(element, horizontal_align, "end")) {
		widget->setHorizontalAlignment(AlignRight);
	} else if (checkAttribute(element, horizontal_align, "stretch")) {
		widget->setHorizontalAlignment(AlignStretch);
	}
	if (checkAttribute(element, vertical_align, "center")) {
		widget->setVerticalAlignment(AlignCenter);
	} else if (checkAttribute(element, vertical_align, "start")) {
		widget->setVerticalAlignment(AlignTop);
	} else if (checkAttribute(element, vertical_align, "end")) {
		widget->setVerticalAlignment(AlignBottom);
	} else if (checkAttribute(element, vertical_align, "stretch")) {
		widget->setVerticalAlignment(AlignStretch);
	}
}

// ugh, very ugly
static std::map<std::string, UWidget*> * s_map = NULL;
static std::map<std::string, UActionSlot> * s_actionMap = NULL;

void
genericWidget(TiXmlElement * element, UWidget * widget) {
	int flex = 0;
	element->QueryIntAttribute("flex", &flex);
	if (flex) {
		// set constraints attribute
		widget->put("layout", new UInteger(flex));
	}

	if (isTrue(element->Attribute("disabled"))) {
		widget->setEnabled(false);
	}

	if (element->Attribute("class")) {
		widget->setCssClass(element->Attribute("class"));
	}
	if (element->Attribute("style")) {
		UStyleHints * hints = widget->getStyleHints()->clone();
		UStringStream stream(element->Attribute("style"));
		UCss::parseBlock(stream, hints);
		widget->setStyleHints(hints);
	}
/*
	UDimension prefSize;//widget->getPreferredSize());
	element->QueryIntAttribute("width", &(prefSize.w));
	element->QueryIntAttribute("height", &(prefSize.h));
	if (!prefSize.isEmpty()) {
		widget->setPreferredSize(prefSize);
	}
*/
	if (s_map) {
		if (element->Attribute("id")) {
			(*s_map)[element->Attribute("id")] = widget;
			widget->setName(element->Attribute("id"));
		}
	}

	//widget->setStyleHints(hints);

	if (element->Attribute("dir")) {
		if (checkAttribute(element, "dir", "rtl")) {
			widget->setDirection(RightToLeft);
		} else {
			widget->setDirection(LeftToRight);
		}
	}
	if (element->Attribute("orient")) {
		if (checkAttribute(element, "orient", "vertical")) {
			widget->setOrientation(Vertical);
		} else {
			widget->setOrientation(Horizontal);
		}
	}
}

void
genericButton(TiXmlElement * element, UButton * button) {
	genericWidget(element, button);
	if (element->Attribute("label")) {
		button->setText(element->Attribute("label"));
	}
	if (element->Attribute("accesskey")) {
		std::string accel("Alt+");
		accel.append(element->Attribute("accesskey"));
		button->setAccelerator(accel);
	}

	if (isTrue(element->Attribute("checked"))) {
		button->setSelected(true);
	}
	if (element->Attribute("image")) {
		button->setIcon(new UImageIcon(element->Attribute("image")));
	}

	if (element->Attribute("oncommand")) {
		std::string command(element->Attribute("oncommand"));

		// search in the action map
		for (std::map<std::string, UActionSlot>::iterator iter = s_actionMap->begin();
				iter != s_actionMap->end();
				++iter) {
			if ((*iter).first == command) {
				button->sigActivated().connect((*s_actionMap)[command]);
			}
		}
	}
}

UMenu *
createMenu(TiXmlElement * elem) {
	//if (elem->Value() != "menu") {
	//	return NULL;
	//}
	UMenu * menu = new UMenu();
	genericButton(elem, menu);

	TiXmlElement* popupElement = elem->FirstChildElement();
	std::string value = UString(popupElement->Value()).lowerCase();
	if ("menupopup" != value) {
		return NULL;
	}
	TiXmlElement* menuElement = popupElement->FirstChildElement();
	for(; menuElement; menuElement = menuElement->NextSiblingElement()) {
		std::string value = UString(menuElement->Value()).lowerCase();
		if ("menuitem" == value) {
			UMenuItem * item = NULL;
			if (menuElement->Attribute("type")) {
				std::string type(menuElement->Attribute("type"));
				if ("checkbox" == type) {
					item = new UCheckBoxMenuItem();
				} else if ("radio" == type) {
					// FIXME:
				}
			} else {
				item = new UMenuItem();
			}
			genericButton(menuElement, item);
			menu->add(item);
		} else if ("menuseparator" == value) {
			menu->addSeparator();
		} else if ("menu" == value) {
			menu->add(createMenu(menuElement));
		}
	}
	return menu;
}

void
createMenuBar(TiXmlElement * elem, URootPane * root) {
	//if ("menubar" != UString(elem->Value()).lowerCase()) {
	//	return;
	//}
	UMenuBar * mbar = new UMenuBar();
	genericWidget(elem, mbar);
	TiXmlElement* menuElement = elem->FirstChildElement();
	for(; menuElement; menuElement = menuElement->NextSiblingElement()) {
		std::string value = UString(menuElement->Value()).lowerCase();
		if ("menu" == value) {
			mbar->add(createMenu(menuElement));
		}
	}
	root->setMenuBar(mbar);
}


void
createRadioGroup(TiXmlElement * elem, UWidget * container) {
	//if ("radiogroup" != UString(elem->Value()).lowerCase()) {
	//	return;
	//}
	UButtonGroup * buttonGroup = new UButtonGroup();
	TiXmlElement* radioElement = elem->FirstChildElement();
	for(; radioElement; radioElement = radioElement->NextSiblingElement()) {
		std::string value = UString(radioElement->Value()).lowerCase();
		if ("radio" == value) {
			URadioButton * radioButton = new URadioButton();
			genericButton(radioElement, radioButton);
			radioButton->setButtonGroup(buttonGroup);
			container->add(radioButton);
		}
	}
}

void
createListBox(TiXmlElement * elem, UListBox * listBox) {
	//if ("radiogroup" != UString(elem->Value()).lowerCase()) {
	//	return;
	//}
	TiXmlElement* listElement = elem->FirstChildElement();
	for(; listElement; listElement = listElement->NextSiblingElement()) {
		std::string value = UString(listElement->Value()).lowerCase();
		if ("listitem" == value && listElement->Attribute("label")) {
			listBox->addItem(listElement->Attribute("label"));
		}
	}
}

UComboBox *
createComboBox(TiXmlElement * elem) {
	UComboBox * box = new UComboBox();
	genericWidget(elem, box);
	if (isTrue(elem->Attribute("editable"))) {
		box->getTextEdit()->setEditable(true);
	} else {
		box->getTextEdit()->setEditable(false);
	}

	TiXmlElement* popupElement = elem->FirstChildElement();
	std::string value = UString(popupElement->Value()).lowerCase();
	if ("menupopup" != value) {
		return box;
	}
	TiXmlElement* menuElement = popupElement->FirstChildElement();
	for(; menuElement; menuElement = menuElement->NextSiblingElement()) {
		std::string value = UString(menuElement->Value()).lowerCase();
		if ("menuitem" == value && menuElement->Attribute("label")) {
			box->addItem(menuElement->Attribute("label"));
		}
	}
	box->setCurrentItem(0);
	return box;
}

void
createChildren(TiXmlElement * elem, UWidget * container);

UTabWidget *
createTabBox(TiXmlElement * elem) {
	std::vector<std::string> m_tabs;
	std::vector<UWidget*> m_panels;

	if (TiXmlElement * tabs = elem->FirstChildElement("tabs")) {
		TiXmlElement* tabElement = tabs->FirstChildElement();
		for(; tabElement; tabElement = tabElement->NextSiblingElement()) {
			std::string value = UString(tabElement->Value()).lowerCase();
			if ("tab" == value && tabElement->Attribute("label")) {
				m_tabs.push_back(tabElement->Attribute("label"));
			}
		}
	}
	if (TiXmlElement * panels = elem->FirstChildElement("tabpanels")) {
		TiXmlElement* panelElement = panels->FirstChildElement();
		for(; panelElement; panelElement = panelElement->NextSiblingElement()) {
			std::string value = UString(panelElement->Value()).lowerCase();
			if ("tabpanel" == value) {
				UWidget * tabPanel = new UWidget();
				genericWidget(panelElement, tabPanel);
				createLayout(panelElement, tabPanel);
				createChildren(panelElement, tabPanel);
				m_panels.push_back(tabPanel);
			}
		}
	}
	UTabWidget * tabBox = new UTabWidget();
	unsigned int min_size = std::min(m_tabs.size(), m_panels.size());
	for (unsigned int i = 0; i < min_size; ++i) {
		tabBox->addTab(m_panels[i], m_tabs[i]);
	}
	return tabBox;
}


void
createChildren(TiXmlElement * elem, UWidget * container) {
	TiXmlElement* widgetElement = elem->FirstChildElement();
	UWidget * widget = NULL;
	// check for widgets
	for(; widgetElement; widgetElement = widgetElement->NextSiblingElement()) {
		std::string value = UString(widgetElement->Value()).lowerCase();
		widget = NULL;
		if ("spacer" == value) {
			widget = new UWidget();
			genericWidget(widgetElement, widget);
			widget->put("layout", new UInteger(1));
			container->add(widget);
		} else if ("vbox" == value || "hbox" == value) {
			widget = new UWidget();
			genericWidget(widgetElement, widget);
			createLayout(widgetElement, widget);
			container->add(widget);
			// recursively create children
			createChildren(widgetElement, widget);
		} else if ("groupbox" == value) {
			UGroupBox * box = new UGroupBox();
			genericWidget(widgetElement, box);
			if (TiXmlElement * caption = widgetElement->FirstChildElement("caption")) {
				if (caption->Attribute("label")) {
					box->setTitle(caption->Attribute("label"));
				}
			}
			container->add(box);
			// recursively create children
			createChildren(widgetElement, box);
		} else if ("deck" == value) {
			UStackWidget * deck = new UStackWidget(1);
			genericWidget(widgetElement, deck);
			container->add(deck);
			// recursively create children
			createChildren(widgetElement, deck);
		} else if ("stack" == value) {
			UStackWidget * stack = new UStackWidget(0);
			genericWidget(widgetElement, stack);
			container->add(stack);
			// recursively create children
			createChildren(widgetElement, stack);
		} else if ("button" == value) {
			UButton * button = new UButton();
			genericButton(widgetElement, button);
			container->add(button);
		} else if ("checkbox" == value) {
			UCheckBox * checkBox = new UCheckBox();
			genericButton(widgetElement, checkBox);
			container->add(checkBox);
		} else if ("radio" == value) {
			URadioButton * radioButton = new URadioButton();
			genericButton(widgetElement, radioButton);
			container->add(radioButton);
		} else if ("label" == value) {
			ULabel * label = new ULabel();
			genericWidget(widgetElement, label);
			char accesskey = 0;
			if (widgetElement->Attribute("accesskey")) {
				accesskey = (widgetElement->Attribute("accesskey"))[0];
			}
			if (widgetElement->Attribute("value")) {
				std::string text = widgetElement->Attribute("value");
				std::string::size_type index = text.find(accesskey);
				if (index < text.length()) {
					text.insert(index, 1, '&');
				}
				label->setText(text);
			}
			if (widgetElement->Attribute("control")) {
				std::string ctrl = widgetElement->Attribute("control");
				if ((*s_map)[ctrl])
					label->setBuddy((*s_map)[ctrl]);
			}
			container->add(label);
		} else if ("description" == value) {
			if (widgetElement->FirstChild()) {
				TiXmlText * text = widgetElement->FirstChild()->ToText();
				if (text && text->Value()) {
					UTextWidget * tw = new UTextWidget(text->Value());
					genericWidget(widgetElement, tw);
					container->add(tw);
				}
			}
		} else if ("textbox" == value) {
			UTextEdit * textEdit;
			if (isTrue(widgetElement->Attribute("multiline"))) {
				textEdit = new UTextEdit();
			} else {
				textEdit = new ULineEdit();
			}
			genericWidget(widgetElement, textEdit);
			if (widgetElement->Attribute("value")) {
				textEdit->setText(widgetElement->Attribute("value"));
			}
			if (widgetElement->Attribute("cols")) {
				int cols = 0;
				widgetElement->QueryIntAttribute("cols", &cols);
				textEdit->setColumns(cols);
			}
			if (widgetElement->Attribute("rows")) {
				int rows = 0;
				widgetElement->QueryIntAttribute("rows", &rows);
				textEdit->setRows(rows);
			}
			if (widgetElement->Attribute("size")) {
				int length = 0;
				widgetElement->QueryIntAttribute("size", &length);
				textEdit->setMaxLength(length);
			}
			container->add(textEdit);
		} else if ("slider" == value) {
			widget = new USlider();
			genericWidget(widgetElement, widget);
			container->add(widget);
		} else if ("progressmeter" == value) {
			UProgressBar * bar = new UProgressBar();
			genericWidget(widgetElement, bar);
			if (widgetElement->Attribute("value")) {
				int value = 0;
				widgetElement->QueryIntAttribute("value", &value);
				bar->setValue(value);
			}
			container->add(bar);
		} else if ("toolbox" == value) {
			TiXmlElement* menubarElement = widgetElement->FirstChildElement();
			std::string value = UString(menubarElement->Value()).lowerCase();
			if ("menubar" == value) {
				createMenuBar(menubarElement, container->getRootPane());
			}
		} else if ("menubar" == value) {
			createMenuBar(widgetElement, container->getRootPane());
		} else if ("listbox" == value) {
			UListBox * listBox = new UListBox();
			genericWidget(widgetElement, listBox);
			createListBox(widgetElement, listBox);
			if (widgetElement->Attribute("rows")) {
				int rows = 0;
				widgetElement->QueryIntAttribute("rows", &rows);
				listBox->setVisibleRowCount(rows);
				container->add(new UScrollPane(listBox));
			} else {
				container->add(listBox);
			}
		} else if ("menulist" == value) {
			UComboBox * box = createComboBox(widgetElement);
			genericWidget(widgetElement, box);
			container->add(box);
		} else if ("radiogroup" == value) {
			createRadioGroup(widgetElement, container);
		} else if ("tabbox" == value) {
			UTabWidget * tabBox = createTabBox(widgetElement);
			genericWidget(widgetElement, tabBox);
			container->add(tabBox);
		}

		// generic widget attributes
		//if (widget) {
		//	genericWidget(widgetElement, widget);
		//}
	}
}

void
UXul::load(const std::string & guiFile) {
	m_doc = new TiXmlDocument(guiFile.c_str());
	bool loadOkay = m_doc->LoadFile();
	if (!loadOkay) {
		uError() << "Couldn't load file: " << guiFile << "\n"
		<< m_doc->ErrorDesc() << "\n";
		delete (m_doc);
		m_doc = NULL;
	}
}

void
UXul::setActionMap(std::map<std::string, UActionSlot> map) {
	m_actionMap = map;
}

URootPane *
UXul::createRootPane() {
	if (!m_doc) {
		uError() << "no XML document loaded.\n";
		return NULL;
	}
	if (!m_root) {
		m_root = new URootPane();
		UWidget * content = m_root->getContentPane();

		TiXmlNode* node = 0;
		TiXmlElement* windowElement = 0;

		node = m_doc->FirstChild();

		// load style sheet
		TiXmlUnknown * unknown = NULL;
		do {
			node = m_doc->IterateChildren(node);
			unknown = node->ToUnknown();
		} while (!unknown && node);
		if (unknown) {
			// we have a style sheet
			std::string tag = unknown->Value();
			std::string href;

			unsigned int index = 0;
			while (href == "" && index < tag.size()) {
				index = tag.find('=', index);
				if (index < tag.size() && tag.substr(index - 4, 4) == "href") {
					unsigned int end = tag.find('"', index + 2);
					href = tag.substr(index + 2, end - index - 2);
				}
			}
			std::string css_path =
				UFileArchive::getDefault()->getAbsolutePath(href);
			if (css_path.length()) {
				m_root->getStyleManager()->loadStyleSheet(css_path);
			}
		}

		// Get the "window" element.
		// It is a child of the document, and can be selected by name.
		node = m_doc->FirstChild("window");
		//assert(node);
		windowElement = node->ToElement();
		//assert(windowElement);

		if (windowElement->Attribute("title")) {
			m_title = windowElement->Attribute("title");
		} else {
			m_title = "";
		}
		UDimension prefWindowSize;
		windowElement->QueryIntAttribute("width", &(prefWindowSize.w));
		windowElement->QueryIntAttribute("height", &(prefWindowSize.h));
		if (!prefWindowSize.isEmpty()) {
			m_root->setPreferredSize(prefWindowSize);
		}

		// oops, this is tricky
		s_map = &m_map;
		s_actionMap = &m_actionMap;
		genericWidget(windowElement, m_root);
		genericWidget(windowElement, content);

		createLayout(windowElement, content);
		createChildren(windowElement, content);
	}
	return m_root;
}

UXFrame *
UXul::createFrame() {
	if (!m_doc) {
		uError() << "no XML document loaded.\n";
		return NULL;
	}
	UXFrame * frame = static_cast<UXDisplay*>(UXDisplay::getDefault())->createFrame();

	createRootPane();
	// some base values
	if (m_title != "") {
		frame->setTitle(m_title);
	} else {
		frame->setTitle("XUL frame");
	}
	//frame->setBounds(0, 0, 1, 1);
	UDimension dim = m_root->getPreferredSize();
	if (dim.isEmpty()) {
		dim = UDimension(200, 200);
	}
	frame->setBounds(0, 0, dim.w, dim.h);
	//frame->setVisible(true);

	frame->getContext()->setRootPane(m_root);
	// pack the frame, FIXME: done above
	//frame->pack();
	return frame;
}

UWidget *
UXul::get(const std::string & key) {
	return m_map[key];
}
