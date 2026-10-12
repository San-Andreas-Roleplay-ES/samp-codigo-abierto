# Local Windows development server

Updated: 2026-10-12. This guide describes reproducible setup and operation for the shooting-practice
MVP. Use `-Action Status` to check the current process instead of treating past deployment notes
as live status. The accepted playable build was started on 2026-10-11. Later integration ports
compile and pass isolated tests but require a requested stop/start to replace that running build.

See the [PR review summary](shooting-practice-pr.md), [configuration/testing guide](shooting-practice-testing.md)
and [MVP demonstration](https://www.youtube.com/watch?v=cpVQbs0igC8) for the feature's current scope.

## Run

From the repository root in Windows PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Start
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Status
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Stop
```

Add **127.0.0.1:7777** to a compatible SA-MP 0.3.DL-R1 client's favourites on the same computer. The mode reports `Servidor de Prueba`. Start builds the current sources, launches the server in the background, and checks both the startup log and the SA-MP UDP query response. Starting again reports the existing instance. To apply code changes, stop and start it again. Closing the launcher's window leaves the server running.

Stop sends an authenticated local RCON `exit` command and waits for shutdown. It only controls this checkout's executable and has a checked process fallback if graceful shutdown fails. Build can also be run independently with `-Action Build` while the server is stopped.

The local configuration is `.local-server/server.cfg`, the runtime log is `.local-server/server_log.txt`, and exports are written under `.local-server/scriptfiles/`. The local RCON password is generated randomly during preparation and is not printed by these scripts. Read the local configuration to log in using `/rcon login <your local password>`, then follow the [shooting practice testing guide](shooting-practice-testing.md).

## Reproduce the installation

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/prepare-local-server.ps1
```

Run this with the local server stopped. The script downloads the pinned packages in `scripts/local-server-packages.json`, checks their SHA256 hashes, and installs explicit files. Existing local configuration and scriptfiles data are preserved. The current directory is not significant because paths are resolved relative to the scripts.

The server runs in `.local-server/`, which Git ignores in full. The compiler executable and DLL are installed in `pawno/`, already covered by the repository's binary ignore rules. Existing includes and the committed `server.cfg` template are preserved.

| Component | Pinned source |
| --- | --- |
| SA-MP server | [0.3.DL-R1 Windows archive linked by the repository README](https://sampforum.blast.hk/files/03DL/samp03DL_svr_R1_win32.zip) |
| Pawn compiler | [3.10.10](https://github.com/pawn-lang/compiler/releases/tag/v3.10.10) |
| CrashDetect | [4.22](https://github.com/Y-Less/samp-plugin-crashdetect/releases/tag/v4.22) |
| Streamer | [2.9.6](https://github.com/samp-incognito/samp-streamer-plugin/releases/tag/v2.9.6) |
| sscanf | [2.13.1](https://github.com/Y-Less/sscanf/releases/tag/v2.13.1) |
| Pawn.CMD | [3.3.6](https://github.com/katursis/Pawn.CMD/releases/tag/3.3.6) |
| Pawn.RakNet | [1.6.0](https://github.com/katursis/Pawn.RakNet/releases/tag/1.6.0) |
| MySQL plugin and supporting DLLs | [R41-4](https://github.com/pBlueG/SA-MP-MySQL/releases/tag/R41-4) |
| FCNPC-DL, bundled ColAndreas and node data | [2.0.11](https://github.com/ziggi/FCNPC/releases/tag/v2.0.11) |
| ColAndreas collision database | [Community database pinned to commit 3a362e5](https://github.com/xorweak/rzmp-gamemode/blob/3a362e5ff50c8d051abcf14aa434f7d597b92033/scriptfiles/colandreas/ColAndreas.cadb) |

The collision database is an existing community copy, not an extraction from this machine's GTA installation. Its `cadf` v2 header declares 11,362 models and 44,759 placements. It loaded successfully. The official [ColAndreas instructions](https://github.com/Pottus/ColAndreas/wiki/Instructions) describe generating a replacement from a game's installation using its wizard if needed later.

FCNPC-DL bundles ColAndreas and registers the `CA_*` natives, so the local plugin line loads FCNPC-DL once and omits the redundant standalone ColAndreas plugin. Seven plugins load in total. MySQL loads as a library; the current test gamemode does not connect to a database server.

The local configuration binds to loopback, retains lag compensation for shot callbacks, and uses a one-second CrashDetect slow-call threshold to accommodate map/YSI initialization. YSI's `scriptfiles/YSI/fixes` and `scriptfiles/YSI/temp` directories are created during preparation. Firewall/router settings are not needed for same-PC access.

## Validation and limits

- The accepted playable MVP was compiled and started on 2026-10-11 after an explicit request.
  All seven plugins and the ColAndreas database loaded, startup reached `OnGameModeInit`, and
  SA-MP UDP status confirmed `127.0.0.1:7777`. The saved configuration hash stayed unchanged
  across that start. The user accepted this development iteration.
- The later integration preparation compiles the full gamemode with Pawn 3.10.10 and warnings
  treated as errors. It passes **1291 Pawn assertions** and **54 schema checks on real MySQL 8.0.34**.
  See [tests/README.md](../tests/README.md) for commands, recorded evidence and test coverage.
  These ports retain file persistence and unrecorded training. They do not enable a SQL backend.
- New configuration saves use **file format v9**, with zones, multiple posts, targets, rules,
  names, active/archive state and optional indicators. Formats 1-8 migrate on load without
  rewriting the source. A later confirmed save writes v9. See the
  [storage contract](shooting-practice-mvp.md#storage-and-migration).
- Runtime files, credentials, caches, AMX files, test databases and saved layouts stay local
  under ignored paths. Each contributor creates their own configuration and RCON password.
- Automated adapters simulate player/UI/streamer/audio behavior. Native client rendering,
  collision, sound, accents and callback dispatch remain manual checks. The
  [testing guide](shooting-practice-testing.md) includes the client scenarios.
- Windows SA-MP 0.3.DL-R1 is the reproducible runtime verified here. A Linux deployment
  requires its own compatible plugin binaries and a runtime smoke test.

During in-game feedback, restart the playable server only on explicit request. Running
`Start` against an existing instance reports its status and does not apply a source update.

## Development shortcut

`/irpruebas` requires RCON and an alive, on-foot player. It moves only the caller to
802.8580, 1674.5205, 5.2813 at angle 181.2324, interior/world 0. Zone configuration is independent
of this convenience command. After teleporting, use `/practicaconfig` to create/select a zone.
