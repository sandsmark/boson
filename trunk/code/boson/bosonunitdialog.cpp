#include "bosonunitdialog.h"

#include <klocale.h>
#include <kfiledialog.h>

#include <qvbox.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "bosonunitdialog.moc"

class BosonUnitDialogPrivate
{
public:
	BosonUnitDialogPrivate()
	{
		mUnitDir = 0;
	}
	
	QPushButton* mUnitDir;
};

BosonUnitDialog::BosonUnitDialog(QWidget* parent) 
		: KDialogBase(Tabbed, i18n("Create Unit"), Close, Close, parent,
		0, true, true)
{
 d = new BosonUnitDialogPrivate;
 
 initDirectories();
 initProperties();
}


BosonUnitDialog::~BosonUnitDialog()
{
 delete d;
}

void BosonUnitDialog::initDirectories()
{
 QVBox* dirs = addVBoxPage(i18n("&Directories and Files"));
 d->mUnitDir = new QPushButton(dirs);
 connect(d->mUnitDir, SIGNAL(pressed()), this, SLOT(slotChangeUnitDir()));
}

void BosonUnitDialog::initProperties()
{
 QVBox* props = addVBoxPage(i18n("&Properties"));
 
}

void BosonUnitDialog::slotChangeUnitDir()
{
 QString dir = KFileDialog::getExistingDirectory();
 if (dir == QString::null) {
	return;
 }
 d->mUnitDir->setText(dir);
}

