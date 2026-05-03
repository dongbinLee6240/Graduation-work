@echo off
echo ==== Protobuf C++ Generation ====

:: protoc.exe를 이용해 Protocol.proto 파일을 C++ 코드로 변환
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto

echo Generation Complete!
pause