@echo off

echo.
echo Debugging SoftTH remotely in FSX
echo.
echo Executing debug session ...
echo. 
echo Attach debugger to process from remote computer
echo.

REM "D:\Games\gdbserver.exe" :2345 "D:\Games\Microsoft Flight Simulator X\fsx.exe"

"D:\Games\dbgsrv.exe" -t tcp:port=2345 -c "D:\Games\Microsoft Flight Simulator X\fsx.exe"

echo.
echo FINISHED!
echo.

pause

exit 0

:DONE