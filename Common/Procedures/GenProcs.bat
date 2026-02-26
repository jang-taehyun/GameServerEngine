@echo off

rem bat 파일이 실행되고 있는 경로를 설정
pushd %~dp0

set SERVER="../../GameServer"

GenProcs.exe --path=../../GameServer/GameDB.xml --output=GenProcedures.h

IF ERRORLEVEL 1 PAUSE

XCOPY /Y GenProcedures.h %SERVER%

DEL /Q /F *.h

PAUSE