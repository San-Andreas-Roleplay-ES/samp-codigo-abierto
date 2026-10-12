#if defined SP_INTEGRATION_INCLUDED
    #endinput
#endif
#define SP_INTEGRATION_INCLUDED

// Explicit integration ports. FILE v9 is the only active repository. These
// functions own no connection, credentials, SQL queries or recorded evaluations.
//
// TODO(SP-ACCESS-UI): add open/restricted settings and selection of one or more
// host factions/groups to zone administration, alongside the SP-ACCESS adapter.
// TODO(SP-SUPERVISION): add administrator-managed individual assignments and
// faction/group rank rules for scoped evaluation history queries/reviews.
// Recheck current host permissions per query/write; supervision does not grant
// practice access or configuration rights. See the administrative data model.

stock bool:PracticeHostIsAdministrator(playerid)
{
    // TODO(SP-ADMIN): resolve the host's cached staff role. Do not grant authority
    // from a whitelist, nickname, zone creator or recorded evaluation subject.
    return IsPlayerAdmin(playerid) != 0;
}

stock bool:PracticeHostCanUseZone(zone, playerid)
{
    #pragma unused zone
    // TODO(SP-ACCESS): file zones are open. SQL must load access_mode and grants
    // from its immutable version; intersect with cached, current host membership.
    // A restricted zone with unresolved identity or no match must deny access.
    return !IsPlayerNPC(playerid);
}

stock bool:PracticeHostResolveSubject(playerid, subjectKey[], size = sizeof(subjectKey))
{
    #pragma unused playerid
    // TODO(SP-DB-IDENTITY): resolve the persistent character through the host.
    // No fallback to playerid/nickname/RCON identity. BIGINT keys stay as strings.
    if (size > 0) subjectKey[0] = EOS;
    return false;
}

stock bool:PracticeRepositoryLoad()
{
    // TODO(SP-DB-SAVE/LOAD): stage versions, start rows, targets, permitted weapons,
    // indicators and access/history policy using the host's shared connection.
    // Map start sort_order through PracticeStartSlot; validate before publishing.
    // Never issue SQL from a shot callback or automatically write to both backends.
    return PracticeLoadConfiguration();
}

stock bool:PracticeRepositorySave()
{
    // TODO(SP-DB-ACK): SQL publishes one complete immutable zone version per
    // transaction, including starts/indicators/removals. Queuing is not success.
    // The future async request/ack API must retain stable zone/subject/request IDs,
    // the expected version and runtime revision; this bool remains FILE-only.
    return PracticeSaveConfiguration();
}

stock bool:PracticeRepositoryBootstrap()
{
    // TODO(SP-DB-BOOTSTRAP): in SQL mode wait for the host's connection and gate
    // configuration/start until validated loading completes. Do not open a connection.
    return PracticeRepositoryLoad();
}

stock bool:PracticeIntegrationBeforeStart(zone, const session[E_PRACTICE_SESSION],
    const origin[E_PRACTICE_POINT], startSlot, const slots[], const models[], const bool:noShoot[])
{
    #pragma unused origin, slots, models, noShoot
    if (!PracticeZoneExists(zone) || !PracticeIsStartSlot(startSlot)) return false;
    if (session[PracticePurpose] == SP_PURPOSE_TRAINING) return true;
    // TODO(SP-RECORDING/SP-DB-IDENTITY): evaluations require history_policy,
    // stable subject/version/start-row IDs, attempt key and UTC timestamps.
    // Register the frozen origin/rules/sequence before countdown, revalidating
    // ownership on the async reply. Add an explicit waiting state when implemented.
    new subjectKey[PRACTICE_SUBJECT_KEY_SIZE];
    if (!PracticeHostResolveSubject(session[PracticePlayer], subjectKey)) return false;
    // This placeholder cannot acknowledge a durable evaluation start.
    return false;
}

stock E_PRACTICE_HISTORY_STATUS:PracticeIntegrationOnEnd(zone, bool:completed, const reason[],
    const session[E_PRACTICE_SESSION], const origin[E_PRACTICE_POINT], startSlot,
    const slots[], const models[], const bool:noShoot[])
{
    #pragma unused zone, completed, reason, origin, startSlot, slots, models, noShoot
    if (session[PracticePurpose] == SP_PURPOSE_TRAINING) return SP_HISTORY_NOT_RECORDED;
    // TODO(SP-DB-RESULT): copy the immutable final result, chosen start and plan
    // before cleanup/disconnect, including partial counters and interruption.
    // reason is the localized interruption_message. Add stable reason codes at
    // call sites before recording SQL evaluations; never key history by chat text.
    // Queue a durable idempotent close/retry. Only a committed callback may claim
    // "saved". The unimplemented SQL port explicitly reports unavailable.
    return SP_HISTORY_UNAVAILABLE;
}

stock PracticeIntegrationShutdown()
{
    // TODO(SP-DB-SHUTDOWN): the host must snapshot/drain recorded evaluations
    // BEFORE closing its shared connection. OnGameModeExit alone cannot guarantee
    // an async flush. Training calls the port but creates no history or UI/audio.
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
        if (PracticeIsBusy(zone))
            PracticeIntegrationOnEnd(zone, false, "server_shutdown", PracticeSession[zone],
                PracticeSessionOrigin[zone], PracticeSessionStartSlot[zone],
                PracticeSequenceSlots[zone], PracticeSequenceModels[zone], PracticeSequenceNoShoot[zone]);
    return 1;
}
