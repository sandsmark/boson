#ifndef FAKEDSG_H
#define FSKEDSG_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SGfloat float

typedef SGfloat sgVec2[ 2 ];
typedef SGfloat sgVec3[ 3 ];
typedef SGfloat sgVec4[ 4 ];



inline void sgSetVec2( sgVec2 dst, const SGfloat x, const SGfloat y )
{
 dst[ 0 ] = x;
 dst[ 1 ] = y;
}

inline void sgSetVec3( sgVec3 dst, const SGfloat x, const SGfloat y, const SGfloat z )
{
 dst[ 0 ] = x;
 dst[ 1 ] = y;
 dst[ 2 ] = z;
}

inline void sgSetVec4( sgVec4 dst, const SGfloat x, const SGfloat y, const SGfloat z, const SGfloat w )
{
 dst[ 0 ] = x;
 dst[ 1 ] = y;
 dst[ 2 ] = z;
 dst[ 3 ] = w;
}

inline void sgCopyVec2( sgVec2 dst, const sgVec2 src )
{
 dst[ 0 ] = src[ 0 ];
 dst[ 1 ] = src[ 1 ];
}

inline void sgCopyVec3( sgVec3 dst, const sgVec3 src )
{
 dst[ 0 ] = src[ 0 ];
 dst[ 1 ] = src[ 1 ];
 dst[ 2 ] = src[ 2 ];
}

inline void sgCopyVec4( sgVec4 dst, const sgVec4 src )
{
 dst[ 0 ] = src[ 0 ];
 dst[ 1 ] = src[ 1 ];
 dst[ 2 ] = src[ 2 ];
 dst[ 3 ] = src[ 3 ];
}



#endif

