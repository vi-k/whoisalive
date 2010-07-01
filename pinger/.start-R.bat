@echo off
copy Release\pinger.exe work
cd work\
pinger.exe %*
