#define MAX_ADMIN_OBJECTS 100
#define DIALOG_OBJLIST 2000
#define DIALOG_OBJLIST_EDIT 2001
#define DIALOG_OBJLIST_DELETE 2002

enum AdminObjectData
{
    objID,
    modelID,
    Float:objX,
    Float:objY,
    Float:objZ,
    Float:objRX,
    Float:objRY,
    Float:objRZ,
    Float:originalX,
    Float:originalY,
    Float:originalZ,
    Float:originalRX,
    Float:originalRY,
    Float:originalRZ,
    creatorID,
    bool:isValid,
    bool:isBeingEdited
}

new AdminObjects[MAX_ADMIN_OBJECTS][AdminObjectData];
new adminObjectCount = 0;
new playerEditingObjectID[MAX_PLAYERS] = {-1, ...};
new bool:playerCreatingObject[MAX_PLAYERS] = {false, ...};

stock GetFreeObjectSlot()
{
    for(new i = 0; i < MAX_ADMIN_OBJECTS; i++)
    {
        if(!AdminObjects[i][isValid])
            return i;
    }
    return -1;
}

stock GetObjectSlotByID(objectid)
{
    for(new i = 0; i < MAX_ADMIN_OBJECTS; i++)
    {
        if(AdminObjects[i][isValid] && AdminObjects[i][objID] == objectid)
            return i;
    }
    return -1;
}

stock GetObjectSlotByInternalID(internalid)
{
    if(internalid >= 0 && internalid < MAX_ADMIN_OBJECTS && AdminObjects[internalid][isValid])
        return internalid;
    return -1;
}

stock IsValidObjectModel(modelid)
{
    if(modelid < 321 || modelid > 20000)
        return false;
    
    if(modelid >= 18632 && modelid <= 18649)
        return false;
    
    return true;
}

stock GetModelName(modelid)
{
    new name[32];
    format(name, sizeof(name), "Objeto %d", modelid);
    return name;
}

alias:crearobjeto("crearobj", "crearob", "crearo")
CMD:crearobjeto(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");

    if(playerCreatingObject[playerid] || playerEditingObjectID[playerid] != -1)
        return SendClientMessage(playerid, COLOR_FADE3, "Ya estás creando o editando un objeto.");

    new modelid;
    if(sscanf(params, "d", modelid))
        return SendClientMessage(playerid, COLOR_GRAD2, "USO: /crearobjeto [modelid]");

    if(!IsValidObjectModel(modelid))
        return SendClientMessage(playerid, COLOR_FADE3, "Model ID inválido. Usa un ID entre 321 y 20000.");

    new slot = GetFreeObjectSlot();
    if(slot == -1)
        return SendClientMessage(playerid, COLOR_FADE3, "Se ha alcanzado el límite máximo de objetos.");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    new Float:angle;
    GetPlayerFacingAngle(playerid, angle);
    x += (3.0 * floatsin(-angle, degrees));
    y += (3.0 * floatcos(-angle, degrees));

    new obj = CreatePlayerObject(playerid, modelid, x, y, z, 0.0, 0.0, 0.0);
    
    if(obj == INVALID_OBJECT_ID)
        return SendClientMessage(playerid, COLOR_FADE3, "Error al crear el objeto.");

    AdminObjects[slot][objID] = obj;
    AdminObjects[slot][modelID] = modelid;
    AdminObjects[slot][objX] = x;
    AdminObjects[slot][objY] = y;
    AdminObjects[slot][objZ] = z;
    AdminObjects[slot][objRX] = 0.0;
    AdminObjects[slot][objRY] = 0.0;
    AdminObjects[slot][objRZ] = 0.0;
    AdminObjects[slot][creatorID] = playerid;
    AdminObjects[slot][isValid] = false;
    AdminObjects[slot][isBeingEdited] = true;
    
    playerCreatingObject[playerid] = true;
    playerEditingObjectID[playerid] = slot;

    EditPlayerObject(playerid, obj);
    
    SendClientMessage(playerid, COLOR_GRAD1, "Creando objeto modelo %d. Posiciónalo y presiona ENTER para guardar o ESC para cancelar.", modelid);
    return 1;
}

