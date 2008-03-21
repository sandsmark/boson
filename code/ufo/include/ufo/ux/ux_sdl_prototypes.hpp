/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ux/ux_sdl_prototypes.hpp
    begin             : Thu Jul 29 2004
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/


#define UFO_SDL_PROC_UNUSED(ret,func,params)

UFO_SDL_PROC(SDL_Surface *,SDL_SetVideoMode,(int, int,int, Uint32))
UFO_SDL_PROC(SDL_Surface *,SDL_GetVideoSurface,(void))
UFO_SDL_PROC(int,SDL_Init,(Uint32 flags))
UFO_SDL_PROC(int,SDL_Quit,(void))
UFO_SDL_PROC(void,SDL_PumpEvents,(void))
UFO_SDL_PROC(int,SDL_PeepEvents,(SDL_Event *, int,SDL_eventaction, Uint32))
UFO_SDL_PROC(Uint8,SDL_EventState,(Uint8, int))
UFO_SDL_PROC(char *,SDL_GetError,(void))
UFO_SDL_PROC(int,SDL_GetWMInfo,(SDL_SysWMinfo *))
UFO_SDL_PROC(void,SDL_WM_SetCaption,(const char *, const char *))
UFO_SDL_PROC(void,SDL_WM_GetCaption,(char **, char **))
UFO_SDL_PROC(void,SDL_GL_SwapBuffers,(void))
UFO_SDL_PROC(SDLMod,SDL_GetModState,(void))
UFO_SDL_PROC(Uint8,SDL_GetMouseState,(int *, int *))
UFO_SDL_PROC(int,SDL_EnableUNICODE,(int))
UFO_SDL_PROC(int,SDL_EnableKeyRepeat,(int, int))
