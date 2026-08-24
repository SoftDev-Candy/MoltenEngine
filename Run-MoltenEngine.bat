@echo off
setlocal

set "ENGINE_BUILD=%~dp0build"
set "ENGINE_EXE=%ENGINE_BUILD%\MoltenEngine.exe"

if not exist "%ENGINE_EXE%" (
    echo MoltenEngine has not been built yet.
    echo Expected executable: "%ENGINE_EXE%"
    pause
    exit /b 1
)

pushd "%ENGINE_BUILD%"
"%ENGINE_EXE%"
popd