alias:editarobjeto("editarobj", "editarob", "editaro")
CMD:editarobjeto(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");

    if(playerCreatingObject[playerid] || playerEditingObjectID[playerid] != -1)
        return SendClientMessage(playerid, COLOR_FADE3, "Ya estás creando o editando un objeto.");

    new internalID;
    if(sscanf(params, "d", internalID))
        return SendClientMessage(playerid, COLOR_GRAD2, "USO: /editarobjeto [id interno]");

    new slot = GetObjectSlotByInternalID(internalID);
    if(slot == -1)
        return SendClientMessage(playerid, COLOR_FADE3, "ID interno de objeto inválido.");

    if(AdminObjects[slot][isBeingEdited])
        return SendClientMessage(playerid, COLOR_FADE3, "Este objeto está siendo editado por otro jugador.");

    AdminObjects[slot][originalX] = AdminObjects[slot][objX];
    AdminObjects[slot][originalY] = AdminObjects[slot][objY];
    AdminObjects[slot][originalZ] = AdminObjects[slot][objZ];
    AdminObjects[slot][originalRX] = AdminObjects[slot][objRX];
    AdminObjects[slot][originalRY] = AdminObjects[slot][objRY];
    AdminObjects[slot][originalRZ] = AdminObjects[slot][objRZ];
    AdminObjects[slot][isBeingEdited] = true;

    playerEditingObjectID[playerid] = slot;
    EditPlayerObject(playerid, AdminObjects[slot][objID]);
    
    SendClientMessage(playerid, COLOR_GRAD1, "Editando objeto modelo %d (ID interno: %d). Presiona ENTER para guardar o ESC para cancelar.", AdminObjects[slot][modelID], internalID);
    return 1;
}

alias:eliminarobjeto("eliminarobj", "eliminarob", "eliminaro")
CMD:eliminarobjeto(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");

    new internalID;
    if(sscanf(params, "d", internalID))
        return SendClientMessage(playerid, COLOR_GRAD2, "USO: /eliminarobjeto [id interno]");

    new slot = GetObjectSlotByInternalID(internalID);
    if(slot == -1)
        return SendClientMessage(playerid, COLOR_FADE3, "ID interno de objeto inválido.");

    if(AdminObjects[slot][isBeingEdited])
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes eliminar un objeto que está siendo editado.");

    new Float:x = AdminObjects[slot][objX], Float:y = AdminObjects[slot][objY], Float:z = AdminObjects[slot][objZ];
    new modelid = AdminObjects[slot][modelID];    
    DestroyPlayerObject(AdminObjects[slot][creatorID], AdminObjects[slot][objID]);
    
    AdminObjects[slot][objID] = INVALID_OBJECT_ID;
    AdminObjects[slot][modelID] = 0;
    AdminObjects[slot][objX] = 0.0;
    AdminObjects[slot][objY] = 0.0;
    AdminObjects[slot][objZ] = 0.0;
    AdminObjects[slot][objRX] = 0.0;
    AdminObjects[slot][objRY] = 0.0;
    AdminObjects[slot][objRZ] = 0.0;
    AdminObjects[slot][creatorID] = INVALID_PLAYER_ID;
    AdminObjects[slot][isValid] = false;
    AdminObjects[slot][isBeingEdited] = false;

    adminObjectCount = 0;
    for(new i = 0; i < MAX_ADMIN_OBJECTS; i++)
    {
        if(AdminObjects[i][isValid])
            adminObjectCount++;
    }

    SendClientMessage(playerid, COLOR_GRAD1, "Objeto eliminado modelo %d (ID interno: %d) en posición %.2f, %.2f, %.2f", modelid, internalID, x, y, z);
    return 1;
}

