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
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "../ui/ui_shared.h"
#ifdef MISSIONPACK
extern menuDef_t *menuScoreboard;
#endif

void CG_PrintClientNumbers(void) {
	int i;

	CG_Printf("slot score ping name\n");
	CG_Printf("---- ----- ---- ----\n");

	for (i = 0; i < cg.numScores; i++) {
		CG_Printf("%-4d", cg.scores[i].client);

		CG_Printf(" %-5d", cg.scores[i].score);

		CG_Printf(" %-4d", cg.scores[i].ping);

		CG_Printf(" %s\n", cgs.clientinfo[cg.scores[i].client].name);
	}
}

void CG_TargetCommand_f(void) {
	int targetNum;
	char test[4];

	targetNum = CG_CrosshairPlayer();
	if (targetNum == -1) {
		return;
	}

	trap_Argv(1, test, 4);
	trap_SendClientCommand(va("gc %i %i", targetNum, atoi(test)));
}

/*
=================
CG_SizeUp_f

Keybinding command
=================
 */
static void CG_SizeUp_f(void) {
	trap_Cvar_Set("cg_viewsize", va("%i", (int) (cg_viewsize.integer + 10)));
}

/*
=================
CG_SizeDown_f

Keybinding command
=================
 */
static void CG_SizeDown_f(void) {
	trap_Cvar_Set("cg_viewsize", va("%i", (int) (cg_viewsize.integer - 10)));
}

/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
 */
static void CG_Viewpos_f(void) {
	CG_Printf("(%i %i %i) : %i\n", (int) cg.refdef.vieworg[0],
			(int) cg.refdef.vieworg[1], (int) cg.refdef.vieworg[2],
			(int) cg.refdefViewAngles[YAW]);
}

static void CG_ScoresDown_f(void) {

#ifdef MISSIONPACK
	CG_BuildSpectatorString();
#endif
	if (cg.scoresRequestTime + 2000 < cg.time) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand("score");

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if (!cg.showScores) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f(void) {
	if (cg.showScores) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}

static void CG_AccDown_f(void) {

	if (cg.accRequestTime + 2000 < cg.time) {

		cg.accRequestTime = cg.time;
		trap_SendClientCommand("acc");

		if (!cg.showAcc) {
			cg.showAcc = qtrue;
		}

	} else {
		cg.showAcc = qtrue;
	}
}

static void CG_AccUp_f(void) {
	if (cg.showAcc) {
		cg.showAcc = qfalse;
		cg.accFadeTime = cg.time;
	}
}


#ifdef MISSIONPACK
extern menuDef_t *menuScoreboard;
void Menu_Reset(void); // FIXME: add to right include file

static void CG_LoadHud_f(void) {
	char buff[1024];
	const char *hudSet;
	memset(buff, 0, sizeof (buff));

	String_Init();
	Menu_Reset();

	trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof (buff));
	hudSet = buff;
	if (hudSet[0] == '\0') {
		hudSet = "ui/hud.txt";
	}

	CG_LoadMenus(hudSet);
	menuScoreboard = NULL;
}

static void CG_scrollScoresDown_f(void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qtrue);
	}
}

static void CG_scrollScoresUp_f(void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qfalse);
	}
}

