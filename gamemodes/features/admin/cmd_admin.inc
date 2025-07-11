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
#include "features/admin/vehicles.inc"

CMD:hora(playerid, params[])
{
    new hora;
    if(sscanf(params, "d", hora))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /hora [hora] (0-23)");

    if(hora < 0 || hora > 23)
        return SendClientMessage(playerid, COLOR_FADE3, "Hora inválida.");

    SetWorldTime(hora);
    return 1;
}

CMD:clima(playerid, params[])
{
    new weatherid;
    if(sscanf(params, "d", weatherid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /clima [weatherid] (0-20)");

    if(weatherid < 0 || weatherid > 20)
        return SendClientMessage(playerid, COLOR_FADE3, "Clima inválido.");

    SetWeather(weatherid);
    return 1;
}

CMD:creararma(playerid, params[])
{
    new weaponid, ammo, skill;
    if(sscanf(params, "k<weapon>dd", weaponid, ammo, skill))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /creararma [weaponid] [ammo] [skill] (1-46)");

    if(weaponid < 1 || weaponid > 46)
        return SendClientMessage(playerid, COLOR_FADE3, "Modelo de arma inválido.");

    GivePlayerWeapon(playerid, weaponid, ammo);
    SetPlayerSkillLevel(playerid, weaponid, skill);

    SendClientMessage(playerid, COLOR_GRAD2, va_return("Arma creada con éxito: ID %d, Munición: %d, Habilidad: %d", weaponid, ammo, skill));
    return 1;
}

CMD:jetpack(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar el jetpack dentro de un vehículo.");

    SetPlayerSpecialAction(playerid, SPECIAL_ACTION_USEJETPACK);
    SendClientMessage(playerid, COLOR_GRAD2, "Jetpack creado.");
    return 1;
}

alias:ropa("skin")
CMD:ropa(playerid, params[])
{
    new skinid;
    if(sscanf(params, "d", skinid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /ropa [skinid]");

    if(skinid < 0 || skinid > 311)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de skin inválido (0-311).");

    SetPlayerSkin(playerid, skinid);
    return 1;
}