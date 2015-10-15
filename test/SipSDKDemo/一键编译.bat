::关闭回显
@echo off

echo －－－－－ 拷贝 IVS SDK 库文件 －－－－－－－－－－－－－-
xcopy /Y /S .\..\..\self_dev\CGW\eSDK_IVS_API_V1.5.00_Windows\bin\*			.\..\..\source\SipSDK\output\debug
xcopy /Y /S .\..\..\self_dev\CGW\eSDK_IVS_API_V1.5.00_Windows\bin\*			.\..\..\source\SipSDK\output\release
echo －－－－－ 拷贝 IVS SDK 库文件 成功－－－－－－－－－－－－

echo －－－－－ 拷贝 BP 版本 －－－－－－－－－－－－－-
xcopy /Y /S .\..\..\platform\IVS\IVS_BP\release\new\windows\debug\bp_based.dll 		.\..\..\source\SipSDK\output\debug
xcopy /Y /S .\..\..\platform\IVS\IVS_BP\release\new\windows\release\bp_base.dll 	.\..\..\source\SipSDK\output\release
echo －－－－－ 拷贝 BP 版本 成功－－－－－－－－－－－－

echo －－－－－ 拷贝 ACE 6.1.0版本 －－－－－－－－－－－－－-
xcopy /Y /S .\..\..\platform\IVS\IVS_COMMON\CBB\release\new\windows\debug\ACEd.dll 		.\..\..\source\SipSDK\output\debug
xcopy /Y /S .\..\..\platform\IVS\IVS_COMMON\CBB\release\new\windows\release\ACE.dll		.\..\..\source\SipSDK\output\release
echo －－－－－ 拷贝 ACE 6.1.0版本 成功－－－－－－－－－－－－

pause

echo.
echo －－－－－－－－－－－－－－编译 SipSDKDemo Debug 版本 －－－－－－－－－－－－－
@"%VS100COMNTOOLS%\..\IDE\devenv.com" .\SipSDKDemo.sln /Rebuild "Debug|Win32"
echo.
echo －－－－－－－－－－－－－－编译 SipSDKDemo Debug 版本成功－－－－－－－－－－－－

echo.
echo －－－－－－－－－－－－－－编译 SipSDKDemo Release 版本 －－－－－－－－－－－－
@"%VS100COMNTOOLS%\..\IDE\devenv.com" .\SipSDKDemo.sln /Rebuild "Release|Win32"
echo.
echo －－－－－－－－－－－－－－编译 SipSDKDemo Release 版本成功－－－－－－－－－－

@echo .
@echo －－－－－－－－－－－－－－解决方案编译完成－－－－－－－－－－－－－－－

pause