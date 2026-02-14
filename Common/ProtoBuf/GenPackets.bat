@echo off

rem bat 파일이 실행되고 있는 경로를 설정
pushd %~dp0

set SERVER="../../GameServer"
set DUMMY_CLIENT="../../DummyClient"

protoc -I=./ --cpp_out=./ ./*.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_

IF ERRORLEVEL 1 PAUSE

XCOPY /Y /I *.pb.h %SERVER%
XCOPY /Y /I *.pb.cc %SERVER%
XCOPY /Y /I ClientPacketHandler.h %SERVER%

XCOPY /Y /I *.pb.h %DUMMY_CLIENT%
XCOPY /Y /I *.pb.cc %DUMMY_CLIENT%
XCOPY /Y /I ServerPacketHandler.h %DUMMY_CLIENT%

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE