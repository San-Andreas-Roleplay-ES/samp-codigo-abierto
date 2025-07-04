// SAN ANDREAS ROLEPLAY
// Copyright (c) 2021 - 2025 pigeon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

public OnPlayerConnect(playerid)
{
    SetPlayerColor(playerid, INVISIBLE);

    for(new i = 0; i < 50; i++)
    {
        SendClientMessage(playerid, -1, " ");
    }

    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof(name));

    SendClientMessageToAll(COLOR_GRAD1, va_return("{ED801A}%s se ha conectado al servidor.", name));
    return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
	if(IsPlayerNPC(playerid))
        return 1;

    SetPlayerInterior(playerid, 11);
    SetPlayerPos(playerid, 508.7362, -87.4335, 998.9609);
    SetPlayerFacingAngle(playerid, 0.0);
    SetPlayerCameraPos(playerid, 508.7362, -83.4335, 998.9609);
    SetPlayerCameraLookAt(playerid, 508.7362, -87.4335, 998.9609);
	return 1;
}

public OnPlayerRequestSpawn(playerid)
{
    if(IsPlayerNPC(playerid))
        return 1;

    return 1;
}

public OnPlayerSpawn(playerid)
{
    if(IsPlayerNPC(playerid))
        return 1;

    SetPlayerPos(playerid, 1544.3810, -1675.4711, 13.5583);
    SetPlayerFacingAngle(playerid, 90.0);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);

    GameTextForPlayer(playerid,"~p~"SERVER_DISCORD"", 5000, 4);

    SendClientMessage(playerid, COLOR_GRAD1, "¡Bienvenido al servidor! Usa /ayuda para ver los comandos disponibles.");
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    // Integrar sistema de lumberjack
    Lumberjack_OnPlayerDisconnect(playerid, reason);
    return 1;
}