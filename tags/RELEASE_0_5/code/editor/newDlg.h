/***************************************************************************
                         newDlg.h  -  description                              
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

#ifndef NEW_DLG_H 
#define NEW_DLG_H 

#include <qdialog.h>

class QPushButton;
class QLabel;
//class QSlider;
class QPixmap;
class QLineEdit;
class QScrollBar;



class newDlg : public QDialog 
{
	Q_OBJECT

public:
	newDlg(QWidget *parent=0l, const char *name=0l);

	QScrollBar	*scb_width;
	QScrollBar	*scb_height;
	QLineEdit	*qle_name, *qle_author;
	int		type;

private slots:
	void	qcb_activated(int);
	void	redrawPreview(int not_used = 0);
private:
	QPixmap		*pix;
	QLabel		*_height, *_width, *_pixLabel;
	QColor		qc_grass, qc_desert, qc_water;
};


#endif // NEW_DLG_H 

