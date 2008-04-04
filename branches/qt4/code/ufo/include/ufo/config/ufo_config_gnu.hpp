#ifndef UFO_CONFIG_GNU_HPP
#define UFO_CONFIG_GNU_HPP
#include <config.h>

#ifdef UFO_USE_X11
#undef UFO_USE_X11
#endif
#ifdef UFO_USE_GLX
#undef UFO_USE_GLX
#endif
#ifdef UFO_USE_SDL
#undef UFO_USE_SDL
#endif
#ifdef UFO_USE_WGL
#undef UFO_USE_WGL
#endif
#define UFO_BUILDING_DLL

// we use an empty DATADIR string here. note that you MUST set the data_dir property in the UToolkit before you can use it!
#define UFO_DATADIR ""

#endif

