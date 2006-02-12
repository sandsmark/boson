#include <ufo/config-ufo.h>
#include <bosound/config-bosound.h>
#include <boson/info/config-info.h>
#include <bobmfconverter/libgfx/config-libgfx.h>

#cmakedefine HAVE_SYS_TIME_H 1

#define BOSON_LINK_STATIC ${BOSON_LINK_STATIC}
#define USE_BO_PLUGINS ${USE_BO_PLUGINS}
#define BOSON_PREFIX "${CMAKE_INSTALL_PREFIX}"
