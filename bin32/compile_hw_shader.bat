@echo off
fxc.exe /E main /T ps_2_0 /Fo media\shader\default_ps.o media\shader\default_ps.hlsl
fxc.exe /E main /T ps_2_0 /Fo media\shader\jitaa_ps.o media\shader\jitaa_ps.hlsl
fxc.exe /E main /T vs_2_0 /Fo media\shader\default_vs.o media\shader\default_vs.hlsl
fxc.exe /E main /T vs_2_0 /Fo media\shader\rhz_vs.o media\shader\rhz_vs.hlsl
pause