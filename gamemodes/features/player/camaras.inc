#include <YSI_Data\y_iterate>

#define MAX_CAMERAS 100
#define CAMERA_ALERT_CHANCE 20 // 20% de probabilidad de alerta policial
#define CAMERA_DETECTION_RANGE 30.0 // Rango de detección en metros
#define CAMERA_DAMAGE_TIME 60 // Segundos para dañar cámara
#define CAMERA_REPAIR_TIME 120 // Segundos para arreglar cámara

enum E_CAMERA_DATA
{
    bool:cam_Exists,
    Float:cam_PosX,
    Float:cam_PosY,
    Float:cam_PosZ,
    Float:cam_RotX,
    Float:cam_RotY,
    Float:cam_RotZ,
    cam_Object,
    cam_Interior,
    cam_VirtualWorld,
    bool:cam_Damaged
};

new CameraData[MAX_CAMERAS][E_CAMERA_DATA];

enum E_PLAYER_CAMERA
{
    bool:pc_Watching,
    pc_CameraID,
    Float:pc_OldPosX,
    Float:pc_OldPosY,
    Float:pc_OldPosZ,
    Float:pc_OldAngle,
    pc_OldInterior,
    pc_OldVirtualWorld,
    bool:pc_DamagingCamera,
    pc_DamageProgress,
    bool:pc_RepairingCamera,
    pc_RepairProgress,
    // TextDraws
    PlayerText:pc_ProgressBox,
    PlayerText:pc_ProgressTitle,
    PlayerText:pc_ProgressTime
};

new PlayerCameraData[MAX_PLAYERS][E_PLAYER_CAMERA];

// Inicializar cámaras al inicio del gamemode
stock InitCameras()
{
    for(new i = 0; i < MAX_CAMERAS; i++)
    {
        CameraData[i][cam_Exists] = false;
        CameraData[i][cam_Object] = INVALID_OBJECT_ID;
        CameraData[i][cam_PosX] = 0.0;
        CameraData[i][cam_PosY] = 0.0;
        CameraData[i][cam_PosZ] = 0.0;
        CameraData[i][cam_Damaged] = false;
    }
    
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerCameraData[i][pc_Watching] = false;
        PlayerCameraData[i][pc_DamagingCamera] = false;
        PlayerCameraData[i][pc_RepairingCamera] = false;
        PlayerCameraData[i][pc_ProgressBox] = PlayerText:INVALID_TEXT_DRAW;
        PlayerCameraData[i][pc_ProgressTitle] = PlayerText:INVALID_TEXT_DRAW;
        PlayerCameraData[i][pc_ProgressTime] = PlayerText:INVALID_TEXT_DRAW;
    }
    
    // Iniciar timer de detección de cámaras cada 2 segundos
    SetTimer("CheckCameraDetection", 2000, true);
    
    // Timer para daño y reparación de cámaras (cada segundo)
    SetTimer("CameraActionTimer", 1000, true);
    
    printf("[Sistema de Camaras] Inicializado correctamente.");
    return 1;
}

//TexDraws
stock CreateProgressTextDraws(playerid)
{
    
    // Título de acción
    PlayerCameraData[playerid][pc_ProgressTitle] = CreatePlayerTextDraw(playerid, 320.0, 395.0, "DAÑANDO CAMARA");
    PlayerTextDrawAlignment(playerid, PlayerCameraData[playerid][pc_ProgressTitle], 2);
    PlayerTextDrawBackgroundColor(playerid, PlayerCameraData[playerid][pc_ProgressTitle], 255);
    PlayerTextDrawFont(playerid, PlayerCameraData[playerid][pc_ProgressTitle], 1);
    PlayerTextDrawLetterSize(playerid, PlayerCameraData[playerid][pc_ProgressTitle], 0.30, 1.3);
    PlayerTextDrawColor(playerid, PlayerCameraData[playerid][pc_ProgressTitle], COLOR_LIGHTRED);
    PlayerTextDrawSetOutline(playerid, PlayerCameraData[playerid][pc_ProgressTitle], 1);
    PlayerTextDrawSetProportional(playerid, PlayerCameraData[playerid][pc_ProgressTitle], 1);
    
    // Tiempo restante
    PlayerCameraData[playerid][pc_ProgressTime] = CreatePlayerTextDraw(playerid, 320.0, 410.0, "60 SEGUNDOS RESTANTES");
    PlayerTextDrawAlignment(playerid, PlayerCameraData[playerid][pc_ProgressTime], 2);
    PlayerTextDrawBackgroundColor(playerid, PlayerCameraData[playerid][pc_ProgressTime], 255);
    PlayerTextDrawFont(playerid, PlayerCameraData[playerid][pc_ProgressTime], 2);
    PlayerTextDrawLetterSize(playerid, PlayerCameraData[playerid][pc_ProgressTime], 0.22, 1.1);
    PlayerTextDrawColor(playerid, PlayerCameraData[playerid][pc_ProgressTime], COLOR_WHITE);
    PlayerTextDrawSetOutline(playerid, PlayerCameraData[playerid][pc_ProgressTime], 1);
    PlayerTextDrawSetProportional(playerid, PlayerCameraData[playerid][pc_ProgressTime], 1);
    
    return 1;
}

