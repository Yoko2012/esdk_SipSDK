::�رջ���
@echo off

::��õ�ǰʱ�䣬��Ϊ���ɰ汾��Ŀ¼��
for /F "tokens=1-4 delims=-/ " %%i in ('date /t') do (
   set Year=%%i
   set Month=%%j
   set Day=%%k
   set DayOfWeek=%%l
)
for /F "tokens=1-2 delims=: " %%i in ('time /t') do (
   set Hour=%%i
   set Minute=%%j
)

::���ø�������
set   	DateTime=%Year%-%Month%-%Day%-%Hour%-%Minute%
set	WinRarRoot=C:\Program Files\WinRAR

@echo off
echo %DateTime%
@echo .

echo.
echo �������������������������������� SIPSDK Debug�汾 ��������������������������
@"%VS100COMNTOOLS%\..\IDE\devenv.com" .\build\sipsdk.sln /Rebuild "Debug|Win32"
echo.
echo �������������������������������� SIPSDK Debug�汾�ɹ�������������������������

echo.
echo �������������������������������� SIPSDK Release�汾 ������������������������
@"%VS100COMNTOOLS%\..\IDE\devenv.com" .\build\sipsdk.sln /Rebuild "Release|Win32" /out output.txt
echo.
echo �������������������������������� SIPSDK Release�汾�ɹ���������������������

@echo .
@echo �����������������������������������������ɣ�����������������������������

@echo .
@echo ������������������������������ʼ�����汾����������������������������������	
@echo .

xcopy /y /i /r /s ".\sipsdk\sipsdk.h"           			"version\include\"
xcopy /y /i /r /s ".\sipsdk\DataType.h"           			"version\include\"
xcopy /y /i /r /s ".\eSDK_RtpPacket\eSDK_RtpPacket.h"        		"version\include\"

xcopy /E /y /i /r /s  ".\output\debug\config\sipstack_config.xml"       "version\debug\config\"
xcopy /y /i /r /s ".\output\debug\sipsdk.dll"           		"version\debug\"
xcopy /y /i /r /s ".\output\debug\sipsdk.lib"           		"version\debug\"
xcopy /y /i /r /s ".\output\debug\sipsdk.pdb"           		"version\debug\"
xcopy /y /i /r /s ".\output\debug\rtpconvertsdk.dll"           		"version\debug\"
xcopy /y /i /r /s ".\output\debug\rtpconvertsdk.lib"           		"version\debug\"
xcopy /y /i /r /s ".\output\debug\rtpconvertsdk.pdb"           		"version\debug\"
xcopy /y /i /r /s ".\..\..\self_dev\eSDKClientLogAPI\debug\eSDKClientLogCfg.ini"  	"version\debug\"
xcopy /y /i /r /s ".\..\..\self_dev\eSDKClientLogAPI\debug\eSDKLogAPI.dll"		"version\debug\"
xcopy /y /i /r /s ".\..\..\self_dev\eSDKClientLogAPI\debug\eSDKLogAPI.lib"         	"version\debug\"

xcopy /E /y /i /r /s  ".\output\release\config\sipstack_config.xml"     "version\release\config\"
xcopy /y /i /r /s ".\output\release\sipsdk.dll"           		"version\release\"
xcopy /y /i /r /s ".\output\release\sipsdk.lib"           		"version\release\"
xcopy /y /i /r /s ".\output\release\sipsdk.pdb"           		"version\release\"
xcopy /y /i /r /s ".\output\release\rtpconvertsdk.dll"           	"version\release\"
xcopy /y /i /r /s ".\output\release\rtpconvertsdk.lib"           	"version\release\"
xcopy /y /i /r /s ".\output\release\rtpconvertsdk.pdb"           	"version\release\"
xcopy /y /i /r /s ".\..\..\self_dev\eSDKClientLogAPI\release\eSDKClientLogCfg.ini"  	"version\release\"
xcopy /y /i /r /s ".\..\..\self_dev\eSDKClientLogAPI\release\eSDKLogAPI.dll"		"version\release\"
xcopy /y /i /r /s ".\..\..\self_dev\eSDKClientLogAPI\release\eSDKLogAPI.lib"         	"version\release\"

@echo .
@echo ������������������������������ʼ����汾��������������������������������	

cd version
"%WinRarRoot%\WinRAR.exe" a -r SIP_SDK_%DateTime%.rar .\
cd ..


pause