#ifndef __BOSONWIDGET_H__
#define __BOSONWIDGET_H__

#include <qwidget.h>
#include <qdatastream.h>

class KPlayer;
class KGameIO;
class QKeyEvent;

class BosonCanvas;
class VisualUnit;
class Player;

class BosonWidgetPrivate;

class BosonWidget : public QWidget 
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	BosonWidget(QWidget* parent, bool editor = false);

	/**
	 * Default Destructor
	 **/
	virtual ~BosonWidget();

	void startEditor();

	void saveConfig();

	void zoom(const QWMatrix&);

public slots:
	void slotDebug();
	void slotNewGame();
	void slotGamePreferences();
	void slotEndGame();

	void slotLoadMap(const QString& map);
	void slotLoadScenario(const QString& scenario);
	void slotChangeLocalPlayer(int playerNumber);

	/**
	 * Called by @ref EditorTop, the map editor, when the construction frame
	 * shall be changed (mobile -> facilities or the other way round)
	 **/
	void slotEditorConstructionChanged(int index);

	void slotEditorSaveMap(const QString& fileName);
	void slotEditorSaveScenario(const QString& fileName);

signals:
	void signalPlayerJoinedGame(KPlayer* p); // used by the map editor
	void signalPlayerLeftGame(KPlayer* p); // used by the map editor

protected slots:
	void slotStartScenario();
	void slotChangeSpecies(const QString& speciesDirectory);

protected:
	void addLocalPlayer();

	void addDummyComputerPlayer(const QString& name); // used by editor only

	void changeLocalPlayer(Player* p);
	virtual void keyReleaseEvent(QKeyEvent* e);

	void quitGame();

	/**
	 * Delete an existing @ref BosonMap object and create a new one. You
	 * will have to call @ref BosonMap::loadMap before using it!
	 **/
	void recreateMap();

	void addEditorCommandFrame();
	void addGameCommandFrame();

protected slots:
	void slotPlayerJoinedGame(KPlayer* p);
	void slotArrowScrollChanged(int speed);
	void slotAddUnit(VisualUnit* unit, int x, int y);
	void slotStartGame();

	void slotReceiveMap(const QByteArray& map);

	void slotAddComputerPlayer(Player*);

	void slotAddCell(int,int,int,unsigned char);

private:
	void init();

private:
	BosonWidgetPrivate* d;
};

#endif // __BOSONWIDGET_H__
