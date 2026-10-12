[CmdletBinding()]
param(
    [ValidateSet('Start', 'Stop', 'Status', 'Build')]
    [string]$Action = 'Start'
)

$ErrorActionPreference = 'Stop'
$repositoryPath = Split-Path -Parent $PSScriptRoot
$runtimePath = Join-Path $repositoryPath '.local-server'
$serverExecutable = Join-Path $runtimePath 'samp-server.exe'
$configPath = Join-Path $runtimePath 'server.cfg'
$logPath = Join-Path $runtimePath 'server_log.txt'
$textEncoding = [Text.Encoding]::GetEncoding(1252)

function Get-LocalConfigValue([string]$Name) {
    $config = [IO.File]::ReadAllText($configPath)
    $match = [regex]::Match($config, '(?m)^' + [regex]::Escape($Name) + '[ \t]+([^\r\n]+)')
    if (!$match.Success) { throw ('Falta la opcion ' + $Name + ' en la configuracion local.') }
    return $match.Groups[1].Value.Trim()
}

function Get-LocalServerProcess {
    $matches = @(Get-Process -Name 'samp-server' -ErrorAction SilentlyContinue |
        Where-Object { $_.Path -eq $serverExecutable })
    if ($matches.Count -gt 1) { throw 'Hay varias instancias de este servidor local.' }
    if ($matches.Count -eq 1) { return $matches[0] }
    return $null
}

function New-QueryHeader([char]$Opcode) {
    return [byte[]](@([Text.Encoding]::ASCII.GetBytes('SAMP')) +
        @([Net.IPAddress]::Parse($serverAddress).GetAddressBytes()) +
        @([BitConverter]::GetBytes([uint16]$serverPort)) + @([byte]$Opcode))
}

function Get-ServerInfo {
    $socket = New-Object Net.Sockets.UdpClient
    $socket.Client.ReceiveTimeout = 500
    try {
        $socket.Connect($serverAddress, $serverPort)
        [byte[]]$packet = New-QueryHeader 'i'
        [void]$socket.Send($packet, $packet.Length)
        $remote = New-Object Net.IPEndPoint ([Net.IPAddress]::Any), 0
        [byte[]]$reply = $socket.Receive([ref]$remote)
        if ($reply.Length -lt 28 -or [Text.Encoding]::ASCII.GetString($reply, 0, 4) -ne 'SAMP' -or $reply[10] -ne 105) {
            throw 'Respuesta de consulta invalida.'
        }
        $offset = 16
        $values = @()
        for ($index = 0; $index -lt 3; $index++) {
            if ($offset + 4 -gt $reply.Length) { throw 'Respuesta incompleta.' }
            $length = [BitConverter]::ToInt32($reply, $offset)
            $offset += 4
            if ($length -lt 0 -or $offset + $length -gt $reply.Length) { throw 'Longitud de respuesta invalida.' }
            $values += $textEncoding.GetString($reply, $offset, $length)
            $offset += $length
        }
        return [pscustomobject]@{
            Address = $serverAddress + ':' + $serverPort
            Hostname = $values[0]
            Gamemode = $values[1]
            Language = $values[2]
            Players = [BitConverter]::ToUInt16($reply, 12)
            MaxPlayers = [BitConverter]::ToUInt16($reply, 14)
        }
    } catch [Net.Sockets.SocketException] {
        return $null
    } finally {
        $socket.Dispose()
    }
}

function Stop-LocalServer {
    $server = Get-LocalServerProcess
    if (!$server) { Write-Host 'El servidor ya esta detenido.'; return }
    # Authenticate directly over loopback; never put the RCON secret in arguments.
    [byte[]]$passwordBytes = [Text.Encoding]::ASCII.GetBytes((Get-LocalConfigValue 'rcon_password'))
    [byte[]]$commandBytes = [Text.Encoding]::ASCII.GetBytes('exit')
    [byte[]]$packet = @(New-QueryHeader 'x') + @([BitConverter]::GetBytes([uint16]$passwordBytes.Length)) +
        @($passwordBytes) + @([BitConverter]::GetBytes([uint16]$commandBytes.Length)) + @($commandBytes)
    $socket = New-Object Net.Sockets.UdpClient
    try {
        $socket.Connect($serverAddress, $serverPort)
        [void]$socket.Send($packet, $packet.Length)
    } finally { $socket.Dispose() }
    if (!$server.WaitForExit(10000)) {
        # Only terminate the exact instance previously identified by executable.
        $current = Get-LocalServerProcess
        if ($current -and $current.Id -eq $server.Id -and $current.StartTime -eq $server.StartTime) {
            Stop-Process -Id $current.Id
            [void]$current.WaitForExit(5000)
        }
    }
    if (Get-LocalServerProcess) { throw 'No se pudo detener el servidor.' }
    Write-Host 'Servidor detenido.'
}

