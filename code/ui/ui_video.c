/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#include "ui_local.h"

void GraphicsOptions_MenuInit(void);

/*
=======================================================================

GRAPHICS OPTIONS MENU

=======================================================================
*/

#define BACK0 "menu/buttons/back0"
#define BACK1 "menu/buttons/back1"
#define GRAPHICS0 "menu/buttons/graphics0"
#define GRASHICS1 "menu/buttons/graphics1"
#define DISPLAY0 "menu/buttons/display0"
#define DISPLAY1 "menu/buttons/display1"
#define SOUND0 "menu/buttons/sound0"
#define SOUND1 "menu/buttons/sound1"
#define NETWORK0 "menu/buttons/network0"
#define NETWORK1 "menu/buttons/network1"
#define ACCEPT0 "menu/buttons/accept"
#define ACCEPT1 "menu/buttons/accept"

#define ID_BACK2 101
#define ID_FULLSCREEN 102
#define ID_LIST 103
#define ID_MODE 104
#define ID_GRAPHICS 105
#define ID_DISPLAY 106
#define ID_SOUND 107
#define ID_NETWORK 108

#define XPOSITION 180

typedef struct {
	menuframework_s menu;

	menubitmap_s graphics;
	menubitmap_s display;
	menubitmap_s sound;
	menubitmap_s network;

	menulist_s list;
	menulist_s mode;
	menulist_s tq;
	menulist_s wm;
	menulist_s lighting;
	menulist_s texturebits;
	menulist_s colordepth;
	menulist_s mdetail;
	menulist_s cdetail;
	menulist_s filter;
	menulist_s ct;
	menulist_s af;
	menulist_s aa;

	menubitmap_s apply;
	menubitmap_s back;
} graphicsoptions_t;

typedef struct {
	int mode;
	int wm;
	int tq;
	int lighting;
	int colordepth;
	int texturebits;
	int mdetail;
	int cdetail;
	int filter;
	qboolean ct;
	int af; // index to af_names!
	int aa; // index to aa_names!
} InitialVideoOptions_s;

static InitialVideoOptions_s s_ivo;
static graphicsoptions_t s_graphicsoptions;

static InitialVideoOptions_s s_ivo_templates[] = {{2, qtrue, 3, 1, 2, 2, 1, 3, 1, qfalse, 4, 2},
												  {2, qtrue, 3, 1, 0, 0, 1, 2, 0, qfalse, 3, 0},
												  {1, qtrue, 2, 1, 1, 0, 0, 1, 0, qfalse, 2, 0},
												  {0, qtrue, 1, 0, 1, 0, 0, 0, 0, qtrue, 0, 0},
												  {
													  2, qtrue, 1, 0, 0, 0, 0, 0, 0, qtrue, 0, 0 // "custom" placeholder
												  }};

#define NUM_IVO_TEMPLATES (ARRAY_LEN(s_ivo_templates))

// x^y
// FIXME: Overflow!
int power(int x, int y) {
	int i, result = 1;

	for (i = 0; i < y; i++) {
		result *= x;
	}

	return result;
}

static const char *builtinResolutions[] = {"640x480",	"800x600",	 "1024x768",  "1152x864",  "1280x720",	"1280x800",
										   "1280x960",	"1280x1024", "1366x768",  "1440x900",  "1600x900",	"1600x1200",
										   "1680x1050", "1920x1080", "1920x1200", "2048x1536", "2560x1600", 0};

static const char *knownRatios[][2] = {{"1.25:1", "5:4"},	{"1.33:1", "4:3"}, {"1.50:1", "3:2"},  {"1.56:1", "14:9"},
									   {"1.60:1", "16:10"}, {"1.67:1", "5:3"}, {"1.78:1", "16:9"}, {NULL, NULL}};

#define MAX_RESOLUTIONS 32

static char resbuf[MAX_STRING_CHARS];
static char resolutionStrings[MAX_RESOLUTIONS][128];
static const char *detectedResolutions[MAX_RESOLUTIONS];

static const char **resolutions = builtinResolutions;
static qboolean resolutionsDetected = qfalse;

/*
=================
GraphicsOptions_FindBuiltinResolution
=================
*/
static int GraphicsOptions_FindBuiltinResolution(int mode) {
	int i;

	if (!resolutionsDetected)
		return mode;

	if (mode < 0)
		return -1;

	for (i = 0; builtinResolutions[i]; i++) {
		if (!Q_stricmp(builtinResolutions[i], detectedResolutions[mode]))
			return i;
	}

	return -1;
}

