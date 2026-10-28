@echo off
REM Local build: needs Visual Studio 2022 (C++ workload), CMake and Inno Setup 6. Internet needed once (JUCE download).
cmake -B build -G "Visual Studio 17 2022" -A x64 || exit /b 1
cmake --build build --config Release --target YMYVerb_VST3 YMYVerb_Standalone test_dsp || exit /b 1
build\Release\test_dsp.exe || exit /b 1
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" "/DVST3Dir=%CD%\build\YMYVerb_artefacts\Release\VST3\YMYVerb.vst3" "/DStandaloneExe=%CD%\build\YMYVerb_artefacts\Release\Standalone\YMYVerb.exe" Installer\YMYVerb.iss || exit /b 1
if not exist dist\YMYVerb_Setup.exe exit /b 1
echo Listo: dist\YMYVerb_Setup.exe
