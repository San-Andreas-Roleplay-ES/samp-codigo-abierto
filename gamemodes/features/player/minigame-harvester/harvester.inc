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
#define MAX_HARVEST_POINTS      25
#define HARVEST_TIME           5

// Lista de puntos de cosecha disponibles
new const Float:HARVEST_POINTS[][3] = 
{
    {-121.83, 102.05, 3.11}, {-115.73, 119.65, 3.11}, {-111.62, 131.87, 3.11}, {-116.30, 144.41, 3.22}, {-121.68, 128.61, 3.11},
    {-131.44, 103.36, 3.11}, {-143.61, 105.15, 3.18}, {-140.36, 113.76, 3.29}, {-134.90, 132.66, 3.46}, {-129.73, 146.90, 3.64},
    {-139.64, 156.90, 4.68}, {-148.56, 132.07, 3.69}, {-146.06, 139.01, 3.82}, {-155.05, 115.50, 3.38}, {-165.16, 123.40, 3.52},
    {-160.55, 136.25, 3.80}, {-154.68, 154.44, 5.15}, {-160.41, 163.90, 6.35}, {-172.02, 132.82, 3.99}, {-169.57, 138.39, 4.37},
    {-186.29, 128.25, 4.19}, {-182.89, 138.89, 4.98}, {-177.77, 154.34, 6.13}, {-173.82, 166.26, 7.48}, {-189.40, 171.07, 8.20},
    {-196.50, 153.80, 5.68}, {-203.94, 135.20, 3.61}, {-132.66, 58.97, 3.11}, {-139.33, 37.86, 3.11}, {-148.90, 12.02, 3.11},
    {-171.65, -44.54, 3.11}, {-184.82, -49.95, 3.11}, {-189.32, -61.58, 3.11}, {-156.80, 27.29, 3.11}, {-160.91, 52.86, 3.11},
    {-204.63, -76.22, 3.11}, {-221.04, -77.68, 3.11}, {-210.19, -46.61, 3.11}, {-194.24, 0.50, 3.10}, {-184.71, 32.63, 3.11},
    {-192.21, 46.47, 3.11}, {-187.12, 64.21, 3.11}, {-212.45, -13.17, 3.11}, {-227.35, -59.50, 3.11}, {-244.12, -59.09, 3.11},
    {-233.08, -13.59, 3.11}, {-234.57, 36.52, 2.68}, {-239.86, 53.99, 2.29}, {-233.63, 92.09, 1.96}, {-259.91, 12.41, 2.10}
};

enum HarvesterPlayerData {
    bool:isActive,
    vehicleNotification,
    bool:isInHarvester,
    bool:isHarvesting,
    harvestPoints[MAX_HARVEST_POINTS],
    harvestObjects[MAX_HARVEST_POINTS],
    STREAMER_TAG_MAP_ICON:harvestMapIcons[MAX_HARVEST_POINTS],
    currentCheckpoint,
    totalCheckpoints,
    checkpointsCompleted,
    harvestTimer,
    PlayerText:harvestTextDraw
}

new HarvesterPlayer[MAX_PLAYERS][HarvesterPlayerData];

stock ResetHarvesterPlayer(playerid)
{
    // Limpiar objetos y mapicons antes de resetear
    DestroyHarvestObjects(playerid);
    
    HarvesterPlayer[playerid][isActive] = false;
    HarvesterPlayer[playerid][vehicleNotification] = -1;
    HarvesterPlayer[playerid][isInHarvester] = false;
    HarvesterPlayer[playerid][isHarvesting] = false;
    HarvesterPlayer[playerid][currentCheckpoint] = -1;
    HarvesterPlayer[playerid][totalCheckpoints] = 0;
    HarvesterPlayer[playerid][checkpointsCompleted] = 0;
    HarvesterPlayer[playerid][harvestTimer] = 0;
    HarvesterPlayer[playerid][harvestTextDraw] = PlayerText:INVALID_TEXT_DRAW;
    
    for(new i = 0; i < MAX_HARVEST_POINTS; i++)
    {
        HarvesterPlayer[playerid][harvestPoints][i] = -1;
        HarvesterPlayer[playerid][harvestObjects][i] = INVALID_OBJECT_ID;
        HarvesterPlayer[playerid][harvestMapIcons][i] = STREAMER_TAG_MAP_ICON:INVALID_STREAMER_ID;
    }
    
    DisablePlayerCheckpoint(playerid);
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
    // Ocultar mensaje anterior si existe
    if(HarvesterPlayer[playerid][vehicleNotification] != -1)
    {
        CallLocalFunction("hideTDN", "ii", playerid, HarvesterPlayer[playerid][vehicleNotification]);
        HarvesterPlayer[playerid][vehicleNotification] = -1;
    }
    
    // Solo mostrar si no tiene trabajo activo
    if(!HarvesterPlayer[playerid][isActive])
    {
        new message[128];
        format(message, sizeof(message), "Presiona 2 para iniciar el trabajo de cosechador.");
        HarvesterPlayer[playerid][vehicleNotification] = CallLocalFunction("ShowTDN_Manual", "is", playerid, message);
        
        // Programar ocultación después de 3 segundos
        SetTimerEx("AutoHideHarvesterNotif", 3000, false, "i", playerid);
    }
}