// Destruir TextDraws de progreso
stock DestroyProgressTextDraws(playerid)
{
    if(PlayerCameraData[playerid][pc_ProgressBox] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, PlayerCameraData[playerid][pc_ProgressBox]);
        PlayerCameraData[playerid][pc_ProgressBox] = PlayerText:INVALID_TEXT_DRAW;
    }
    if(PlayerCameraData[playerid][pc_ProgressTitle] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, PlayerCameraData[playerid][pc_ProgressTitle]);
        PlayerCameraData[playerid][pc_ProgressTitle] = PlayerText:INVALID_TEXT_DRAW;
    }
    if(PlayerCameraData[playerid][pc_ProgressTime] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, PlayerCameraData[playerid][pc_ProgressTime]);
        PlayerCameraData[playerid][pc_ProgressTime] = PlayerText:INVALID_TEXT_DRAW;
    }
    return 1;
}

// Mostrar TextDraws de progreso
stock ShowProgressTextDraws(playerid, bool:isDamaging)
{
    if(isDamaging)
    {
        PlayerTextDrawSetString(playerid, PlayerCameraData[playerid][pc_ProgressTitle], "DAÑANDO CAMARA");
        PlayerTextDrawColor(playerid, PlayerCameraData[playerid][pc_ProgressTitle], COLOR_LIGHTRED);
        PlayerTextDrawShow(playerid, PlayerCameraData[playerid][pc_ProgressTitle]);
    }
    else
    {
        PlayerTextDrawSetString(playerid, PlayerCameraData[playerid][pc_ProgressTitle], "REPARANDO CAMARA");
        PlayerTextDrawColor(playerid, PlayerCameraData[playerid][pc_ProgressTitle], COLOR_LIGHTBLUE);
        PlayerTextDrawShow(playerid, PlayerCameraData[playerid][pc_ProgressTitle]);
    }
    
    PlayerTextDrawShow(playerid, PlayerCameraData[playerid][pc_ProgressBox]);
    PlayerTextDrawShow(playerid, PlayerCameraData[playerid][pc_ProgressTime]);
    
    return 1;
}

// Ocultar TextDraws de progreso
stock HideProgressTextDraws(playerid)
{
    PlayerTextDrawHide(playerid, PlayerCameraData[playerid][pc_ProgressBox]);
    PlayerTextDrawHide(playerid, PlayerCameraData[playerid][pc_ProgressTitle]);
    PlayerTextDrawHide(playerid, PlayerCameraData[playerid][pc_ProgressTime]);
    return 1;
}

// Actualizar tiempo restante
stock UpdateProgressTime(playerid, remaining)
{
    new str[64];
    
    if(remaining > 1)
        format(str, sizeof(str), "%d SEGUNDOS RESTANTES", remaining);
    else
        format(str, sizeof(str), "%d SEGUNDO RESTANTE", remaining);
    
    PlayerTextDrawSetString(playerid, PlayerCameraData[playerid][pc_ProgressTime], str);
    PlayerTextDrawShow(playerid, PlayerCameraData[playerid][pc_ProgressTime]);
    
    return 1;
}

