static const animNames[][] = {
    "bailar",
    "sentarse",
    "saludar"
};

CMD:anims(playerid, params[])
{
    new dialog[512];

    for(new i = 0; i < sizeof(animNames); i++)
    {
        strcat(dialog, va_return("/%s\n", animNames[i]));
    }

    ShowPlayerDialog(playerid, 0, DIALOG_STYLE_MSGBOX, "Animaciones", dialog, "Cerrar", "");
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

CMD:pararanim(playerid, params[])
{
    ClearAnimations(playerid);
    return 1;
}

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{    
    if(newkeys & KEY_FIRE)
    {
       
        ClearAnimations(playerid);
    }
    return 1;
}