/*
=================
GraphicsOptions_FindDetectedResolution
=================
*/
static int GraphicsOptions_FindDetectedResolution(int mode) {
	int i;
	int len;

	if (!resolutionsDetected)
		return mode;

	if (mode < 0)
		return -1;

	len = strlen(builtinResolutions[mode]);
	for (i = 0; detectedResolutions[i]; i++) {
		if (!Q_stricmpn(builtinResolutions[mode], detectedResolutions[i], len))
			return i;
	}

	return -1;
}

/*
=================
GraphicsOptions_GetAspectRatios
=================
*/
static void AddAspectRatios(void) {
	int i, r;

	// add aspect ratio to the resolution strings
	for (r = 0; resolutions[r]; r++) {
		int w, h;
		char *x;
		char str[8];

		// calculate resolution's aspect ratio
		x = strchr(resolutions[r], 'x') + 1;
		Q_strncpyz(str, resolutions[r], x - resolutions[r]);
		w = atoi(str);
		h = atoi(x);
		Com_sprintf(str, sizeof(str), "%.2f:1", (float)w / (float)h);

		for (i = 0; knownRatios[i][0]; i++) {
			if (!Q_stricmp(str, knownRatios[i][0])) {
				Q_strncpyz(str, knownRatios[i][1], sizeof(str));
				break;
			}
		}

		Com_sprintf(resolutionStrings[r], 128, "%s (%s)", resolutions[r], str);
		resolutions[r] = resolutionStrings[r];
	}
}

/*
=================
GraphicsOptions_GetResolutions
=================
*/
static void GraphicsOptions_GetResolutions(void) {
	Q_strncpyz(resbuf, UI_Cvar_VariableString("r_availableModes"), sizeof(resbuf));
	if (*resbuf) {
		char *s = resbuf;
		unsigned int i = 0;
		while (s && i < ARRAY_LEN(detectedResolutions) - 1) {
			detectedResolutions[i++] = s;
			s = strchr(s, ' ');
			if (s)
				*s++ = '\0';
		}
		detectedResolutions[i] = NULL;

		if (i > 0) {
			resolutions = detectedResolutions;
			resolutionsDetected = qtrue;
		}
	}

	AddAspectRatios();
}

/*
=================
GraphicsOptions_GetInitialVideo
=================
*/
static void GraphicsOptions_GetInitialVideo(void) {
	s_ivo.colordepth = s_graphicsoptions.colordepth.curvalue;
	s_ivo.mode = s_graphicsoptions.mode.curvalue;
	s_ivo.wm = s_graphicsoptions.wm.curvalue;
	s_ivo.tq = s_graphicsoptions.tq.curvalue;
	s_ivo.lighting = s_graphicsoptions.lighting.curvalue;
	s_ivo.mdetail = s_graphicsoptions.mdetail.curvalue;
	s_ivo.cdetail = s_graphicsoptions.cdetail.curvalue;
	s_ivo.filter = s_graphicsoptions.filter.curvalue;
	s_ivo.texturebits = s_graphicsoptions.texturebits.curvalue;
	s_ivo.ct = s_graphicsoptions.ct.curvalue;
	s_ivo.af = s_graphicsoptions.af.curvalue;
	s_ivo.aa = s_graphicsoptions.aa.curvalue;
}

/*
=================
GraphicsOptions_CheckConfig
=================
*/
static void GraphicsOptions_CheckConfig(void) {
	int i;

	for (i = 0; i < (NUM_IVO_TEMPLATES - 1); i++) {
		if (s_ivo_templates[i].colordepth != s_graphicsoptions.colordepth.curvalue)
			continue;
		if (s_ivo_templates[i].mode != s_graphicsoptions.mode.curvalue)
			continue;
		if (s_ivo_templates[i].wm != s_graphicsoptions.wm.curvalue)
			continue;
		if (s_ivo_templates[i].tq != s_graphicsoptions.tq.curvalue)
			continue;
		if (s_ivo_templates[i].lighting != s_graphicsoptions.lighting.curvalue)
			continue;
		if (s_ivo_templates[i].mdetail != s_graphicsoptions.mdetail.curvalue)
			continue;
		if (s_ivo_templates[i].cdetail != s_graphicsoptions.cdetail.curvalue)
			continue;
		if (s_ivo_templates[i].filter != s_graphicsoptions.filter.curvalue)
			continue;
		if (s_ivo_templates[i].ct != s_graphicsoptions.ct.curvalue)
			continue;
		if (s_ivo_templates[i].af != s_graphicsoptions.af.curvalue)
			continue;
		if (s_ivo_templates[i].aa != s_graphicsoptions.aa.curvalue)
			continue;

		s_graphicsoptions.list.curvalue = i;
		return;
	}

	s_graphicsoptions.list.curvalue = (NUM_IVO_TEMPLATES - 1); // "custom"
}

