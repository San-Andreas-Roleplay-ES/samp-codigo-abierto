[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repositoryPath = Split-Path -Parent $PSScriptRoot
$runtimePath = Join-Path $repositoryPath '.local-server'
$compilerPath = Join-Path $repositoryPath 'pawno/pawncc.exe'
$serverPath = Join-Path $runtimePath 'samp-server.exe'
$pluginPath = Join-Path $runtimePath 'plugins/sscanf.dll'
foreach ($requiredFile in @($compilerPath, $serverPath, $pluginPath)) {
    if (!(Test-Path -LiteralPath $requiredFile)) { throw 'Ejecuta primero scripts/prepare-local-server.ps1.' }
}
if (@(Get-NetUDPEndpoint -LocalPort 7779 -ErrorAction SilentlyContinue).Count) {
    throw 'El puerto UDP 7779 de las pruebas esta ocupado.'
}

# Always use a fresh directory: tests intentionally overwrite their layout file.
$testPath = Join-Path $runtimePath ('practice-tests/' + [guid]::NewGuid().ToString('N'))
foreach ($folder in @('', 'plugins', 'gamemodes', 'scriptfiles')) {
    New-Item -ItemType Directory -Force -Path (Join-Path $testPath $folder) | Out-Null
}
Copy-Item -LiteralPath $serverPath -Destination $testPath
Copy-Item -LiteralPath $pluginPath -Destination (Join-Path $testPath 'plugins')
$configuration = @(
    ('rcon_password ' + [guid]::NewGuid().ToString('N'))
    'maxplayers 2'
    'port 7779'
    'bind 127.0.0.1'
    'hostname Practice Tests'
    'gamemode0 practice_tests 1'
    'plugins sscanf'
    'announce 0'
    'query 0'
    'lagcompmode 1'
)
[IO.File]::WriteAllLines((Join-Path $testPath 'server.cfg'), $configuration, [Text.Encoding]::ASCII)
Push-Location (Join-Path $repositoryPath 'tests')
try {
    & $compilerPath 'shooting_practice.pwn' ('-i' + (Join-Path $repositoryPath 'pawno/include')) '-;+' '-(+' '-d3' '-E' `
        ('-o' + (Join-Path $testPath 'gamemodes/practice_tests.amx')) |
        Tee-Object -FilePath (Join-Path $testPath 'build.log')
    if ($LASTEXITCODE -ne 0) { throw 'La compilacion de las pruebas fallo.' }
} finally { Pop-Location }

$testExecutable = Join-Path $testPath 'samp-server.exe'
$testProcess = Start-Process -FilePath $testExecutable -WorkingDirectory $testPath -WindowStyle Hidden -PassThru
$completed = $false
try {
    $completed = $testProcess.WaitForExit(15000)
} finally {
    $testProcess.Refresh()
    if (!$testProcess.HasExited) {
        $currentProcess = Get-Process -Id $testProcess.Id -ErrorAction SilentlyContinue
        if ($currentProcess -and $currentProcess.Path -eq $testExecutable -and $currentProcess.StartTime -eq $testProcess.StartTime) {
            Stop-Process -Id $currentProcess.Id
            [void]$testProcess.WaitForExit(5000)
        }
    }
}
$logPath = Join-Path $testPath 'server_log.txt'
Write-Host ('Registro de pruebas: ' + $logPath)
if (!(Test-Path -LiteralPath $logPath)) { throw 'El servidor de pruebas no genero su registro.' }
$testLog = Get-Content -LiteralPath $logPath -Raw
if (!$completed -or $testLog -notmatch '\[Practice tests\] PASS' -or
    $testLog -match '\[Practice tests\] FAIL:|Run time error|Failed to load|Function not registered') {
    Get-Content -LiteralPath $logPath | Select-Object -Last 40
    throw 'Las pruebas de practica fallaron o el proceso no termino en 15 segundos.'
}
$testLog -split '\r?\n' | Where-Object { $_ -match '\[Practice tests\]' } | Write-Output
