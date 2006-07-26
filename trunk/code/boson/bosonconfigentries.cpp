/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "bosonconfig.h"

#include "defines.h"
#include "../bomemory/bodummymemory.h"
#include "boglobal.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include <bogl.h>

#include <klocale.h>

#include <stdlib.h>

void BosonConfig::initConfigEntries()
{
 addDynamicEntryBool("Sound", false);
 addDynamicEntryBool("Music", true);
 addDynamicEntryBool("MMBMove", true);
 addDynamicEntryBool("RMBMove", true);
 addDynamicEntryBool("WheelMoveZoom", false); // whether we center the screen on the mouse on mouse wheel zoom
 addDynamicEntryUInt("ArrowKeyStep", 10);
 addDynamicEntryUInt("CursorEdgeSensity", 20);
 addDynamicEntryUInt("GLUpdateInterval", 20);
 addDynamicEntryDouble("MiniMapScale", 2.0);
 addDynamicEntryDouble("MiniMapZoom", 1.0);
 addDynamicEntryUInt("ChatScreenRemoveTime", 10);
 addDynamicEntryInt("ChatScreenMaxItems", 5);
 addDynamicEntryIntList("DeactivateUnitSounds", QValueList<int>());
 addDynamicEntryBool("AlignSelectionBoxes", true);
 addDynamicEntryBool("RMBMovesWithAttack", true);
 addDynamicEntryInt("MouseWheelAction", CameraZoom);
 addDynamicEntryInt("MouseWheelShiftAction", CameraRotate);
 addDynamicEntryBool("DeactivateWeaponSounds", false);
 addDynamicEntryBool("UseLight", true);
 addDynamicEntryBool("UseMaterials", false);
 addDynamicEntryInt("CursorMode", (int)CursorOpenGL);
 addDynamicEntryString("CursorDir", QString::null); // QString::null means use BosonCursor::defaultTheme()
 addDynamicEntryInt("ToolTipUpdatePeriod", 300);
 addDynamicEntryInt("ToolTipCreator", 1); // FIXME: should be BoToolTipCreator::Extended, but I don't want to include the file here
 addDynamicEntryInt("GameLogInterval", 10);
 addDynamicEntryBool("UseLOD", true);
 addDynamicEntryBool("UseVBO", false); // NVidia drivers don't properly support VBOs
 addDynamicEntryBool("WaterShaders", true);
 addDynamicEntryBool("WaterReflections", true);
 addDynamicEntryBool("WaterTranslucency", true);
 addDynamicEntryBool("WaterBumpmapping", true);
 addDynamicEntryBool("WaterAnimatedBumpmaps", true);
 addDynamicEntryInt("TextureFilter", GL_LINEAR_MIPMAP_LINEAR);
 addDynamicEntryBool("TextureCompression", true);
 addDynamicEntryBool("TextureColorMipmaps", false);
 addDynamicEntryInt("TextureAnisotropy", 1);
 addDynamicEntryUInt("MaxProfilingEntriesGL", 1000);
 addDynamicEntryUInt("MaxProfilingEntriesAdvance", 200);
 addDynamicEntryUInt("MaxProfilingEntries", 1000);
 addDynamicEntryBool("UseGroundShaders", false);
 addDynamicEntryBool("UseUnitShaders", false);
 addDynamicEntryString("ShaderSuffixes", "-med,-low");
 addDynamicEntryInt("ShadowMapResolution", 2048);

 addDynamicEntry(new BoConfigUIntEntry(this, "GroundRenderer", 0)); // obsolete
 addDynamicEntry(new BoConfigUIntEntry(this, "DefaultLOD", 0));
 addDynamicEntry(new BoConfigBoolEntry(this, "EnableATIDepthWorkaround", false));
 addDynamicEntry(new BoConfigDoubleEntry(this, "ATIDepthWorkaroundValue", 0.00390625));
 addDynamicEntry(new BoConfigStringEntry(this, "GLFont", QString::null));
 addDynamicEntry(new BoConfigBoolEntry(this, "SmoothShading", true));
 addDynamicEntry(new BoConfigStringEntry(this, "MeshRenderer", "BoMeshRendererVertexArray"));
 addDynamicEntry(new BoConfigStringEntry(this, "GroundRendererClass", "BoDefaultGroundRenderer"));
 addDynamicEntry(new BoConfigStringEntry(this, "GameViewPlugin", "BosonGameViewDefault"));
 addDynamicEntry(new BoConfigBoolEntry(this, "EditorShowRandomMapGenerationWidget", false));
 addDynamicEntry(new BoConfigBoolEntry(this, "ShowUnitDebugWidget", false));
 addDynamicEntry(new BoConfigIntEntry(this, "GameSpeed", DEFAULT_GAME_SPEED));


 // the following are NOT stored into the config file
 addDynamicEntryBool("debug_fps", false, false);

 // sound is disabled by default, to make bounit and other non-sound applications work correctly, without calling setDisableSound(true) explicitly.
 addDynamicEntryBool("ForceDisableSound", true, false); // command line arg! do NOT save to config

 addDynamicEntryInt("DebugMode", (int)DebugNormal, false);
 addDynamicEntryDouble("AIDelay", 3.0, false);
 addDynamicEntryBool("ForceWantDirect", true, false); // command line arg! do NOT save to config
 addDynamicEntryBool("debug_wireframes", false, false);
 addDynamicEntryBool("debug_matrices", false, false);
 addDynamicEntryBool("debug_map_coordinates", false, false);
 addDynamicEntryBool("debug_pf_data", false, false);
 addDynamicEntryBool("debug_cell_grid", false, false);
 addDynamicEntryBool("debug_works", false, false);
 addDynamicEntryBool("debug_camera", false, false);
 addDynamicEntryBool("debug_rendercounts", false, false);
 addDynamicEntryBool("debug_boundingboxes", false, false);
 addDynamicEntryBool("debug_advance_calls", false, false);
 addDynamicEntryBool("debug_texture_memory", false, false);
 addDynamicEntryBool("debug_memory_usage", false, false);
 addDynamicEntryBool("debug_memory_vmdata_only", false, false);
 addDynamicEntryBool("debug_cpu_usage", false, false);
 addDynamicEntryBool("debug_groundrenderer_debug", false, false);
 addDynamicEntryBool("show_resources", true, false);
 addDynamicEntryBool("debug_profiling_graph", false, false);
 addDynamicEntryBool("debug_rendering_config", false, false);
 addDynamicEntryBool("debug_network_traffic", false, false);
 addDynamicEntryBool("debug_glfinish_before_profiling", false, false);
 addDynamicEntryBool("ForceDisableModelLoading", false, false); // command line arg! do NOT save to config
 addDynamicEntryBool("ForceDisableTextureCompression", false, false); // command line arg! do NOT save to config
 addDynamicEntryBool("TextureFOW", true, false);
 addDynamicEntryInt("DefaultLodCount", 5, false);
 addDynamicEntryBool("debug_render_ground", true, false);
 addDynamicEntryBool("debug_render_items", true, false);
 addDynamicEntryBool("debug_render_water", true, false);
 addDynamicEntryBool("debug_render_particles", true, false);
}

