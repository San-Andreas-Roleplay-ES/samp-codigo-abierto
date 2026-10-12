#if defined SP_REGISTRY_INCLUDED
    #endinput
#endif
#define SP_REGISTRY_INCLUDED

#define PRACTICE_ZONE_NAME_SIZE (49)
#define PRACTICE_ZONE_PAGE_SIZE (10)
// The file backend allocates monotonically increasing positive 32-bit IDs.
// Array indices are private runtime handles; archived IDs are never recycled.
// TODO(SP-DB-ID): map the host's persistent SQL IDs without truncating BIGINTs.
new PracticeZoneID[PRACTICE_MAX_ZONES];
new PracticeZoneName[PRACTICE_MAX_ZONES][PRACTICE_ZONE_NAME_SIZE];
new bool:PracticeZoneEnabled[PRACTICE_MAX_ZONES], bool:PracticeZoneArchived[PRACTICE_MAX_ZONES];
new bool:PracticeZoneIndicators[PRACTICE_MAX_ZONES];
new PracticeNextZoneID = 1, PracticeRegistryRevision;
new bool:PracticeRegistrySavePending, bool:PracticeStorageBlocked;
new PracticeSelectedZone[MAX_PLAYERS] = {-1, ...};
new PracticeSelectedZoneID[MAX_PLAYERS];

// Host policy is supplied through the explicit integration port.
// Configuration authority stays separate from participation/supervision grants.
stock bool:PracticeIsAdministrator(playerid)
{
    return playerid >= 0 && playerid < MAX_PLAYERS && IsPlayerConnected(playerid)
        && !IsPlayerNPC(playerid) && PracticeHostIsAdministrator(playerid);
}

stock bool:PracticeZoneExists(zone)
{
    return zone >= 0 && zone < PRACTICE_MAX_ZONES && PracticeZoneID[zone] > 0;
}

stock bool:PracticeSelectedZoneValid(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    new zone = PracticeSelectedZone[playerid];
    return PracticeZoneExists(zone) && !PracticeZoneArchived[zone]
        && PracticeSelectedZoneID[playerid] == PracticeZoneID[zone];
}

stock PracticeFindZone(id)
{
    if (id <= 0) return -1;
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
        if (PracticeZoneID[zone] == id) return zone;
    return -1;
}

stock bool:PracticeParseZoneID(const input[], &id)
{
    new length = strlen(input);
    id = 0;
    if (length < 1 || length > 10) return false;
    for (new i = 0; i < length; i++)
    {
        if (input[i] < '0' || input[i] > '9') return false;
        new digit = input[i] - '0';
        if (id > (2147483647 - digit) / 10) return false;
        id = id * 10 + digit;
    }
    return id > 0;
}

stock bool:PracticeZoneNameValid(const name[])
{
    if (!name[0] || strlen(name) >= PRACTICE_ZONE_NAME_SIZE) return false;
    new bool:visible;
    for (new i = 0; name[i]; i++)
    {
        if (name[i] < ' ' || name[i] == '{' || name[i] == '}') return false;
        if (name[i] != ' ') visible = true;
    }
    return visible;
}

stock PracticeInitializeRegistry()
{
    PracticeResetDiscovery();
    new empty[E_PRACTICE_POINT];
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
    {
        PracticeZoneID[zone] = 0;
        PracticeZoneName[zone][0] = EOS;
        PracticeZoneEnabled[zone] = PracticeZoneArchived[zone] = PracticeSavePending[zone] = PracticeZoneIndicators[zone] = false;
        PracticeRevision[zone]++;
        PracticeRules[zone] = PracticeDefaultRules;
        for (new slot = 0; slot < PRACTICE_POINT_COUNT; slot++) PracticePoints[zone][slot] = empty;
    }
    PracticeNextZoneID = 1;
    PracticeRegistryRevision++;
    PracticeRegistrySavePending = PracticeStorageBlocked = false;
    return 1;
}

stock PracticeCreateZone(const name[])
{
    if (PracticeStorageBlocked || !PracticeZoneNameValid(name) || PracticeNextZoneID >= 2147483647) return -1;
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
    {
        if (PracticeZoneID[zone]) continue;
        PracticeZoneID[zone] = PracticeNextZoneID++;
        format(PracticeZoneName[zone], PRACTICE_ZONE_NAME_SIZE, "%s", name);
        PracticeZoneEnabled[zone] = PracticeZoneArchived[zone] = PracticeZoneIndicators[zone] = false;
        PracticeRules[zone] = PracticeDefaultRules;
        PracticeRevision[zone]++;
        PracticeRegistryRevision++;
        return zone;
    }
    return -1;
}

stock bool:PracticeBindZone(playerid, zone)
{
    if (!PracticeZoneExists(zone) || PracticeZoneArchived[zone]) return false;
    // Reset owned UI against its old binding before changing the selection.
    PracticeResetUI(playerid);
    PracticeSelectedZone[playerid] = zone;
    PracticeSelectedZoneID[playerid] = PracticeZoneID[zone];
    return true;
}
