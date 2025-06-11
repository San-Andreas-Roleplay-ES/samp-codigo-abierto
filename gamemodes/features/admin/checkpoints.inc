#define MAX_CHECKPOINTS 100

enum CheckpointData
{
    Float:cpX,
    Float:cpY,
    Float:cpZ
}
new Checkpoints[MAX_CHECKPOINTS][CheckpointData];
new checkpointCount = 0;

alias:marcarcoord("marcarc")
CMD:marcarcoord(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    new msg[128];
    format(msg, sizeof(msg), "Coordenadas marcadas: X: %.2f, Y: %.2f, Z: %.2f", x, y, z);
    SendClientMessage(playerid, COLOR_GRAD1, msg);

    if(checkpointCount < MAX_CHECKPOINTS)
    {
        Checkpoints[checkpointCount][cpX] = x;
        Checkpoints[checkpointCount][cpY] = y;
        Checkpoints[checkpointCount][cpZ] = z;
        checkpointCount++;
        SendClientMessage(playerid, COLOR_GRAD2, "Checkpoint guardado en la lista.");
    }
    else
    {
        SendClientMessage(playerid, COLOR_FADE3, "Límite de checkpoints alcanzado.");
    }

    new File:file = fopen("checkpoints.txt", io_append);
    if(file)
    {
        new line[64];
        format(line, sizeof(line), "%.2f %.2f %.2f\r\n", x, y, z);
        fwrite(file, line);
        fclose(file);
        SendClientMessage(playerid, COLOR_GRAD2, "Checkpoint guardado en checkpoints.txt.");
    }
    return 1;
}

alias:listarcoords("listarc")
CMD:listarcoords(playerid, params[])
{
    if(checkpointCount == 0)
        return SendClientMessage(playerid, COLOR_FADE3, "No hay checkpoints guardados.");

    new msg[128];
    for(new i = 0; i < checkpointCount; i++)
    {
        format(msg, sizeof(msg), "ID: %d | X: %.2f, Y: %.2f, Z: %.2f", i, Checkpoints[i][cpX], Checkpoints[i][cpY], Checkpoints[i][cpZ]);
        SendClientMessage(playerid, COLOR_GRAD1, msg);
    }
    return 1;
}