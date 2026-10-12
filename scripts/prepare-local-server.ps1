[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repositoryPath = Split-Path -Parent $PSScriptRoot
$runtimePath = Join-Path $repositoryPath '.local-server'
$serverExecutable = Join-Path $runtimePath 'samp-server.exe'
$runningServer = Get-Process -Name 'samp-server' -ErrorAction SilentlyContinue |
    Where-Object { $_.Path -eq $serverExecutable }
if ($runningServer) { throw 'Deten este servidor antes de preparar sus dependencias.' }

# Match the checked-in includes. Keep all local state and credentials ignored.
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'local-server-packages.json') -Raw | ConvertFrom-Json
$cachePath = Join-Path $runtimePath 'downloads'
foreach ($directory in @('downloads', 'plugins', 'gamemodes', 'models', 'npcmodes', 'licenses',
    'scriptfiles/colandreas', 'scriptfiles/FCNPC', 'scriptfiles/YSI/fixes', 'scriptfiles/YSI/temp')) {
    New-Item -ItemType Directory -Force -Path (Join-Path $runtimePath $directory) | Out-Null
}

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
foreach ($package in $manifest.packages) {
    $archivePath = Join-Path $cachePath $package.file
    if (!(Test-Path -LiteralPath $archivePath)) {
        Write-Host ('Descargando: ' + $package.name)
        Invoke-WebRequest -UseBasicParsing -Uri $package.url -OutFile $archivePath -TimeoutSec 120
    }
    if ((Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash -ne $package.sha256) {
        throw ('La descarga no coincide con el SHA256 esperado: ' + $archivePath)
    }

    $extractPath = Join-Path $cachePath $package.name
    if ([IO.Path]::GetExtension($archivePath) -eq '.zip') {
        Expand-Archive -LiteralPath $archivePath -DestinationPath $extractPath -Force
    }
    foreach ($file in $package.files) {
        $sourcePath = if ($file[0]) { Join-Path $extractPath $file[0] } else { $archivePath }
        $destinationPath = [IO.Path]::GetFullPath((Join-Path $repositoryPath $file[1]))
        if (!$destinationPath.StartsWith($repositoryPath + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
            throw 'El destino debe estar dentro del repositorio.'
        }
        if (Test-Path -LiteralPath $sourcePath -PathType Container) {
            New-Item -ItemType Directory -Force -Path $destinationPath | Out-Null
            Get-ChildItem -LiteralPath $sourcePath | Copy-Item -Destination $destinationPath -Recurse -Force
        } else {
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destinationPath) | Out-Null
            Copy-Item -LiteralPath $sourcePath -Destination $destinationPath -Force
        }
    }
    Write-Host ('Preparado: ' + $package.name)
}

$configPath = Join-Path $runtimePath 'server.cfg'
if (!(Test-Path -LiteralPath $configPath)) {
    $randomBytes = New-Object byte[] 18
    $generator = [Security.Cryptography.RandomNumberGenerator]::Create()
    try { $generator.GetBytes($randomBytes) } finally { $generator.Dispose() }
    $adminPassword = -join ($randomBytes | ForEach-Object { $_.ToString('x2') })
    $config = [IO.File]::ReadAllText((Join-Path $repositoryPath 'server.cfg'))
    $config = [regex]::Replace($config, '(?m)^rcon_password[^\r\n]*', ('rcon_password ' + $adminPassword))
    # FCNPC-DL already includes ColAndreas and registers its CA_* natives.
    $config = [regex]::Replace($config, '(?m)^plugins[^\r\n]*', 'plugins crashdetect streamer pawnraknet sscanf mysql pawncmd FCNPC-DL')
    $config = [regex]::Replace($config, '(?m)^language[^\r\n]*', 'language Spanish')
    # Allow the map/YSI initialization work while retaining slow-call diagnostics.
    $config = [regex]::Replace($config, '(?m)^long_call_time[^\r\n]*', 'long_call_time 1000000')
    $config += "`r`nbind 127.0.0.1`r`nquery 1`r`n"
    [IO.File]::WriteAllText($configPath, $config, [Text.Encoding]::ASCII)
    Write-Host 'Configuracion local creada con una clave RCON aleatoria.'
} else {
    Write-Host 'Se conserva la configuracion local existente.'
}

Write-Host 'Preparacion completa. Ejecuta scripts/local-server.ps1 -Action Start.'
