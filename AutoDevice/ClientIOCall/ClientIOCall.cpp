// adddevice.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>

#include <initguid.h>
#include <setupapi.h>
#include <stdio.h>
#include ".\..\common\common.h"
#include ".\..\common\hidmapper.h"
#include ".\..\common\driverapi.h"

#pragma comment(lib,"setupapi.lib")

#define LEDSHOWTEST

int main(int argc, char* argv[])
{
	HANDLE hDevice;
	BOOLEAN bRet = FALSE;
	int count = 0;
	HIDKEYCODE HKeyCode = { 0, }; // �迭�� �����ؼ� �ѹ��� �������� ��Ʈ���� �����°͵� �����ϴ�
	HIDMOUSECODE HMouseCode = { 0, }; // �迭�� �����ؼ� �ѹ��� �������� ��Ʈ���� �����°͵� �����ϴ�

	_HK_ClientDriverApiInitialize();

	hDevice = _HK_ClientDriverApiFindDriverName();
	if (hDevice == (HANDLE)-1)
	{
		return 0; 
	}

#ifdef LEDEVENTTEST
	// ����̹��� Ű���� LED �̺�Ʈ�� �����Ѵ�
	HANDLE hLED_Event = 0;
	hLED_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
	bRet = _HK_ClientDriverApiSetLEDEvent(hDevice, hLED_Event);

	// Ű������ LED�� �Ѻ���
	memset(&HKeyCode, 0, sizeof(HIDKEYCODE));

	HKeyCode.Key[0] = 0x53; // NUMLOCK��ư�� ������
	_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
	Sleep(100);
	HKeyCode.Key[0] = 0;
	_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

	// �ü���� LED�� �Ѷ�� �޽����� �����ߴ����� Ȯ���Ѵ�
	WaitForSingleObject(hLED_Event, INFINITE);

	// ����̹��κ��� Ű���� LED ����Ÿ�� �о��
	HIDLED	HidLedData = { 0, };
	_HK_ClientDriverApiQueryIndicators(hDevice, &HidLedData);

	if (hLED_Event)
		CloseHandle(hLED_Event);
#endif

#ifdef LEDSHOWTEST 

	for (count = 0; count < 1000; count++)
	{
		// Ű������ LED�� �Ѻ���
		memset(&HKeyCode, 0, sizeof(HIDKEYCODE));

		HKeyCode.Key[0] = 0x53;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0x39;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0x47;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
	}

	for (count = 0; count < 10; count++)
	{
		// Ű������ LED�� �Ѻ���
		memset(&HKeyCode, 0, sizeof(HIDKEYCODE));

		HKeyCode.Key[0] = 0x53; // NUMLOCK��ư�� ������
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		Sleep(100);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(100);

		HKeyCode.Key[0] = 0x39; // CAPSLOCK��ư�� ������
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		Sleep(100);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(100);

		HKeyCode.Key[0] = 0x47; // SCROLLLOCK��ư�� ������
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		Sleep(100);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(100);

		HKeyCode.u.Modifier.LWINDOW = 1; // Window Ű�� ��������ó�� �غ���
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(100);

		HKeyCode.u.Modifier.LWINDOW = 0; // Window Ű�� �������ó�� �غ���
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(100);
	}

	for (count = 0; count < 1000; count++)
	{
		// Ű������ LED�� �Ѻ���
		memset(&HKeyCode, 0, sizeof(HIDKEYCODE));

		HKeyCode.Key[0] = 0x53;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0x39;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0x47;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
	}

	for (count = 0; count < 10; count++)
	{
		// Ű������ LED�� �Ѻ���
		memset(&HKeyCode, 0, sizeof(HIDKEYCODE));

		// NUMLOCK, CAPSLOCK, SCROLLLOCK��ư�� ������
		HKeyCode.Key[0] = 0x53; 
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(10);

		HKeyCode.Key[0] = 0x39;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(10);
		HKeyCode.Key[0] = 0x47;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(10);

		HKeyCode.u.Modifier.LWINDOW = 1; // Window Ű�� ��������ó�� �غ���
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(10);

		HKeyCode.u.Modifier.LWINDOW = 0; // Window Ű�� �������ó�� �غ���
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);

		Sleep(10);
	}

	for (count = 0; count < 1000; count++)
	{
		// Ű������ LED�� �Ѻ���
		memset(&HKeyCode, 0, sizeof(HIDKEYCODE));

		HKeyCode.Key[0] = 0x53;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0x39;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0x47;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
		HKeyCode.Key[0] = 0;
		_HK_ClientDriverApiSimulateKey(hDevice, &HKeyCode);
	}
#endif

#ifdef MOUSETEST 

	srand(GetTickCount());

	for (count = 0; count < 500; count++)
	{
		memset(&HMouseCode, 0, sizeof(HIDMOUSECODE));
		int z = rand();
		if (z%4 == 0)
		{
			HMouseCode.X = rand()%40;
			HMouseCode.Y = rand() % 40;
		}
		if (z%4 == 1)
		{
			HMouseCode.X = (rand() % 40) * -1;
			HMouseCode.Y = rand() % 40;
		}
		if (z%4 == 2)
		{
			HMouseCode.X = rand() % 40;
			HMouseCode.Y = (rand() % 40) * -1;
		}
		if (z%4 == 3)
		{
			HMouseCode.X = (rand() % 40) * -1;
			HMouseCode.Y = (rand() % 40) * -1;
		}
		_HK_ClientDriverApiSimulateMouse(handle, &HMouseCode);
		Sleep(8);
	}
#endif

	_HK_ClientDriverApiCloseHandle( hDevice );

	return 0;
}
