#ifndef __BOSONBIGDISPLAY_H__
#define __BOSONBIGDISPLAY_H__

#include <qcanvas.h>
#include <qptrlist.h>

class KGameIO;

class VisualUnit;
class Player;

class BosonBigDisplayPrivate;
class BosonBigDisplay : public QCanvasView
{
	Q_OBJECT
public:
	BosonBigDisplay(QCanvas* c, QWidget* parent);
	BosonBigDisplay(QWidget* parent);
	~BosonBigDisplay();

	enum SelectionMode {
		SelectNone = 0,
		SelectSingle = 1,
		SelectRect = 2
	};

	void setLocalPlayer(Player* p);

public slots:
	/**
	 * @param pos The new position - cell coordinates! so you have to
	 * multiply the coordinates with BO_TILE_SIZE
	 **/
	void slotReCenterView(const QPoint& pos);

	/**
	 * Mark the unitType for construction. Inform boson about this and place
	 * the unit on the screen if in editor mode - otherwise delay.
	 **/
	void slotWillConstructUnit(int unitType, VisualUnit* facility, Player* owner);

	void slotUnitChanged(VisualUnit* unit);

signals:
	/**
	 * Emitted when a single unit (@ref selectionMode == Select_Single) is
	 * selected. The image of the unit should be displayed now.
	 *
	 * If this is a factory the order buttons also should be changed.
	 **/
	void signalSingleUnitSelected(VisualUnit* unit);

	/**
	 * Emitted by @ref resizeEvent
	 **/
	void signalSizeChanged(int w, int h);

	void signalEditorAddUnit(int type, int x, int y, int owner);
//	void signalEditorConstruction(int unitType, Player* owner);

	void signalConstructUnit(int type, VisualUnit* facility, Player* owner);
	void signalConstructUnit(int type, int x, int y, Player* owner); // editor mode only

protected:
	/**
	 * Sets the selection mode after clearing any previous selection.
	 **/
	void setSelectionMode(SelectionMode mode);

	/**
	 * Clears the selection list and sets @ref selectionMode = Select_None
	 **/
	void clearSelection();

	/**
	 * Selects units in the specified rect. See also @ref addUnitSelection
	 **/
	void selectArea(const QRect& rect);

	/**
	 * @return The start point of the selection rect
	 **/
	const QPoint& selectionStart() const;

	/**
	 * @return The end point of the selection rect
	 **/
	const QPoint& selectionEnd() const;

	/**
	 * @return The currently selected units. See also @ref addUnitSelection
	 **/
	QPtrList<VisualUnit>& selection() const;

	/**
	 * @return The selection mode. See @ref setSelectionMode
	 **/
	SelectionMode selectionMode() const;

	/**
	 * Add a unit to the selection list
	 **/
	void addUnitSelection(VisualUnit* unit);

	/**
	 * Draw the selection rect from @ref selectionStart to @ref selectionEnd
	 **/
	void drawSelectionRect();

	/**
	 * Called when the user right-clicks on the big display.
	 * 
	 * Should e.g. move a unit
	 * @param point The point where the click was done
	 * @param stream The move should be placed here. A move should
	 * <em>not</em> be done in this method but rather sent to @ref KGame
	 * which performs the move on every client
	 * @param send Set to true if you actually want to send the stream
	 **/
	void actionClicked(const QPoint& point, QDataStream& stream, bool& send);

	void editorActionClicked(const QPoint& point);

	virtual void resizeEvent(QResizeEvent*);

	/**
	 * Start the selection at pos. Either draw the selection rect here or
	 * select a single unit (if at pos is a unit)
	 * @param pos The start point of the selection rect or the position of a
	 * unit
	 **/
	void startSelection(const QPoint& pos);

	/**
	 * Move the selection rect. @ref selectionStart is still the start point
	 * but @ref selectionEnd is now newEnd
	 *
	 * Do nothing if @ref selectionMode is not Select_Rect
	 **/
	void moveSelectionRect(const QPoint& newEnd);

	/**
	 * Remove a currently drawn selection rect. Do nothing if no selection
	 * rect is currently drawn.
	 **/
	void removeSelectionRect();
	
protected slots:
	void slotMouseEvent(KGameIO*, QDataStream& stream, QMouseEvent* e, bool *eatevent);

	void slotEditorMouseEvent(QMouseEvent* e, bool* eatevent);

private:
	void init();

private:
	BosonBigDisplayPrivate* d;
};

#endif
