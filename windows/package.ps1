$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Set-Location $root
$stage = Join-Path $root 'dist/click-meow-windows-x64'
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item 'build/Release/click-meow.exe', 'build/Release/windows-check.exe', 'cat.svg', 'LICENSE', 'SOUNDS.md' $stage
Copy-Item 'windows/README-Windows.txt' $stage
New-Item -ItemType Directory -Force "$stage/sounds" | Out-Null
Copy-Item 'sounds/*.wav' "$stage/sounds"
windeployqt --release --no-translations --no-compiler-runtime --svg "$stage/click-meow.exe"
if ($LASTEXITCODE) { throw 'windeployqt failed' }
# App-local MSVC runtime: users should not need a separate runtime installer.
$vswhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$crt = Get-ChildItem "$vs/VC/Redist/MSVC/*/x64/Microsoft.VC143.CRT" -Directory | Sort-Object FullName -Descending | Select-Object -First 1
if (-not $crt) { throw 'MSVC runtime not found' }
Copy-Item "$($crt.FullName)/*.dll" $stage
# Keep the licenses supplied with this exact Qt distribution.
$licenses = Join-Path $env:QT_ROOT_DIR 'LICENSES'
if (Test-Path $licenses) { Copy-Item -Recurse $licenses "$stage/Qt-LICENSES" }
Copy-Item 'windows/THIRD-PARTY.txt' $stage
Copy-Item -Recurse 'windows/licenses' "$stage/licenses"
# The deployment test runs without Qt or Visual Studio directories in PATH.
$savedPath = $env:PATH
try {
    $env:PATH = "$env:SystemRoot/System32;$env:SystemRoot"
    & "$stage/windows-check.exe" $stage
    if ($LASTEXITCODE) { throw 'Deployed input/audio check failed' }
    $app = Start-Process "$stage/click-meow.exe" -PassThru
    Start-Sleep -Seconds 4
    if ($app.HasExited) { throw "Portable app exited during startup: $($app.ExitCode)" }
    $second = Start-Process "$stage/click-meow.exe" -PassThru
    if (-not $second.WaitForExit(8000) -or $second.ExitCode -ne 0) { throw 'Single-instance test failed' }
} finally {
    if ($app -and -not $app.HasExited) { Stop-Process -Id $app.Id }
    if ($second -and -not $second.HasExited) { Stop-Process -Id $second.Id }
    $env:PATH = $savedPath
}
Remove-Item "$stage/windows-check.exe"
Compress-Archive -Path $stage -DestinationPath 'dist/click-meow-windows-x64.zip' -Force
$hash = (Get-FileHash 'dist/click-meow-windows-x64.zip' -Algorithm SHA256).Hash.ToLower()
"$hash  click-meow-windows-x64.zip" | Set-Content -Encoding ascii 'dist/SHA256SUMS-windows.txt'
