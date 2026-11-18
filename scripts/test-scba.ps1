[CmdletBinding()]
param(
    [string]$CompilerExecutable = '',
    [string]$ServerExecutable = '',
    [switch]$NativeAttachments,
    [string]$NpcPlugin = ''
)

$ErrorActionPreference = 'Stop'
$repositoryPath = Split-Path -Parent $PSScriptRoot
if (!$CompilerExecutable) { $CompilerExecutable = Join-Path $repositoryPath 'pawno/pawncc.exe' }
if (!$ServerExecutable) {
    $ServerExecutable = Join-Path $repositoryPath '.local-server/samp-server.exe'
    if (!(Test-Path -LiteralPath $ServerExecutable)) { $ServerExecutable = Join-Path $repositoryPath 'samp-server.exe' }
}
foreach ($path in @($CompilerExecutable, $ServerExecutable)) {
    if (!(Test-Path -LiteralPath $path -PathType Leaf)) {
        throw 'Install Pawn 3.10.10 and SA-MP 0.3.DL-R1, or provide -CompilerExecutable and -ServerExecutable.'
    }
}
$CompilerExecutable = (Resolve-Path -LiteralPath $CompilerExecutable).ProviderPath
$ServerExecutable = (Resolve-Path -LiteralPath $ServerExecutable).ProviderPath
$testPort = 7781
while (@(Get-NetUDPEndpoint -LocalPort $testPort -ErrorAction SilentlyContinue).Count -or
       @(Get-NetTCPConnection -LocalPort $testPort -ErrorAction SilentlyContinue).Count) {
    $testPort++
    if ($testPort -gt 7791) { throw 'No isolated test port is available in 7781-7791.' }
}
$testPath = Join-Path $repositoryPath ('.local-server/scba-tests/' + [guid]::NewGuid().ToString('N'))
foreach ($folder in @('', 'gamemodes', 'scriptfiles', 'models/scba')) {
    New-Item -ItemType Directory -Path (Join-Path $testPath $folder) | Out-Null
}
Copy-Item -LiteralPath $ServerExecutable -Destination (Join-Path $testPath 'samp-server.exe')
foreach ($assetFile in Get-ChildItem -LiteralPath (Join-Path $repositoryPath 'models/scba') -File) {
    if ($assetFile.Extension -notin @('.dff', '.txd')) { continue }
    $name = $assetFile.Name
    $asset = $assetFile.FullName
    $bytes = [IO.File]::ReadAllBytes($asset)
    $expectedChunk = if ([IO.Path]::GetExtension($name) -eq '.dff') { 0x10 } else { 0x16 }
    if ($bytes.Length -lt 12 -or [BitConverter]::ToUInt32($bytes, 0) -ne $expectedChunk -or
        [BitConverter]::ToUInt32($bytes, 4) + 12 -ne $bytes.Length) {
        throw ('Invalid RenderWare asset header: ' + $name)
    }
    Copy-Item -LiteralPath $asset -Destination (Join-Path $testPath ('models/scba/' + $name))
}
# Font-4 sprites live in the same streamed dictionary as the face mask.
$maskBytes = [IO.File]::ReadAllBytes((Join-Path $testPath 'models/scba/mask.txd'))
$textureNames = @{}
for ($offset = 12; $offset -lt $maskBytes.Length; $offset = $nextOffset) {
    if ($offset + 12 -gt $maskBytes.Length) { throw 'Truncated mask texture dictionary.' }
    $chunkType = [BitConverter]::ToUInt32($maskBytes, $offset)
    $chunkSize = [BitConverter]::ToUInt32($maskBytes, $offset + 4)
    $nextOffset = $offset + 12L + $chunkSize
    if ($nextOffset -gt $maskBytes.Length) { throw 'Invalid mask dictionary chunk size.' }
    if ($chunkType -ne 21) { continue }
    if ($chunkSize -lt 100 -or [BitConverter]::ToUInt32($maskBytes, $offset + 12) -ne 1) {
        throw 'Invalid RenderWare native texture.'
    }
    $textureName = [Text.Encoding]::ASCII.GetString($maskBytes, $offset + 32, 32).Split([char]0)[0]
    $textureNames.Add($textureName, $true)
}
$settings = Get-Content -Raw -LiteralPath (Join-Path $repositoryPath 'gamemodes/features/scba/config.inc')
foreach ($effect in 'SCBA_RESERVE_CLIP','SCBA_CRITICAL_CLIP','SCBA_STATION_CLIP','SCBA_TRUCK_CLIP') {
    $clip = [regex]::Match($settings, '(?m)^#define ' + $effect + ' "([\w-]+\.mp3)"')
    if (!$clip.Success) { throw ('Missing SCBA sound filename: ' + $effect) }
    $path = Join-Path $repositoryPath ('audio/scba/' + $clip.Groups[1].Value)
    $audio = [IO.File]::ReadAllBytes($path)
    if ($audio.Length -lt 4 -or
        ([Text.Encoding]::ASCII.GetString($audio, 0, 3) -ne 'ID3' -and
        !($audio[0] -eq 0xFF -and ($audio[1] -band 0xE0) -eq 0xE0))) { throw ('Invalid MP3: ' + $path) }
}
$steps = [regex]::Match($settings, '(?m)^#define SCBA_GAUGE_NEEDLE_STEPS \((\d+)\)')
if (!$steps.Success) { throw 'Missing SCBA needle frame count.' }
$requiredTextures = @('mask', 'trump8406_scbarevamp_diff', 'air_dial')
$requiredTextures += [regex]::Matches($settings, '\{"(air_s_\w+)", "') | ForEach-Object { $_.Groups[1].Value }
$requiredTextures += 0..([int]$steps.Groups[1].Value) | ForEach-Object { 'air_n{0:D2}' -f $_ }
foreach ($texture in $requiredTextures) {
    if (!$textureNames.ContainsKey($texture)) { throw ('Missing mask dictionary texture: ' + $texture) }
}
$configuration = @(
    ('rcon_password ' + [guid]::NewGuid().ToString('N'))
    'maxplayers 5'
    ('port ' + $testPort)
    'bind 127.0.0.1'
    'hostname SCBA isolated tests'
    'gamemode0 scba_tests 1'
    'announce 0'
    'query 0'
    'useartwork 1'
)
[IO.File]::WriteAllLines((Join-Path $testPath 'server.cfg'), $configuration, [Text.Encoding]::ASCII)
$includePaths = @(('-i' + (Join-Path $repositoryPath 'pawno/include')), ('-i' + (Join-Path $repositoryPath 'gamemodes')))
$compilerArguments = $includePaths + @('-;+', '-(+', '-d3', '-E')
Push-Location (Join-Path $repositoryPath 'gamemodes')
try {
    & $CompilerExecutable 'SARP.pwn' @compilerArguments ('-o' + (Join-Path $testPath 'SARP.candidate.amx')) |
        Tee-Object -FilePath (Join-Path $testPath 'gamemode-build.log')
    if ($LASTEXITCODE -ne 0) { throw 'Full gamemode compilation failed.' }
} finally { Pop-Location }
function Invoke-ScbaTestServer([string]$LogName, [string]$ResultPrefix) {
    $testExecutable = Join-Path $testPath 'samp-server.exe'
    $process = Start-Process -FilePath $testExecutable -WorkingDirectory $testPath -WindowStyle Hidden -PassThru
    $completed = $false
    try {
        $completed = $process.WaitForExit(15000)
    } finally {
        $process.Refresh()
        if (!$process.HasExited) {
            $ownedProcess = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
            if ($ownedProcess -and $ownedProcess.Path -eq $testExecutable -and $ownedProcess.StartTime -eq $process.StartTime) {
                Stop-Process -Id $process.Id
                [void]$process.WaitForExit(5000)
            }
        }
    }
    $runtimeLog = Join-Path $testPath 'server_log.txt'
    if (!(Test-Path -LiteralPath $runtimeLog)) { throw 'The isolated server did not create a log.' }
    $logPath = Join-Path $testPath $LogName
    Move-Item -LiteralPath $runtimeLog -Destination $logPath
    Write-Host ('SCBA test log: ' + $logPath)
    $testLog = [IO.File]::ReadAllText($logPath, [Text.Encoding]::GetEncoding(1252))
    $prefix = [regex]::Escape($ResultPrefix)
    if (!$completed -or $testLog -notmatch ($prefix + ' PASS') -or
        $testLog -match ($prefix + ' FAIL:|Run time error|Failed to load|Function not registered')) {
        $testLog -split '\r?\n' | Select-Object -Last 45 | Write-Output
        throw 'SCBA tests failed or did not shut down normally.'
    }
    $testLog -split '\r?\n' | Where-Object { $_ -match $prefix } | Write-Output
}

