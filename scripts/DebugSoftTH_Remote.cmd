@echo off

echo.
echo Getting the remote task list
echo.

:: Set the process we want to debug
set PROGRAM=fsx.exe

::set "debugfolder=C:\Program^ Files\Debugging^ Tools^ for^ Windows^ ^(x64^)"
::for /f %%a in ('"%debugfolder%\rtlist.exe"^ ^-premote^ tcp^:server^=192^.168^.0^.75^,port^=2345^ ^-pn^ "fsx^.exe"') do echo [%%a]

:: Call RTlist to get the task list on the remote machine and search for "PROGRAM" and save to temp file
REM "C:\Program Files\Debugging Tools for Windows (x64)\rtlist.exe" -premote tcp:server=192.168.0.75,port=2345 -pn "%PROGRAM%" > tmp.txt

:: Read the temp file output to a variable
REM set /p PID= < tmp.txt

:: Extract the PID from the variable string
REM for /f %%a in ("%PID%") do set PID=%%a

:: Remove the leading "0n"
REM for /f "tokens=1,2 delims=n" %%a in ("%PID%") do set waste=%%a&set PID=%%b

REM echo Program %PROGRAM% found as process ID = %PID%


echo.
echo Running the debugger
echo.

:: CDB
::"C:\Program Files\Debugging Tools for Windows (x64)\cdb.exe" -G -lines -premote tcp:server=192.168.0.75,port=2345 -y "D:\Development\SoftTH\bin\win\x86\dx9\debug" -srcpath "D:\Development\SoftTH\src" -pd -pr -pn "%PROGRAM%" -c "ld d3d9; bm IDirect3D9New::CreateDevice" -xi asrt -xi av -xi wob -xi wos -xi ld -xi out

:: CDB non-intrusive
::"C:\Program Files\Debugging Tools for Windows (x64)\cdb.exe" -lines -premote tcp:server=192.168.0.75,port=2345 -y "D:\Development\SoftTH\bin\win\x86\dx9\debug;C:\Windows\Microsoft.NET;C:\Windows\system32" -srcpath "D:\Development\SoftTH\src" -pv -pn "%PROGRAM%" -c "ld d3d9; bm IDirect3D9New::CreateDevice" -xi asrt -xi av -xi wob -xi wos -xi ld -xi out

:: WinDbg
"C:\Program Files\Debugging Tools for Windows (x64)\windbg.exe" -QY -QSY -premote tcp:server=192.168.0.75,port=2345 -y "D:\Development\SoftTH\bin\win\x86\dx9\debug" -srcpath "D:\Development\SoftTH\src" -pd -pr -pn "%PROGRAM%" -c "ld d3d9; bm IDirect3D9New::CreateDevice" -xi asrt -xi av -xi wob -xi wos -xd ld -xi out

:: WinDbg non-intrusive
::"C:\Program Files\Debugging Tools for Windows (x64)\windbg.exe" -QY -QSY -premote tcp:server=192.168.0.75,port=2345 -y "D:\Development\SoftTH\bin\win\x86\dx9\debug" -srcpath "D:\Development\SoftTH\src" -pv -pn "%PROGRAM%" -c "ld d3d9; bm IDirect3D9New::CreateDevice" -xi asrt -xi av -xi wob -xi wos -xi ld -xi out

echo.
echo FINISHED!
echo.

pause

:DONE