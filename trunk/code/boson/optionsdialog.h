#ifndef __OPTIONSDIALOG_H__
#define __OPTIONSDIALOG_H__

#include <kdialogbase.h>

class OptionsDialogPrivate;
class OptionsDialog : public KDialogBase
{
	Q_OBJECT
public:
	OptionsDialog(QWidget* parent, bool modal = false);
	~OptionsDialog();

	/**
	 * Set the shown value for the game speed. Note that this value is the
	 * time between 2 @ref QCanvas::advance calls in ms while the dialog
	 * does not show anything in ms. The dialog values are just the
	 * opposite: higher values mean higher speed.
	 **/
	void setGameSpeed(int ms);

	void setArrowScrollSpeed(int);

protected slots:
	/**
	 * @param ms The new game speed in ms
	 **/
	void slotSpeedChanged(int ms);

signals:
	void signalArrowScrollChanged(int);
	void signalSpeedChanged(int);

private:
	OptionsDialogPrivate* d;

};

#endif