$profiles = @(
    @{ Seconds = $null; Log = 'core-server.log'; Build = 'tests-build.log' }
    @{ Seconds = 60; Log = 'short-cylinder.log'; Build = 'short-cylinder-build.log' }
)
$referenceCadence = $null
Push-Location (Join-Path $repositoryPath 'tests')
try {
    foreach ($profile in $profiles) {
        $arguments = $compilerArguments
        if ($profile.Seconds) { $arguments += ('SCBA_FULL_AIR_SECONDS=' + $profile.Seconds) }
        & $CompilerExecutable 'scba.pwn' @arguments ('-o' + (Join-Path $testPath 'gamemodes/scba_tests.amx')) |
            Tee-Object -FilePath (Join-Path $testPath $profile.Build)
        if ($LASTEXITCODE -ne 0) { throw 'SCBA test compilation failed.' }
        Invoke-ScbaTestServer $profile.Log '[SCBA tests]'

        $report = Select-String -LiteralPath (Join-Path $testPath $profile.Log) -Pattern '\[SCBA cadence\] cylinder=(\d+)s (.+)$'
        if (!$report -or ($profile.Seconds -and [int]$report.Matches[0].Groups[1].Value -ne $profile.Seconds)) {
            throw 'Missing cadence report for the configured cylinder.'
        }
        $observed = $report.Matches[0].Groups[2].Value
        if ($referenceCadence -and $observed -ne $referenceCadence) { throw 'Air capacity changed the post-exhaustion damage cadence.' }
        $referenceCadence = $observed
        Write-Host $report.Matches[0].Value
    }
} finally { Pop-Location }

