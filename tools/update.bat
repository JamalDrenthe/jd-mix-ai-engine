@echo off
title JD Mix AI Engine - Update Utility
echo Checking for updates...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0update.ps1"
pause
