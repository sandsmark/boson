/***************************************************************************
                         newDlg.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Apr 20 18:29:17 CEST 2000
                                           
    copyright            : (C) 2000 by Thomas Capricelli                         
    email                : orzel@yalbi.com                                    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

//#include <stdlib.h>	// atoi

#include <qpushbutton.h>
#include <qlabel.h>
#include <qscrollbar.h>
//#include <qslider.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qlineedit.h> 

#include "newDlg.h"

//#include "common/log.h"

newDlg::newDlg(QWidget *parent, const char *name)
	:QDialog(parent,name,true)
{

        QPushButton	*button;
	QLabel		*label;


	type = 0; // all_water
	qc_grass  = qRgb ( 0x0808, 0x7373, 0x2121);
	qc_desert = qRgb ( 0xdede, 0xbdbd, 0x7373); 
	qc_water  = qRgb ( 0,0,200);

	/* layout */
	resize( 580, 330 );
	setCaption( "Create a scenario" );     
	
	/* Name */

	qle_name = new QLineEdit(this, "Scenario Name");
	qle_name->setGeometry(360, 40, 200, 20);
	qle_name->setText("Scenario Name");

	qle_author = new QLineEdit(this, "Author Name");
	qle_author->setGeometry(360, 90, 200, 20);
	qle_author->setText("Author Name");

	/* qsliders */

	label = new QLabel("Width :", this);
	label->setGeometry( 360,  130, 60, 30);
	label->setAlignment(AlignVCenter | AlignLeft);

	_width = new QLabel("150", this);
	_width->setGeometry( 420,  130, 60, 30);
	_width->setAlignment(AlignVCenter | AlignLeft);

	scb_width  = new QScrollBar( 50, 300, 1, 20, 150, QScrollBar::Horizontal, this, "width slider");
	scb_width->setGeometry( 360, 160, 190, 15);

	connect( scb_width, SIGNAL(valueChanged(int)), _width, SLOT(setNum(int)));
	connect( scb_width, SIGNAL(sliderReleased(void)), this, SLOT(redrawPreview(void)));


	label = new QLabel("Height :", this);
	label->setGeometry( 360, 180, 60, 30);
	label->setAlignment(AlignVCenter | AlignLeft);

	_height = new QLabel("150", this);
	_height->setGeometry( 420,  180, 60, 30);
	_height->setAlignment(AlignVCenter | AlignLeft);

	scb_height = new QScrollBar( 50, 300, 1, 20, 150, QScrollBar::Horizontal, this, "height scrollbar");
	scb_height->setGeometry( 360, 210, 190, 15);

	connect( scb_height, SIGNAL(valueChanged(int)), _height, SLOT(setNum(int)));
	connect( scb_height, SIGNAL(sliderReleased(void)), this, SLOT(redrawPreview(void)));


	/* preview window */
	_pixLabel = new QLabel(this, "preview");
	_pixLabel->setGeometry(10, 10, 310, 310);
	_pixLabel->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	_pixLabel->setLineWidth(4);
	_pixLabel->setAlignment(AlignVCenter | AlignHCenter);
	pix = new QPixmap();
	redrawPreview();


	/* QComboBox pour la liste des trucs disponible */

	QComboBox *qcb = new QComboBox(this, "qcb_type");
	qcb->setGeometry( 400, 240, 110, 30);

	qcb->insertItem("All water", 0);
	qcb->insertItem("All grass", 1);
	qcb->insertItem("All desert", 2);

	connect( qcb, SIGNAL(activated(int)), this, SLOT(qcb_activated(int)) );

	/* buttons */
	button = new QPushButton( "Create", this );
	button->setGeometry( 360, 280, 80, 30 );
	connect( button, SIGNAL(clicked()), SLOT(accept()) );

	button = new QPushButton( "Cancel", this );
	button->setGeometry( 460, 280, 80, 30 );
	connect( button, SIGNAL(clicked()), SLOT(reject()) );

}


void newDlg::qcb_activated(int which)
{
	type = which;
	redrawPreview();
}

void newDlg::redrawPreview(void)
{
	pix->resize(scb_width->value(), scb_height->value());
	switch(type) {
		case 0: // full water
			pix->fill(qc_water);
			break;
		case 1: // full grass
			pix->fill(qc_grass);
			break;
		case 2: // full desert
			pix->fill(qc_desert);
			break;
	}
	_pixLabel->setPixmap(*pix);
}


