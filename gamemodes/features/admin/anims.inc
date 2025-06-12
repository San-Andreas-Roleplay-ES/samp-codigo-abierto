#define DIALOG_ANIMS 1000

static const animNames[][] = {
    "bailar",
    "sentarse",
    "saludar"
};

CMD:anims(playerid, params[])
{
    new dialogStr[256];
    for(new i = 0; i < sizeof(animNames); i++)
    {
        format(dialogStr, sizeof(dialogStr), "%s/%s\n", dialogStr, animNames[i]);
    }
    ShowPlayerDialog(playerid, DIALOG_ANIMS, DIALOG_STYLE_MSGBOX, "Animaciones", dialogStr, "Cerrar", "");
    return 1;
}

CMD:bailar(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");
    ApplyAnimation(playerid, "DANCING", "dnce_M_b", 4.1, 1, 0, 0, 0, 0);
    return 1;
}

CMD:sentarse(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");
    ApplyAnimation(playerid, "PED", "SEAT_idle", 4.1, 1, 0, 0, 0, 0);
    return 1;
}

CMD:saludar(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No puedes usar este comando dentro de un vehículo.");
    ApplyAnimation(playerid, "ON_LOOKERS", "wave_loop", 4.1, 1, 0, 0, 0, 0);
    return 1;
}