#define MAX_COORDENADAS 100
#define MAX_NOMBRE 32

new Float: Coordenadas[MAX_COORDENADAS][5]; // 0: x, 1: y, 2: z, 3: angle, 4: usado (1/0)
new CoordenadaNombre[MAX_COORDENADAS][MAX_NOMBRE];
new coordenadaCount = 0;

alias:marcarcoord("marcarc")
CMD:marcarcoord(playerid, params[])
{
    new nombre[32];
    if(sscanf(params, "s[32]", nombre))
        return SendClientMessage(playerid, COLOR_GRAD2, "USO: /marcarcoord [nombre]");

    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);

    new File:file = fopen("coordenadas.txt", io_append);
    if(file)
    {
        new str[128];
        format(str, sizeof(str), "%s %.2f %.2f %.2f %.2f\r\n", nombre, x, y, z, angle);
        fwrite(file, str);
        fclose(file);
    }

    if(coordenadaCount < MAX_COORDENADAS)
    {
        format(CoordenadaNombre[coordenadaCount], MAX_NOMBRE, "%s", nombre);
        Coordenadas[coordenadaCount][0] = x;
        Coordenadas[coordenadaCount][1] = y;
        Coordenadas[coordenadaCount][2] = z;
        Coordenadas[coordenadaCount][3] = angle;
        Coordenadas[coordenadaCount][4] = 1;
        coordenadaCount++;
    }

    SendClientMessage(playerid, COLOR_GRAD1, "Coordenada marcada y guardada con nombre: %s (Angle: %.2f)", nombre, angle);
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
    new Float:x, Float:y, Float:z, Float:angle;
    new count = 0, id = 0;

    new encabezado[64];
    format(encabezado, sizeof(encabezado), "---------- Coordenadas guardadas -------------");
    SendClientMessage(playerid, COLOR_FADE2, encabezado);

    while(fread(file, str))
    {
        // Intentar leer con ángulo primero (nuevo formato)
        if(sscanf(str, "s[32]ffff", nombre, x, y, z, angle))
        {
            // Si falla, intentar leer sin ángulo (formato antiguo)
            if(sscanf(str, "s[32]fff", nombre, x, y, z))
                continue;
            angle = 0.0; // Ángulo por defecto para coordenadas antiguas
        }
        
        new msg[144];
        format(msg, sizeof(msg), "ID: %d | %s | %.2f, %.2f, %.2f, %.2f", id, nombre, x, y, z, angle);
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

CMD:irpos(playerid, params[])
{
    new Float:posX, Float:posY, Float:posZ, vw, interior;

    if(sscanf(params, "fffD(0)D(0)", posX, posY, posZ, vw, interior)) 
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /irpos [x] [y] [z] [virtualworld] [interior]");

    SetPlayerPos(playerid, posX, posY, posZ);
    SetPlayerInterior(playerid, interior);
    SetPlayerVirtualWorld(playerid, vw);

    SendClientMessage(playerid, COLOR_GRAD2, "Has sido teletransportado a las coordenadas: X: %.2f, Y: %.2f, Z: %.2f", posX, posY, posZ);
    return 1;
}