void BosonConfig::initScripts()
{
 // note: this method won't modify any config values!

 BosonConfigScript* defaultRendering = new BosonConfigScript("DefaultRendering");
 addConfigScript(defaultRendering);
 defaultRendering->addDefaultValueOf("TextureFilter", this);
 defaultRendering->addDefaultValueOf("TextureCompression", this);
 defaultRendering->addDefaultValueOf("TextureColorMipmaps", this);
 defaultRendering->addDefaultValueOf("UseLight", this);
 defaultRendering->addDefaultValueOf("UseMaterials", this);
 defaultRendering->addDefaultValueOf("GroundRendererClass", this);
 defaultRendering->addDefaultValueOf("UseGroundShaders", this);
 defaultRendering->addDefaultValueOf("UseUnitShaders", this);
 defaultRendering->addDefaultValueOf("ShaderSuffixes", this);
 defaultRendering->addDefaultValueOf("ShadowMapResolution", this);
 defaultRendering->addDefaultValueOf("MeshRenderer", this);
 defaultRendering->addDefaultValueOf("UseLOD", this);
 defaultRendering->addDefaultValueOf("DefaultLOD", this);
 defaultRendering->addDefaultValueOf("SmoothShading", this);

 BosonConfigScript* bestRendering = new BosonConfigScript("BestQualityRendering");
 addConfigScript(bestRendering);
 bestRendering->addDefaultValueOf("TextureFilter", this);
 bestRendering->addDefaultValueOf("TextureCompression", this);
 bestRendering->addDefaultValueOf("TextureColorMipmaps", this);
 bestRendering->addDefaultValueOf("UseLight", this);
 bestRendering->addDefaultValueOf("UseMaterials", this);
 bestRendering->addDefaultValueOf("GroundRendererClass", this);
 bestRendering->addBoolValue("UseGroundShaders", true);
 bestRendering->addBoolValue("UseUnitShaders", true);
 bestRendering->addStringValue("ShaderSuffixes", "-vhi,-hi,-med,-low");
 bestRendering->addIntValue("ShadowMapResolution", 2048);
 bestRendering->addDefaultValueOf("MeshRenderer", this);
 bestRendering->addDefaultValueOf("UseLOD", this);
 bestRendering->addDefaultValueOf("DefaultLOD", this);
 bestRendering->addDefaultValueOf("SmoothShading", this);


 BosonConfigScript* fastRendering = new BosonConfigScript("FastRendering");
 addConfigScript(fastRendering);
 fastRendering->addIntValue("TextureFilter", GL_LINEAR);
 fastRendering->addBoolValue("TextureCompression", true);
 fastRendering->addDefaultValueOf("TextureColorMipmaps", this);
 fastRendering->addBoolValue("UseLight", false);
 fastRendering->addBoolValue("UseMaterials", false);
 fastRendering->addStringValue("GroundRendererClass", "BoFastGroundRenderer");
 fastRendering->addBoolValue("UseGroundShaders", false);
 fastRendering->addBoolValue("UseUnitShaders", false);
 fastRendering->addStringValue("ShaderSuffixes", "-low");
 fastRendering->addIntValue("ShadowMapResolution", 512);
 fastRendering->addDefaultValueOf("MeshRenderer", this);
 fastRendering->addBoolValue("UseLOD", true);
 fastRendering->addDefaultValueOf("DefaultLOD", this);
 fastRendering->addDefaultValueOf("SmoothShading", this);



 BosonConfigScript* softwareRendering = new BosonConfigScript("SoftwareRendering");
 addConfigScript(softwareRendering);
 softwareRendering->addBoolValue("UseLight", false);
 softwareRendering->addBoolValue("UseMaterials", false);
 softwareRendering->addBoolValue("UseLOD", true);
 softwareRendering->addBoolValue("UseVBO", true);
 softwareRendering->addBoolValue("WaterShaders", false);
 softwareRendering->addBoolValue("WaterReflections", false);
 softwareRendering->addBoolValue("WaterTranslucency", false);
 softwareRendering->addBoolValue("WaterBumpmapping", false);
 softwareRendering->addBoolValue("WaterAnimatedBumpmaps", false);
 softwareRendering->addIntValue("TextureFilter", GL_NEAREST);
 softwareRendering->addBoolValue("TextureCompression", false);
 softwareRendering->addBoolValue("UseGroundShaders", false);
 softwareRendering->addBoolValue("UseUnitShaders", false);
 softwareRendering->addBoolValue("SmoothShading", false);
// softwareRendering->addStringValue("MeshRenderer", "BoMeshRendererVertexArray");
 softwareRendering->addStringValue("GroundRendererClass", "BoVeryFastGroundRenderer");


 BosonConfigScript* fastStartup = new BosonConfigScript("FastStartup");
 addConfigScript(fastStartup);
 fastStartup->addBoolValue("ForceDisableTextureCompression", true);
 fastStartup->addBoolValue("ForceDisableSound", true);

 BosonConfigScript* veryFastStartup = new BosonConfigScript("VeryFastStartup");
 addConfigScript(veryFastStartup);
 veryFastStartup->copyScript(*fastStartup);
 veryFastStartup->addBoolValue("ForceDisableModelLoading", true);
}

