#include "bosoncomputerio.h"

#include "player.h"

#include <kdebug.h>

#include "bosoncomputerio.moc"

BosonComputerIO::BosonComputerIO() : KGameComputerIO()
{
}

BosonComputerIO::BosonComputerIO(KPlayer* p) : KGameComputerIO(p)
{
}

BosonComputerIO::~BosonComputerIO()
{
}

void BosonComputerIO::reaction()
{
 kdDebug() << "BosonComputerIO::reaction()" << endl;

}

