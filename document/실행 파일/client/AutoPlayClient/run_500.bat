for /L %%i in (1, 1, 500) do (
    start /b AutoPlayClient.exe user%%i false
)