#ifndef __RTTI_H__
#define __RTTI_H__

/**
 * This class consists of a single enum @ref Rtti which defines the rtti values
 * used in Boson. Note that @ref VisualUnit has many different rtti values as
 * there are different units. I want to implement code-independent units (i.e.
 * they get defined in index.desktop files on runtime) and therefore one
 * doesn't know the actual number of rttis used by @ref VisualUnit. So don't
 * add any rttis here after @ref UnitStart!
 *
 * Not that you have to recompile nearly everything if you change this file!
 * @short Boson rttis
 **/
class RTTI
{
public:
	RTTI() { }
	~RTTI() { }

	enum Rtti {
		// QCanvasItem defines RTTIs from 0 to 8. We don't use them
		// currently but we better start at a higher value.
		SelectPart = 15,
		BoShot = 16,


		UnitStart = 200 // the IDs of the units start at this value. 
		                // Do not insert RTTIs after this!
	};

	static bool isUnit(int rtti)
	{
		return (rtti >= UnitStart);
	}
};

#endif
