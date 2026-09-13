@echo off
chcp 65001 >nul
echo ========================================================
echo   KHOI DONG MAY CHU GPT-SoVITS API CHO CATCHIM (PORT 9880)
echo ========================================================
echo.

set GPT_DIR=D:\DATA\source\GPT-SoVITS

if not exist "%GPT_DIR%" (
    echo [LOI] Khong tim thay thu muc GPT-SoVITS tai %GPT_DIR%
    pause
    exit /b 1
)

cd /d "%GPT_DIR%"

echo [INFO] Dang kiem tra moi truong Python...
where python >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [LOI] Khong tim thay lenh python trong PATH.
    echo Vui long cai dat Python hoac kich hoat moi truong conda / venv cua GPT-SoVITS.
    pause
    exit /b 1
)

echo [INFO] Dang khoi dong api_v2.py tai http://127.0.0.1:9880 ...
echo [INFO] Hay giu cua so nay mo khi su dung giong Clone tren web Catchim.
echo.

python api_v2.py -a 127.0.0.1 -p 9880 -c GPT_SoVITS/configs/tts_infer.yaml

pause