static void CG_spWin_f(void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
	CG_AddBufferedSound(cgs.media.winnerSound);
	//trap_S_StartLocalSound(cgs.media.winnerSound, CHAN_ANNOUNCER);
	CG_CenterPrint("YOU WIN!", SCREEN_HEIGHT * .30, 0);

	// leilei - Unlock stuff!!! Trophies crap.
	{
		char berf[4];
		//const char	*info;
		int trophyearn;
		int trophyhad;

		// trophyearn = 1 = GOLD!!!
		// trophyearn = 2 = silver!!
		// trophyearn = 3 = bronze...
		// trophyearn = 5 = PLATINUM!!!!!!!!!!!!!!!!!!!!


		trophyearn = 1; // gold... if we're good


		//info = CG_ConfigString( CS_SERVERINFO );
		//s = Info_ValueForKey( info, "mapname" );
		trap_Cvar_VariableStringBuffer("ui_currentMap", berf, sizeof (berf)); // get map number instead for list consistency

		trophyhad = CG_GetCVar(va("ui_sp_unlock_%s", berf));

		if (trophyhad > trophyearn) {
			trophyearn = 0;
		}

		// leilei - unlocking maps (for the SP UI) by setting a cvar
		if (trophyearn) {
			if (trophyearn == 1) {
				CG_CenterPrint("Here the player\nhave a gold trophy!", SCREEN_HEIGHT * .40, 0);
			} else if (trophyearn == 2) {
				CG_CenterPrint("Here the player\nhave a silver trophy", SCREEN_HEIGHT * .40, 0);
			} else if (trophyearn == 3) {
				CG_CenterPrint("You earned bronze.\nNOT GOOD ENOUGH DAMMIT\nNOT GOOD ENOUGH", SCREEN_HEIGHT * .40, 0);
			}
			trap_Cvar_Set(va("ui_sp_unlock_%s", berf), va("%i", trophyearn)); // YA YUO DID IT!!!1
		}

		// leilei - get all the total trophies. Should really be done in the single player ui scripts, but
		// 		doing it here could make a nice verifier for legitimacy :)
		{
			int tropees = 0;
			int tropgold = 0;
			int tropsilv = 0;
			int tropbrnz = 0;

			for (tropees = 0; tropees < 42; tropees++) {
				int yeah;
				yeah = CG_GetCVar(va("ui_sp_unlock_%i", tropees));
				if (yeah == 1)
					tropgold++;
				if (yeah == 2)
					tropsilv++;
				if (yeah == 3)
					tropbrnz++;
			}
			trap_Cvar_Set("ui_sp_trophies_gold", va("%i", tropgold));
			trap_Cvar_Set("ui_sp_trophies_silver", va("%i", tropsilv));
			trap_Cvar_Set("ui_sp_trophies_bronze", va("%i", tropbrnz));
		}
	}
}

static void CG_spLose_f(void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
	CG_AddBufferedSound(cgs.media.loserSound);
	//trap_S_StartLocalSound(cgs.media.loserSound, CHAN_ANNOUNCER);
	CG_CenterPrint("YOU LOSE...", SCREEN_HEIGHT * .30, 0);
}

#endif

static void CG_TellTarget_f(void) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_CrosshairPlayer();
	if (clientNum == -1) {
		return;
	}

	trap_Args(message, 128);
	Com_sprintf(command, 128, "tell %i %s", clientNum, message);
	trap_SendClientCommand(command);
}

static void CG_TellAttacker_f(void) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_LastAttacker();
	if (clientNum == -1) {
		return;
	}

	trap_Args(message, 128);
	Com_sprintf(command, 128, "tell %i %s", clientNum, message);
	trap_SendClientCommand(command);
}

static void CG_VoiceTellTarget_f(void) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_CrosshairPlayer();
	if (clientNum == -1) {
		return;
	}

	trap_Args(message, 128);
	Com_sprintf(command, 128, "vtell %i %s", clientNum, message);
	trap_SendClientCommand(command);
}

static void CG_VoiceTellAttacker_f(void) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_LastAttacker();
	if (clientNum == -1) {
		return;
	}

	trap_Args(message, 128);
	Com_sprintf(command, 128, "vtell %i %s", clientNum, message);
	trap_SendClientCommand(command);
}

#ifdef MISSIONPACK

static void CG_NextTeamMember_f(void) {
	CG_SelectNextPlayer();
}

static void CG_PrevTeamMember_f(void) {
	CG_SelectPrevPlayer();
}

// ASS U ME's enumeration order as far as task specific orders, OFFENSE is zero, CAMP is last
//

static void CG_NextOrder_f(void) {
	clientInfo_t *ci = cgs.clientinfo + cg.snap->ps.clientNum;
	if (ci) {
		if (!ci->teamLeader && sortedTeamPlayers[cg_currentSelectedPlayer.integer] != cg.snap->ps.clientNum) {
			return;
		}
	}
	if (cgs.currentOrder < TEAMTASK_CAMP) {
		cgs.currentOrder++;

		if (cgs.currentOrder == TEAMTASK_RETRIEVE) {
			if (!CG_OtherTeamHasFlag()) {
				cgs.currentOrder++;
			}
		}

		if (cgs.currentOrder == TEAMTASK_ESCORT) {
			if (!CG_YourTeamHasFlag()) {
				cgs.currentOrder++;
			}
		}

	} else {
		cgs.currentOrder = TEAMTASK_OFFENSE;
	}
	cgs.orderPending = qtrue;
	cgs.orderTime = cg.time + 3000;
}

static void CG_ConfirmOrder_f(void) {
	trap_SendConsoleCommand(va("cmd vtell %d %s\n", cgs.acceptLeader, VOICECHAT_YES));
	trap_SendConsoleCommand("+button5; wait; -button5");
	if (cg.time < cgs.acceptOrderTime) {
		trap_SendClientCommand(va("teamtask %d\n", cgs.acceptTask));
		cgs.acceptOrderTime = 0;
	}
}

