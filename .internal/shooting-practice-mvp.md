# Shooting Practice MVP - Implementation Handoff

Updated: 2026-10-12. Multiple-zone administration, multiple starts per zone, independent zone sessions/editing,
stable IDs, proximity hints, optional start pickups and v9 file persistence/migration are implemented. Existing model editing,
private previews, mixed shoot/no-shoot scoring, permitted weapons, decimal timing and results remain.
The native object selector now uses ordinary-font cards, category filters and pagination. Two glass
bottles, a beverage can and a traffic cone extend the catalogue to nine models. The initial catalogue
was deployed through an explicitly requested restart. The latest pickup/report/layout and opening
corrections compile and pass isolated Windows SA-MP 0.3.DL-R1 tests. They were applied to the local
server on 2026-10-11 after the user's explicit start request. The user accepted the current MVP
and closed this development iteration. Account/faction adapters, restricted access, SQL and recorded
history remain future integrations, described in the data model linked below.

- Fork: https://github.com/SouthViking/samp-codigo-abierto
- Branch: feat/shooting-practice-mvp
- Upstream: https://github.com/San-Andreas-Roleplay-ES/samp-codigo-abierto
- Original upstream base: e7f84d1294a523fe2aa7c0e5f1c6ae1e8d87f907.
- Public handoff; internal does not mean private.
- [Configuration/testing guide](shooting-practice-testing.md) and [Windows server](local-windows-server.md).
- [Administrative data model proposal](shooting-practice-data-model.md) and [draft SQL](sql/shooting-practice.sql).

## Review entry point and demonstration

Start with the [PR review summary](shooting-practice-pr.md) for the implemented scope, commands,
reproduction steps, verification and remaining host integration. This document describes the
runtime architecture, editing/session lifecycle, scoring, persistence and extension boundaries.

