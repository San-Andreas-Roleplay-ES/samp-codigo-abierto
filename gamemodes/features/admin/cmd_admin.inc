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

new bool:PlayerGodMode[MAX_PLAYERS];

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
        return SendClientMessage(playerid, COLOR_DARKRED, "Que quieres hacer panflin bugear el sv?");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(targetid, x, y, z);
    
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
        return SendClientMessage(playerid, COLOR_DARKRED, "que quieres hacer panflin bugear el sv?");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
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
    
    SetPlayerVirtualWorld(targetid, adminVW);
    SetPlayerInterior(targetid, adminInterior);

    new string[128], targetname[MAX_PLAYER_NAME], adminname[MAX_PLAYER_NAME];
    GetPlayerName(targetid, targetname, sizeof(targetname));
    GetPlayerName(playerid, adminname, sizeof(adminname));
    
    // format(string, sizeof(string), "Has traído a %s (ID: %d) hacia tu posición.", targetname, targetid);
    // SendClientMessage(playerid, COLOR_GRAD2, string);
    
    // format(string, sizeof(string), "Has sido teletransportado por %s.", adminname);
    // SendClientMessage(targetid, COLOR_GRAD2, string);
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
    
    // format(string, sizeof(string), "Has cambiado el skin de %s (ID: %d) al skin ID: %d", targetname, targetid, skinid);
    // SendClientMessage(playerid, COLOR_GRAD2, string);
    
    // format(string, sizeof(string), "%s te ha cambiado el skin al ID: %d", adminname, skinid);
    // SendClientMessage(targetid, COLOR_GRAD2, string);
    return 1;
}

CMD:adelante(playerid, params[])
{
    new Float:metros;
    if(sscanf(params, "f", metros))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /adelante [metros]");

    // if(metros < 0.1 || metros > 500.0)
       // return SendClientMessage(playerid, COLOR_FADE3, "Distancia inválida (0.1-500 metros).");

    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);
    
    x += metros * floatsin(-angle, degrees);
    y += metros * floatcos(-angle, degrees);
    
    SetPlayerPos(playerid, x, y, z);
    // SendClientMessage(playerid, COLOR_GRAD2, va_return("Te moviste %.1f metros hacia adelante.", metros));
    return 1;
}

CMD:atras(playerid, params[])
{
    new Float:metros;
    if(sscanf(params, "f", metros))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /atras [metros]");

    // if(metros < 0.1 || metros > 500.0)
        // return SendClientMessage(playerid, COLOR_FADE3, "Distancia inválida (0.1-500 metros).");

    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);
    
    x -= metros * floatsin(-angle, degrees);
    y -= metros * floatcos(-angle, degrees);
    
    SetPlayerPos(playerid, x, y, z);
    // SendClientMessage(playerid, COLOR_GRAD2, va_return("Te moviste %.1f metros hacia atrás.", metros));
    return 1;
}

CMD:arriba(playerid, params[])
{
    new Float:metros;
    if(sscanf(params, "f", metros))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /arriba [metros]");

    // if(metros < 0.1 || metros > 500.0)
       // return SendClientMessage(playerid, COLOR_FADE3, "Altura inválida (0.1-500 metros).");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    z += metros;
    
    SetPlayerPos(playerid, x, y, z);
    // SendClientMessage(playerid, COLOR_GRAD2, va_return("Te moviste %.1f metros hacia arriba.", metros));
    return 1;
}

CMD:abajo(playerid, params[])
{
    new Float:metros;
    if(sscanf(params, "f", metros))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /abajo [metros]");

    // if(metros < 0.1 || metros > 500.0)
       // return SendClientMessage(playerid, COLOR_FADE3, "Altura inválida (0.1-500 metros).");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    z -= metros;
    
    SetPlayerPos(playerid, x, y, z);
    // SendClientMessage(playerid, COLOR_GRAD2, va_return("Te moviste %.1f metros hacia abajo.", metros));
    return 1;
}

CMD:slap(playerid, params[])
{
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /slap [playerid]");

    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_FADE3, "Jugador no conectado.");

    new Float:x, Float:y, Float:z;
    new targetName[MAX_PLAYER_NAME], adminName[MAX_PLAYER_NAME];
    
    GetPlayerPos(targetid, x, y, z);
    GetPlayerName(targetid, targetName, sizeof(targetName));
    GetPlayerName(playerid, adminName, sizeof(adminName));
    
    SetPlayerPos(targetid, x, y, z + 7.0);
    
    // PlayerPlaySound(playerid, 1190, 0.0, 0.0, 0.0);
    PlayerPlaySound(targetid, 1190, 0.0, 0.0, 0.0);
    
    SendClientMessage(playerid, COLOR_GRAD2, va_return("%s le dio una cachetada a %s.", adminName, targetName));
    
    return 1;
}

alias:godmode("gg")
CMD:godmode(playerid, params[])
{
    if(PlayerGodMode[playerid])
    {
        PlayerGodMode[playerid] = false;
        SetPlayerHealth(playerid, 100.0);
        SendClientMessage(playerid, COLOR_GRAD2, "Godmode desactivado. Ahora puedes recibir daño.");
    }
    else
    {
        PlayerGodMode[playerid] = true;
        SetPlayerHealth(playerid, 9999.0);
        SendClientMessage(playerid, COLOR_GRAD2, "Godmode activado. Eres invencible.");
    }
    return 1;
}

hook OnPlayerConnect(playerid)
{
    PlayerGodMode[playerid] = false;
    return 1;
}

hook OnPlayerTakeDamage(playerid, issuerid, Float:amount, weaponid, bodypart)
{
    if(PlayerGodMode[playerid])
    {
        SetPlayerHealth(playerid, 9999.0);
        return 0;
    }
    return 1;
}

