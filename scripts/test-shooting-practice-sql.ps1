[CmdletBinding()]
param(
    [string]$MySqlBin = ''
)

$ErrorActionPreference = 'Stop'
$repositoryPath = Split-Path -Parent $PSScriptRoot
if (!$MySqlBin) {
    $serverCommand = Get-Command mysqld.exe -ErrorAction SilentlyContinue
    if (!$serverCommand) { throw 'Indica -MySqlBin con la carpeta bin de MySQL 8.0.16 o posterior.' }
    $MySqlBin = Split-Path -Parent $serverCommand.Source
}
$serverExecutable = Join-Path $MySqlBin 'mysqld.exe'
$clientExecutable = Join-Path $MySqlBin 'mysql.exe'
foreach ($requiredPath in @($serverExecutable, $clientExecutable)) {
    if (!(Test-Path -LiteralPath $requiredPath)) { throw ('Falta ' + $requiredPath) }
}
$testRunKey = [guid]::NewGuid().ToString('N')
$testPath = Join-Path $repositoryPath ('.local-server/sql-tests/' + $testRunKey)
$dataPath = Join-Path $testPath 'data'
$tempPath = Join-Path $testPath 'tmp'
foreach ($folder in @($testPath, $dataPath, $tempPath)) {
    New-Item -ItemType Directory -Path $folder | Out-Null
}
# Always initialize a private data directory. No installed service/credentials are used.
$basePath = Split-Path -Parent $MySqlBin
$initializationLog = Join-Path $testPath 'initialize.log'
$initializer = Start-Process -FilePath $serverExecutable -WindowStyle Hidden -PassThru -ArgumentList @(
    '--no-defaults', '--initialize-insecure',
    ('--basedir="' + $basePath + '"'), ('--datadir="' + $dataPath + '"'),
    ('--log-error="' + $initializationLog + '"')
)
[void]$initializer.Handle
if (!$initializer.WaitForExit(60000)) {
    $initializer.Refresh()
    if (!$initializer.HasExited) { $initializer.Kill(); [void]$initializer.WaitForExit(5000) }
    throw ('La inicialización aislada agotó el tiempo. Revisa ' + $initializationLog)
}
$initializer.WaitForExit()
if ($initializer.ExitCode -ne 0) { throw ('Falló la inicialización aislada. Revisa ' + $initializationLog) }

$testPort = 33079
while (@(Get-NetTCPConnection -LocalPort $testPort -ErrorAction SilentlyContinue).Count) {
    $testPort++
    if ($testPort -gt 33099) { throw 'No hay un puerto local libre para la BD de pruebas.' }
}
$serverLog = Join-Path $testPath 'mysql.log'
$server = Start-Process -FilePath $serverExecutable -WindowStyle Hidden -PassThru -ArgumentList @(
    '--no-defaults', ('--basedir="' + $basePath + '"'), ('--datadir="' + $dataPath + '"'),
    ('--tmpdir="' + $tempPath + '"'), ('--log-error="' + $serverLog + '"'),
    '--bind-address=127.0.0.1', ('--port=' + $testPort), '--mysqlx=OFF',
    '--skip-log-bin', '--innodb-buffer-pool-size=32M', '--performance-schema=OFF'
)
$connectionArguments = @('--no-defaults', '--skip-password', '--protocol=TCP',
    '--host=127.0.0.1', ('--port=' + $testPort), '--user=root', '--connect-timeout=1',
    '--default-character-set=utf8mb4', '--batch', '--skip-column-names')