forward AutoHideHarvesterNotif(playerid);
public AutoHideHarvesterNotif(playerid)
{
    if(IsPlayerConnected(playerid))
    {
        HideHarvesterVehicleNotif(playerid);
    }
}

stock GenerateRandomHarvestPoints(playerid)
{
    new tempArray[sizeof(HARVEST_POINTS)];
    
    // Copiar todos los índices disponibles
    for(new i = 0; i < sizeof(HARVEST_POINTS); i++)
    {
        tempArray[i] = i;
    }
    
    // Mezclar usando Fisher-Yates shuffle
    for(new i = sizeof(HARVEST_POINTS) - 1; i > 0; i--)
    {
        new j = random(i + 1);
        new temp = tempArray[i];
        tempArray[i] = tempArray[j];
        tempArray[j] = temp;
    }
    
    // Seleccionar los primeros 25
    for(new i = 0; i < MAX_HARVEST_POINTS; i++)
    {
        HarvesterPlayer[playerid][harvestPoints][i] = tempArray[i];
    }
    
    HarvesterPlayer[playerid][totalCheckpoints] = MAX_HARVEST_POINTS;
}

stock CreateHarvestObjects(playerid)
{
    // Crear 25 objetos model ID 3409 en las coordenadas seleccionadas
    for(new i = 0; i < MAX_HARVEST_POINTS; i++)
    {
        new pointIndex = HarvesterPlayer[playerid][harvestPoints][i];
        
        // Crear objeto visible solo para el jugador
        HarvesterPlayer[playerid][harvestObjects][i] = CreatePlayerObject(playerid, 3409,
            HARVEST_POINTS[pointIndex][0],
            HARVEST_POINTS[pointIndex][1], 
            HARVEST_POINTS[pointIndex][2],
            0.0, 0.0, float(random(360))
        );
        
        // Crear mapicon en el mapa (similar al lumberjack)
        HarvesterPlayer[playerid][harvestMapIcons][i] = CreateDynamicMapIcon(
            HARVEST_POINTS[pointIndex][0],
            HARVEST_POINTS[pointIndex][1], 
            HARVEST_POINTS[pointIndex][2],
            0, 0xC2A636FF, -1, -1, playerid); // Icono amarillento color maiz
    }
}

stock DestroyHarvestObjects(playerid)
{
    // Eliminar todos los objetos y mapicons
    for(new i = 0; i < MAX_HARVEST_POINTS; i++)
    {
        if(HarvesterPlayer[playerid][harvestObjects][i] != INVALID_OBJECT_ID)
        {
            DestroyPlayerObject(playerid, HarvesterPlayer[playerid][harvestObjects][i]);
            HarvesterPlayer[playerid][harvestObjects][i] = INVALID_OBJECT_ID;
        }
        
        if(HarvesterPlayer[playerid][harvestMapIcons][i] != STREAMER_TAG_MAP_ICON:INVALID_STREAMER_ID)
        {
            DestroyDynamicMapIcon(HarvesterPlayer[playerid][harvestMapIcons][i]);
            HarvesterPlayer[playerid][harvestMapIcons][i] = STREAMER_TAG_MAP_ICON:INVALID_STREAMER_ID;
        }
    }
}

