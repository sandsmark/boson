#ifndef __DEFINES_H__
#define __DEFINES_H__

#define BOSON_COOKIE 992

#define BO_TILE_SIZE 48

#define PIXMAP_PER_MOBILE 9 // 8 different directions + 1 "destroyed" pix
#define PIXMAP_PER_FIX 5
#define PIXMAP_FIX_DESTROYED (PIXMAP_PER_FIX - 1)
#define PIXMAP_MOBILE_DESTROYED (PIXMAP_PER_MOBILE - 1)
#define FACILITY_CONSTRUCTION_STEPS PIXMAP_FIX_DESTROYED

#define SHOT_FRAMES 5
#define MOBILE_SHOT_FRAMES 16
#define FACILITY_SHOT_FRAMES 16
#define MOBILE_SHOT_NB 4 // FIXME hardcoded
#define FACILITY_SHOT_NB 4// FIXME hardcoded

#define BOSON_MAX_PLAYERS 10 // test if this is working - 2 is tested

#define MAX_MAP_HEIGHT 500
#define MAX_MAP_WIDTH 500

#define ARROW_KEY_STEP 10 

#define MAX_GAME_SPEED 10 // the advance period - lower means faster
#define MIN_GAME_SPEED 800 // the advance period - higher means slower
#define DEFAULT_GAME_SPEED 50


#define Z_MOBILE 400
#define Z_FACILITY 300
#define Z_DESTROYED_MOBILE 200
#define Z_DESTROYED_FACILITY 100


#define BOSON_PORT 5454

#endif