// Timer para acciones de daño y reparación
forward CameraActionTimer();
public CameraActionTimer()
{
    foreach(new playerid : Player)
    {
        // Verifica si está dañando cámara
        if(PlayerCameraData[playerid][pc_DamagingCamera])
        {
            new cameraid = PlayerCameraData[playerid][pc_CameraID];
            
            // Verifica si sigue cerca de la cámara
            if(!IsPlayerInRangeOfPoint(playerid, 5.0, 
                CameraData[cameraid][cam_PosX], CameraData[cameraid][cam_PosY], CameraData[cameraid][cam_PosZ]))
            {
                PlayerCameraData[playerid][pc_DamagingCamera] = false;
                PlayerCameraData[playerid][pc_DamageProgress] = 0;
                HideProgressTextDraws(playerid);
                GameTextForPlayer(playerid, "~r~DAÑO CANCELADO", 2000, 3);
                continue;
            }
            
            PlayerCameraData[playerid][pc_DamageProgress]++;
            
            new remaining = CAMERA_DAMAGE_TIME - PlayerCameraData[playerid][pc_DamageProgress];
            UpdateProgressTime(playerid, remaining);
            
            // Si completó el daño
            if(PlayerCameraData[playerid][pc_DamageProgress] >= CAMERA_DAMAGE_TIME)
            {
                CameraData[cameraid][cam_Damaged] = true;
                PlayerCameraData[playerid][pc_DamagingCamera] = false;
                PlayerCameraData[playerid][pc_DamageProgress] = 0;
                HideProgressTextDraws(playerid);
                
                GameTextForPlayer(playerid, "~g~CAMARA DAÑADA", 3000, 3);
                SendClientMessage(playerid, COLOR_LIGHTGREEN, "* Has dañado la camara exitosamente. Ya no detectara sospechosos.");
            }
        }
        
        // Verificar si está reparando cámara
        if(PlayerCameraData[playerid][pc_RepairingCamera])
        {
            new cameraid = PlayerCameraData[playerid][pc_CameraID];
            
            // Verificar si sigue cerca de la cámara
            if(!IsPlayerInRangeOfPoint(playerid, 5.0, 
                CameraData[cameraid][cam_PosX], CameraData[cameraid][cam_PosY], CameraData[cameraid][cam_PosZ]))
            {
                PlayerCameraData[playerid][pc_RepairingCamera] = false;
                PlayerCameraData[playerid][pc_RepairProgress] = 0;
                HideProgressTextDraws(playerid);
                GameTextForPlayer(playerid, "~r~REPARACION CANCELADA", 2000, 3);
                continue;
            }
            
            PlayerCameraData[playerid][pc_RepairProgress]++;
            
            new remaining = CAMERA_REPAIR_TIME - PlayerCameraData[playerid][pc_RepairProgress];
            UpdateProgressTime(playerid, remaining);
            
            // Si completó la reparación
            if(PlayerCameraData[playerid][pc_RepairProgress] >= CAMERA_REPAIR_TIME)
            {
                CameraData[cameraid][cam_Damaged] = false;
                PlayerCameraData[playerid][pc_RepairingCamera] = false;
                PlayerCameraData[playerid][pc_RepairProgress] = 0;
                HideProgressTextDraws(playerid);
                
                GameTextForPlayer(playerid, "~g~CAMARA REPARADA", 3000, 3);
                SendClientMessage(playerid, COLOR_LIGHTGREEN, "* Has reparado la camara exitosamente. Ahora funcionara correctamente.");
            }
        }
    }
    return 1;
}

// Timer que verifica jugadores cerca de las cámaras
forward CheckCameraDetection();
public CheckCameraDetection()
{
    for(new i = 0; i < MAX_CAMERAS; i++)
    {
        if(!CameraData[i][cam_Exists]) continue;
        if(CameraData[i][cam_Damaged]) continue;
        
        foreach(new playerid : Player)
        {
            if(GetPlayerInterior(playerid) != CameraData[i][cam_Interior]) continue;
            if(GetPlayerVirtualWorld(playerid) != CameraData[i][cam_VirtualWorld]) continue;
            
            if(!IsPlayerInRangeOfPoint(playerid, CAMERA_DETECTION_RANGE, 
                CameraData[i][cam_PosX], CameraData[i][cam_PosY], CameraData[i][cam_PosZ])) continue;
            
            new weapon = GetPlayerWeapon(playerid);
            
            // Usar IsValidWeapon y verificar que sea arma de fuego (22-38)
            if(IsValidWeapon(weapon))
            {
                if(random(100) < CAMERA_ALERT_CHANCE)
                {
                    SendClientMessage(playerid, COLOR_LIGHTORANGE, "ALERTA: Una camara de seguridad te ha detectado con un arma!");
                    
                    new msg[256], pName[MAX_PLAYER_NAME], weaponName[32];
                    GetPlayerName(playerid, pName, sizeof(pName));
                    GetWeaponName(weapon, weaponName, sizeof(weaponName));
                    
                    format(msg, sizeof(msg), 
                        "POLICIA: Camara [ID %d]: Sospechoso detectado con %s | Identificado: %s", 
                        i, weaponName, pName);
                    
                    foreach(new x : Player)
                    {
                        SendClientMessage(x, TEAM_LSPD_COLOR, msg);
                    }
                }
            }
        }
    }
    return 1;
}

