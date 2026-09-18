// ODP TextDraw proposal for the public SARP development base.
// Queries are fixtures: this repository has no production police database.
#include <YSI_Coding\y_hooks>
#include "features/odp/core.inc"

// Public sandbox policy. Replace with the production faction/duty check.
stock bool:ODPCanUse(playerid)
{
    return IsPlayerConnected(playerid) && !IsPlayerNPC(playerid)
        && GetPlayerState(playerid) != PLAYER_STATE_NONE
        && GetPlayerState(playerid) != PLAYER_STATE_WASTED
        && GetPlayerState(playerid) != PLAYER_STATE_SPECTATING;
}

CMD:odp(playerid, params[])
{
    #pragma unused params
    if (ODPOpen[playerid]) return ODPClose(playerid);
    if (!ODPCanUse(playerid)) return SendClientMessage(playerid, -1, "ODP: aparece en el mundo antes de abrir el terminal.");
    return ODPRender(playerid, 0);
}

CMD:odphelp(playerid, params[])
{
    #pragma unused params
    return ODPCommand(playerid, "/odphelp");
}

Dialog:ODP_SEARCH_INPUT(playerid, response, listitem, inputtext[])
{
    #pragma unused listitem
    if (!ODPCanUse(playerid)) return ODPClose(playerid);
    return ODPSearchResponse(playerid, response, inputtext);
}

Dialog:ODP_EMISSION_INPUT(playerid, response, listitem, inputtext[])
{
    #pragma unused listitem
    if (!ODPCanUse(playerid)) return ODPClose(playerid);
    return ODPEmissionResponse(playerid, response, inputtext);
}

hook OnGameModeInit()
{
    ODPInit();
    return 1;
}

hook OnGameModeExit()
{
    ODPExit();
    return 1;
}

hook OnPlayerConnect(playerid)
{
    ODPConnect(playerid);
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
    ODPDisconnect(playerid, reason);
    return 1;
}

hook OnPlayerDeath(playerid, killerid, reason)
{
    if (ODPOpen[playerid]) ODPClose(playerid);
    return 1;
}

hook OnPlayerSpawn(playerid)
{
    if (ODPOpen[playerid]) ODPClose(playerid);
    return 1;
}

hook OnPlayerClickTextDraw(playerid, Text:clickedid)
{
    if (ODPClickGlobal(playerid, clickedid)) return Y_HOOKS_BREAK_RETURN_1;
    return 0;
}

hook OnPlayerClickPlayerTD(playerid, PlayerText:playertextid)
{
    if (!ODPOpen[playerid]) return 0;
    if (!ODPCanUse(playerid))
    {
        ODPClose(playerid);
        return Y_HOOKS_BREAK_RETURN_1;
    }
    if (ODPClickPlayer(playerid, playertextid)) return Y_HOOKS_BREAK_RETURN_1;
    return 0;
}