alias:listarobjetos("listarobj", "listarob", "listaro")
CMD:listarobjetos(playerid, params[])
{
    if(adminObjectCount == 0)
        return SendClientMessage(playerid, COLOR_FADE3, "No hay objetos creados.");

    new dialogStr[2048];
    new count = 0;
    
    strcat(dialogStr, "ID\tModelo\tNombre\tPosición\n");
    
    for(new i = 0; i < MAX_ADMIN_OBJECTS; i++)
    {
        if(!AdminObjects[i][isValid]) continue;
        
        new line[128];
        
        format(line, sizeof(line), "%d\t%d\tObjeto %d\t%.1f, %.1f, %.1f\n", 
               i, AdminObjects[i][modelID], AdminObjects[i][modelID], 
               AdminObjects[i][objX], AdminObjects[i][objY], AdminObjects[i][objZ]);
        strcat(dialogStr, line);
        count++;
    }
    
    new title[64];
    format(title, sizeof(title), "Lista de Objetos (%d/%d)", count, MAX_ADMIN_OBJECTS);
    Dialog_Show(playerid, DIALOG_OBJLIST, DIALOG_STYLE_TABLIST_HEADERS, title, dialogStr, "Cerrar", "");
    return 1;
}

public OnPlayerEditObject(playerid, playerobject, objectid, response, Float:fX, Float:fY, Float:fZ, Float:fRotX, Float:fRotY, Float:fRotZ)
{
    if(!playerobject) return 1;
    
    new slot = playerEditingObjectID[playerid];
    if(slot == -1) return 1;
    
    if(AdminObjects[slot][objID] != objectid) return 1;
    
    switch(response)
    {
        case EDIT_RESPONSE_FINAL:
        {
            AdminObjects[slot][objX] = fX;
            AdminObjects[slot][objY] = fY;
            AdminObjects[slot][objZ] = fZ;
            AdminObjects[slot][objRX] = fRotX;
            AdminObjects[slot][objRY] = fRotY;
            AdminObjects[slot][objRZ] = fRotZ;
            AdminObjects[slot][isBeingEdited] = false;
            
            if(playerCreatingObject[playerid])
            {
                AdminObjects[slot][isValid] = true;
                adminObjectCount++;
                playerCreatingObject[playerid] = false;
                
                SendClientMessage(playerid, COLOR_GRAD1, "Objeto creado modelo %d (ID interno: %d) en posición %.2f, %.2f, %.2f", 
                                AdminObjects[slot][modelID], slot, fX, fY, fZ);
            }
            else
            {
                SendClientMessage(playerid, COLOR_GRAD1, "Objeto editado modelo %d (ID interno: %d) nueva posición %.2f, %.2f, %.2f", 
                                AdminObjects[slot][modelID], slot, fX, fY, fZ);
            }
            
            playerEditingObjectID[playerid] = -1;
        }
        
        case EDIT_RESPONSE_CANCEL:
        {
            if(playerCreatingObject[playerid])
            {
                DestroyPlayerObject(playerid, AdminObjects[slot][objID]);
                AdminObjects[slot][objID] = INVALID_OBJECT_ID;
                AdminObjects[slot][isValid] = false;
                AdminObjects[slot][isBeingEdited] = false;
                playerCreatingObject[playerid] = false;
                
                SendClientMessage(playerid, COLOR_FADE3, "Creación de objeto cancelada.");
            }
            else
            {
                SetPlayerObjectPos(playerid, objectid, AdminObjects[slot][originalX], AdminObjects[slot][originalY], AdminObjects[slot][originalZ]);
                SetPlayerObjectRot(playerid, objectid, AdminObjects[slot][originalRX], AdminObjects[slot][originalRY], AdminObjects[slot][originalRZ]);
                
                AdminObjects[slot][objX] = AdminObjects[slot][originalX];
                AdminObjects[slot][objY] = AdminObjects[slot][originalY];
                AdminObjects[slot][objZ] = AdminObjects[slot][originalZ];
                AdminObjects[slot][objRX] = AdminObjects[slot][originalRX];
                AdminObjects[slot][objRY] = AdminObjects[slot][originalRY];
                AdminObjects[slot][objRZ] = AdminObjects[slot][originalRZ];
                AdminObjects[slot][isBeingEdited] = false;
                
                SendClientMessage(playerid, COLOR_FADE3, "Edición cancelada. Objeto restaurado a su posición original.");
            }
            
            playerEditingObjectID[playerid] = -1;
        }
        
        case EDIT_RESPONSE_UPDATE:
        {
        // SendClientMessage(playerid, COLOR_GREY, "Posición: %.2f, %.2f, %.2f", fX, fY, fZ);
        }
    }
    
    return 1;
}

