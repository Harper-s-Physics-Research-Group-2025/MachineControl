@echo off

REM This is a comment

REM This script initializes some shell variables for working with the teknic and labjack library
REM Author: Joshua Darrow (jd62) June 6, 2025


REM init environment vars to make cmd a developers command prompt (necesssary for microsoft c++ compiler)
call "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat"        

REM add paths to teknic and labjack sdk
set "INCLUDE=C:\Program Files (x86)\Teknic\ClearView\sdk\inc;C:\Program Files (x86)\LabJack\Drivers;%INCLUDE%"
set "LIB=C:\Program Files (x86)\Teknic\ClearView\sdk\lib\win\release\x64;C:\Program Files (x86)\LabJack\Drivers\64bit;%LIB%"
set "PATH=C:\Program Files (x86)\Teknic\ClearView\sdk\lib\win\Release\x64;%PATH%"
echo Teknic and LabJack SDK environment loaded.