static void CG_DenyOrder_f(void) {
	trap_SendConsoleCommand(va("cmd vtell %d %s\n", cgs.acceptLeader, VOICECHAT_NO));
	trap_SendConsoleCommand("+button6; wait; -button6");
	if (cg.time < cgs.acceptOrderTime) {
		cgs.acceptOrderTime = 0;
	}
}

static void CG_TaskOffense_f(void) {
	if (CG_UsesTeamFlags(cgs.gametype)) {
		trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONGETFLAG));
	} else {
		trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONOFFENSE));
	}
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_OFFENSE));
}

static void CG_TaskDefense_f(void) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONDEFENSE));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_DEFENSE));
}

static void CG_TaskPatrol_f(void) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONPATROL));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_PATROL));
}

static void CG_TaskCamp_f(void) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONCAMPING));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_CAMP));
}

static void CG_TaskFollow_f(void) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONFOLLOW));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_FOLLOW));
}

static void CG_TaskRetrieve_f(void) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONRETURNFLAG));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_RETRIEVE));
}

static void CG_TaskEscort_f(void) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_ONFOLLOWCARRIER));
	trap_SendClientCommand(va("teamtask %d\n", TEAMTASK_ESCORT));
}

static void CG_TaskOwnFlag_f(void) {
	trap_SendConsoleCommand(va("cmd vsay_team %s\n", VOICECHAT_IHAVEFLAG));
}

static void CG_TauntKillInsult_f(void) {
	trap_SendConsoleCommand("cmd vsay kill_insult\n");
}

static void CG_TauntPraise_f(void) {
	trap_SendConsoleCommand("cmd vsay praise\n");
}

static void CG_TauntTaunt_f(void) {
	trap_SendConsoleCommand("cmd vtaunt\n");
}

static void CG_TauntDeathInsult_f(void) {
	trap_SendConsoleCommand("cmd vsay death_insult\n");
}

static void CG_TauntGauntlet_f(void) {
	trap_SendConsoleCommand("cmd vsay kill_gauntlet\n");
}

static void CG_TaskSuicide_f(void) {
	int clientNum;
	char command[128];

	clientNum = CG_CrosshairPlayer();
	if (clientNum == -1) {
		return;
	}

	Com_sprintf(command, 128, "tell %i suicide", clientNum);
	trap_SendClientCommand(command);
}



/*
==================
CG_TeamMenu_f
==================
 */
/*
static void CG_TeamMenu_f( void ) {
	if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
		CG_EventHandling(CGAME_EVENT_NONE);
		trap_Key_SetCatcher(0);
	} else {
		CG_EventHandling(CGAME_EVENT_TEAMMENU);
		//trap_Key_SetCatcher(KEYCATCH_CGAME);
	}
}
 */

/*
==================
CG_EditHud_f
==================
 */
/*
static void CG_EditHud_f( void ) {
	//cls.keyCatchers ^= KEYCATCH_CGAME;
	//VM_Call (cgvm, CG_EVENT_HANDLING, (cls.keyCatchers & KEYCATCH_CGAME) ? CGAME_EVENT_EDITHUD : CGAME_EVENT_NONE);
}
 */

#endif

/*
==================
CG_StartOrbit_f
==================
 */

static void CG_StartOrbit_f(void) {
	if (!cg_developer.integer) {
		return;
	}
	if (cg_cameraOrbit.value != 0) {
		trap_Cvar_Set("cg_cameraOrbit", "0");
		trap_Cvar_Set("cg_thirdPerson", "0");
	} else {
		trap_Cvar_Set("cg_cameraOrbit", "5");
		trap_Cvar_Set("cg_thirdPerson", "1");
		trap_Cvar_Set("cg_thirdPersonAngle", "0");
		trap_Cvar_Set("cg_thirdPersonRange", "100");
	}
}

/*
static void CG_Camera_f( void ) {
	char name[1024];
	trap_Argv( 1, name, sizeof(name));
	if (trap_loadCamera(name)) {
		cg.cameraMode = qtrue;
		trap_startCamera(cg.time);
	} else {
		CG_Printf ("Unable to load camera %s\n",name);
	}
}
 */


typedef struct {
	char *cmd;
	void (*function)(void);
} consoleCommand_t;