/*
=================
GraphicsOptions_UpdateMenuItems
=================
*/
static void GraphicsOptions_UpdateMenuItems(void) {
	if (s_graphicsoptions.wm.curvalue != 0) {
		s_graphicsoptions.colordepth.curvalue = 0;
		s_graphicsoptions.colordepth.generic.flags |= QMF_GRAYED;
	} else {
		s_graphicsoptions.colordepth.generic.flags &= ~QMF_GRAYED;
	}

	s_graphicsoptions.apply.generic.flags |= QMF_HIDDEN | QMF_INACTIVE;

	if (s_ivo.mode != s_graphicsoptions.mode.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.wm != s_graphicsoptions.wm.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.tq != s_graphicsoptions.tq.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.lighting != s_graphicsoptions.lighting.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.colordepth != s_graphicsoptions.colordepth.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.texturebits != s_graphicsoptions.texturebits.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.mdetail != s_graphicsoptions.mdetail.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.cdetail != s_graphicsoptions.cdetail.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.filter != s_graphicsoptions.filter.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.ct != s_graphicsoptions.ct.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.af != s_graphicsoptions.af.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}
	if (s_ivo.aa != s_graphicsoptions.aa.curvalue) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
	}

	GraphicsOptions_CheckConfig();
}

/*
=================
GraphicsOptions_ApplyChanges
=================
*/
static void GraphicsOptions_ApplyChanges(void *unused, int notification) {
	if (notification != QM_ACTIVATED)
		return;

	switch (s_graphicsoptions.texturebits.curvalue) {
	case 0:
		trap_Cvar_SetValue("r_texturebits", 0);
		break;
	case 1:
		trap_Cvar_SetValue("r_texturebits", 16);
		break;
	case 2:
		trap_Cvar_SetValue("r_texturebits", 32);
		break;
	}
	trap_Cvar_SetValue("r_picmip", (float)(3 - s_graphicsoptions.tq.curvalue));

	if (s_graphicsoptions.wm.curvalue == 2) {
		trap_Cvar_SetValue("r_fullscreen", 0);
		trap_Cvar_SetValue("r_noborder", 1);
	} else if (s_graphicsoptions.wm.curvalue == 1) {
		trap_Cvar_SetValue("r_fullscreen", 0);
		trap_Cvar_SetValue("r_noborder", 0);
	} else {
		trap_Cvar_SetValue("r_fullscreen", 1);
		trap_Cvar_SetValue("r_noborder", 0);
	}

	switch (s_graphicsoptions.colordepth.curvalue) {
	case 0:
		trap_Cvar_SetValue("r_colorbits", 0);
		trap_Cvar_SetValue("r_depthbits", 0);
		trap_Cvar_SetValue("r_stencilbits", 0);
		break;
	case 1:
		trap_Cvar_SetValue("r_colorbits", 16);
		trap_Cvar_SetValue("r_depthbits", 16);
		trap_Cvar_SetValue("r_stencilbits", 0);
		break;
	case 2:
		trap_Cvar_SetValue("r_colorbits", 32);
		trap_Cvar_SetValue("r_depthbits", 24);
		break;
	}
	trap_Cvar_SetValue("r_vertexLight", !s_graphicsoptions.lighting.curvalue);

	if (s_graphicsoptions.mdetail.curvalue == 2) {
		trap_Cvar_SetValue("r_lodBias", 0);
	} else if (s_graphicsoptions.mdetail.curvalue == 1) {
		trap_Cvar_SetValue("r_lodBias", 1);
	} else {
		trap_Cvar_SetValue("r_lodBias", 2);
	}

	if (s_graphicsoptions.cdetail.curvalue == 3) {
		trap_Cvar_SetValue("r_subdivisions", 2);
	} else if (s_graphicsoptions.cdetail.curvalue == 2) {
		trap_Cvar_SetValue("r_subdivisions", 4);
	} else if (s_graphicsoptions.cdetail.curvalue == 1) {
		trap_Cvar_SetValue("r_subdivisions", 12);
	} else {
		trap_Cvar_SetValue("r_subdivisions", 20);
	}

	if (s_graphicsoptions.filter.curvalue) {
		trap_Cvar_Set("r_textureMode", "GL_LINEAR_MIPMAP_LINEAR");
	} else {
		trap_Cvar_Set("r_textureMode", "GL_LINEAR_MIPMAP_NEAREST");
	}

	trap_Cvar_SetValue("r_ext_compressed_textures", s_graphicsoptions.ct.curvalue);

	if (s_graphicsoptions.af.curvalue > 0) {
		trap_Cvar_SetValue("r_ext_max_anisotropy", power(2, s_graphicsoptions.af.curvalue));
	} else {
		trap_Cvar_Set("r_ext_max_anisotropy", "0");
	}

	if (s_graphicsoptions.aa.curvalue > 0) {
		trap_Cvar_SetValue("r_ext_multisample", power(2, s_graphicsoptions.aa.curvalue));
	} else {
		trap_Cvar_Set("r_ext_multisample", "0");
	}

	if (resolutionsDetected) {
		// search for builtin mode that matches the detected mode
		int mode;
		if (s_graphicsoptions.mode.curvalue == -1 ||
			s_graphicsoptions.mode.curvalue >= sizeof(detectedResolutions) / sizeof(detectedResolutions[0]))
			s_graphicsoptions.mode.curvalue = 0;

		mode = GraphicsOptions_FindBuiltinResolution(s_graphicsoptions.mode.curvalue);
		if (mode == -1) {
			char w[16], h[16];
			Q_strncpyz(w, detectedResolutions[s_graphicsoptions.mode.curvalue], sizeof(w));
			*strchr(w, 'x') = 0;
			Q_strncpyz(h, strchr(detectedResolutions[s_graphicsoptions.mode.curvalue], 'x') + 1, sizeof(h));
			trap_Cvar_Set("r_customwidth", w);
			trap_Cvar_Set("r_customheight", h);
		}

		trap_Cvar_SetValue("r_mode", (float)mode);
	} else
		trap_Cvar_SetValue("r_mode", (float)s_graphicsoptions.mode.curvalue);

	trap_Cmd_ExecuteText(EXEC_APPEND, "vid_restart\n");
}