Dialog:DIALOG_OBJLIST(playerid, response, listitem, inputtext[])
{
    return 1;
}

Dialog:DIALOG_OBJLIST_EDIT(playerid, response, listitem, inputtext[])
{
    new slot = GetPVarInt(playerid, "SelectedObjectSlot");
    DeletePVar(playerid, "SelectedObjectSlot");
    
    if(slot == -1 || !AdminObjects[slot][isValid]) return 1;
    
    if(response)
    {
        new cmd[32];
        format(cmd, sizeof(cmd), "%d", slot);
        CallLocalFunction("OnPlayerCommandText", "is", playerid, sprintf("/editarobjeto %s", cmd));
    }
    else
    {
        new dialogStr[256];
        
        format(dialogStr, sizeof(dialogStr), 
               "¿Estás seguro de eliminar este objeto?\n\nModelo %d (ID interno: %d)\nPosición: %.2f, %.2f, %.2f",
               AdminObjects[slot][modelID], slot, AdminObjects[slot][objX], AdminObjects[slot][objY], AdminObjects[slot][objZ]);
        
        SetPVarInt(playerid, "DeleteObjectSlot", slot);
        Dialog_Show(playerid, DIALOG_OBJLIST_DELETE, DIALOG_STYLE_MSGBOX, "Confirmar Eliminación", dialogStr, "Sí", "No");
    }
    return 1;
}

Dialog:DIALOG_OBJLIST_DELETE(playerid, response, listitem, inputtext[])
{
    new slot = GetPVarInt(playerid, "DeleteObjectSlot");
    DeletePVar(playerid, "DeleteObjectSlot");
    
    if(!response) 
    {
        CallLocalFunction("OnPlayerCommandText", "is", playerid, "/listarobjetos");
        return 1;
    }
    
    if(slot == -1 || !AdminObjects[slot][isValid]) return 1;
    
    new cmd[32];
    format(cmd, sizeof(cmd), "%d", slot);
    CallLocalFunction("OnPlayerCommandText", "is", playerid, sprintf("/eliminarobjeto %s", cmd));
    
    SetTimerEx("ShowObjectListDelayed", 500, false, "i", playerid);
    return 1;
}

forward ShowObjectListDelayed(playerid);
public ShowObjectListDelayed(playerid)
{
    if(IsPlayerConnected(playerid))
        CallLocalFunction("OnPlayerCommandText", "is", playerid, "/listarobjetos");
}

stock InitObjectSystem()
{
    for(new i = 0; i < MAX_ADMIN_OBJECTS; i++)
    {
        AdminObjects[i][objID] = INVALID_OBJECT_ID;
        AdminObjects[i][modelID] = 0;
        AdminObjects[i][objX] = 0.0;
        AdminObjects[i][objY] = 0.0;
        AdminObjects[i][objZ] = 0.0;
        AdminObjects[i][objRX] = 0.0;
        AdminObjects[i][objRY] = 0.0;
        AdminObjects[i][objRZ] = 0.0;
        AdminObjects[i][creatorID] = INVALID_PLAYER_ID;
        AdminObjects[i][isValid] = false;
        AdminObjects[i][isBeingEdited] = false;
    }
    
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        playerEditingObjectID[i] = -1;
        playerCreatingObject[i] = false;
    }
    
    adminObjectCount = 0;
    printf("[ObjectSystem] Sistema de objetos inicializado.");
}