static consoleCommand_t commands[] = {
	{ "testgun", CG_TestGun_f},
	{ "testmodel", CG_TestModel_f},
	{ "nextframe", CG_TestModelNextFrame_f},
	{ "prevframe", CG_TestModelPrevFrame_f},
	{ "nextskin", CG_TestModelNextSkin_f},
	{ "prevskin", CG_TestModelPrevSkin_f},
	{ "viewpos", CG_Viewpos_f},
	{ "+scores", CG_ScoresDown_f},
	{ "-scores", CG_ScoresUp_f},
	{ "+zoom", CG_ZoomDown_f},
	{ "-zoom", CG_ZoomUp_f},
	{ "sizeup", CG_SizeUp_f},
	{ "sizedown", CG_SizeDown_f},
	{ "weapbest", CG_BestWeapon_f},
	{ "weapnext", CG_NextWeapon_f},
	{ "weapprev", CG_PrevWeapon_f},
	{ "weapon", CG_Weapon_f},
	{ "tell_target", CG_TellTarget_f},
	{ "tell_attacker", CG_TellAttacker_f},
	{ "vtell_target", CG_VoiceTellTarget_f},
	{ "vtell_attacker", CG_VoiceTellAttacker_f},
	{ "tcmd", CG_TargetCommand_f},
#ifdef MISSIONPACK
	{ "loadhud", CG_LoadHud_f},
	{ "nextTeamMember", CG_NextTeamMember_f},
	{ "prevTeamMember", CG_PrevTeamMember_f},
	{ "nextOrder", CG_NextOrder_f},
	{ "confirmOrder", CG_ConfirmOrder_f},
	{ "denyOrder", CG_DenyOrder_f},
	{ "taskOffense", CG_TaskOffense_f},
	{ "taskDefense", CG_TaskDefense_f},
	{ "taskPatrol", CG_TaskPatrol_f},
	{ "taskCamp", CG_TaskCamp_f},
	{ "taskFollow", CG_TaskFollow_f},
	{ "taskRetrieve", CG_TaskRetrieve_f},
	{ "taskEscort", CG_TaskEscort_f},
	{ "taskSuicide", CG_TaskSuicide_f},
	{ "taskOwnFlag", CG_TaskOwnFlag_f},
	{ "tauntKillInsult", CG_TauntKillInsult_f},
	{ "tauntPraise", CG_TauntPraise_f},
	{ "tauntTaunt", CG_TauntTaunt_f},
	{ "tauntDeathInsult", CG_TauntDeathInsult_f},
	{ "tauntGauntlet", CG_TauntGauntlet_f},
	{ "spWin", CG_spWin_f},
	{ "spLose", CG_spLose_f},
	{ "scoresDown", CG_scrollScoresDown_f},
	{ "scoresUp", CG_scrollScoresUp_f},
#endif
	{ "startOrbit", CG_StartOrbit_f},
	//	{ "camera", CG_Camera_f },
	{ "loaddeferred", CG_LoadDeferredPlayers},
	{ "+acc", CG_AccDown_f},
	{ "-acc", CG_AccUp_f},
	{ "clients", CG_PrintClientNumbers}
};

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
 */
qboolean CG_ConsoleCommand(void) {
	const char *cmd;
	int i;

	cmd = CG_Argv(0);

	for (i = 0; i < sizeof ( commands) / sizeof ( commands[0]); i++) {
		if (Q_strequal(cmd, commands[i].cmd)) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
 */
void CG_InitConsoleCommands(void) {
	int i;

	for (i = 0; i < sizeof ( commands) / sizeof ( commands[0]); i++) {
		trap_AddCommand(commands[i].cmd);
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand("kill");
	trap_AddCommand("say");
	trap_AddCommand("say_team");
	trap_AddCommand("tell");
	trap_AddCommand("vsay");
	trap_AddCommand("vsay_team");
	trap_AddCommand("vtell");
	trap_AddCommand("vtaunt");
	trap_AddCommand("vosay");
	trap_AddCommand("vosay_team");
	trap_AddCommand("votell");
	trap_AddCommand("give");
	trap_AddCommand("god");
	trap_AddCommand("notarget");
	trap_AddCommand("noclip");
	trap_AddCommand("team");
	trap_AddCommand("follow");
	trap_AddCommand("levelshot");
	trap_AddCommand("addbot");
	trap_AddCommand("setviewpos");
	trap_AddCommand("callvote");
	trap_AddCommand("cv");
	trap_AddCommand("getmappage");
	trap_AddCommand("vote");
	trap_AddCommand("callteamvote");
	trap_AddCommand("teamvote");
	trap_AddCommand("stats");
	trap_AddCommand("teamtask");
	trap_AddCommand("loaddefered"); // spelled wrong, but not changing for demo
	/* Origami Mod */
	trap_AddCommand("dropammo");
	trap_AddCommand("dropweapon");
}
