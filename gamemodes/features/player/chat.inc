#define CHAT_DISTANCE_NORMAL 5.0
#define CHAT_DISTANCE_ME 10.0
#define CHAT_DISTANCE_DO 10.0
#define CHAT_DISTANCE_OOC 10.0

stock SendProximityMessage(playerid, Float:range, color, const message[])
{
    if(!IsPlayerConnected(playerid)) return 0;
    
    new Float:x, Float:y, Float:z;
    new virtualworld = GetPlayerVirtualWorld(playerid);
    new interior = GetPlayerInterior(playerid);
    
    GetPlayerPos(playerid, x, y, z);
    
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(!IsPlayerConnected(i)) continue;
        if(GetPlayerVirtualWorld(i) != virtualworld) continue;
        if(GetPlayerInterior(i) != interior) continue;
        
        if(IsPlayerInRangeOfPoint(i, range, x, y, z))
        {
            SendClientMessage(i, color, message);
        }
    }
    return 1;
}

stock GetPlayerRoleplayName(playerid, name[], size = sizeof(name))
{
    GetPlayerName(playerid, name, size);
    
    for(new i = 0; i < strlen(name); i++)
    {
        if(name[i] == '_')
        {
            name[i] = ' ';
        }
    }
    return 1;
}

public OnPlayerText(playerid, text[])
{
    if(!IsPlayerConnected(playerid)) return 0;
    
    new name[MAX_PLAYER_NAME], rpName[MAX_PLAYER_NAME];
    new string[256];
    
    GetPlayerName(playerid, name, sizeof(name));
    GetPlayerRoleplayName(playerid, rpName, sizeof(rpName));
    
    format(string, sizeof(string), "%s dice: %s", rpName, text);
    SendProximityMessage(playerid, CHAT_DISTANCE_NORMAL, COLOR_WHITE, string);
    
    return 0;
}

CMD:limpiarchat(playerid, params[])
{
    for(new i = 0; i < 50; i++)
    {
        SendClientMessage(playerid, -1, " ");
    }
    return 1;
}

CMD:me(playerid, params[])
{
    if(isnull(params))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Uso: /me [acción]");
        return 1;
    }
    
    new name[MAX_PLAYER_NAME], rpName[MAX_PLAYER_NAME];
    new string[256];
    
    GetPlayerName(playerid, name, sizeof(name));
    GetPlayerRoleplayName(playerid, rpName, sizeof(rpName));
    
    format(string, sizeof(string), "* %s %s", rpName, params);
    SendProximityMessage(playerid, CHAT_DISTANCE_ME, COLOR_PURPLE, string);
    
    return 1;
}

CMD:do(playerid, params[])
{
    if(isnull(params))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Uso: /do [entorno]");
        return 1;
    }
    
    new name[MAX_PLAYER_NAME], rpName[MAX_PLAYER_NAME];
    new string[256];
    
    GetPlayerName(playerid, name, sizeof(name));
    GetPlayerRoleplayName(playerid, rpName, sizeof(rpName));
    
    format(string, sizeof(string), "* %s (( %s ))", params, rpName);
    SendProximityMessage(playerid, CHAT_DISTANCE_DO, COLOR_PURPLE, string);
    
    return 1;
}

CMD:b(playerid, params[])
{
    if(isnull(params))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Uso: /b [OOC]");
        return 1;
    }
    
    new name[MAX_PLAYER_NAME], rpName[MAX_PLAYER_NAME];
    new string[256];
    
    GetPlayerName(playerid, name, sizeof(name));
    GetPlayerRoleplayName(playerid, rpName, sizeof(rpName));
    
    format(string, sizeof(string), "(( %s: %s ))", rpName, params);
    SendProximityMessage(playerid, CHAT_DISTANCE_OOC, COLOR_GREY, string);
    
    return 1;
}

alias:gritar("gr")
CMD:gritar(playerid, params[])
{
    if(isnull(params))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Uso: /gr [mensaje]");
        return 1;
    }
    
    new name[MAX_PLAYER_NAME], rpName[MAX_PLAYER_NAME];
    new string[256];
    
    GetPlayerName(playerid, name, sizeof(name));
    GetPlayerRoleplayName(playerid, rpName, sizeof(rpName));
    
    format(string, sizeof(string), "%s grita: ¡%s!", rpName, params);
    SendProximityMessage(playerid, CHAT_DISTANCE_NORMAL * 2, COLOR_WHITE, string);
    
    return 1;
}

alias:cercano("c")
CMD:cercano(playerid, params[])
{
    if(isnull(params))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Uso: /c [mensaje]");
        return 1;
    }
    
    new name[MAX_PLAYER_NAME], rpName[MAX_PLAYER_NAME];
    new string[256];
    
    GetPlayerName(playerid, name, sizeof(name));
    GetPlayerRoleplayName(playerid, rpName, sizeof(rpName));
    
    format(string, sizeof(string), "%s dice en voz baja: %s", rpName, params);
    SendProximityMessage(playerid, CHAT_DISTANCE_NORMAL / 2, COLOR_FADE3, string);
    
    return 1;
}

CMD:ame(playerid, params[])
{
    if(isnull(params))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Uso: /ame [acción corta]");
        return 1;
    }
    
    new string[64];
    
    format(string, sizeof(string), "> %s", params);
    SendClientMessage(playerid, COLOR_PURPLE, string);
    
    new Text3D:floatingText;
    format(string, sizeof(string), "%s", params);
    floatingText = Create3DTextLabel(string, COLOR_PURPLE, 0.0, 0.0, 0.0, 15.0, 0, 1);
    Attach3DTextLabelToPlayer(floatingText, playerid, 0.0, 0.0, 0.3);
    
    SetTimerEx("Remove3DTextLabelTimer", 4000, false, "i", _:floatingText);
    
    return 1;
}

forward Remove3DTextLabelTimer(Text3D:labelid);
public Remove3DTextLabelTimer(Text3D:labelid)
{
    Delete3DTextLabel(labelid);
    return 1;
}