if ($NativeAttachments) {
    if (!$NpcPlugin) { $NpcPlugin = Join-Path $repositoryPath '.local-server/plugins/FCNPC-DL.dll' }
    if (!(Test-Path -LiteralPath $NpcPlugin -PathType Leaf)) {
        throw 'Provide -NpcPlugin pointing to the compatible FCNPC-DL 2.0.11 DLL.'
    }
    New-Item -ItemType Directory -Path (Join-Path $testPath 'plugins') | Out-Null
    New-Item -ItemType Directory -Path (Join-Path $testPath 'scriptfiles/FCNPC') | Out-Null
    Copy-Item -LiteralPath $NpcPlugin -Destination (Join-Path $testPath 'plugins/FCNPC-DL.dll')
    Push-Location (Join-Path $repositoryPath 'tests')
    try {
        & $CompilerExecutable 'scba_native.pwn' @compilerArguments ('-o' + (Join-Path $testPath 'gamemodes/scba_native_tests.amx')) |
            Tee-Object -FilePath (Join-Path $testPath 'native-build.log')
        if ($LASTEXITCODE -ne 0) { throw 'Native attachment test compilation failed.' }
    } finally { Pop-Location }
    $configuration = $configuration | ForEach-Object {
        if ($_ -eq 'gamemode0 scba_tests 1') { 'gamemode0 scba_native_tests 1' } else { $_ }
    }
    $configuration += @('plugins FCNPC-DL', 'maxnpc 1')
    [IO.File]::WriteAllLines((Join-Path $testPath 'server.cfg'), $configuration, [Text.Encoding]::ASCII)
    Invoke-ScbaTestServer 'native-server.log' '[SCBA native tests]'
}
