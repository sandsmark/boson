#ifndef __BOSONUNITDIALOG_H__
#define __BOSONUNITDIALOG_H__

#include <kdialogbase.h>

class BosonUnitDialogPrivate;
class BosonUnitDialog : public KDialogBase
{
	Q_OBJECT
public:
	BosonUnitDialog(QWidget* parent = 0);
	~BosonUnitDialog();

protected slots:
	void slotChangeUnitDir();

private:
	void initDirectories();
	void initProperties();

private:
	BosonUnitDialogPrivate* d;
};

#endif
