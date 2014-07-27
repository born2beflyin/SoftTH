@echo off

echo.
echo Running the debugger
echo.

REM "C:\Program Files\Debugging Tools for Windows (x64)\cdb.exe" -G -lines -y "D:\Development\SoftTH\bin\win\x86\dx9\debug" -srcpath "D:\Development\SoftTH\src" "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Samples\C++\Direct3D\Bin\x86\HDRCubeMap.exe"

REM "C:\Program Files\Debugging Tools for Windows (x64)\windbg.exe" -G -y "D:\Development\SoftTH\bin\win\x86\dx10\debug" -srcpath "D:\Development\SoftTH\src" "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Samples\C++\Direct3D\Bin\x86\HDRCubeMap.exe"

"C:\Program Files\Debugging Tools for Windows (x64)\windbg.exe" -G -y "D:\Development\SoftTH\bin\win\x86\dx9\debug;D:\Development\SoftTH\bin\win\x86\dx10\debug" -srcpath "D:\Development\SoftTH\src" -c "ld d3d9; ld dxgi" "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Samples\C++\Direct3D10\Bin\x86\SimpleSample10.exe"

echo FINISHED!

:DONE