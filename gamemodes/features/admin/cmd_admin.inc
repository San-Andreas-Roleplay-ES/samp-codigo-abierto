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

CMD:ir(playerid, params[])
{
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /ir [playerid]");

    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_FADE3, "El jugador no está conectado.");

    if(targetid == playerid)
        return SendClientMessage(playerid, COLOR_FADE3, "¿Qué quieres hacer panflín, bugear el sv?");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(targetid, x, y, z);
    
    // Obtener Virtual World e Interior del objetivo
    new targetVW = GetPlayerVirtualWorld(targetid);
    new targetInterior = GetPlayerInterior(targetid);
    
    if(IsPlayerInAnyVehicle(playerid))
    {
        new vehicleid = GetPlayerVehicleID(playerid);
        SetVehiclePos(vehicleid, x + 2.0, y, z);
        SetVehicleVirtualWorld(vehicleid, targetVW);
        LinkVehicleToInterior(vehicleid, targetInterior);
    }
    else
    {
        SetPlayerPos(playerid, x + 1.0, y, z + 1.0);
    }
    
    // Actualizar Virtual World e Interior del jugador
    SetPlayerVirtualWorld(playerid, targetVW);
    SetPlayerInterior(playerid, targetInterior);

    new string[128], targetname[MAX_PLAYER_NAME];
    GetPlayerName(targetid, targetname, sizeof(targetname));
    format(string, sizeof(string), "Te has teletransportado hacia %s (ID: %d).", targetname, targetid);
    SendClientMessage(playerid, COLOR_GRAD2, string);
    return 1;
}

CMD:traer(playerid, params[])
{
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /traer [playerid]");

    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_FADE3, "El jugador no está conectado.");

    if(targetid == playerid)
        return SendClientMessage(playerid, COLOR_FADE3, "¿Qué quieres hacer panflín, bugear el sv?");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    // Obtener Virtual World e Interior del admin
    new adminVW = GetPlayerVirtualWorld(playerid);
    new adminInterior = GetPlayerInterior(playerid);
    
    if(IsPlayerInAnyVehicle(targetid))
    {
        new vehicleid = GetPlayerVehicleID(targetid);
        SetVehiclePos(vehicleid, x + 2.0, y, z);
        SetVehicleVirtualWorld(vehicleid, adminVW);
        LinkVehicleToInterior(vehicleid, adminInterior);
    }
    else
    {
        SetPlayerPos(targetid, x + 1.0, y, z + 1.0);
    }
    
    // Actualizar Virtual World e Interior del objetivo
    SetPlayerVirtualWorld(targetid, adminVW);
    SetPlayerInterior(targetid, adminInterior);

    new string[128], targetname[MAX_PLAYER_NAME], adminname[MAX_PLAYER_NAME];
    GetPlayerName(targetid, targetname, sizeof(targetname));
    GetPlayerName(playerid, adminname, sizeof(adminname));
    
    format(string, sizeof(string), "Has traído a %s (ID: %d) hacia tu posición.", targetname, targetid);
    SendClientMessage(playerid, COLOR_GRAD2, string);
    
    format(string, sizeof(string), "Has sido teletransportado por %s.", adminname);
    SendClientMessage(targetid, COLOR_GRAD2, string);
    return 1;
}

CMD:darskin(playerid, params[])
{
    new targetid, skinid;
    if(sscanf(params, "ud", targetid, skinid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /darskin [playerid] [skinid]");

    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_FADE3, "El jugador no está conectado.");

    if(skinid < 0 || skinid > 311)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de skin inválido (0-311).");

    SetPlayerSkin(targetid, skinid);

    new string[128], targetname[MAX_PLAYER_NAME], adminname[MAX_PLAYER_NAME];
    GetPlayerName(targetid, targetname, sizeof(targetname));
    GetPlayerName(playerid, adminname, sizeof(adminname));
    
    format(string, sizeof(string), "Has cambiado el skin de %s (ID: %d) al skin ID: %d", targetname, targetid, skinid);
    SendClientMessage(playerid, COLOR_GRAD2, string);
    
    format(string, sizeof(string), "%s te ha cambiado el skin al ID: %d", adminname, skinid);
    SendClientMessage(targetid, COLOR_GRAD2, string);
    return 1;
}