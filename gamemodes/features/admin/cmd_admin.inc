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

#include "features/admin/objects.inc"
#include "features/admin/teleports.inc"
#include "features/admin/coordenadas.inc"
#include "features/admin/anims.inc"

CMD:hora(playerid, params[])
{
    new hora;
    if(sscanf(params, "d", hora))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /hora [hora] (0-23)");

    if(hora < 0 || hora > 23)
        return SendClientMessage(playerid, COLOR_FADE3, "Hora inv�lida.");

    SetWorldTime(hora);
    return 1;
}

CMD:clima(playerid, params[])
{
    new weatherid;
    if(sscanf(params, "d", weatherid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /clima [weatherid] (0-20)");

    if(weatherid < 0 || weatherid > 20)
        return SendClientMessage(playerid, COLOR_FADE3, "Clima inv�lido.");

    SetWeather(weatherid);
    return 1;
}

CMD:crearveh(playerid, params[])
{
    new vehiclemodelid, color1, color2, sirena;
    if(sscanf(params, "dddD(0)", vehiclemodelid, color1, color2, sirena))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /crearveh [vehicleid] [color1] [color2] [sirena (opcional)]");

    if(vehiclemodelid < 400 || vehiclemodelid > 611 ||
        vehiclemodelid == 449 || vehiclemodelid == 537 || vehiclemodelid == 538 ||
        vehiclemodelid == 569 || vehiclemodelid == 570 || vehiclemodelid == 590)
    {
        return SendClientMessage(playerid, COLOR_FADE3, "Modelo de veh�culo inv�lido.");
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    new sampvehicleid = CreateVehicle(vehiclemodelid, x, y, z, a, color1, color2, sirena == 1 ? 0 : -1);
    PutPlayerInVehicle(playerid, sampvehicleid, 0);

    SendClientMessage(playerid, COLOR_GRAD2, va_return("Veh�culo creado con: Model ID: %d | Samp ID: %d ", vehiclemodelid, sampvehicleid));
    return 1;
}

CMD:creararma(playerid, params[])
{
    new weaponid, ammo, skill;
    if(sscanf(params, "k<weapon>dd", weaponid, ammo, skill))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /creararma [weaponid] [ammo] [skill] (1-46)");

    if(weaponid < 1 || weaponid > 46)
        return SendClientMessage(playerid, COLOR_FADE3, "Modelo de arma inv�lido.");

    GivePlayerWeapon(playerid, weaponid, ammo);
    SetPlayerSkillLevel(playerid, weaponid, skill);

    SendClientMessage(playerid, COLOR_GRAD2, va_return("Arma creada con �xito: ID %d, Munici�n: %d, Habilidad: %d", weaponid, ammo, skill));
    return 1;
}

CMD:jetpack(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar el jetpack dentro de un veh�culo.");

    SetPlayerSpecialAction(playerid, SPECIAL_ACTION_USEJETPACK);
    SendClientMessage(playerid, COLOR_GRAD2, "Jetpack creado.");
    return 1;
}

CMD:limpiarchat(playerid, params[])
{
    for(new i = 0; i < 50; i++)
    {
        SendClientMessage(playerid, -1, " ");
    }
    return 1;
}

CMD:ropa(playerid, params[])
{
    new skinid;
    if(sscanf(params, "d", skinid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /ropa [skinid]");

    if(skinid < 0 || skinid > 311)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de skin inv�lido (0-311).");

    SetPlayerSkin(playerid, skinid);
    return 1;
}