stock OnPlayerDisconnectObjectSystem(playerid)
{
    if(playerEditingObjectID[playerid] != -1)
    {
        new slot = playerEditingObjectID[playerid];
        
        if(playerCreatingObject[playerid])
        {
            if(AdminObjects[slot][objID] != INVALID_OBJECT_ID)
            {
                DestroyPlayerObject(playerid, AdminObjects[slot][objID]);
            }
            AdminObjects[slot][objID] = INVALID_OBJECT_ID;
            AdminObjects[slot][isValid] = false;
            AdminObjects[slot][isBeingEdited] = false;
        }
        else
        {
            if(AdminObjects[slot][isValid])
            {
                SetPlayerObjectPos(playerid, AdminObjects[slot][objID], 
                                 AdminObjects[slot][originalX], AdminObjects[slot][originalY], AdminObjects[slot][originalZ]);
                SetPlayerObjectRot(playerid, AdminObjects[slot][objID], 
                                 AdminObjects[slot][originalRX], AdminObjects[slot][originalRY], AdminObjects[slot][originalRZ]);
                
                AdminObjects[slot][objX] = AdminObjects[slot][originalX];
                AdminObjects[slot][objY] = AdminObjects[slot][originalY];
                AdminObjects[slot][objZ] = AdminObjects[slot][originalZ];
                AdminObjects[slot][objRX] = AdminObjects[slot][originalRX];
                AdminObjects[slot][objRY] = AdminObjects[slot][originalRY];
                AdminObjects[slot][objRZ] = AdminObjects[slot][originalRZ];
                AdminObjects[slot][isBeingEdited] = false;
            }
        }
        
        playerEditingObjectID[playerid] = -1;
        playerCreatingObject[playerid] = false;
    }
    
    DeletePVar(playerid, "SelectedObjectSlot");
    DeletePVar(playerid, "DeleteObjectSlot");
}

// Comando para teletransportarse a un objeto
alias:irobjeto("irobj", "gotoobj", "tpobj")
CMD:irobjeto(playerid, params[])
{
    new internalID;
    if(sscanf(params, "d", internalID))
        return SendClientMessage(playerid, COLOR_GRAD2, "USO: /irobjeto [id interno]");

    new slot = GetObjectSlotByInternalID(internalID);
    if(slot == -1)
        return SendClientMessage(playerid, COLOR_FADE3, "ID interno de objeto inválido.");

    new Float:x = AdminObjects[slot][objX];
    new Float:y = AdminObjects[slot][objY];
    new Float:z = AdminObjects[slot][objZ];
    
    SetPlayerPos(playerid, x + 2.0, y + 2.0, z + 1.0);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    
    SendClientMessage(playerid, COLOR_GRAD1, "Teletransportado al objeto modelo %d (ID interno: %d)", AdminObjects[slot][modelID], slot);
    return 1;
}

alias:objetostats("objstats", "ostats")
CMD:objetostats(playerid, params[])
{
    new validObjects = 0, beingEdited = 0;
    new Float:totalDistance = 0.0;
    new Float:playerX, Float:playerY, Float:playerZ;
    GetPlayerPos(playerid, playerX, playerY, playerZ);
    
    for(new i = 0; i < MAX_ADMIN_OBJECTS; i++)
    {
        if(AdminObjects[i][isValid])
        {
            validObjects++;
            if(AdminObjects[i][isBeingEdited])
                beingEdited++;
            
            totalDistance += GetPlayerDistanceFromPoint(playerid, AdminObjects[i][objX], AdminObjects[i][objY], AdminObjects[i][objZ]);
        }
    }
    
    new Float:avgDistance = validObjects > 0 ? totalDistance / validObjects : 0.0;
    
    SendClientMessage(playerid, COLOR_GRAD1, "=== Estadísticas del Sistema de Objetos ===");
    SendClientMessage(playerid, COLOR_GRAD2, "Objetos creados: %d/%d", validObjects, MAX_ADMIN_OBJECTS);
    SendClientMessage(playerid, COLOR_GRAD2, "Siendo editados: %d", beingEdited);
    SendClientMessage(playerid, COLOR_GRAD2, "Distancia promedio: %.2f metros", avgDistance);
    SendClientMessage(playerid, COLOR_GRAD2, "Slots libres: %d", MAX_ADMIN_OBJECTS - validObjects);
    
    return 1;
}

stock ObjectSystem_OnGameModeInit()
{
    InitObjectSystem();
    return 1;
}

// Función para ser llamada en OnPlayerDisconnect
stock ObjectSystem_OnPlayerDisconnect(playerid, reason)
{
    OnPlayerDisconnectObjectSystem(playerid);
    return 1;
}