/*
=================
GraphicsOptions_Event
=================
*/
static void GraphicsOptions_Event(void *ptr, int event) {
	InitialVideoOptions_s *ivo;

	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
	case ID_LIST:
		ivo = &s_ivo_templates[s_graphicsoptions.list.curvalue];
		s_graphicsoptions.mode.curvalue = ivo->mode;
		s_graphicsoptions.tq.curvalue = ivo->tq;
		s_graphicsoptions.lighting.curvalue = ivo->lighting;
		s_graphicsoptions.colordepth.curvalue = ivo->colordepth;
		s_graphicsoptions.texturebits.curvalue = ivo->texturebits;
		s_graphicsoptions.mdetail.curvalue = ivo->mdetail;
		s_graphicsoptions.cdetail.curvalue = ivo->cdetail;
		s_graphicsoptions.filter.curvalue = ivo->filter;
		s_graphicsoptions.wm.curvalue = ivo->wm;
		s_graphicsoptions.ct.curvalue = ivo->ct;
		s_graphicsoptions.af.curvalue = ivo->af;
		s_graphicsoptions.aa.curvalue = ivo->aa;
		break;

	case ID_BACK2:
		UI_PopMenu();
		break;

	case ID_GRAPHICS:
		break;

	case ID_DISPLAY:
		UI_PopMenu();
		UI_DisplayOptionsMenu();
		break;

	case ID_SOUND:
		UI_PopMenu();
		UI_SoundOptionsMenu();
		break;

	case ID_NETWORK:
		UI_PopMenu();
		UI_NetworkOptionsMenu();
		break;
	}
}

/*
================
GraphicsOptions_MenuDraw
================
*/
void GraphicsOptions_MenuDraw(void) {
	// APSFIX - rework this
	GraphicsOptions_UpdateMenuItems();

	Menu_Draw(&s_graphicsoptions.menu);
}

