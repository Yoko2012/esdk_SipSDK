::�رջ���
@echo off

echo ���������� ���� IVS SDK ���ļ� ��������������������������-
xcopy /Y /S .\..\..\self_dev\CGW\eSDK_IVS_API_V1.5.00_Windows\bin\*			.\..\..\source\SipSDK\output\debug
xcopy /Y /S .\..\..\self_dev\CGW\eSDK_IVS_API_V1.5.00_Windows\bin\*			.\..\..\source\SipSDK\output\release
echo ���������� ���� IVS SDK ���ļ� �ɹ�������������������������

echo ���������� ���� BP �汾 ��������������������������-
xcopy /Y /S .\..\..\platform\IVS\IVS_BP\release\new\windows\debug\bp_based.dll 		.\..\..\source\SipSDK\output\debug
xcopy /Y /S .\..\..\platform\IVS\IVS_BP\release\new\windows\release\bp_base.dll 	.\..\..\source\SipSDK\output\release
echo ���������� ���� BP �汾 �ɹ�������������������������

echo ���������� ���� ACE 6.1.0�汾 ��������������������������-
xcopy /Y /S .\..\..\platform\IVS\IVS_COMMON\CBB\release\new\windows\debug\ACEd.dll 		.\..\..\source\SipSDK\output\debug
xcopy /Y /S .\..\..\platform\IVS\IVS_COMMON\CBB\release\new\windows\release\ACE.dll		.\..\..\source\SipSDK\output\release
echo ���������� ���� ACE 6.1.0�汾 �ɹ�������������������������

pause

echo.
echo �������������������������������� SipSDKDemo Debug �汾 ��������������������������
@"%VS100COMNTOOLS%\..\IDE\devenv.com" .\SipSDKDemo.sln /Rebuild "Debug|Win32"
echo.
echo �������������������������������� SipSDKDemo Debug �汾�ɹ�������������������������

echo.
echo �������������������������������� SipSDKDemo Release �汾 ������������������������
@"%VS100COMNTOOLS%\..\IDE\devenv.com" .\SipSDKDemo.sln /Rebuild "Release|Win32"
echo.
echo �������������������������������� SipSDKDemo Release �汾�ɹ���������������������

@echo .
@echo �����������������������������������������ɣ�����������������������������

pause