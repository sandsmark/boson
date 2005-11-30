/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : ucss.hpp
    begin             : Wed Mar 02 2005
    $ $
 ***************************************************************************/

#ifndef UCSS_HPP
#define UCSS_HPP

#include "ufo/uobject.hpp"

#include <iostream>
#include <map>

namespace ufo {

class UStyleHints;

/** @short This class can load cascading style sheets and convert them to
  *  style hints.
  * @ingroup appearance
  *
  * @see UStyleHints
  * @author Johannes Schmidt
  */
class UFO_EXPORT UCss : public UObject {
public:
	UCss();
	UCss(const std::string & filename);
	/** Deletes all style hints created by this object. */
	virtual ~UCss();

	void load(const std::string & filename);
	void load(std::istream & stream);

	UStyleHints * getStyleHints(const std::string & key);
	std::map<std::string, UStyleHints*> getStyleHints();

public:
	static UStyleHints * parseBlock(std::istream & stream, UStyleHints * hints = NULL);

private:
	std::map<std::string, UStyleHints*> m_hints;
};

} // namespace ufo

#endif // UCSS_HPP
