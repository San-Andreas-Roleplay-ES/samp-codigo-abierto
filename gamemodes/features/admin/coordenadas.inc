#define MAX_COORDENADAS 100
#define MAX_NOMBRE 32

new Float: Coordenadas[MAX_COORDENADAS][4]; // 0: x, 1: y, 2: z, 3: usado (1/0)
new CoordenadaNombre[MAX_COORDENADAS][MAX_NOMBRE];
new coordenadaCount = 0;

alias:marcarcoord("marcarc")
CMD:marcarcoord(playerid, params[])
{
    new nombre[32];
    if(sscanf(params, "s[32]", nombre))
        return SendClientMessage(playerid, COLOR_GRAD2, "USO: /marcarcoord [nombre]");

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    new File:file = fopen("coordenadas.txt", io_append);
    if(file)
    {
        new str[128];
        format(str, sizeof(str), "%s %.2f %.2f %.2f\r\n", nombre, x, y, z);
        fwrite(file, str);
        fclose(file);
    }

    if(coordenadaCount < MAX_COORDENADAS)
    {
        format(CoordenadaNombre[coordenadaCount], MAX_NOMBRE, "%s", nombre);
        Coordenadas[coordenadaCount][0] = x;
        Coordenadas[coordenadaCount][1] = y;
        Coordenadas[coordenadaCount][2] = z;
        Coordenadas[coordenadaCount][3] = 1;
        coordenadaCount++;
    }

    SendClientMessage(playerid, COLOR_GRAD1, "Coordenada marcada y guardada con nombre: %s", nombre);
    return 1;
}

alias:listarcoords("listarc")
CMD:listarcoords(playerid, params[])
{
    new File:file = fopen("coordenadas.txt", io_read);
    if(!file)
    {
        SendClientMessage(playerid, COLOR_GRAD2, "No hay coordenadas guardadas.");
        return 1;
    }

    new str[128], nombre[32];
    new Float:x, Float:y, Float:z;
    new count = 0, id = 0;

    new encabezado[64];
    format(encabezado, sizeof(encabezado), "---------- Coordenadas guardadas -------------");
    SendClientMessage(playerid, COLOR_FADE2, encabezado);

    while(fread(file, str))
    {
        if(sscanf(str, "s[32]f f f", nombre, x, y, z)) continue;
        new msg[144];
        format(msg, sizeof(msg), "ID: %d | %s | %.2f, %.2f, %.2f", id, nombre, x, y, z);
        SendClientMessage(playerid, COLOR_FADE3, msg);
        id++;
        count++;
    }
    fclose(file);

    if(count == 0)
    {
        SendClientMessage(playerid, COLOR_FADE3, "No hay coordenadas guardadas.");
    }
    return 1;
}