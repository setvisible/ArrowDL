@echo off

echo .. Deleting Chrome Registry
REG DELETE "HKCU\Software\Google\Chrome\NativeMessagingHosts\DownRightNow" /f

echo .. Deleting Firefox Registry
REG DELETE "HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow" /f

echo .. Deleting Waterfox Registry
REG DELETE "HKCU\SOFTWARE\Waterfox\NativeMessagingHosts\DownRightNow" /f

echo .. Deleting Thunderbird Registry
REG DELETE "HKCU\SOFTWARE\Thunderbird\NativeMessagingHosts\DownRightNow" /f

echo.
echo ^>^>^> Done! ^<^<^<
echo.
pause

