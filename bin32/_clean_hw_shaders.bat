@echo off
for /R shader %%A in (*.srvs) do (
     rem echo %%~dpnA
     rem echo %%~sA
     rem echo %%~nA
     del %%A
)

for /R shader %%A in (*.srps) do (
     rem echo %%~dpnA
     rem echo %%~sA
     rem echo %%~nA
     del %%A
)
