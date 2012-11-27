@echo off
for /R shader\hwcode %%A in (*.hlsl) do (
     rem echo %%~dpnA
     rem echo %%~sA
     rem echo %%~nA
     "fxc.exe" /E vs_main /Zpc /T vs_3_0 /Fo shader\%%~nA.srvs %%~dpnA.hlsl
     "fxc.exe" /E ps_main /Zpc /T ps_3_0 /Fo shader\%%~nA.srps %%~dpnA.hlsl
)

pause