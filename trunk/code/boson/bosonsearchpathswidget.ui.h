/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <kfiledialog.h>
#include <klocale.h>

#include <qdir.h>

void BosonSearchPathsWidget::slotAppendPath(QString path)
{
    mCurrentPaths->insertItem(path);
}

void BosonSearchPathsWidget::slotSetPaths( QStringList paths )
{
    mCurrentPaths->clear();
    // Maybe use slotAppendPath() which checks for errors?
    mCurrentPaths->insertStringList(paths);
}

void BosonSearchPathsWidget::slotRemovePath()
{
    mCurrentPaths->removeItem(mCurrentPaths->currentItem());
}

void BosonSearchPathsWidget::slotAddPath()
{
    if(mNewPath->text().isEmpty()) {
	return;
    }
    QDir d(mNewPath->text());
    if(!d.exists()) {
	// Directory doesn't exist. Display error?
	return;
    }
    mCurrentPaths->insertItem(d.absPath(), 0);
}

void BosonSearchPathsWidget::slotBrowse()
{
   mNewPath->setText(KFileDialog::getExistingDirectory());
}

QStringList BosonSearchPathsWidget::currentPaths()
{
    QStringList paths;
    for(unsigned int i = 0; i < mCurrentPaths->count(); i++) {
	paths.append(mCurrentPaths->item(i)->text());
    }
    return paths;
}
