@echo off

setlocal

set timestring=%date:~10%%date:~4,2%%date:~7,2%-%time:~0,2%%time:~3,2%%time:~6,2%
set stdoutpath=stdout_%timestring%.txt
set stderrpath=stderr_%timestring%.txt

echo Current folder is "%~dp0"
echo.
echo Started recording game logs to:
echo - "%stdoutpath%"
echo - "%stderrpath%"

if exist Umineko1to4.exe (
    Umineko1to4.exe -d > "%stdoutpath%" 2> "%stderrpath%"
    goto finished
)

if exist Umineko5to8.exe (
    Umineko5to8.exe -d > "%stdoutpath%" 2> "%stderrpath%"
    goto finished
)

echo Error starting the game. Please check for Umineko1to4.exe or Umineko5to8.exe is in the current folder.

:finished

echo.
echo ------- Game has finished. Please send the above log files to the developers. -------
echo.

pause

endlocal