$priorLoginFile = [Environment]::GetEnvironmentVariable('MYSQL_TEST_LOGIN_FILE', 'Process')
[Environment]::SetEnvironmentVariable('MYSQL_TEST_LOGIN_FILE', (Join-Path $testPath 'absent-login.cnf'), 'Process')
try {
    $ready = $false
    $deadline = (Get-Date).AddSeconds(30)
    do {
        $server.Refresh()
        if ($server.HasExited) { throw ('La BD aislada terminó durante el arranque. Revisa ' + $serverLog) }
        $nativeErrorPreference = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $version = & $clientExecutable @connectionArguments '--execute=SELECT VERSION();' 2>$null
            $probeExitCode = $LASTEXITCODE
        } finally { $ErrorActionPreference = $nativeErrorPreference }
        if ($probeExitCode -eq 0) { $ready = $true; break }
        Start-Sleep -Milliseconds 250
    } while ((Get-Date) -lt $deadline)
    if (!$ready) { throw ('La BD aislada no respondió. Revisa ' + $serverLog) }
    Write-Host ('MySQL aislado: ' + $version)

    $schema = [IO.File]::ReadAllText((Join-Path $repositoryPath '.internal/sql/shooting-practice.sql'), [Text.Encoding]::UTF8)
    $checks = [IO.File]::ReadAllText((Join-Path $repositoryPath 'tests/shooting_practice_schema.sql'), [Text.Encoding]::UTF8)
    $driver = @(
        'CREATE DATABASE practice_schema_tests CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;'
        'USE practice_schema_tests;'
        $schema
        $checks
    ) -join [Environment]::NewLine
    $driverPath = Join-Path $testPath 'driver.sql'
    [IO.File]::WriteAllText($driverPath, $driver, [Text.UTF8Encoding]::new($false))
    $checksLog = Join-Path $testPath 'checks.log'
    $checksErrorLog = Join-Path $testPath 'checks-error.log'
    # Real batch input aborts on SQL errors. No client SOURCE/force mode can mask failures.
    $checker = Start-Process -FilePath $clientExecutable -WindowStyle Hidden -PassThru -ArgumentList $connectionArguments `
        -RedirectStandardInput $driverPath -RedirectStandardOutput $checksLog -RedirectStandardError $checksErrorLog
    [void]$checker.Handle
    if (!$checker.WaitForExit(30000)) {
        $checker.Refresh()
        if (!$checker.HasExited) { $checker.Kill() }
        throw ('Las pruebas SQL agotaron el tiempo. Revisa ' + $checksLog)
    }
    $checker.WaitForExit()
    $testExitCode = $checker.ExitCode
    $testOutput = [IO.File]::ReadAllText($checksLog, [Text.Encoding]::UTF8)
    $testError = [IO.File]::ReadAllText($checksErrorLog, [Text.Encoding]::UTF8)
    if ($testError) { Write-Output $testError }
    Write-Output $testOutput
    if ($testExitCode -ne 0 -or $testOutput -notmatch '\[Practice SQL tests\] PASS') {
        throw ('Fallaron las pruebas SQL. Revisa ' + $checksLog)
    }
    Write-Host ('Registro de pruebas SQL: ' + (Join-Path $testPath 'checks.log'))
} finally {
    $server.Refresh()
    if (!$server.HasExited) {
        $nativeErrorPreference = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            & $clientExecutable @connectionArguments '--execute=SHUTDOWN;' 2>$null | Out-Null
        } finally { $ErrorActionPreference = $nativeErrorPreference }
        if (!$server.WaitForExit(5000)) {
            $current = Get-CimInstance Win32_Process -Filter ('ProcessId=' + $server.Id) -ErrorAction SilentlyContinue
            if ($current -and $current.CommandLine -and $current.CommandLine.Contains($testRunKey)) {
                Stop-Process -Id $current.ProcessId
                [void]$server.WaitForExit(5000)
            }
        }
    }
    # MySQL on Windows can retain a monitor/child after shutdown. Match only this
    # generated run key, including normalized paths, never installed service PIDs.
    $ownedProcesses = @(Get-CimInstance Win32_Process -Filter "Name='mysqld.exe'" |
        Where-Object { $_.CommandLine -and $_.CommandLine.Contains($testRunKey) })
    foreach ($ownedProcess in $ownedProcesses) {
        Stop-Process -Id $ownedProcess.ProcessId -ErrorAction SilentlyContinue
    }
    [Environment]::SetEnvironmentVariable('MYSQL_TEST_LOGIN_FILE', $priorLoginFile, 'Process')
}
