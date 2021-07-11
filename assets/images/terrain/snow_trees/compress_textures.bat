@echo off

for %%f in (*.png) do (
    kram.exe encode -f bc7 -mipnone -type 2d -quality 50 -i %%f -o %%~nf.ktx
)