function Build-LocalGamemode {
    if (Get-LocalServerProcess) { throw 'Deten el servidor antes de compilar.' }
    $compiler = Join-Path $repositoryPath 'pawno/pawncc.exe'
    $includes = Join-Path $repositoryPath 'pawno/include'
    $temporaryAmx = Join-Path $runtimePath 'gamemodes/SARP.next.amx'
    if (!(Test-Path -LiteralPath $compiler)) { throw 'Falta el compilador. Ejecuta prepare-local-server.ps1.' }
    Push-Location (Join-Path $repositoryPath 'gamemodes')
    try {
        & $compiler 'SARP.pwn' ('-i' + $includes) '-;+' '-(+' '-d3' ('-o' + $temporaryAmx) |
            Tee-Object -FilePath (Join-Path $runtimePath 'build.log')
        if ($LASTEXITCODE -ne 0 -or !(Test-Path -LiteralPath $temporaryAmx)) {
            throw 'La compilacion fallo. Se conserva el AMX anterior.'
        }
        Copy-Item -LiteralPath $temporaryAmx -Destination (Join-Path $runtimePath 'gamemodes/SARP.amx') -Force
    } finally { Pop-Location }
}

function Show-LocalStatus {
    $server = Get-LocalServerProcess
    if (!$server) { Write-Host 'Servidor detenido.'; return }
    $info = Get-ServerInfo
    if (!$info) { throw ('El proceso existe (PID ' + $server.Id + ') pero no responde. Revisa ' + $logPath) }
    Write-Host ('Servidor activo: ' + $info.Address)
    Write-Host ('Modo: ' + $info.Gamemode + ' | Jugadores: ' + $info.Players + '/' + $info.MaxPlayers)
    Write-Host ('Registro: ' + $logPath)
}

if (!(Test-Path -LiteralPath $configPath)) { throw 'Falta preparar el servidor. Ejecuta scripts/prepare-local-server.ps1.' }
$serverAddress = Get-LocalConfigValue 'bind'
$serverPort = [int](Get-LocalConfigValue 'port')
if ($serverAddress -ne '127.0.0.1') { throw 'Este lanzador esta preparado para pruebas locales en 127.0.0.1.' }

switch ($Action) {
    'Stop' { Stop-LocalServer }
    'Status' { Show-LocalStatus }
    'Build' { Build-LocalGamemode }
    'Start' {
        if (Get-LocalServerProcess) { Show-LocalStatus; break }
        if (Get-ServerInfo) { throw 'El puerto ya esta ocupado por otro servidor.' }
        $pluginNames = (Get-LocalConfigValue 'plugins') -split '\s+'
        $requiredFiles = @($serverExecutable, (Join-Path $runtimePath 'scriptfiles/colandreas/ColAndreas.cadb'))
        $requiredFiles += @($pluginNames | ForEach-Object { Join-Path $runtimePath ('plugins/' + $_ + '.dll') })
        foreach ($requiredFile in $requiredFiles) {
            if (!(Test-Path -LiteralPath $requiredFile)) { throw ('Falta ' + $requiredFile + '. Ejecuta prepare-local-server.ps1.') }
        }
        Build-LocalGamemode
        $previousLogLength = if (Test-Path -LiteralPath $logPath) { (Get-Item -LiteralPath $logPath).Length } else { 0 }
        $server = Start-Process -FilePath $serverExecutable -WorkingDirectory $runtimePath -WindowStyle Hidden -PassThru
        $deadline = (Get-Date).AddSeconds(30)
        try {
            do {
                Start-Sleep -Milliseconds 500
                $server.Refresh()
                if ($server.HasExited) { throw ('El servidor termino durante el arranque. Revisa ' + $logPath) }
                $info = Get-ServerInfo
                $newLog = ''
                if (Test-Path -LiteralPath $logPath) {
                    $logStream = [IO.File]::Open($logPath, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::ReadWrite)
                    try {
                        if ($logStream.Length -ge $previousLogLength) {
                            $logStream.Position = $previousLogLength
                            $reader = New-Object IO.StreamReader($logStream, $textEncoding)
                            try { $newLog = $reader.ReadToEnd() } finally { $reader.Dispose() }
                        }
                    } finally { $logStream.Dispose() }
                }
                if ($newLog -match 'Run time error|Failed to load|Function not registered|Script\[gamemodes.*: (?:Run time|Error)') {
                    throw ('Error de carga. Revisa ' + $logPath)
                }
                $ready = $info -and $info.Gamemode -eq 'Servidor de Prueba' -and
                    $newLog -match 'Number of vehicle models:' -and
                    $newLog -match ('Loaded ' + $pluginNames.Count + ' plugins\.')
            } while (!$ready -and (Get-Date) -lt $deadline)
            if (!$ready) { throw ('El arranque no se confirmo en 30 segundos. Revisa ' + $logPath) }
            Show-LocalStatus
        } catch {
            Stop-LocalServer
            throw
        }
    }
}