// Comando para dañar cámara
CMD:dañarcamara(playerid, params[])
{
    if(PlayerCameraData[playerid][pc_DamagingCamera])
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: Ya estas dañando una camara.");
    
    if(PlayerCameraData[playerid][pc_RepairingCamera])
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: No puedes dañar mientras reparas.");
    
    new cameraid = -1;
    new Float:minDist = 5.0;
    
    for(new i = 0; i < MAX_CAMERAS; i++)
    {
        if(!CameraData[i][cam_Exists]) continue;
        if(CameraData[i][cam_Damaged]) continue;
        
        if(GetPlayerInterior(playerid) != CameraData[i][cam_Interior]) continue;
        if(GetPlayerVirtualWorld(playerid) != CameraData[i][cam_VirtualWorld]) continue;
        
        if(IsPlayerInRangeOfPoint(playerid, minDist, 
            CameraData[i][cam_PosX], CameraData[i][cam_PosY], CameraData[i][cam_PosZ]))
        {
            cameraid = i;
            break;
        }
    }
    
    if(cameraid == -1)
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: No hay ninguna camara funcional cerca de ti (5 metros).");
    
    PlayerCameraData[playerid][pc_DamagingCamera] = true;
    PlayerCameraData[playerid][pc_CameraID] = cameraid;
    PlayerCameraData[playerid][pc_DamageProgress] = 0;
    
    CreateProgressTextDraws(playerid);
    ShowProgressTextDraws(playerid, true);
    UpdateProgressTime(playerid, CAMERA_DAMAGE_TIME);
    
    SendClientMessage(playerid, COLOR_LIGHTORANGE, "AVISO: Comenzando a dañar la camara... No te muevas!");
    return 1;
}

// Comando para reparar cámara
CMD:repararcamara(playerid, params[])
{
    if(PlayerCameraData[playerid][pc_RepairingCamera])
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: Ya estas reparando una camara.");
    
    if(PlayerCameraData[playerid][pc_DamagingCamera])
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: No puedes reparar mientras dañas.");
    
    new cameraid = -1;
    new Float:minDist = 5.0;
    
    for(new i = 0; i < MAX_CAMERAS; i++)
    {
        if(!CameraData[i][cam_Exists]) continue;
        if(!CameraData[i][cam_Damaged]) continue;
        
        if(GetPlayerInterior(playerid) != CameraData[i][cam_Interior]) continue;
        if(GetPlayerVirtualWorld(playerid) != CameraData[i][cam_VirtualWorld]) continue;
        
        if(IsPlayerInRangeOfPoint(playerid, minDist, 
            CameraData[i][cam_PosX], CameraData[i][cam_PosY], CameraData[i][cam_PosZ]))
        {
            cameraid = i;
            break;
        }
    }
    
    if(cameraid == -1)
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: No hay ninguna camara dañada cerca de ti (5 metros).");
    
    PlayerCameraData[playerid][pc_RepairingCamera] = true;
    PlayerCameraData[playerid][pc_CameraID] = cameraid;
    PlayerCameraData[playerid][pc_RepairProgress] = 0;
    
    CreateProgressTextDraws(playerid);
    ShowProgressTextDraws(playerid, false);
    UpdateProgressTime(playerid, CAMERA_REPAIR_TIME);
    
    SendClientMessage(playerid, COLOR_LIGHTBLUE, "AVISO: Comenzando a reparar la camara... No te muevas!");
    return 1;
}

