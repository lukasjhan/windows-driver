// AddDevice.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include <windows.h>

#include <initguid.h>
#include <setupapi.h>
#include <stdio.h>

DEFINE_GUID(SampleGuid, 0x5665dec0, 0xa40a, 0x11d1, 0xb9, 0x84, 0x0, 0x20, 0xaf, 0xd7, 0x97, 0x78);
// SampleGuid�� ����մϴ�. �̷� GUID�� ���� ��������� ���ǵ��� ���� �����μ�, ���÷� �����Ͽ� ����մϴ�

#define MAXDEVICENUMBER 10 // 10�� ������ ���� Guid�� ����ϴ� ��ġ�� �����Ѵٴ� �ǹ��Դϴ�

#include <winioctl.h>

// �������α׷��� �ְ���� ControlCode�� �����մϴ�
#define IOCTL_BUSDRIVER_ADDDEVICE		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BUSDRIVER_REMOVEDEVICE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma comment(lib,"SetupApi")

// ���� �����Ǵ� GUID�� �����ϴ� DeviceStack�� ������ �˷��ݴϴ�
int GetDeviceStackNameCount(struct _GUID * pGuid)
{
	SP_INTERFACE_DEVICE_DATA interfaceData;
	int index = 0;
	HDEVINFO Info = SetupDiGetClassDevs(pGuid, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (Info == (HDEVINFO)-1)
		return 0; // �ý����� �̷� ����� �������� ���Ѵ�

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

	while (1)
	{
		BOOLEAN bl;
		bl = SetupDiEnumDeviceInterfaces(Info, 0, pGuid, index, &interfaceData);
		if (bl == FALSE)
			break;
		index++;
	}

	SetupDiDestroyDeviceInfoList(Info);

	return index;
}

// ���� �����Ǵ� GUID�� �����ϴ� DeviceStack�� �̸����� �˷��ݴϴ�
BOOLEAN GetDeviceStackName(struct _GUID * pGuid, WCHAR ** ppDeviceName, int index)
{
	DWORD size;
	BOOLEAN bl;
	SP_INTERFACE_DEVICE_DATA interfaceData;
	PSP_INTERFACE_DEVICE_DETAIL_DATA pData;
	HDEVINFO Info = SetupDiGetClassDevs(pGuid, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	WCHAR *pDeviceName;
	*ppDeviceName = (WCHAR *)0;

	if (Info == (HANDLE)-1)
		return FALSE;

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

	bl = SetupDiEnumDeviceInterfaces(Info, 0, pGuid, index, &interfaceData);
	if (bl == FALSE)
		return bl;

	SetupDiGetDeviceInterfaceDetail(Info, &interfaceData, 0, 0, &size, 0);
	pData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
	pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	SetupDiGetDeviceInterfaceDetail(Info, &interfaceData, pData, size, 0, 0);

	pDeviceName = (WCHAR *)malloc((wcslen(pData->DevicePath) + 1)*sizeof(WCHAR));
	pDeviceName[0] = NULL;
	wcscpy(pDeviceName, pData->DevicePath);
	free(pData);

	SetupDiDestroyDeviceInfoList(Info);
	*ppDeviceName = pDeviceName;
	return TRUE;
}

int main()
{
	HANDLE handle;
	int count;
	WCHAR *pDeviceName;
	BOOLEAN bl;
	ULONG ret;
	
	count = GetDeviceStackNameCount((struct _GUID *)&SampleGuid);
	if (count == 0)
		return 0; // �ý����� SampleGuid�� �����ϴ� ��ġ�� ��ġ���� �ʾҽ��ϴ�

	bl = GetDeviceStackName((struct _GUID *)&SampleGuid, &pDeviceName, 0); // �翬�� 1���̻��� ��ġ�� ��ġ�Ǿ������Ƿ�..0�� ����Ѵ�
																		   // pDeviceName�� �Լ������� �Ҵ�Ǵ� �޸��̹Ƿ�, ����� ������ �ݵ�� �����Ͽ��� �Ѵ�

	if (bl == FALSE)
		return 0; // �̷����� ����� �Ѵ�

	handle = CreateFile(pDeviceName, GENERIC_READ | GENERIC_WRITE
		, 0, 0, OPEN_EXISTING, 0
		, 0);
	if (handle == (HANDLE)-1)
	{
		free(pDeviceName);
		return 0; // Stack�� ������, ���� ������ �����Ǿ� �ִ�
	}

	DeviceIoControl(handle, IOCTL_BUSDRIVER_ADDDEVICE, NULL, 0, NULL, 0, &ret, NULL);

	CloseHandle(handle);

	free(pDeviceName);
	return 0;

	return 0;
}

