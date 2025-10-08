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

#define HARVESTER_MODEL_ID      532

enum HarvesterPlayerData {
    bool:isActive,
    vehicleNotification
}

new HarvesterPlayer[MAX_PLAYERS][HarvesterPlayerData];

stock ResetHarvesterPlayer(playerid)
{
    HarvesterPlayer[playerid][isActive] = false;
    HarvesterPlayer[playerid][vehicleNotification] = -1;
}

stock bool:IsPlayerInHarvester(playerid)
{
    if(!IsPlayerInAnyVehicle(playerid)) return false;
    
    new vehicleid = GetPlayerVehicleID(playerid);
    if(!IsValidVehicle(vehicleid)) return false;
    
    return (GetVehicleModel(vehicleid) == HARVESTER_MODEL_ID && GetPlayerState(playerid) == PLAYER_STATE_DRIVER);
}

stock ShowHarvesterVehicleNotif(playerid)
{
    if(HarvesterPlayer[playerid][vehicleNotification] != -1)
    {
        CallLocalFunction("hideTDN", "ii", playerid, HarvesterPlayer[playerid][vehicleNotification]);
        HarvesterPlayer[playerid][vehicleNotification] = -1;
    }
    
    // Solo mostrar el mensaje si el trabajo NO está activo
    if(!HarvesterPlayer[playerid][isActive])
    {
        new message[128];
        format(message, sizeof(message), "Presiona 2 para iniciar el trabajo de cosechador.");
        HarvesterPlayer[playerid][vehicleNotification] = CallLocalFunction("ShowTDN_Manual", "is", playerid, message);
    }
}

stock HideHarvesterVehicleNotif(playerid)
{
    if(HarvesterPlayer[playerid][vehicleNotification] != -1)
    {
        CallLocalFunction("hideTDN", "ii", playerid, HarvesterPlayer[playerid][vehicleNotification]);
        HarvesterPlayer[playerid][vehicleNotification] = -1;
    }
}

forward UpdateHarvesterNotification(playerid);
public UpdateHarvesterNotification(playerid)
{
    if(IsPlayerConnected(playerid) && IsPlayerInHarvester(playerid))
    {
        ShowHarvesterVehicleNotif(playerid);
    }
}

forward CheckHarvesterVehicleProximity();
public CheckHarvesterVehicleProximity()
{
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(!IsPlayerConnected(i)) continue;
        
        if(IsPlayerInHarvester(i))
        {
            if(HarvesterPlayer[i][vehicleNotification] == -1)
            {
                ShowHarvesterVehicleNotif(i);
            }
        }
        else
        {
            // Si el jugador no está en una cosechadora pero tiene un trabajo activo, cancelarlo automáticamente
            if(HarvesterPlayer[i][isActive])
            {
                HideHarvesterVehicleNotif(i);
                ResetHarvesterPlayer(i);
                SendClientMessage(i, COLOR_FADE3, "[DEBUG] Trabajo finalizado - saliste del vehículo.");
            }
            else if(HarvesterPlayer[i][vehicleNotification] != -1)
            {
                HideHarvesterVehicleNotif(i);
            }
        }
    }
}

stock StartHarvesterMinigame(playerid)
{
    if(!IsPlayerInHarvester(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "[DEBUG] Error: No estás en una cosechadora.");
        return 0;
    }
    
    if(HarvesterPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "[DEBUG] Error: Trabajo ya activo.");
        return 0;
    }
    
    HarvesterPlayer[playerid][isActive] = true;
    
    // Ocultar la notificación ya que el trabajo está activo
    HideHarvesterVehicleNotif(playerid);
    
    SendClientMessage(playerid, COLOR_GREEN, "[DEBUG] Trabajo iniciado - cosechador activo.");
    
    return 1;
}

stock CancelHarvesterMinigame(playerid)
{
    if(!HarvesterPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "[DEBUG] Error: No hay trabajo activo.");
        return 0;
    }
    
    HideHarvesterVehicleNotif(playerid);
    
    ResetHarvesterPlayer(playerid);
    
    if(IsPlayerInHarvester(playerid))
    {
        SetTimerEx("UpdateHarvesterNotification", 100, false, "i", playerid);
    }
    
    SendClientMessage(playerid, COLOR_FADE3, "[DEBUG] Trabajo cancelado.");
    return 1;
}

hook OnScriptInit()
{
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        ResetHarvesterPlayer(i);
    }
    
    SetTimer("CheckHarvesterVehicleProximity", 1000, true);
    return 1;
}

hook OnPlayerConnect(playerid)
{
    ResetHarvesterPlayer(playerid);
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{    
    HideHarvesterVehicleNotif(playerid);
    
    if(HarvesterPlayer[playerid][isActive])
    {
        CancelHarvesterMinigame(playerid);
    }
    return 1;
}

hook OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{    
    if(IsPlayerInHarvester(playerid))
    {
        if(newkeys & KEY_SUBMISSION) // Tecla 2
        {
            if(!HarvesterPlayer[playerid][isActive])
            {
                StartHarvesterMinigame(playerid);
            }
            else
            {
                CancelHarvesterMinigame(playerid);
            }
            return 1;
        }
    }
    
    return 0;
}

// Comandos para debug y testing
CMD:iniciarcos(playerid, params[])
{
    if(!IsPlayerInHarvester(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "[DEBUG] Debes estar en una cosechadora (ID 532).");
        return 1;
    }
    
    StartHarvesterMinigame(playerid);
    return 1;
}

CMD:cancelarcos(playerid, params[])
{
    if(!HarvesterPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "[DEBUG] No hay trabajo activo para cancelar.");
        return 1;
    }
    
    CancelHarvesterMinigame(playerid);
    return 1;
}