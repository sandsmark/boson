#ifndef __EDITORINPUT_H__
#define __EDITORINPUT_H__

#include <qobject.h>

/**
 * Essentially like @ref KGameIO but doesn't send anything
 **/
class EditorInput : public QObject
{
	Q_OBJECT
public:
	EditorInput(QObject* parent);
	~EditorInput();

	virtual bool eventFilter(QObject* o, QEvent* e);
	
signals:
	void signalMouseEvent(QMouseEvent* e, bool* eatevent);
};

#endif
