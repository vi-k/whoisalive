@echo off
copy Debug\pinger.exe work
cd work\
pinger.exe %*
