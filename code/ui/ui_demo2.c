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
/*
=======================================================================

DEMOS MENU

=======================================================================
*/

#include "ui_local.h"

#define BACK0 "menu/buttons/back0"
#define BACK1 "menu/buttons/back1"
#define GO0 "menu/buttons/play_yel0"
#define GO1 "menu/buttons/play_yel1"
#define ARROWUP0 "menu/arrows/headblu_up0"
#define ARROWUP1 "menu/arrows/headblu_up1"
#define ARROWDN0 "menu/arrows/headblu_dn0"
#define ARROWDN1 "menu/arrows/headblu_dn1"

#define MAX_DEMOS 1024
#define NAMEBUFSIZE (MAX_DEMOS * 32)

#define ID_BACK 10
#define ID_GO 11
#define ID_LIST 12
#define ID_SCROLL_UP 13
#define ID_SCROLL_DOWN 14

typedef struct {
	menuframework_s menu;

	menulist_s list;

	menubitmap1024s_s left;
	menubitmap1024s_s right;
	menubitmap_s back;
	menubitmap1024s_s go;

	char names[NAMEBUFSIZE];
	char *demolist[MAX_DEMOS];
} demos_t;

static demos_t s_demos;

/*
===============
Demos_MenuEvent
===============
*/
static void Demos_MenuEvent(void *ptr, int event) {
	if (event != QM_ACTIVATED) {
		return;
	}

	switch (((menucommon_s *)ptr)->id) {
	case ID_GO:
		UI_ForceMenuOff();
		trap_Cmd_ExecuteText(EXEC_APPEND, va("demo %s\n", s_demos.list.itemnames[s_demos.list.curvalue]));
		break;

	case ID_BACK:
		UI_PopMenu();
		break;

	case ID_SCROLL_UP:
		ScrollList_Key(&s_demos.list, K_DOWNARROW);
		break;

	case ID_SCROLL_DOWN:
		ScrollList_Key(&s_demos.list, K_UPARROW);
		break;
	}
}

/*
=================
UI_DemosMenu_Key
=================
*/
static sfxHandle_t UI_DemosMenu_Key(int key) {

	if (key == K_MWHEELUP) {
		ScrollList_Key(&s_demos.list, K_UPARROW);
	}
	if (key == K_MWHEELDOWN) {
		ScrollList_Key(&s_demos.list, K_DOWNARROW);
	}

	if (key == K_ENTER || key == K_KP_ENTER) {
		UI_ForceMenuOff();
		trap_Cmd_ExecuteText(EXEC_APPEND, va("demo %s\n", s_demos.list.itemnames[s_demos.list.curvalue]));
	}

	return Menu_DefaultKey(&s_demos.menu, key);
}

static int QDECL UI_SortDemos(const void *a, const void *b) {
	const char *ac, *bc;

	ac = *(const char **)a;
	bc = *(const char **)b;

	// FIXME: NULL
	return Q_stricmp(ac, bc);
}

