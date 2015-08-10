@echo off

:: Set up the Visual Studio command prompt environment variables
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\vsvars32.bat"

:: Set the System32 folder location
set sys32="C:\Windows\System32"
set sth="..\bin\win\x86\dxgi\release"

echo.
echo Dumping relevant Direct3D exports
echo.
echo Dumping ...

:: Loop through and grab the exports
for /L %%f in (9,1,11) do (
  echo   d3d%%f
  dumpbin /exports "%sys32%\d3d%%f*.dll" > d3d%%f_dll_exports.txt
  
  echo   d3dx%%f
  dumpbin /exports "%sys32%\d3dx%%f*.dll" > d3dx%%f_dll_exports.txt
  )

echo   dxgi  
dumpbin /exports "%sys32%\dxgi.dll" > dxgi_dll_exports.txt

echo   dxgi debug
dumpbin /exports "%sys32%\dxgidebug.dll" > dxgidebug_dll_exports.txt

echo   SoftTH dxgi
dumpbin /exports "%sth%\dxgi.dll" > sth_dxgi_dll_exports.txt

echo.
echo FINISHED!
echo.

pause

:DONE