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
	void slotCreateUnit();
	void slotTypeChanged(int);

protected:
	void loadConfig(const QString& file);

private:
	void initDirectories();
	void initProperties();

private:
	BosonUnitDialogPrivate* d;
};

#endif