/*
=================
GraphicsOptions_SetMenuItems
=================
*/
static void GraphicsOptions_SetMenuItems(void) {
	s_graphicsoptions.mode.curvalue = GraphicsOptions_FindDetectedResolution(UI_GetCvarInt("r_mode"));

	if (s_graphicsoptions.mode.curvalue < 0) {
		if (resolutionsDetected) {
			int i;
			char buf[MAX_STRING_CHARS];
			trap_Cvar_VariableStringBuffer("r_customwidth", buf, sizeof(buf) - 2);
			buf[strlen(buf) + 1] = 0;
			buf[strlen(buf)] = 'x';
			trap_Cvar_VariableStringBuffer("r_customheight", buf + strlen(buf), sizeof(buf) - strlen(buf));

			for (i = 0; detectedResolutions[i]; ++i) {
				if (!Q_stricmp(buf, detectedResolutions[i])) {
					s_graphicsoptions.mode.curvalue = i;
					break;
				}
			}
			if (s_graphicsoptions.mode.curvalue < 0)
				s_graphicsoptions.mode.curvalue = 0;
		} else {
			s_graphicsoptions.mode.curvalue = 2;
		}
	}

	if (trap_Cvar_VariableValue("r_fullscreen") == 0) {
		if (trap_Cvar_VariableValue("r_noborder") == 1) {
			s_graphicsoptions.wm.curvalue = 2;
		} else {
			s_graphicsoptions.wm.curvalue = 1;
		}
	} else {
		s_graphicsoptions.wm.curvalue = 0;
	}

	s_graphicsoptions.tq.curvalue = 3 - UI_GetCvarInt("r_picmip");
	if (s_graphicsoptions.tq.curvalue < 0) {
		s_graphicsoptions.tq.curvalue = 0;
	} else if (s_graphicsoptions.tq.curvalue > 3) {
		s_graphicsoptions.tq.curvalue = 3;
	}

	s_graphicsoptions.lighting.curvalue = trap_Cvar_VariableValue("r_vertexLight") == 0;
	switch (UI_GetCvarInt("r_texturebits")) {
	default:
	case 0:
		s_graphicsoptions.texturebits.curvalue = 0;
		break;
	case 16:
		s_graphicsoptions.texturebits.curvalue = 1;
		break;
	case 32:
		s_graphicsoptions.texturebits.curvalue = 2;
		break;
	}

	if (!Q_stricmp(UI_Cvar_VariableString("r_textureMode"), "GL_LINEAR_MIPMAP_NEAREST")) {
		s_graphicsoptions.filter.curvalue = 0;
	} else {
		s_graphicsoptions.filter.curvalue = 1;
	}

	if (trap_Cvar_VariableValue("r_lodBias") < 1) {
		s_graphicsoptions.mdetail.curvalue = 2;
	} else if (trap_Cvar_VariableValue("r_lodBias") < 2) {
		s_graphicsoptions.mdetail.curvalue = 1;
	} else {
		s_graphicsoptions.mdetail.curvalue = 0;
	}

	if (trap_Cvar_VariableValue("r_subdivisions") < 4) {
		s_graphicsoptions.cdetail.curvalue = 3;
	} else if (trap_Cvar_VariableValue("r_subdivisions") < 12) {
		s_graphicsoptions.cdetail.curvalue = 2;
	} else if (trap_Cvar_VariableValue("r_subdivisions") < 20) {
		s_graphicsoptions.cdetail.curvalue = 1;
	} else {
		s_graphicsoptions.cdetail.curvalue = 0;
	}

	switch (UI_GetCvarInt("r_colorbits")) {
	default:
	case 0:
		s_graphicsoptions.colordepth.curvalue = 0;
		break;
	case 16:
		s_graphicsoptions.colordepth.curvalue = 1;
		break;
	case 32:
		s_graphicsoptions.colordepth.curvalue = 2;
		break;
	}

	if (s_graphicsoptions.wm.curvalue != 0) {
		s_graphicsoptions.colordepth.curvalue = 0;
	}

	s_graphicsoptions.ct.curvalue = UI_GetCvarInt("r_ext_compressed_textures");

	if (trap_Cvar_VariableValue("r_ext_multisample") > 2) {
		s_graphicsoptions.aa.curvalue = ceil(sqrt(trap_Cvar_VariableValue("r_ext_multisample")));
	} else {
		s_graphicsoptions.aa.curvalue = floor(sqrt(trap_Cvar_VariableValue("r_ext_multisample")));
	}

	if (trap_Cvar_VariableValue("r_ext_max_anisotropy") > 2) {
		s_graphicsoptions.af.curvalue = ceil(sqrt(trap_Cvar_VariableValue("r_ext_max_anisotropy")));
	} else if (trap_Cvar_VariableValue("r_ext_max_anisotropy") > 0) {
		s_graphicsoptions.af.curvalue = floor(sqrt(trap_Cvar_VariableValue("r_ext_max_anisotropy")));
	} else {
		s_graphicsoptions.af.curvalue = 0;
	}
}

