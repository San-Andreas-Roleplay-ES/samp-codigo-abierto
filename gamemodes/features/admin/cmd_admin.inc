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

#include <YSI_Coding\y_hooks>

hook OnPlayerSpawn(playerid)
{
    return SendClientMessage(playerid, COLOR_YELLOW, "Comandos disponibles: /irpos, /crearveh, /creararma, /hora, /clima");
}

CMD:hora(playerid, params[])
{
	if(sscanf(params, "d", params[0]))
		return SendClientMessage(playerid, COLOR_GRAD2, "USO: /hora [hora]");

	if(params[0] < 0 || params[0] > 23)
		return SendClientMessage(playerid, COLOR_FADE3, "Hora inválida.");

	SetWorldTime(params[0]);
	return 1;
}

CMD:clima(playerid, params[])
{
	if(sscanf(params, "d", params[0]))
		return SendClientMessage(playerid, COLOR_GRAD2, "USO: /clima [clima]");

    if(params[0] < 0 || params[0] > 20)
        return SendClientMessage(playerid, COLOR_FADE3, "Clima inválido.");

	SetWeather(params[0]);
	return 1;
}

CMD:irpos(playerid, params[])
{
	new Float:posX, Float:posY, Float:posZ, vw, interior;

    if(sscanf(params, "fffD(0)D(0)", posX, posY, posZ, vw, interior)) 
		return SendClientMessage(playerid, COLOR_GRAD2, "USO: /irpos [x] [y] [z] [virtualworld] [interior]");

	SetPlayerPos(playerid, posX, posY, posZ);
	SetPlayerInterior(playerid, interior);
	SetPlayerVirtualWorld(playerid, vw);

	SendClientMessage(playerid, COLOR_GRAD1, "Has sido teletransportado a las coordenadas.");
    return 1;
}

CMD:crearveh(playerid, params[])
{
    if(sscanf(params, "k<vehicle>ddD(0)", params[0], params[1], params[2], params[3]))
		return SendClientMessage(playerid, COLOR_GRAD2, "USO: /crearveh [modelid] [col1] [col2] [sirena (opcional)]");

    if(params[0] == -1 || params[0] == 449 || params[0] == 537 || params[0] == 538 || params[0] == 569 || params[0] == 570 || params[0] == 590)
        return SendClientMessage(playerid, COLOR_FADE3, "Modelo de vehículo inválido.");

	new Float: x, Float: y, Float: z, Float: a;
	GetPlayerPos(playerid, x, y, z);
	GetPlayerFacingAngle(playerid, a);

	new vehicleid = CreateVehicle(params[0], x, y, z, a, params[1], params[2], params[3] == 1 ? 0 : -1);
	PutPlayerInVehicle(playerid, vehicleid, 0);
	return 1;
}

CMD:creararma(playerid, params[])
{

	if(sscanf(params, "k<weapon>d", params[0], params[1]))
		return SendClientMessage(playerid, COLOR_GRAD2, "USO: /creararma [modelid] [ammo] [skill]");

	if(params[0] < 1 || params[0] > 46)
		return SendClientMessage(playerid, COLOR_FADE3, "Modelo de arma inválido.");

	GivePlayerWeapon(playerid, params[0], params[1]);
	return 1;
}

CMD:jetpack(playerid)
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar el jetpack dentro de un vehículo.");

    SetPlayerSpecialAction(playerid, SPECIAL_ACTION_USEJETPACK);
    return SendClientMessage(playerid, COLOR_GRAD1, "Jetpack creado.");
}