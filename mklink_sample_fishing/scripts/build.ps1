param(
    [string]$SdkRoot,
    [ValidateSet('flash_uf2','flash_xip')][string]$BuildType='flash_uf2',
    [int]$Jobs=6
)
$ErrorActionPreference='Stop'
$taskDemo=Split-Path -Parent $PSScriptRoot
$taskRepo=Split-Path -Parent $taskDemo
if ($SdkRoot) { $env:HPM_SDK_BASE=Join-Path $SdkRoot 'hpm_sdk' }
if (!$env:HPM_SDK_BASE) { throw 'Set HPM_SDK_BASE or supply -SdkRoot <SDK bundle directory>' }
$taskSdk=(Resolve-Path -LiteralPath $env:HPM_SDK_BASE).Path
$taskBundle=Split-Path -Parent $taskSdk
if (!$env:GNURISCV_TOOLCHAIN_PATH) {
    $taskToolchain=Join-Path $taskBundle 'toolchains/rv32imac_zicsr_zifencei_multilib_b_ext-win'
    if (!(Test-Path -LiteralPath $taskToolchain)) { throw 'Set GNURISCV_TOOLCHAIN_PATH to the RISC-V toolchain' }
    $env:GNURISCV_TOOLCHAIN_PATH=$taskToolchain
}
$taskCmake=Join-Path $taskBundle 'tools/cmake/bin/cmake.exe'
$taskPython=Join-Path $taskBundle 'tools/python3/python3.exe'
if (!(Test-Path -LiteralPath $taskCmake)) { $taskCmake=(Get-Command cmake -ErrorAction Stop).Source }
if (!(Test-Path -LiteralPath $taskPython)) { $taskPython=(Get-Command python -ErrorAction Stop).Source }
$env:PATH=(Join-Path $taskBundle 'tools/ninja')+';'+(Join-Path $env:GNURISCV_TOOLCHAIN_PATH 'bin')+';'+$env:PATH
$env:PYTHONUTF8='1'
$taskBuild=Join-Path $taskDemo ('hpm5301evklite_'+$BuildType+'_debug')
& $taskCmake -S $taskDemo -B $taskBuild -G Ninja '-DCMAKE_BUILD_TYPE=Debug' `
    '-DBOARD=hpm5301evklite' "-DBOARD_SEARCH_PATH=$taskRepo" "-DHPM_BUILD_TYPE=$BuildType" `
    "-DPYTHON_EXECUTABLE=$taskPython" "-DFISH_PYTHON_EXECUTABLE=$taskPython"
if ($LASTEXITCODE) {exit $LASTEXITCODE}
& $taskCmake --build $taskBuild -j $Jobs
exit $LASTEXITCODE