/*
================
GraphicsOptions_MenuInit
================
*/
void GraphicsOptions_MenuInit(void) {
	static const char *tq_names[] = {"Default", "16 bit", "32 bit", NULL};
	static const char *s_graphics_options_names[] = {"High Quality", "Normal", "Fast", "Faster", "Custom", NULL};
	static const char *lighting_names[] = {"Low (Vertex)", "High (Lightmap)", NULL};
	static const char *colordepth_names[] = {"Default", "16 bit", "32 bit", NULL};
	static const char *wm_names[] = {"Off (Fullscreen)", "On (Border)", "On (No Border)", NULL};
	static const char *filter_names[] = {"Bilinear", "Trilinear", NULL};
	static const char *td_names[] = {"Low", "Medium", "High", "Maximum", NULL};
	static const char *af_names[] = {"Off", "2x", "4x", "8x", "16x", NULL};
	static const char *aa_names[] = {"Off", "2x", "4x", "8x", NULL};
	static const char *mdetail_names[] = {"Low", "Medium", "High", NULL};
	static const char *cdetail_names[] = {"Low", "Medium", "High", "Maximum", NULL};
	static const char *enabled_names[] = {"Off", "On", NULL};

	int y;

	// zero set all our globals
	memset(&s_graphicsoptions, 0, sizeof(graphicsoptions_t));

	GraphicsOptions_GetResolutions();

	GraphicsOptions_Cache();

	s_graphicsoptions.menu.wrapAround = qtrue;
	s_graphicsoptions.menu.fullscreen = qtrue;
	s_graphicsoptions.menu.draw = GraphicsOptions_MenuDraw;
	s_graphicsoptions.menu.bgparts = BGP_SYSTEMBG | BGP_SIMPLEBG;

	s_graphicsoptions.graphics.generic.type = MTYPE_BITMAP;
	s_graphicsoptions.graphics.generic.name = GRAPHICS0;
	s_graphicsoptions.graphics.generic.flags = QMF_LEFT_JUSTIFY | QMF_HIGHLIGHT;
	s_graphicsoptions.graphics.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.graphics.generic.id = ID_GRAPHICS;
	s_graphicsoptions.graphics.generic.x = 16;
	s_graphicsoptions.graphics.generic.y = 37;
	s_graphicsoptions.graphics.width = 160;
	s_graphicsoptions.graphics.height = 40;
	s_graphicsoptions.graphics.focuspic = GRASHICS1;
	s_graphicsoptions.graphics.focuspicinstead = qtrue;

	s_graphicsoptions.display.generic.type = MTYPE_BITMAP;
	s_graphicsoptions.display.generic.name = DISPLAY0;
	s_graphicsoptions.display.generic.flags = QMF_LEFT_JUSTIFY | QMF_HIGHLIGHT_IF_FOCUS;
	s_graphicsoptions.display.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.display.generic.id = ID_DISPLAY;
	s_graphicsoptions.display.generic.x = 169;
	s_graphicsoptions.display.generic.y = 30;
	s_graphicsoptions.display.width = 120;
	s_graphicsoptions.display.height = 40;
	s_graphicsoptions.display.focuspic = DISPLAY1;
	s_graphicsoptions.display.focuspicinstead = qtrue;

	s_graphicsoptions.sound.generic.type = MTYPE_BITMAP;
	s_graphicsoptions.sound.generic.name = SOUND0;
	s_graphicsoptions.sound.generic.flags = QMF_LEFT_JUSTIFY | QMF_HIGHLIGHT_IF_FOCUS;
	s_graphicsoptions.sound.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.sound.generic.id = ID_SOUND;
	s_graphicsoptions.sound.generic.x = 36;
	s_graphicsoptions.sound.generic.y = 79;
	s_graphicsoptions.sound.width = 120;
	s_graphicsoptions.sound.height = 40;
	s_graphicsoptions.sound.focuspic = SOUND1;
	s_graphicsoptions.sound.focuspicinstead = qtrue;

	s_graphicsoptions.network.generic.type = MTYPE_BITMAP;
	s_graphicsoptions.network.generic.name = NETWORK0;
	s_graphicsoptions.network.generic.flags = QMF_LEFT_JUSTIFY | QMF_HIGHLIGHT_IF_FOCUS;
	s_graphicsoptions.network.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.network.generic.id = ID_NETWORK;
	s_graphicsoptions.network.generic.x = 142;
	s_graphicsoptions.network.generic.y = 82;
	s_graphicsoptions.network.width = 160;
	s_graphicsoptions.network.height = 40;
	s_graphicsoptions.network.focuspic = NETWORK1;
	s_graphicsoptions.network.focuspicinstead = qtrue;

	y = 180;
	s_graphicsoptions.list.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.list.generic.name = "Graphics Settings:";
	s_graphicsoptions.list.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.list.generic.x = XPOSITION;
	s_graphicsoptions.list.generic.y = y;
	s_graphicsoptions.list.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.list.generic.id = ID_LIST;
	s_graphicsoptions.list.itemnames = s_graphics_options_names;
	y += 2 * (BIGCHAR_HEIGHT + 2);

	// references/modifies "r_mode"
	s_graphicsoptions.mode.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.mode.generic.name = "Video Mode:";
	s_graphicsoptions.mode.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.mode.generic.x = XPOSITION;
	s_graphicsoptions.mode.generic.y = y;
	s_graphicsoptions.mode.itemnames = resolutions;
	s_graphicsoptions.mode.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.mode.generic.id = ID_MODE;
	y += BIGCHAR_HEIGHT + 2;

	// references "r_colorbits"
	s_graphicsoptions.colordepth.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.colordepth.generic.name = "Color Depth:";
	s_graphicsoptions.colordepth.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.colordepth.generic.x = XPOSITION;
	s_graphicsoptions.colordepth.generic.y = y;
	s_graphicsoptions.colordepth.itemnames = colordepth_names;
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_fullscreen"
	// references/modifies "r_noborder"
	s_graphicsoptions.wm.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.wm.itemnames = wm_names;
	s_graphicsoptions.wm.generic.name = "Window Mode:";
	s_graphicsoptions.wm.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.wm.generic.x = XPOSITION;
	s_graphicsoptions.wm.generic.y = y;
	s_graphicsoptions.wm.generic.toolTip =
		"Switch on to play the game in a window, change resolution to change size of the window. "
		"Choose no border to remove window decoration from window managers, like borders and titlebar.";
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_vertexLight"
	s_graphicsoptions.lighting.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.lighting.generic.name = "Lighting:";
	s_graphicsoptions.lighting.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.lighting.generic.x = XPOSITION;
	s_graphicsoptions.lighting.generic.y = y;
	s_graphicsoptions.lighting.itemnames = lighting_names;
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_lodBias"
	s_graphicsoptions.mdetail.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.mdetail.generic.name = "Models Detail:";
	s_graphicsoptions.mdetail.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.mdetail.generic.x = XPOSITION;
	s_graphicsoptions.mdetail.generic.y = y;
	s_graphicsoptions.mdetail.itemnames = mdetail_names;
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_subdivisions"
	s_graphicsoptions.cdetail.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.cdetail.generic.name = "Curves Detail:";
	s_graphicsoptions.cdetail.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.cdetail.generic.x = XPOSITION;
	s_graphicsoptions.cdetail.generic.y = y;
	s_graphicsoptions.cdetail.itemnames = cdetail_names;
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_picmip"
	s_graphicsoptions.tq.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.tq.generic.name = "Texture Detail:";
	s_graphicsoptions.tq.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.tq.generic.x = XPOSITION;
	s_graphicsoptions.tq.generic.y = y;
	s_graphicsoptions.tq.itemnames = td_names;
	s_graphicsoptions.tq.generic.toolTip = "Adjust overall texture detail levels based on graphics card performance.";
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_textureBits"
	s_graphicsoptions.texturebits.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.texturebits.generic.name = "Texture Quality:";
	s_graphicsoptions.texturebits.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.texturebits.generic.x = XPOSITION;
	s_graphicsoptions.texturebits.generic.y = y;
	s_graphicsoptions.texturebits.itemnames = tq_names;
	s_graphicsoptions.texturebits.generic.toolTip = "Adjust texture detail based on graphics card performance.";
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_textureMode"
	s_graphicsoptions.filter.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.filter.generic.name = "Texture Filter:";
	s_graphicsoptions.filter.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.filter.generic.x = XPOSITION;
	s_graphicsoptions.filter.generic.y = y;
	s_graphicsoptions.filter.itemnames = filter_names;
	s_graphicsoptions.filter.generic.toolTip = "A graphic sharpness filter. Use bilinear for lower end graphics cards. "
											   "Use Trilinear for mid to higher range graphics cards.";
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_ext_compressed_textures"
	s_graphicsoptions.ct.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.ct.itemnames = enabled_names;
	s_graphicsoptions.ct.generic.name = "Compress Textures:";
	s_graphicsoptions.ct.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.ct.generic.x = XPOSITION;
	s_graphicsoptions.ct.generic.y = y;
	s_graphicsoptions.ct.generic.toolTip = "Switch on to allow your graphics card to store texture data compressed if "
										   "supported (most graphics card regardless of type will support this).";
	y += BIGCHAR_HEIGHT + 2;

	// references/modifies "r_ext_max_anisotropy"
	s_graphicsoptions.af.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.af.generic.name = "Anisotropy:";
	s_graphicsoptions.af.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.af.generic.x = XPOSITION;
	s_graphicsoptions.af.generic.y = y;
	s_graphicsoptions.af.itemnames = af_names;
	s_graphicsoptions.af.generic.toolTip = "Sharpens game textures. Not recommended for low end graphics cards. "
										   "Requires high graphics card performance and memory!";
	y += (BIGCHAR_HEIGHT + 2);

	// references/modifies "r_ext_max_multisampling"
	s_graphicsoptions.aa.generic.type = MTYPE_SPINCONTROL;
	s_graphicsoptions.aa.generic.name = "Antialiasing:";
	s_graphicsoptions.aa.generic.flags = QMF_SMALLFONT;
	s_graphicsoptions.aa.generic.x = XPOSITION;
	s_graphicsoptions.aa.generic.y = y;
	s_graphicsoptions.aa.itemnames = aa_names;
	s_graphicsoptions.aa.generic.toolTip = "Smooth out rough edges. Not recommended for low end graphics cards. "
										   "Requires high graphics card performance and memory!";

	s_graphicsoptions.back.generic.type = MTYPE_BITMAP;
	s_graphicsoptions.back.generic.name = BACK0;
	s_graphicsoptions.back.generic.flags = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
	s_graphicsoptions.back.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.back.generic.id = ID_BACK2;
	s_graphicsoptions.back.generic.x = 8;
	s_graphicsoptions.back.generic.y = 440;
	s_graphicsoptions.back.width = 80;
	s_graphicsoptions.back.height = 40;
	s_graphicsoptions.back.focuspic = BACK1;
	s_graphicsoptions.back.focuspicinstead = qtrue;

	s_graphicsoptions.apply.generic.type = MTYPE_BITMAP;
	s_graphicsoptions.apply.generic.name = ACCEPT0;
	s_graphicsoptions.apply.generic.flags = QMF_PULSEIFFOCUS | QMF_HIDDEN | QMF_INACTIVE;
	s_graphicsoptions.apply.generic.callback = GraphicsOptions_ApplyChanges;
	s_graphicsoptions.apply.generic.x = 512;
	s_graphicsoptions.apply.generic.y = 440;
	s_graphicsoptions.apply.width = 120;
	s_graphicsoptions.apply.height = 40;
	s_graphicsoptions.apply.focuspic = ACCEPT1;

	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.graphics);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.display);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.sound);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.network);

	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.list);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.mode);

	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.colordepth);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.wm);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.lighting);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.mdetail);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.cdetail);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.tq);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.texturebits);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.filter);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.ct);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.af);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.aa);

	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.back);
	Menu_AddItem(&s_graphicsoptions.menu, (void *)&s_graphicsoptions.apply);

	GraphicsOptions_SetMenuItems();
	GraphicsOptions_GetInitialVideo();
}

/*
=================
GraphicsOptions_Cache
=================
*/
void GraphicsOptions_Cache(void) {
	trap_R_RegisterShaderNoMip(BACK0);
	trap_R_RegisterShaderNoMip(BACK1);
	trap_R_RegisterShaderNoMip(GRAPHICS0);
	trap_R_RegisterShaderNoMip(GRASHICS1);
	trap_R_RegisterShaderNoMip(DISPLAY0);
	trap_R_RegisterShaderNoMip(DISPLAY1);
	trap_R_RegisterShaderNoMip(SOUND0);
	trap_R_RegisterShaderNoMip(SOUND1);
	trap_R_RegisterShaderNoMip(NETWORK0);
	trap_R_RegisterShaderNoMip(NETWORK1);
	trap_R_RegisterShaderNoMip(ACCEPT0);
	trap_R_RegisterShaderNoMip(ACCEPT1);
}

/*
=================
UI_GraphicsOptionsMenu
=================
*/
void UI_GraphicsOptionsMenu(void) {
	GraphicsOptions_MenuInit();
	UI_PushMenu(&s_graphicsoptions.menu);
	Menu_SetCursorToItem(&s_graphicsoptions.menu, &s_graphicsoptions.graphics);
}