// Comando para buscar cámaras dañadas
CMD:buscarcamaras(playerid, params[])
{
    new count = 0;
    SendClientMessage(playerid, COLOR_LIGHTRED, "________ CAMARAS DANIADAS ________");
    
    for(new i = 0; i < MAX_CAMERAS; i++)
    {
        if(CameraData[i][cam_Exists] && CameraData[i][cam_Damaged])
        {
            new Float:distance = GetPlayerDistanceFromPoint(playerid, 
                CameraData[i][cam_PosX], CameraData[i][cam_PosY], CameraData[i][cam_PosZ]);
            
            new msg[180];
            format(msg, sizeof(msg), "ID %d | Posicion: %.1f, %.1f, %.1f | Distancia: %.1fm", 
                i, CameraData[i][cam_PosX], CameraData[i][cam_PosY], CameraData[i][cam_PosZ], distance);
            SendClientMessage(playerid, COLOR_LIGHTORANGE, msg);
            count++;
        }
    }
    
    if(count == 0)
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "AVISO: No hay camaras dañadas en este momento.");
    else
    {
        new msg[64];
        format(msg, sizeof(msg), "INFO: Total: %d camaras daniadas", count);
        SendClientMessage(playerid, COLOR_LIGHTORANGE, msg);
    }
    
    return 1;
}

// Comando para instalar cámara
CMD:instalarcamara(playerid, params[])
{
    new cameraid = GetFreeCameraSlot();
    if(cameraid == -1)
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: No hay slots disponibles para mas camaras.");
    
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);
    
    x += (2.0 * floatsin(-a, degrees));
    y += (2.0 * floatcos(-a, degrees));
    z += 2.5;
    
    CameraData[cameraid][cam_PosX] = x;
    CameraData[cameraid][cam_PosY] = y;
    CameraData[cameraid][cam_PosZ] = z;
    CameraData[cameraid][cam_RotX] = 0.0;
    CameraData[cameraid][cam_RotY] = 0.0;
    CameraData[cameraid][cam_RotZ] = a;
    CameraData[cameraid][cam_Interior] = GetPlayerInterior(playerid);
    CameraData[cameraid][cam_VirtualWorld] = GetPlayerVirtualWorld(playerid);
    CameraData[cameraid][cam_Damaged] = false;
    
    CameraData[cameraid][cam_Object] = CreateDynamicObject(1622, x, y, z, 0.0, 0.0, a,
        GetPlayerVirtualWorld(playerid), GetPlayerInterior(playerid));
    
    EditDynamicObject(playerid, CameraData[cameraid][cam_Object]);
    
    SetPVarInt(playerid, "EditingCameraID", cameraid);
    
    new msg[128];
    format(msg, sizeof(msg), "INFO: Posiciona la camara ID %d. Guarda para confirmar.", cameraid);
    SendClientMessage(playerid, COLOR_LIGHTGREEN, msg);
    
    return 1;
}

// Comando para ver cámara
CMD:camara(playerid, params[])
{
    if(isnull(params))
        return SendClientMessage(playerid, COLOR_LIGHTORANGE, "USO: /camara [ID] | /camara off | /camara lista");
    
    if(!strcmp(params, "off", true))
    {
        if(!PlayerCameraData[playerid][pc_Watching])
            return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: No estas viendo ninguna camara.");
        
        StopWatchingCamera(playerid);
        return 1;
    }
    
    if(!strcmp(params, "lista", true))
    {
        new count = 0, damaged = 0;
        SendClientMessage(playerid, TEAM_LSPD_COLOR, "________ CAMARAS DISPONIBLES ________");
        
        for(new i = 0; i < MAX_CAMERAS; i++)
        {
            if(CameraData[i][cam_Exists])
            {
                new msg[180];
                new status[20];
                
                if(CameraData[i][cam_Damaged])
                {
                    status = "{FF6347}[DAÑADA]";
                    damaged++;
                }
                else
                {
                    status = "{32CD32}[FUNCIONAL]";
                }
                
                format(msg, sizeof(msg), "%s ID %d | Posicion: %.1f, %.1f, %.1f", 
                    status, i, CameraData[i][cam_PosX], CameraData[i][cam_PosY], CameraData[i][cam_PosZ]);
                SendClientMessage(playerid, -1, msg);
                count++;
            }
        }
        
        if(count == 0)
            SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: No hay camaras instaladas.");
        else
        {
            new msg[128];
            format(msg, sizeof(msg), "INFO: Total: %d camaras | Funcionales: %d | Dañadas: %d", 
                count, count - damaged, damaged);
            SendClientMessage(playerid, COLOR_LIGHTORANGE, msg);
        }
        
        return 1;
    }
    
    new cameraid = strval(params);
    
    if(cameraid < 0 || cameraid >= MAX_CAMERAS)
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: ID de camara invalido.");
    
    if(!CameraData[cameraid][cam_Exists])
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: Esta camara no existe.");
    
    if(CameraData[cameraid][cam_Damaged])
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: Esta camara esta dañada. Debe ser reparada primero.");
    
    if(PlayerCameraData[playerid][pc_Watching])
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: Ya estas viendo una camara. Usa /camara off primero.");
    
    StartWatchingCamera(playerid, cameraid);
    
    return 1;
}

