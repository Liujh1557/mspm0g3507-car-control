$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $env:TEMP 'mspm0-control-math-test.exe'
& gcc -x c -std=c11 -Wall -Wextra -Werror -I (Join-Path $root 'user_driver') (Join-Path $PSScriptRoot 'test_control_math.c.txt') -o $exe
if ($LASTEXITCODE -ne 0) { throw 'Control math test compilation failed' }
& $exe
if ($LASTEXITCODE -ne 0) { throw 'Control math test failed' }