/*
// Funciones de checkpoints comentadas - ahora se usan objetos directamente
stock SetNextHarvestCheckpoint(playerid)
{
    if(HarvesterPlayer[playerid][checkpointsCompleted] >= HarvesterPlayer[playerid][totalCheckpoints])
    {
        // Todos los checkpoints completados
        FinishHarvesterMinigame(playerid);
        return 1;
    }
    
    new pointIndex = HarvesterPlayer[playerid][harvestPoints][HarvesterPlayer[playerid][checkpointsCompleted]];
    
    SetPlayerCheckpoint(playerid, 
        HARVEST_POINTS[pointIndex][0], 
        HARVEST_POINTS[pointIndex][1], 
        HARVEST_POINTS[pointIndex][2], 
        3.0
    );
    
    HarvesterPlayer[playerid][currentCheckpoint] = pointIndex;
    
    SendClientMessage(playerid, COLOR_GREEN, sprintf("[DEBUG] Checkpoint %d/%d - Ve al punto de cosecha marcado.", 
        HarvesterPlayer[playerid][checkpointsCompleted] + 1, 
        HarvesterPlayer[playerid][totalCheckpoints]
    ));
    
    return 1;
}
*/

forward ShowHarvesterStartMessage(playerid);
public ShowHarvesterStartMessage(playerid)
{
    if(!IsPlayerConnected(playerid) || !HarvesterPlayer[playerid][isActive])
    {
        return 0;
    }
    
    // Ocultar el textdraw después de 2.5 segundos
    if(HarvesterPlayer[playerid][harvestTextDraw] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, HarvesterPlayer[playerid][harvestTextDraw]);
        HarvesterPlayer[playerid][harvestTextDraw] = PlayerText:INVALID_TEXT_DRAW;
    }
    
    return 1;
}

/*
// Timer de cosecha comentado - ya no se utiliza
forward HarvesterCuttingTimer(playerid, seconds);
public HarvesterCuttingTimer(playerid, seconds)
{
    if(!IsPlayerConnected(playerid) || !HarvesterPlayer[playerid][isActive] || !HarvesterPlayer[playerid][isHarvesting])
    {
        return 0;
    }
    
    if(seconds > 0)
    {
        new text[64];
        format(text, sizeof(text), "Espera %d segundos cosechar el maiz.", seconds);
        
        if(HarvesterPlayer[playerid][harvestTextDraw] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, HarvesterPlayer[playerid][harvestTextDraw]);
        }
        
        HarvesterPlayer[playerid][harvestTextDraw] = CreatePlayerTextDraw(playerid, 325.000, 369.000, text);
        PlayerTextDrawLetterSize(playerid, HarvesterPlayer[playerid][harvestTextDraw], 0.300, 1.500);
        PlayerTextDrawAlignment(playerid, HarvesterPlayer[playerid][harvestTextDraw], 2);
        PlayerTextDrawColor(playerid, HarvesterPlayer[playerid][harvestTextDraw], -1);
        PlayerTextDrawSetShadow(playerid, HarvesterPlayer[playerid][harvestTextDraw], 1);
        PlayerTextDrawSetOutline(playerid, HarvesterPlayer[playerid][harvestTextDraw], 1);
        PlayerTextDrawBackgroundColor(playerid, HarvesterPlayer[playerid][harvestTextDraw], 150);
        PlayerTextDrawFont(playerid, HarvesterPlayer[playerid][harvestTextDraw], 1);
        PlayerTextDrawSetProportional(playerid, HarvesterPlayer[playerid][harvestTextDraw], true);
        PlayerTextDrawShow(playerid, HarvesterPlayer[playerid][harvestTextDraw]);
        
        HarvesterPlayer[playerid][harvestTimer] = SetTimerEx("HarvesterCuttingTimer", 1000, false, "ii", playerid, seconds - 1);
    }
    else
    {
        // Cosecha completada
        new pointIndex = HarvesterPlayer[playerid][currentCheckpoint];
        
        // Crear partículas
        // new particle = CreateDynamicObject(18679, 
        //     HARVEST_POINTS[pointIndex][0], 
        //     HARVEST_POINTS[pointIndex][1], 
        //     HARVEST_POINTS[pointIndex][2], 
        //     0.0, 0.0, 0.0
        // );
        
        // Sonido al completar cosecha
        PlayerPlaySound(playerid, 1139, 
            HARVEST_POINTS[pointIndex][0], 
            HARVEST_POINTS[pointIndex][1], 
            HARVEST_POINTS[pointIndex][2]
        );
        
        // Obtener posición del jugador para calcular dirección hacia el checkpoint
        new Float:px, Float:py, Float:pz;
        GetPlayerPos(playerid, px, py, pz);
        
        // Calcular ángulo desde el jugador hacia el checkpoint
        new Float:angle = atan2(HARVEST_POINTS[pointIndex][1] - py, HARVEST_POINTS[pointIndex][0] - px);
        
        // Calcular posición 7 metros atrás del checkpoint (opuesto a la dirección del jugador)
        new Float:backX = HARVEST_POINTS[pointIndex][0] - (7.0 * floatcos(angle, degrees));
        new Float:backY = HARVEST_POINTS[pointIndex][1] - (7.0 * floatsin(angle, degrees));
        
        // Crear objeto de cosecha 7 metros atrás del checkpoint
        CreateDynamicObject(14875, 
            backX, 
            backY, 
            HARVEST_POINTS[pointIndex][2], 
            0.0, 0.0, float(random(360))
        );
        
        // Limpiar partículas después de un tiempo
        // SetTimerEx("DeleteParticle", 1000, false, "d", particle);
        
        // Limpiar estado
        HarvesterPlayer[playerid][isHarvesting] = false;
        HarvesterPlayer[playerid][harvestTimer] = 0;
        HarvesterPlayer[playerid][currentCheckpoint] = -1;
        
        if(HarvesterPlayer[playerid][harvestTextDraw] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, HarvesterPlayer[playerid][harvestTextDraw]);
            HarvesterPlayer[playerid][harvestTextDraw] = PlayerText:INVALID_TEXT_DRAW;
        }
        
        TogglePlayerControllable(playerid, 1);
        DisablePlayerCheckpoint(playerid);
        
        HarvesterPlayer[playerid][checkpointsCompleted]++;
        
        SendClientMessage(playerid, COLOR_GREEN, sprintf("[DEBUG] Cosecha completada! (%d/%d)", 
            HarvesterPlayer[playerid][checkpointsCompleted], 
            HarvesterPlayer[playerid][totalCheckpoints]
        ));
        
        // Establecer siguiente checkpoint
        SetNextHarvestCheckpoint(playerid);
    }
    
    return 1;
}
*/

