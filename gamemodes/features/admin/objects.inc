#define MAX_ADMIN_OBJECTS 100
#define DIALOG_OBJLIST 2000

enum AdminObjectData
{
    objID,
    Float:objX,
    Float:objY,
    Float:objZ,
    Float:objRX,
    Float:objRY,
    Float:objRZ
}
new AdminObjects[MAX_ADMIN_OBJECTS][AdminObjectData];
new adminObjectCount = 0;

alias:crearobjeto("crearobj", "crearob", "crearo")
CMD:crearobjeto(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");

    new modelid;
    if(sscanf(params, "d", modelid))
        return SendClientMessage(playerid, COLOR_GRAD2, "USO: /crearobjeto [modelid]");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    new obj = CreatePlayerObject(playerid, modelid, x, y, z, 0.0, 0.0, 0.0);

    EditPlayerObject(playerid, obj);

    if(adminObjectCount < MAX_ADMIN_OBJECTS)
    {
        AdminObjects[adminObjectCount][objID] = obj;
        AdminObjects[adminObjectCount][objX] = x;
        AdminObjects[adminObjectCount][objY] = y;
        AdminObjects[adminObjectCount][objZ] = z;
        AdminObjects[adminObjectCount][objRX] = 0.0;
        AdminObjects[adminObjectCount][objRY] = 0.0;
        AdminObjects[adminObjectCount][objRZ] = 0.0;
        adminObjectCount++;
    }
    SendClientMessage(playerid, COLOR_GRAD1, "Creaste el objeto ID: %d", modelid);
    return 1;
}

alias:editarobjeto("editarobj", "editarob", "editaro")
CMD:editarobjeto(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");

    new objIndex = -1;
    if(sscanf(params, "d", objIndex))
    {
        if(adminObjectCount == 0)
            return SendClientMessage(playerid, COLOR_FADE3, "No hay objetos para editar.");

        new dialogStr[512];
        for(new i = 0; i < adminObjectCount; i++)
        {
            new line[64];
            format(line, sizeof(line), "ID: %d (Model: %d)\n", i, GetPlayerObjectModel(playerid, AdminObjects[i][objID]));
            strcat(dialogStr, line);
        }
        Dialog_Show(playerid, DIALOG_OBJLIST, DIALOG_STYLE_LIST, "Selecciona un objeto para editar:", dialogStr, "Editar", "Cerrar");
        return 1;
    }

    if(objIndex < 0 || objIndex >= adminObjectCount)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de objeto inválido.");

    EditPlayerObject(playerid, AdminObjects[objIndex][objID]);
    SendClientMessage(playerid, COLOR_GRAD1, "Editando objeto seleccionado.");
    return 1;
}

alias:eliminarobjeto("eliminarobj", "eliminarob", "eliminaro")
CMD:eliminarobjeto(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");

    new objIndex;
    if(sscanf(params, "d", objIndex))
        return SendClientMessage(playerid, COLOR_GRAD2, "USO: /eliminarobjeto [id]");

    if(objIndex < 0 || objIndex >= adminObjectCount)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de objeto inválido.");

    DestroyPlayerObject(playerid, AdminObjects[objIndex][objID]);

    for(new i = objIndex; i < adminObjectCount - 1; i++)
    {
        AdminObjects[i][objID] = AdminObjects[i+1][objID];
        AdminObjects[i][objX] = AdminObjects[i+1][objX];
        AdminObjects[i][objY] = AdminObjects[i+1][objY];
        AdminObjects[i][objZ] = AdminObjects[i+1][objZ];
        AdminObjects[i][objRX] = AdminObjects[i+1][objRX];
        AdminObjects[i][objRY] = AdminObjects[i+1][objRY];
        AdminObjects[i][objRZ] = AdminObjects[i+1][objRZ];
    }
    adminObjectCount--;

    SendClientMessage(playerid, COLOR_GRAD1, "Objeto eliminado correctamente.");
    return 1;
}

Dialog:DIALOG_OBJLIST(playerid, response, listitem, inputtext[])
{
    if(listitem >= 0 && listitem < adminObjectCount)
    {
        EditPlayerObject(playerid, AdminObjects[listitem][objID]);
        SendClientMessage(playerid, COLOR_GRAD1, "Editando objeto seleccionado.");
    }
    return 1;
}