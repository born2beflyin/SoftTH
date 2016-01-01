@echo off

:: Set up the Visual Studio command prompt environment variables
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\vsvars32.bat"

:: Set the System32 folder location
set sys32=C:\Windows\System32
set sdk=C:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x86
set sth_dxgi=..\bin\win\x86\dxgi\debug
set sth_d3d11=..\bin\win\x86\d3d11\debug

echo.
echo Dumping relevant Direct3D exports
echo.
echo Dumping ...

:: Loop through and grab the exports
for /L %%f in (9,1,11) do (
  echo   d3d%%f_SDK_LIB
  dumpbin /exports "%sdk%\d3d%%f*.lib" > d3d%%f_sdk_lib_exports.txt
  
  echo   d3dx%%f_SDK_LIB
  dumpbin /exports "%sdk%\d3dx%%f*.lib" > d3dx%%f_sdk_lib_exports.txt
  
  echo   d3d%%f_SYS32_DLL
  dumpbin /exports "%sys32%\d3d%%f*.dll" > d3d%%f_sys32_dll_exports.txt
  
  echo   d3dx%%f_SYS32_DLL
  dumpbin /exports "%sys32%\d3dx%%f*.dll" > d3dx%%f_sys32_dll_exports.txt
)

echo   dxgi_SDK_LIB
dumpbin /exports "%sdk%\dxgi.lib" > dxgi_sdk_lib_exports.txt

echo   dxgi_SYS32_DLL
dumpbin /exports "%sys32%\dxgi.dll" > dxgi_sys32_dll_exports.txt

echo   dxgi_debug_SYS32_DLL
dumpbin /exports "%sys32%\dxgidebug.dll" > dxgidebug_sys32_dll_exports.txt

echo   SoftTH_dxgi_LIB
dumpbin /exports "%sth_dxgi%\dxgi.lib" > sth_dxgi_lib_exports.txt

echo   SoftTH_d3d11_LIB
dumpbin /exports "%sth_d3d11%\d3d11.lib" > sth_d3d11_lib_exports.txt

echo.
echo FINISHED!
echo.

pause

:DONE