/*
===============
Demos_MenuInit
===============
*/
static void Demos_MenuInit(void) {
	int i, j;
	int len;
	char *demoname, extension[32];

	memset(&s_demos, 0, sizeof(demos_t));
	s_demos.menu.key = UI_DemosMenu_Key;

	Demos_Cache();

	s_demos.menu.fullscreen = qtrue;
	s_demos.menu.wrapAround = qtrue;
	s_demos.menu.bgparts = BGP_DEMOBG | BGP_SIMPLEBG;

	s_demos.left.generic.type = MTYPE_BITMAP1024S;
	s_demos.left.x = 96;  // 367;
	s_demos.left.y = 240; // 524;
	s_demos.left.w = 38;  // 99;
	s_demos.left.h = 98;  // 38;
	s_demos.left.shader = trap_R_RegisterShaderNoMip(ARROWUP0);
	s_demos.left.mouseovershader = trap_R_RegisterShaderNoMip(ARROWUP1);
	s_demos.left.generic.callback = Demos_MenuEvent;
	s_demos.left.generic.id = ID_SCROLL_DOWN;

	s_demos.right.generic.type = MTYPE_BITMAP1024S;
	s_demos.right.x = 96;  // 561;
	s_demos.right.y = 422; // 524;
	s_demos.right.w = 38;  // 98;
	s_demos.right.h = 98;  // 38;
	s_demos.right.shader = trap_R_RegisterShaderNoMip(ARROWDN0);
	s_demos.right.mouseovershader = trap_R_RegisterShaderNoMip(ARROWDN1);
	s_demos.right.generic.callback = Demos_MenuEvent;
	s_demos.right.generic.id = ID_SCROLL_UP;

	s_demos.back.generic.type = MTYPE_BITMAP;
	s_demos.back.generic.name = BACK0;
	s_demos.back.generic.flags = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
	s_demos.back.generic.x = 8;
	s_demos.back.generic.y = 440;
	s_demos.back.generic.id = ID_BACK;
	s_demos.back.generic.callback = Demos_MenuEvent;
	s_demos.back.width = 80;
	s_demos.back.height = 40;
	s_demos.back.focuspic = BACK1;
	s_demos.back.focuspicinstead = qtrue;

	s_demos.go.generic.type = MTYPE_BITMAP1024S;
	s_demos.go.x = 84;	// 815;
	s_demos.go.y = 350; // 633;
	s_demos.go.w = 63;	// 181;
	s_demos.go.h = 63;	// 110;
	s_demos.go.shader = trap_R_RegisterShaderNoMip(GO0);
	s_demos.go.mouseovershader = trap_R_RegisterShaderNoMip(GO1);
	s_demos.go.generic.callback = Demos_MenuEvent;
	s_demos.go.generic.id = ID_GO;

	s_demos.list.generic.type = MTYPE_SCROLLLIST;
	s_demos.list.generic.flags = QMF_PULSEIFFOCUS;
	s_demos.list.generic.callback = Demos_MenuEvent;
	s_demos.list.generic.id = ID_LIST;
	s_demos.list.generic.x = 100;
	s_demos.list.generic.y = 60;
	s_demos.list.width = 28;
	s_demos.list.height = 20;
	s_demos.list.columns = 1;

	demoname = s_demos.names;
	s_demos.list.itemnames = (const char **)s_demos.demolist;

	j = 0;
	while (demo_protocols[j]) {
		int demosToAdd;
		Com_sprintf(extension, sizeof(extension), ".%s%d", DEMOEXT, demo_protocols[j]);
		demosToAdd =
			trap_FS_GetFileList("demos", extension, demoname, ARRAY_LEN(s_demos.names) - (demoname - s_demos.names));

		for (i = 0; i < demosToAdd; i++) {
			s_demos.list.itemnames[s_demos.list.numitems++] = demoname;

			len = strlen(demoname);
			demoname += len + 1;

			if (s_demos.list.numitems >= MAX_DEMOS)
				break;
		}

		if (s_demos.list.numitems >= MAX_DEMOS)
			break;
		j++;
	}

	if (0 == s_demos.list.numitems) {
		s_demos.list.itemnames[0] = "No Demos Found.";
		s_demos.list.numitems = 1;

		// degenerate case, not selectable
		s_demos.go.generic.flags |= (QMF_INACTIVE | QMF_HIDDEN);
	}

	qsort(s_demos.list.itemnames, s_demos.list.numitems, sizeof(s_demos.list.itemnames[0]), UI_SortDemos);

	Menu_AddItem(&s_demos.menu, &s_demos.list);
	Menu_AddItem(&s_demos.menu, &s_demos.left);
	Menu_AddItem(&s_demos.menu, &s_demos.right);
	Menu_AddItem(&s_demos.menu, &s_demos.go);
	Menu_AddItem(&s_demos.menu, &s_demos.back);
}

/*
=================
Demos_Cache
=================
*/
void Demos_Cache(void) {
	trap_R_RegisterShaderNoMip(BACK0);
	trap_R_RegisterShaderNoMip(BACK1);
	trap_R_RegisterShaderNoMip(GO0);
	trap_R_RegisterShaderNoMip(GO1);
}

/*
===============
UI_DemosMenu
===============
*/
void UI_DemosMenu(void) {
	Demos_MenuInit();
	UI_PushMenu(&s_demos.menu);
}