[Author-provided MVP demonstration on YouTube](https://www.youtube.com/watch?v=cpVQbs0igC8).
The demonstration accompanies the playable MVP. SQL, restricted access and recorded evaluations
are future scope, and the later integration ports are verified separately by compilation/tests.

## Integration preparation after MVP closure (2026-10-11)

The SQL proposal now matches multiple starts and zone-level pickup preference: sixteen tables,
sp_zone_starts as immutable version children, and a same-version chosen-start FK on evaluations.
Its names/coordinate/weapon/timing limits match current configuration. The SQL model remains a proposal.

integration.inc supplies explicit ports called by the existing load/save, authority/access, start/end
and shutdown paths. File v9 remains the only backend. Training is explicitly frozen in PracticePurpose
and remains unrecorded. Missing stable identity and evaluation storage fail instead of claiming success.
The source update compiles and passes 1291 Pawn assertions plus 54 real MySQL 8.0.34 schema checks.
The running server retains the accepted MVP build until a later explicitly requested restart.

## Current implementation

A bounded registry supports up to PRACTICE_MAX_ZONES (64 by default) independent zones, including
archived records. Each has a stable positive ID, name, active/inactive state, up to ten starts and up to ten targets.
Array slots are private handles; IDs increase monotonically and archived IDs/slots are not recycled.
One participant per zone and one active round per player; different zones can operate simultaneously.
Each position appears once per round, one at a time. Faction/civilian ranges can reuse the engine.
Results can inform an instructor's assessment. Formal certification is outside this MVP.

Targets store optional name (32 characters), model, independent behavior, XYZ position, XYZ rotation,
interior/world. Catalogue: box 1271, armed silhouettes 1583/1584, civilian silhouettes 1585/1586,
glass bottles 1543/1544, beverage can 2601 and traffic cone 1238.
No custom downloads or new plugins. The box gets red DISPARAR / blue NO DISPARAR material.
All other models retain their native textures. Instructors should use distinguishable figures and explain
their criteria. Model and behavior are independent.

Shoot-target impact: +1; expiry: omission. Any participant discharge during no-shoot, including
air/scenery, gives -1 and one identification error, then retires that target. No-shoot expiry is correct.
Net points = hits - errors, without clamping. Both minimum points and maximum errors determine passing.
Precision separately uses all counted shots, including repeated attempts and interval shots.
Extra shots lower accuracy without changing net points, identification errors or passing thresholds.
The recommended clean exercise uses one shot per shoot target and none per protected target.

One or more permitted weapon IDs 22-34, selectable independently. Players may switch between them;
unlisted weapons interrupt the round. Defaults: only 24, 1 point, 0 errors, preparation/exposure/interval 3/5/1 seconds,
fixed order/distribution. Preparation accepts whole seconds from 1-30. Exposure accepts 0.1-60.0
seconds and interval 0.0-30.0 seconds, in tenths, using a dot or comma decimal separator. Start radius 1.5 m, movement radius
3 m, targets 3-100 m from every post in the same interior/world. Equipment is never granted, removed or replenished.
Spanish user-facing text; English implementation identifiers.

## Configuration, ownership and editing

/practicaconfig requires RCON and opens a paginated zone list (ten per page), with creation and global
save retry. /practicaconfig <id> opens a nonarchived zone directly; invalid/overflowing IDs fail.
Names have up to 48 characters and reject whitespace-only values, controls and color codes.
New zones start inactive. Activation requires a complete valid configuration; edits which make an
active layout incomplete close it automatically until an administrator fixes and activates it again.

The zone root offers a start-point list, targets, rules, preview toggle, eligible start/own cancellation,
confirmed reset, rename, activation/deactivation and confirmed archive. Archiving removes the zone
from lists/usage but retains its ID/configuration in storage; there is no restore/purge UI this increment.
Occupied zones cannot be edited, disabled, reset or archived. Other zones remain available.
Failed saves add a retry row; the zone list exposes pending writes even after an archive.
Configuration dialogs reuse easyDialog. The object selector is an owned PlayerTextDraw panel.
Volver/Zonas returns to the registry.

Iniciar práctica appears only when PracticeStartReady succeeds for this player: completed layout,
alive/on-foot access, start proximity, same dimension, permitted held weapon, no participant or editor.
The command uses the same gate. Cancelar práctica appears only for the participant. Main-row actions
are captured per rendered menu, so optional rows cannot shift reset/retry into another action.
Start eligibility is rechecked on response, including movement after opening the dialog.

Rules > Armas permitidas toggles each weapon and saves immediately; at least one must remain enabled.
Volver returns to rules. Each numeric input explains its effect, units, bounds, zero where applicable,
and current value. Minimum net points and maximum identification errors are independent passing
requirements. Without shoot targets, the points input explains that an objective must be added first.

Puntos de inicio opens a bounded list of up to ten posts. Añadir punto captures the current
character position/facing/dimension and saves immediately. Selecting a point offers name, relocation
to the character position, and confirmed deletion. Names are optional, up to 32 characters, with
Punto N as the fallback. The root preview action remains in place. Reset uses Reiniciar and clears
all starts, targets and rules. Removing the last start retains targets/rules and closes the zone.

Posts share the zone's targets/rules and one exclusive session. Additional posts must share the
dimension, remain within 100 m of other posts, and place existing targets 3-100 m away.
Target editing validates against every marked post. A sole start retains legacy relocation behavior:
same-dimension moves retain targets and may close an invalid zone, while dimension changes require
confirmation to clear the old targets. With multiple posts a dimension change is rejected.

Start selection uses the nearest marked post within 1.5 m in the player's dimension, with stable
slot order for ties. The selected origin is frozen for movement checks, public cues, environmental
messages and target dimension. Moving to another post does not change an ongoing round's origin.
Objects remain public, with one active participant per zone. Different zones remain independent.
Partial configurations can be saved. Readiness is checked before practicing.

Creation chooses model and behavior, then opens native object editing. The object selector has
six cards per page, with names and native 3D previews, and filters Todos, Siluetas, Botellas, Latas,
Otros. All text uses ordinary GTA font 1, without the old gothic header. Atrás or ESC returns to
an existing draft, or cancels an unplaced new target. Filters and pages do not save changes.
Only handles owned by the open panel can select a bound catalogue index. Each interaction rechecks
draft ownership, zone revision, RCON, position and dimension. Partial allocation failure rolls back
the panel. Selection ownership clears before native cancellation can trigger the ESC callback.
The panel needs at most 38 player textdraws and retains the cursor across redraws.
Backgrounds and click areas use font-4 LD_SPAC:white rectangles with explicit sizes. Font-1 captions
are visual only, centered with padding inside the card/button. Hover uses dark slate for contrast.
Opening is queued for 150 ms on the existing 50 ms pulse so the native dialog closes first.
Pending work rechecks the draft and is cancelled by UI cleanup. Cancellation acknowledgements are
tracked before CancelSelectTextDraw, including when delivered after another panel opens.
Connection/disconnection clears acknowledgements for reused player IDs. Existing targets offer
name/model/behavior, move/rotate, duplicate, confirmed delete, then save as the last row.
New drafts end with save after move/rotate. The behavior menu consistently uses Blanco protegido. Draft changes are applied only
by Save or EDIT_RESPONSE_FINAL; cancel discards all pending fields. Duplicates fill a free slot without
changing their source. Choosing behavior sets fixed distribution on commit; administrators may then
enable random distribution explicitly. Model/name/position-only edits preserve distribution.

Target/start lists and target/start edit menus offer Cerrar menú with Continuar con /practicaconfig
as the detail. Closing hides only the modal, preserving the private setup view, current section,
selected start and owned target draft. Bare /practicaconfig or the matching explicit zone ID resumes
that section; ordinary bare-command zone listing remains unchanged when no section is suspended.
Lists refresh after another writer changes the zone. A stale selected-start context returns to
the list instead of acting on a reused slot. Closing never saves a draft; Save remains the final row.
Paused drafts retain writer ownership, while paused lists do not reserve the range. Lifecycle,
permission/range loss and zone changes clear the context through the existing UI cleanup boundary.

Each zone has one writer and its own revision. Other admins can inspect it but cannot mutate it while
owned; its practice starts wait for commit/discard. Mutation requires alive/on-foot RCON state and is
blocked only by that zone's round/editor. Other zones can be configured concurrently, including while
another zone is occupied. Menus capture stable selected ID and zone revision; zone lists capture
registry revision and stable row IDs. Unrelated changes do not invalidate a zone's draft/menu.

The native editor owns a private temporary object, separate from saved records and session objects.
FINAL coordinates are authoritative. UPDATE only previews on the client. Finite values, distance and
dimension are checked and XYZ rotation normalized before committing. Invalid placement offers
retry/discard while retaining the previous saved record. Ownership clears before native cancellation.
Target creation/editing must begin within 100 m of the start in the same dimension, before allocating a draft
or opening the wizard. The initial new-object position is kept inside the same 3-100 m final bounds.
Every native FINAL rechecks finite coordinates, distance and dimension. Leaving the 100 m editing
radius also releases the draft and references.
Death, disconnect, vehicles, dimension changes, lost RCON or lost temporary objects release resources.
The direct ID command resumes pending edits for the same zone. Switching zones discards the old draft;
commands cannot replace an active native editor until confirmation/cancellation.

General and practice editors mutually exclude via existing playerEditingObjectID and SP_ObjectEditor
PVar. No general object records are reused. Vendored ShowModelSelectionMenuEx now declares read-only
array parameters const. YSI OnPlayerEditDynObject uses its supported Dyn alias to avoid symbol truncation.

## Private preview

Vista previa closes the dialog and shows a player-restricted dynamic checkpoint at the saved start,
all objects with their exact rotations, and named behavior labels without a duplicated numeric prefix. The administrator can walk
around and reopen the root to hide it. No scoring, countdown or sounds.

Each viewer owns independent handles. Hide/death/disconnect/vehicles/dimension changes/lost RCON clean
them. Partial allocation failures roll back everything. Confirmed changes synchronize affected slots.
Unchanged objects keep their handles and remain visible. Successful practice starts remove views
of that zone and any preview owned by its participant. Other zones retain their previews. Only owned dynamic checkpoints are used,
never SetPlayerCheckpoint or DisablePlayerCheckpoint.

Opening Blancos also enables a private automatic setup view through catalog, draft and native editing.
Only the actively edited slot's saved ghost is removed, replaced by the owned native editing object.
Other saved references stay visible. Save/cancel restore the saved slot, retaining neighbours.
A per-view point cache compares complete records and updates only changed/invalid slots.
Returning to the root clears the automatic view, while an explicitly enabled root preview remains
active. ResetUI and the shared tick clean both modes on lifecycle/access/distance changes.

Hits on owned preview objects return the YSI break-with-zero value from OnPlayerShootDynObject,
vetoing the bullet before the scoring callback. The client can already have broken a native model
locally, so the next 50 ms pulse recreates just the struck private slot, preserving its saved model,
position, rotation and material. Repeated hits coalesce. Neighbours, other viewers and the native
editing object retain their handles. Hide/access loss cannot resurrect a queued slot. Allocation
failure removes the owned view and leaves configuration intact. This is bullet repair, not a GTA
model invulnerability flag. Melee, explosions and rendered replacement timing need client checks.

## Runtime, randomization and scoring

/practica resolves active starts within 1.5 m in the current interior/world. One candidate starts
directly; overlapping starts open explicit selection (also available to nonadministrators). Selection
captures IDs and rechecks proximity, availability, occupancy and weapon on response. /practica <id>
selects explicitly and still requires being at that zone's start. /practica cancelar interrupts only
the caller's active zone; /practica ayuda explains the active/nearby zone's rules without starting.
Public selection never grants administrative access or changes its zone binding.
Any non-NPC can currently practice; PracticeCanUseStation is the future permission integration point.

IDLE -> COUNTDOWN -> TARGET -> INTERVAL -> TARGET -> ... -> IDLE.

Start validates readiness, access, alive/on-foot state, dimension, proximity, held weapon and occupancy.
The sequence, model/behavior pairs, permitted weapon set and thresholds freeze once. Fixed order uses ascending marked slots.
Fisher-Yates provides independent randomization:

- Random order moves position/model/behavior together.
- Random distribution moves model and behavior together between positions, preserving exact counts and
  associations. Rotations belong to positions, so use compatible orientations.
- Saved assignments stay intact. A sequence can legitimately repeat.

Session targets are dimension-restricted but visible to observers; only the participant scores.
Creation/material/streaming failures interrupt and clean up.
OnPlayerWeaponShot processes each discharge once, without also counting the dynamic-object callback.
Shoot impacts resolve through Streamer_GetItemStreamerID to the active target; for no-shoot, any
configured-weapon discharge is an error regardless of hit type/ID.

Countdown shots are excluded; interval shots count for precision. Expiry resolves before a late shot
can affect anything. Final-deadline shots lie outside the completed scoring window. A phase retires
before its object is destroyed, preventing duplicate scoring/omission. Complete rounds satisfy
hits + omissions = shoot count; correct ignores + errors = no-shoot count; hits + errors <= shots.
Interrupted rounds do not classify unpresented targets.

One gamemode-owned 50 ms timer polls session deadlines and pending preview repair. It is killed on
mode exit and holds no player-specific delayed callback. The shared 500 ms task checks editor/view
validity. Tick differences handle signed wraparound. Transitions can wait for the next pulse,
including interval 0. Exposure and interval are integer milliseconds, in 100 ms steps. Leaving the radius, changing dimension, equipping/firing an unlisted weapon,
death, disconnect, vehicle entry or lost objects interrupt. Ammo exhaustion, including switching to
fists, continues if the last permitted weapon's slot is empty; other slots may still contain ammo.
No teleport/freeze.

## Roleplay/audio

One neutral preparation action uses the existing roleplay name. One environmental /do-style line
announces closure. Leaving the radius/dimension replaces the generic closure with
"* Nombre Apellido abandona el puesto de tiro." using the participant's roleplay name, once.
Only the participant receives the final result/reason. Completed results use eight chat lines:
an opening divider, CAMPO DE TIRO title, five unindented rows pairing fields, and a closing divider.
The opening/closing dividers share the same 94-dash string so their lengths cannot drift. Title, dividers
and inline pipe separators use soft blue #A8C4D9. Data and labels stay white, with only Aprobado
green or Reprobado red. The rows cover minimum points/maximum errors, hits/expired targets,
protected targets respected/identification errors, shots/precision, and net points/verdict.
Precision includes the literal % sign, with no parenthesis. Incomplete rounds use the same header,
one white Incompleta reason line and closing divider. No per-line [Práctica] prefix.

Built-in sounds: 1056 each countdown second, 1057 each successfully created/streamed target,
1139 at closure. First appearance is the start signal; both behaviors sound identical.
A real-clock guard keeps at least 1000 ms between delivered countdown cues and between the last
countdown cue and first appearance. Delayed ticks skip missed values without a burst, and a late
final cue extends preparation instead of shortening its pause. SA-MP timer/network delays can still
make cues arrive late. Audio/actions explicitly reach non-NPC observers within 10 m
in the station's dimension; they do not assume 3D attenuation.

A small centered player textdraw reads Comienza en 3, 2, 1 (or the configured preparation time).
Only the participant sees it; it follows the same tick/beeps and disappears at first target or any
interruption. It owns its handle, cleans on disconnect/ID reuse, and does not replace unrelated UI.
Textdraw allocation failure preserves the audible countdown. The plain text avoids GTA font accents.

## Storage and migration

File: scriptfiles/shooting_practice.cfg relative to server working directory. The Windows launcher
uses .local-server/scriptfiles/shooting_practice.cfg. New saves use SARP_PRACTICE 9:

```text
SARP_PRACTICE 9
REGISTRY <nextPositiveId> <recordCount>
ZONE <id> <enabled:0|1> <archived:0|1> <indicators:0|1>
ZONE_NAME <name>
<weaponMask> <points> <preparation> <exposureMs> <intervalMs> <maxErrors> <randomOrder> <randomDistribution>
<marked> <x> <y> <z> <rotZ> <interior> <world> <noShoot> <model> <rotX> <rotY>
NAME <optional point name>
... (twenty point records: original start, ten target slots, nine additional starts)
END_ZONE
... (one record for each zone, including archived zones)
END
```

Preparation is whole seconds; exposure and interval are integer milliseconds in 100 ms steps.
Weapon bits use (weapon ID - 22): default Desert Eagle = 4, all supported weapons = 8191.
Flags are 0/1. IDs must be unique/positive/below nextPositiveId; active archived records are invalid.
Unmarked/start points have model 0 and noShoot 0. Marked models must be in the catalog; names reject
controls and color braces. Partial disabled configurations may be saved, but cannot start.

Formats 1-6 migrate into public zone #1, Campo de tiro, preserving existing layout/rules/model names
and transforms. A playable legacy layout stays active; an incomplete one loads inactive. V1-V4
convert their single weapon into one mask bit; V1-V3 use model 1271 and default X/Y rotation/name.
V2 materializes its old protected quota on highest marked slots. V5 keeps the mask. V1-V5 exposure
and interval convert validated seconds to milliseconds once; V6 retains exact milliseconds.
Loading does not rewrite the source; the next confirmed edit/save upgrades the whole registry to v9.
V7 retains every zone, ID, state, rule and target, with its former start becoming point one.
V8 preserves all twenty fixed point records per zone, allowing sparse starts and partial layouts
without a start. Marked points share a dimension, starts have no model/target behavior, and active
readiness requires all targets to fit every post. An empty installation starts with no zones.

Loading validates all temporary records before publication and refuses reload while any zone is
occupied or has a draft. Loading clears owned UI, increments revisions and rebinds selected stable IDs.
V9 retains those point records and adds the per-zone indicator flag. Formats 1-8 default it to off.
An active incomplete v7/v8/v9 record is safely closed and exposes pending repair. Corrupt/unsupported,
duplicate-ID or truncated files cannot partially replace live records.

Saving writes shooting_practice.cfg.tmp and validates it before touching the primary. It preserves a
validated shooting_practice.cfg.bak before copying the staged file, then validates the new primary.
SA-MP lacks a native atomic rename; this is validated staging plus recovery, not a SQL transaction.
Startup can recover the validated backup after a torn primary, or the initial stage if the primary is
missing, and exposes repair/retry in the list. A valid primary wins over abandoned staging.
If all available files are invalid, writes/creation are blocked to preserve evidence for manual repair.
Failed saves keep changes in memory and show retry; restarting before successful save can lose them.
Keep external backups of valuable layouts. Archived records consume registry capacity by design.
Runtime state, attempts, credentials, logs and binaries remain ignored. Old include exports are not imported.

## Future administrative requirements (2026-10-08)

The complete design below includes future access/history requirements. The current increment implements
the registry, administration, independent ownership and file persistence. All current zones are public
and /practica is unrecorded. Do not describe access lists, supervision or SQL as implemented.

- Administrators create, select, configure and delete independent named zones. Each zone has a stable
  ID, up to ten start points, a common interior/world, up to ten targets, its own rules and practice-access grants.
  A configurable registry capacity bounds server resources; the design must not assume one zone.
- Only administrators can configure zones. /practicaconfig opens their zone list; /practicaconfig <id>
  opens that zone's configuration directly. Both paths and menu responses enforce administrative access.
  The list includes zone ID/name, operational state, access mode and evaluation availability.
- Practice has exactly two access modes: open, or restricted to one or more groups/factions.
  Restricted access requires at least one matching whitelist entry from current host membership.
  There are no individual-user practice grants. Open mode stores an empty whitelist; restricted mode
  with no grants or unresolved identity never becomes public.
- Administrative authority and practice access are separate. Being a permitted trainee or member
  of a permitted faction does not grant zone configuration rights. RCON is the current test policy.
- One participant and one editing owner per zone; one active attempt per player. Different zones
  may run independently. Locks, save state, UI revisions, previews and cleanup must belong to a zone.
  Edits/deletion of an occupied zone are blocked, while other zones remain usable.
- /practica resolves an eligible nearby zone in the player's interior/world. If several match, use
  explicit selection. Administrative menus retain the selected zone ID and revalidate it on responses.
- Proposed session purposes: /practica is unrecorded training; /evaluaciontiro is a recorded evaluation.
  Both share the same engine, zone rules, access checks and occupancy. Purpose is fixed before start;
  a finished training round cannot be promoted into an evaluation after its result is known.
- Each immutable zone version has an administrator-controlled history_policy: none (default) allows
  training only; evaluations_only also allows recorded evaluations. This policy is independent of
  open/restricted access. Turning it off prevents new evaluations and preserves existing history.
  Passed, failed and interrupted evaluations all remain visible to authorized supervisors.
- Supervision is separate from configuration and participation. Individual assignments explicitly
  cover the whole zone; group/faction rules use a minimum rank or exact selected ranks and cover only
  attempts admitted through that group. One current rule per zone/group; can_review additionally
  permits observations within the same scope. Permissions use current host identity/membership/rank,
  are rechecked per attempt/query/write, and never alter automatic scores. Numeric rank IDs do not
  encode hierarchy. Rank bridges/rules/selections are included in the SQL proposal, not runtime code.
  Instructor/FTO roles and different rank thresholds for reading versus annotating are later extensions.
- Configuration, access grants and future results reference stable zone/account IDs. SA-MP playerid,
  array slots, object handles and GetTickCount are temporary values and cannot identify stored users
  or zones. Deletion must invalidate menus, drafts and runtime lookups before an array slot is reused.

This repository includes a_mysql and a configured MySQL plugin, but has no account login, persistent
player account IDs, user-group/faction membership service, database connection or application queries.
GetPlayerRoleplayName only formats the connected nickname; it is not an identity provider. The zone
registry and editor are implemented here; restricted access still needs the host membership adapters. Real account/group/faction grants
need adapters to the host server's existing identity and membership services; development fixtures
must remain explicitly separate from production authorization.

## Persistence and future integration boundaries

The supported prototype backend is the versioned local configuration file described above. Confirmed
configuration changes survive restart. Session state and reports live only in memory; completed and
interrupted attempts are not saved. The explicit integration ports provide real file operations and unavailable/skipped identity/history
placeholders. They contain no SQL queries, host connection or fabricated persistence acknowledgements.

Confirmed changes follow PracticeConfigurationChanged -> PracticeSaveAndNotify ->
PracticeRepositorySave -> PracticeSaveConfiguration. Start, target creation/edit/duplicate/delete, rules and reset already use
this path. Keep persistence at that boundary rather than adding database calls to individual dialogs,
native editor UPDATE callbacks, previews, weapon-shot callbacks or timer ticks.

Search the production module for TODO(SP- to locate the planned boundaries:

| Marker | Location | Integration work |
| --- | --- | --- |
| SP-ACCESS/HISTORY | config.inc | Versioned zone access grants and history policy, once host identity adapters exist. |
| SP-ADMIN | integration.inc, PracticeHostIsAdministrator | Replace RCON with the host administrative role; retain separate training/supervision grants. |
| SP-DB-ID | registry.inc | Map persistent SQL BIGINT identifiers without truncation into runtime handles. |
| SP-ACCESS | integration.inc, PracticeHostCanUseZone | Open mode or a zone-specific group/faction whitelist checked against cached host membership. |
| SP-ACCESS-UI | integration.inc, future zone administration dialogs | Let staff select open/restricted access and assign one or more host factions/groups to a zone. |
| SP-SUPERVISION | integration.inc, future history queries/reviews | Add staff-managed supervisor assignments and group/rank rules; recheck the allowed attempt scope for each query/write. |
| SP-RECORDING | integration.inc, PracticeIntegrationBeforeStart | Select training/evaluation purpose and freeze zone history policy; gate evaluations on durable start registration. |
| SP-DB-SAVE/LOAD | integration.inc, PracticeRepositoryLoad | Replace validated file persistence with host-connection transactions and immutable configuration versions/identity references. |
| SP-DB-ACK | integration.inc, PracticeRepositorySave | Confirm committed asynchronous writes, pending/retry state per zone/revision, safe requester notification. |
| SP-DB-IDENTITY | integration.inc, PracticeHostResolveSubject / BeforeStart | For recorded evaluations, capture stable subject/zone/attempt IDs, UTC start and configuration/sequence snapshot. |
| SP-DB-RESULT | integration.inc, PracticeIntegrationOnEnd before cleanup | For evaluations only, save immutable final counters, frozen rules, completion/reason and timestamps once per attempt. |
| SP-DB-BOOTSTRAP | integration.inc, PracticeRepositoryBootstrap via OnGameModeInit | Load after the host connection is ready; gate zone usage/editing until validated loading finishes. |
| SP-DB-SHUTDOWN | integration.inc, PracticeIntegrationShutdown via OnGameModeExit | Record active evaluations as incomplete and coordinate flushing before the shared connection closes. |

The zone registry/ownership and legacy migration are now implemented. Restricted practice, real
account/group/faction identities and history policy require the host adapters described in the SQL
proposal. The file registry currently contains public zones without access grants/history settings.
Do not invent account or faction IDs or infer authorization from display names.

For a database backend, use the host gamemode's shared connection. Persist each complete zone and
its starts/indicators/rules/targets/grants consistently, including removals/reset. Load into temporary records and use
the existing validators before publication. Incomplete layouts may be configured but cannot start.
Choose one authoritative backend; automatic file/database dual writes are not part of this version.
Queued writes do not mean success. Late load/save callbacks must not overwrite a newer zone state,
clear a newer pending revision or notify a different player after playerid reuse. PracticeRevision
currently protects menus within one run and is not a persistent database revision.

When history is added, /practica remains unrecorded and creates no history rows. /evaluaciontiro requires
an eligible zone version, stable identity and confirmed durable start registration before countdown;
unavailable storage never silently downgrades an evaluation to training. Capture identity, a unique
attempt ID, UTC timestamps and a configuration/sequence snapshot for evaluated sessions at start.
PracticeEnd already copies final counters before cleanup and is the common path for
completion, cancellation, death, disconnect and other interruptions. Store the completion flag and
reason; only completed attempts receive an approved/failed verdict. Derive net points and precision
with the same rules as the existing report; zero shots have unavailable precision. An immutable
snapshot must remain usable after disconnect, even when notify is false. Record once per attempt with
an idempotent key; database errors must not hold occupancy or prevent normal cleanup/results.
If closing persistence fails, keep a durable final snapshot for retry and explicitly report pending
or failed storage; displaying a local result does not confirm that the evaluation was saved.
The start row alone cannot recover final counters after a crash. Failed and cancelled evaluations
must be retained alongside passed ones; supervisors see stored evaluations, not unrecorded training.
Game-mode shutdown calls the close port for every active session before cleanup, without UI/audio.
The port currently skips training. Durable interrupted-evaluation capture/flush needs the host backend. Database-backed history remains future work.

PracticeIsAdministrator centralizes the current RCON policy for commands, zone list/dialogs and
ongoing editor/view checks. PracticeAdminAvailable additionally requires being alive/on foot.
Connect the central check to the host's cached administrative role when integrating it. Participation and
future supervision grants stay separate from configuration authority.

## File boundaries

| File under gamemodes/features/shooting_practice/ | Responsibility |
| --- | --- |
| config.inc | Records, model catalog, rules, readiness. |
| registry.inc | Stable IDs, zone metadata, player selection, capacity. |
| storage.inc | FILE whole-registry serialization, migration, validation, recovery and save status. |
| integration.inc | Host authority/access/identity, repository bootstrap/save/load, session and shutdown ports. |
| sequence.inc | Frozen positions, pairs and randomization. |
| editor.inc | Draft ownership, private preview, native editor lifecycle. |
| start_setup.inc | Multiple start-point menus, capture, naming, relocation and deletion. |
| setup.inc | Selected-zone dialogs, confirmations and revision guards. |
| catalogue.inc | Native object cards, category/page bindings, selection ownership and cleanup. |
| zone_setup.inc | Administrative registry commands/lists/lifecycle and public proximity selection. |
| session.inc | Per-zone sessions, player lookup, target lifecycle and scoring. |
| presentation.inc | Materials, audio, countdown overlay, roleplay, help/results. |
| discovery.inc | Public decorative start pickups, proximity chat hints and resource lifecycle. |
| hooks.inc | YSI callbacks, 50 ms pulse, 500 ms editor maintenance, shots, cleanup. |
| header.inc | Module include order. |

## Validation and scope

- 1291 assertions, 0 failures on isolated Windows SA-MP 0.3.DL-R1 with sscanf 2.13.1, normal exit.
  Deterministic adapters simulate player/UI/streamer/audio. Pawn VM, parsers and file I/O are real.
  New start coverage includes CRUD, bounded/sparse lists, stale/revoked dialogs, common geometry,
  multiple private markers and cleanup, nearest-post selection, public objects, exclusive zone use,
  frozen origin/departure, v7 migration and v8 round trips/corrupt added starts.
  Close/resume tests cover retained views/drafts, creation from a new position, matching/bare IDs,
  stale records, zone changes, save ordering and permission/lifecycle/range cleanup.
  Existing multi-zone, scoring/editor/timing/report and recovery regressions pass.
  Discovery coverage includes per-zone pickup lifecycle, allocation rollback, sparse posts, dimension filters,
  private entry hints with hysteresis, independent flags, v1-v8 migration and rejected v9 metadata.
  Tests live in tests/shooting_practice_discovery.inc, tests/shooting_practice_starts.inc,
  tests/shooting_practice_pause.inc and the existing test modules.
  Catalogue coverage in tests/shooting_practice_catalogue.inc checks card/model/name bindings,
  ordinary font IDs, native preview dimensions, filtering, page bounds, cursor ordering,
  back/ESC cancellation re-entry, permission/revision/range loss and allocation rollback.
  Expanded models round-trip through v9, preserve model/behavior pairs under randomization,
  and use the existing public spawn and one-hit/air-shot scoring paths.
  New regressions cover full rectangle coverage, padded captions and button click areas,
  deferred opening, delayed native cancellation echoes, pending-draft cleanup, the lowered pickup
  and matching report borders.
  Integration tests verify real file save/load/bootstrap calls, separate staff/public access,
  unresolved stable identity, recorded-start rejection, origin/plan snapshot before cleanup
  including no-notification disconnect, rejected-start cleanup and silent unrecorded shutdown.
  The SQL proposal passes 54 real MySQL 8.0.34 assertions in a disposable independent instance.
- Full SARP.pwn and tests compile with Pawn 3.10.10, -;+ -(+ -d3 -E. Existing gamemode warning
  suppressions remain. Unix full compilation normalizes include separators in an ignored copy only.
  Spanish Pawn source and the legacy root README use Windows-1252; feature documents use UTF-8. Static dialog buffers avoid enlarging the
  gamemode's existing stack allowance.
- The accepted playable MVP build, including the selector corrections, lowered pickups and matching
  report borders, was compiled and started locally on 2026-10-11 after the explicit service request.
  UDP status confirmed 127.0.0.1:7777, all seven plugins loaded, and startup reported no load errors.
  The saved layout hash stayed unchanged across that start. The user subsequently accepted the
  current MVP and requested development closure. Detailed evidence remains in tests/README.md.
  Mouse hit areas, GXT accents, model rendering/collisions/native breakage and actual
  easyDialog/YSI/timer dispatch are manual client checks, not claims of automatic coverage.

Pending: real faction/group restricted access, host identities, SQL backend, history policy,
/evaluaciontiro and authorized supervisor queries/reviews. Their normalized draft is documented,
not applied to a database or enabled by runtime placeholders. No fake persistence acknowledgments.

Later extensions: leaderboards, formal certification, equipment administration, custom assets,
moving/simultaneous targets within a round, scoring rings and live scoring HUD.

## Engine references

- [TextDraw rectangle sizing](https://open.mp/docs/scripting/functions/PlayerTextDrawTextSize): font 4/5 uses explicit width and height.

- [GTA object IDs](https://wiki.multitheftauto.com/wiki/IDE_List): catalogue model names/IDs.
- [Native model previews](https://open.mp/docs/scripting/functions/PlayerTextDrawSetPreviewModel).
- [Native cursor cancellation](https://open.mp/docs/scripting/functions/CancelSelectTextDraw): emits the ESC callback.

- [Streamer 2.9.6 shot dispatch](https://github.com/samp-incognito/samp-streamer-plugin/blob/v2.9.6/src/callbacks.cpp):
  the dynamic-object callback can veto a reported shot. Owned repairs run after dispatch to avoid
  mutating the streamer's iterated object registry.
- [SA-MP timer limitations](https://open.mp/docs/scripting/functions/SetTimer): polling is approximate,
  so GetTickCount guards actual deadlines and cue spacing.
- [Built-in sounds](https://open.mp/docs/scripting/resources/sound-ids): 1056 is RACE_321 and 1057 is RACE_GO.

## Spanish interface wording

The registry title is Zonas de tiro; the selected-zone root is Zona #ID: Nombre. Readiness and
pending-save state live in its content. Volver/Zonas navigates back to the registry. Dialogs use
short titles without pipe separators or semicolons in user-facing text. Target behavior is
Blanco de tiro (disparar) or Blanco protegido (no disparar).
Weapon fields show names. Dialog/chat/3D-label strings use Windows-1252; the catalog back button is
Atrás, with GTA font 1's GXT glyph 0x98 for á. The custom-list selector accepts an optional back_text
label, preserving defaults for other callers. Glyph reference: https://github.com/gta-reversed/gta-reversed/blob/master/source/game_sa/GxtChar.cpp.

## Start indicators and discovery (2026-10-10)

Indicadores de inicio is a saved per-zone toggle, off for new zones and all formats 1-8.
It creates one public Desert Eagle pickup (model 348, type 1) per marked start, 0.65 m below
its saved character-origin Z in the same interior/world. This lowers the previous position by
one meter to compensate for the native floating weapon presentation. Pickups have no public labels, grant no equipment,
and do not start a round. They remain visible during a practice. Disabling or archiving the
zone hides them while preserving the preference. Resetting the layout restores off.
Position changes refresh only the affected pickup. Allocation failures remove partial
zone sets and maintenance can retry. Loading and shutdown dispose of owned handles.

Independent of pickups, living non-NPC players on foot receive a private chat hint when
entering the /practica radius (1.5 m) of an active playable start. The latch stays until
leaving 3 m from that post, preventing boundary spam. Overlapping entries emit one hint.
Dialogs/drafts and active participation suppress hints. This uses the same access seam
as the command, with no new faction grant or public 3D text.

Engine references: [model IDs](https://open.mp/docs/scripting/resources/pickupids) and
[type 1 scripted pickups](https://open.mp/docs/scripting/resources/pickuptypes).
