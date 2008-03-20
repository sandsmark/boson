#include <ufo/config-ufo.h>
#include <boson/sound/bosound/config-bosound.h>
#include <boson/info/config-info.h>
#include <bobmfconverter/libgfx/config-libgfx.h>
#include <bomemory/config-bomemory.h>
#include <boson/config-boson.h>
#include <bodebug/config-bodebug.h>

#cmakedefine HAVE_SYS_TIME_H 1

#define BOSON_LINK_STATIC ${BOSON_LINK_STATIC}
#define BOSON_USE_DEBUG_PLUGINS ${BOSON_USE_DEBUG_PLUGINS}
#define BOSON_PREFIX "${CMAKE_INSTALL_PREFIX}"

/* AB: this define is obsolete. use BOSON_USE_DEBUG_PLUGINS instead */
#define USE_BO_PLUGINS ${BOSON_USE_DEBUG_PLUGINS}

/* Whether (if 1) to use dlopen() to open libGL and libGLU or (if 0) to link
   directly to these libraries */
#define BOGL_DO_DLOPEN ${BOGL_DO_DLOPEN}