// Comando para eliminar cámara
CMD:eliminarcam(playerid, params[])
{
    if(isnull(params))
        return SendClientMessage(playerid, COLOR_LIGHTORANGE, "USO: /eliminarcam [ID]");
    
    new cameraid = strval(params);
    
    if(cameraid < 0 || cameraid >= MAX_CAMERAS)
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: ID de camara invalido.");
    
    if(!CameraData[cameraid][cam_Exists])
        return SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: Esta camara no existe.");
    
    // Destruir objeto
    if(IsValidDynamicObject(CameraData[cameraid][cam_Object]))
        DestroyDynamicObject(CameraData[cameraid][cam_Object]);
    
    CameraData[cameraid][cam_Exists] = false;
    CameraData[cameraid][cam_Object] = INVALID_OBJECT_ID;
    CameraData[cameraid][cam_Damaged] = false;
    
    new msg[128];
    format(msg, sizeof(msg), "INFO: Camara ID %d eliminada correctamente.", cameraid);
    SendClientMessage(playerid, COLOR_LIGHTGREEN, msg);
    
    return 1;
}

// Funciones auxiliares
stock GetFreeCameraSlot()
{
    for(new i = 0; i < MAX_CAMERAS; i++)
    {
        if(!CameraData[i][cam_Exists])
            return i;
    }
    return -1;
}

stock StartWatchingCamera(playerid, cameraid)
{
    // Guarda posición actual del jugador
    GetPlayerPos(playerid, 
        PlayerCameraData[playerid][pc_OldPosX],
        PlayerCameraData[playerid][pc_OldPosY],
        PlayerCameraData[playerid][pc_OldPosZ]);
    
    GetPlayerFacingAngle(playerid, PlayerCameraData[playerid][pc_OldAngle]);
    PlayerCameraData[playerid][pc_OldInterior] = GetPlayerInterior(playerid);
    PlayerCameraData[playerid][pc_OldVirtualWorld] = GetPlayerVirtualWorld(playerid);
    
    // Configura cámara
    PlayerCameraData[playerid][pc_Watching] = true;
    PlayerCameraData[playerid][pc_CameraID] = cameraid;
    
    // Cambia interior y VW
    SetPlayerInterior(playerid, CameraData[cameraid][cam_Interior]);
    SetPlayerVirtualWorld(playerid, CameraData[cameraid][cam_VirtualWorld]);
    
    // Calcula posición de vista de la cámara
    new Float:camX, Float:camY, Float:camZ;
    camX = CameraData[cameraid][cam_PosX];
    camY = CameraData[cameraid][cam_PosY];
    camZ = CameraData[cameraid][cam_PosZ];
    
    new Float:angle = CameraData[cameraid][cam_RotZ];
    new Float:lookX, Float:lookY, Float:lookZ;
    
    lookX = camX + (10.0 * floatsin(angle, degrees));
    lookY = camY + (10.0 * floatcos(angle, degrees));
    lookZ = camZ;
    
    // Establece la cámara
    SetPlayerCameraPos(playerid, camX, camY, camZ);
    SetPlayerCameraLookAt(playerid, lookX, lookY, lookZ);
    
    new msg[128];
    format(msg, sizeof(msg), "INFO: Viendo camara ID %d. Usa /camara off para salir.", cameraid);
    SendClientMessage(playerid, COLOR_LIGHTGREEN, msg);
    
    return 1;
}

stock StopWatchingCamera(playerid)
{
    SetPlayerPos(playerid,
        PlayerCameraData[playerid][pc_OldPosX],
        PlayerCameraData[playerid][pc_OldPosY],
        PlayerCameraData[playerid][pc_OldPosZ]);
    
    SetPlayerFacingAngle(playerid, PlayerCameraData[playerid][pc_OldAngle]);
    SetPlayerInterior(playerid, PlayerCameraData[playerid][pc_OldInterior]);
    SetPlayerVirtualWorld(playerid, PlayerCameraData[playerid][pc_OldVirtualWorld]);
    
    SetCameraBehindPlayer(playerid);
    
    PlayerCameraData[playerid][pc_Watching] = false;
    
    SendClientMessage(playerid, COLOR_LIGHTGREEN, "INFO: Has dejado de ver la camara.");
    
    return 1;
}

