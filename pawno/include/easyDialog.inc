/*
	easyDialog.inc - Dialogs made easier!

	With this useful include, scripters can easily create
	dialogs and show them to players.

	This include will prevent dialog spoofing, ID collision
	and a lot more.

	Created by Emmet on Friday, January 24, 2014.
*/

#if !defined isnull
	#define isnull(%1) \
		((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))
#endif

#define Dialog:%0(%1) \
	forward dialog_%0(%1); public dialog_%0(%1)

#define Dialog_Show(%0,%1, \
	Dialog_Open(%0, #%1,

#define Dialog_Opened(%0) \
	(CallRemoteFunction("Dialog_IsOpened", "i", (%0)))

static
	s_DialogName[MAX_PLAYERS][32 char],
	s_DialogOpened[MAX_PLAYERS]
;

forward OnDialogPerformed(playerid, dialog[], response, success);

/*forward @dialog_format(); @dialog_format() {
	format("", 0, "");
}*/

forward Dialog_IsOpened(playerid);
public Dialog_IsOpened(playerid)
{
	return (s_DialogOpened[playerid]);
}

stock Dialog_Open(playerid, const dialogFunction[], style, const caption[], const info[], const button1[], const button2[], {Float,_}:...)
{
    if (!strlen(info))
    {
        return 0;
    }
    
    if (info[0] == EOS) {
        return 0;
    }

    if (numargs() < 7) {
        ShowPlayerDialog(playerid, 32700, style, caption, info, button1, button2);
        return 1;
    }

    ShowPlayerDialog(playerid, 32700, style, caption, info, button1, button2);
    strpack(s_DialogName[playerid], dialogFunction, 32 char);

    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	static
		s_Public = cellmax;

	if (s_Public == cellmax)
	{
	    s_Public = funcidx("OnDialogPerformed");
	}

	// Sanitize inputs.
	for (new i = 0, l = strlen(inputtext); i < l; i ++)
	{
	    if (inputtext[i] == '%')
		{
			inputtext[i] = '#';
		}
	}
	if (dialogid == 32700 && strlen(s_DialogName[playerid]) > 0)
	{
		new
		    string[40];

		strcat(string, "dialog_");
		strcat(string, s_DialogName[playerid]);

		HidePlayerDialog(playerid);

		if ((s_Public == -1) || (CallLocalFunction("OnDialogPerformed", "dsdd", playerid, string[7], response, funcidx(string) != -1)))
		{
	 		CallLocalFunction(string, "ddds", playerid, response, listitem, (!inputtext[0]) ? ("\1") : (inputtext));
		}
	}
	#if defined DR_OnDialogResponse
	    return DR_OnDialogResponse(playerid, dialogid, response, listitem, inputtext);
	#else
	    return 0;
	#endif
}

#if defined _ALS_OnDialogResponse
	#undef OnDialogResponse
#else
	#define _ALS_OnDialogResponse
#endif

#define OnDialogResponse DR_OnDialogResponse

#if defined DR_OnDialogResponse
	forward DR_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
#endif