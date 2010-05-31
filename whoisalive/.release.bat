@echo off

set project-name=whoisalive
set path-to-archive=D:\My\C++\!backup\%project-name%
set state=Release

call .ver.bat -
if not %verstate%=="" set verstate=-%verstate%
set project=%project-name%-%vermajor%.%verminor%.%verrelease%%verpatch%[%verbranch%%verbuild%]%verstate%

if "%1"=="Debug" (
  set state=Debug
  set project=%project%-[Debug]
)

set dir=%path-to-archive%\%project%
rd /S /Q %dir%
del .release.log 2>nul
md %dir% 2>>.release.log

echo.
echo %dir%.zip
echo.

call :md \%project%
call :copy %state%\whoisalive.exe %project%\
call :copy work\config-release.xml %project%\config.xml

echo zip: %dir%.zip
pkzipc -add -dir=relative %dir%.zip %dir%\*.* >>.release.log
rd /S /Q %dir%
del .release.log 2>nul

goto :eof

:md
echo md: %1
echo md: %1 >>.release.log
md "%dir%\%1" 2>>.release.log
goto :eof

:copy
echo copy: %1 -^> %2
echo copy: %1 -^> %2 >>.release.log
copy "%1" "%dir%\%2" >>.release.log
if not %ERRORLEVEL%==0 (
  echo �訡��: ERRORLEVEL=%ERRORLEVEL%
  goto :fail
)
goto :eof

:fail
echo.
echo ������ �����襭� � �訡���
echo ���஡���� ᬮ��� � ".release.log"
exit
