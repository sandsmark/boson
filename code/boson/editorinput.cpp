#include "editorinput.h"

#include <qevent.h>

#include "editorinput.moc"

EditorInput::EditorInput(QObject* parent) : QObject(parent)
{
 if (parent) {
	parent->installEventFilter(this);
//	parent->setMouseTracking();
 }
}

EditorInput::~EditorInput()
{

}

bool EditorInput::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::Wheel:
	case QEvent::MouseMove:
	{
		bool eatevent = false;
		emit signalMouseEvent((QMouseEvent*)e, &eatevent);
		return eatevent;
		break;
	}
	default:
		break;
 }
 return QObject::eventFilter(o, e);
}