forward DeleteHarvesterParticle(objectid);
public DeleteHarvesterParticle(objectid)
{
    if(!IsValidDynamicObject(objectid))
        return 0;

    return DestroyDynamicObject(objectid);
}

stock FinishHarvesterMinigame(playerid)
{
    if(!HarvesterPlayer[playerid][isActive]) return 0;
    
    DisablePlayerCheckpoint(playerid);
    
    new reward = 2500 + random(3001); // Entre $2500 y $5500
    GivePlayerMoney(playerid, reward);
    
    // Reproducir sonido de finalización
    PlayerPlaySound(playerid, 183, 0.0, 0.0, 0.0);
    
    // Mensaje de finalización similar al lumberjack
    SendClientMessage(playerid, COLOR_LIGHTGREEN, sprintf("¡Recolectaste $%d, por todo el maíz!", reward));
    
    ResetHarvesterPlayer(playerid);
    
    if(IsPlayerInHarvester(playerid))
    {
        HarvesterPlayer[playerid][isInHarvester] = true; // Mantener estado del vehículo
        ShowHarvesterVehicleNotif(playerid);
    }
    
    return 1;
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
        
        new bool:currentlyInHarvester = IsPlayerInHarvester(i);
        
        // Detectar cuando ENTRA a la cosechadora
        if(currentlyInHarvester && !HarvesterPlayer[i][isInHarvester])
        {
            HarvesterPlayer[i][isInHarvester] = true;
            
            // Mostrar mensaje solo si no tiene trabajo activo
            if(!HarvesterPlayer[i][isActive])
            {
                ShowHarvesterVehicleNotif(i);
            }
        }
        // Detectar cuando SALE de la cosechadora
        else if(!currentlyInHarvester && HarvesterPlayer[i][isInHarvester])
        {
            HarvesterPlayer[i][isInHarvester] = false;
            
            // Ocultar cualquier mensaje visible
            HideHarvesterVehicleNotif(i);
            
            // Si tenía trabajo activo, cancelarlo
            if(HarvesterPlayer[i][isActive])
            {
                // Limpiar timer de cosecha si existe
                if(HarvesterPlayer[i][harvestTimer] != 0)
                {
                    KillTimer(HarvesterPlayer[i][harvestTimer]);
                }
                
                // Limpiar textdraw si existe
                if(HarvesterPlayer[i][harvestTextDraw] != PlayerText:INVALID_TEXT_DRAW)
                {
                    PlayerTextDrawDestroy(i, HarvesterPlayer[i][harvestTextDraw]);
                }
                
                // Descongelar jugador
                TogglePlayerControllable(i, 1);
                
                ResetHarvesterPlayer(i);
                SendClientMessage(i, COLOR_FADE3, "[DEBUG] Trabajo finalizado - saliste del vehículo.");
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
    
    // Generar puntos aleatorios de cosecha
    GenerateRandomHarvestPoints(playerid);
    
    // Crear los objetos y mapicons
    CreateHarvestObjects(playerid);
    
    // Mostrar mensaje inicial "Cosecha todo el maiz" durante 2.5 segundos
    HarvesterPlayer[playerid][harvestTextDraw] = CreatePlayerTextDraw(playerid, 325.000, 369.000, "Cosecha todo el maiz");
    PlayerTextDrawLetterSize(playerid, HarvesterPlayer[playerid][harvestTextDraw], 0.300, 1.500);
    PlayerTextDrawAlignment(playerid, HarvesterPlayer[playerid][harvestTextDraw], 2);
    PlayerTextDrawColor(playerid, HarvesterPlayer[playerid][harvestTextDraw], -1);
    PlayerTextDrawSetShadow(playerid, HarvesterPlayer[playerid][harvestTextDraw], 1);
    PlayerTextDrawSetOutline(playerid, HarvesterPlayer[playerid][harvestTextDraw], 1);
    PlayerTextDrawBackgroundColor(playerid, HarvesterPlayer[playerid][harvestTextDraw], 150);
    PlayerTextDrawFont(playerid, HarvesterPlayer[playerid][harvestTextDraw], 1);
    PlayerTextDrawSetProportional(playerid, HarvesterPlayer[playerid][harvestTextDraw], true);
    PlayerTextDrawShow(playerid, HarvesterPlayer[playerid][harvestTextDraw]);
    
    // Programar ocultación del mensaje después de 2.5 segundos
    SetTimerEx("ShowHarvesterStartMessage", 2500, false, "i", playerid);
    
    SendClientMessage(playerid, COLOR_GREEN, "[DEBUG] Trabajo iniciado - cosechador activo.");
    SendClientMessage(playerid, COLOR_GREEN, sprintf("[DEBUG] Se generaron %d objetos de maíz. Acércate a ellos para cosechar.", MAX_HARVEST_POINTS));
    
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
    
    // Limpiar timer de cosecha si existe
    if(HarvesterPlayer[playerid][harvestTimer] != 0)
    {
        KillTimer(HarvesterPlayer[playerid][harvestTimer]);
    }
    
    // Limpiar textdraw si existe
    if(HarvesterPlayer[playerid][harvestTextDraw] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, HarvesterPlayer[playerid][harvestTextDraw]);
    }
    
    // Descongelar jugador si estaba congelado
    TogglePlayerControllable(playerid, 1);
    
    ResetHarvesterPlayer(playerid);
    
    // Si está en la cosechadora, mostrar mensaje de nuevo por 3 segundos
    if(IsPlayerInHarvester(playerid))
    {
        HarvesterPlayer[playerid][isInHarvester] = true; // Mantener el estado
        ShowHarvesterVehicleNotif(playerid);
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

/*
// Hook de checkpoint comentado - ahora se usan objetos directamente
hook OnPlayerEnterCheckpoint(playerid)
{
    if(!HarvesterPlayer[playerid][isActive] || HarvesterPlayer[playerid][isHarvesting])
        return 0;
    
    if(!IsPlayerInHarvester(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "[DEBUG] Debes estar en la cosechadora para cosechar.");
        return 1;
    }
    
    // Iniciar proceso de cosecha
    HarvesterPlayer[playerid][isHarvesting] = true;
    
    // Congelar jugador
    TogglePlayerControllable(playerid, 0);
    
    SendClientMessage(playerid, COLOR_GREEN, "[DEBUG] Cosechando maíz...");
    
    // Iniciar timer de cosecha
    HarvesterPlayer[playerid][harvestTimer] = SetTimerEx("HarvesterCuttingTimer", 1000, false, "ii", playerid, HARVEST_TIME);
    
    return 1;
}
*/

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