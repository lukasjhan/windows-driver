// app.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include <Windows.h>
#include <winioctl.h>
#include <setupapi.h>

#pragma comment(lib,"setupapi.lib")

// ����̹��� �����ϱ� ���ؼ� ����̹��ҽ����� ����� ������ GUID�� �����Ѵ�
#include <initguid.h>

DEFINE_GUID(GUID_SIMPLE, 
0x5665dec0, 0xa40a, 0x11d1, 0xb9, 0x84, 0x0, 0x20, 0xaf, 0xd7, 0x97, 0x62);

// DeviceIoControl API�� ����� IOControlCode�� �����Ѵ�
#define SIMPLEIOCTLBASE		       0xA00
#define IOCTL_SIMPLE_CONTROL	CTL_CODE(FILE_DEVICE_UNKNOWN, SIMPLEIOCTLBASE, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MAXDEVICENUMBER 10 // ���� Interface GUID�� ����ϴ� ����̽��� �ִ� 10������ �˻��Ϸ��� �ǵ��� �����ϴ�

/*
	GetDeviceStackNameCount()�Լ��� GetDeviceStackName()�Լ��� ���� ���Ƿ� �ۼ��� �����Լ��Դϴ�.
	�̰������� � ������� InterfaceGUID�� ����Ͽ� ����̽������� �̸��� ���ϴ����� �� �� �ֽ��ϴ�
*/

// ���� �����Ǵ� GUID�� �����ϴ� DeviceStack�� ������ �˷��ݴϴ�
int GetDeviceStackNameCount( struct _GUID * pGuid )
{
	SP_INTERFACE_DEVICE_DATA interfaceData;
	int index=0;
	HDEVINFO Info = SetupDiGetClassDevs( pGuid, 0, 0, DIGCF_PRESENT|DIGCF_INTERFACEDEVICE );

	if( Info == (HDEVINFO) -1 )
		return 0; // �ý����� �̷� ����� �������� ���Ѵ�

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
	
	while( 1 )
	{
		BOOLEAN bl;
		bl = SetupDiEnumDeviceInterfaces( Info, 0, pGuid, index, &interfaceData );
		if( bl == FALSE )
			break;
		index++;
	}

	SetupDiDestroyDeviceInfoList( Info );

	return index;
}

// ���� �����Ǵ� GUID�� �����ϴ� DeviceStack�� �̸����� �˷��ݴϴ�
BOOLEAN GetDeviceStackName( struct _GUID * pGuid, WCHAR ** ppDeviceName, int index )
// index : ���� Interface GUID�� ���� ��ġ�� �������ΰ��, ���� ���� �˻��Ǵ� ��ġ�� �����Ϸ��� �̰��� 0�� ����մϴ�
{
	DWORD size;
	BOOLEAN bl;
	SP_INTERFACE_DEVICE_DATA interfaceData;
	PSP_INTERFACE_DEVICE_DETAIL_DATA pData;
	HDEVINFO Info = SetupDiGetClassDevs( pGuid, 0, 0, DIGCF_PRESENT|DIGCF_INTERFACEDEVICE );
	WCHAR *pDeviceName;
	*ppDeviceName = (WCHAR *)0;

	if( Info == (HANDLE) -1 )
		return FALSE;

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
	
	bl = SetupDiEnumDeviceInterfaces( Info, 0, pGuid, index, &interfaceData );
	if( bl == FALSE )
		return bl;

	SetupDiGetDeviceInterfaceDetail( Info, &interfaceData, 0, 0, &size, 0 );
	pData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc( size );
	pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	SetupDiGetDeviceInterfaceDetail( Info, &interfaceData, pData, size, 0, 0 );

	pDeviceName = (WCHAR *)malloc( (wcslen(pData->DevicePath) + 1) * sizeof(WCHAR) );
	memset( pDeviceName, 0, (wcslen(pData->DevicePath) + 1) * sizeof(WCHAR));
	wcscpy( pDeviceName, pData->DevicePath );
	free( pData );

	SetupDiDestroyDeviceInfoList( Info );
	*ppDeviceName = pDeviceName;
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int count;
	HANDLE handle;
	WCHAR *pDeviceName;
	BOOLEAN bl;

	count = GetDeviceStackNameCount( (struct _GUID *)&GUID_SIMPLE );
	if( count == 0 )
	{
		MessageBox( NULL, L"����̹���ġ�� �����ϴ�", L"����", MB_ICONERROR );
		return 0; // �ý����� SampleGuid�� �����ϴ� ��ġ�� ��ġ���� �ʾҽ��ϴ�
	}

	bl = GetDeviceStackName( (struct _GUID *)&GUID_SIMPLE, &pDeviceName, 0 ); 

	if( bl == FALSE )
	{
		MessageBox( NULL, L"����̹���ġ�� �����ϴ�", L"����", MB_ICONERROR );
		return 0;
	}

	// pDeviceName�� ����̽����ÿ� �����ϱ� ���� �̸��� �������ϴ�
	handle = CreateFile( pDeviceName, GENERIC_READ|GENERIC_WRITE
					   , 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL
					   , 0 );
	// �̶� ����̹������� IRP_MJ_CREATE��ɾ ���޵˴ϴ�
	if( handle == (HANDLE)-1 )
	{
		MessageBox( NULL, L"����̹��� ������� �ʽ��ϴ�(CreateFile ����)", L"����", MB_ICONERROR );

		free( pDeviceName );
		return 0; // Stack�� ������, ���� ������ �����Ǿ� �ִ�
	}

	MessageBox( NULL, L"����̹��� ����Ǿ����ϴ�. ����� �����մϴ�", L"����", MB_OK );

	// ����� �б⸦ �մϴ�
	// ���� Ư���� ����� �����ϴ�
	DWORD ret;
	char WriteData[100];
	char ReadData[100];

	WriteFile( handle, WriteData, sizeof(WriteData), &ret, NULL );
	ReadFile( handle, ReadData, sizeof(ReadData), &ret, NULL );

	// DeviceIoControl �Լ��� ����غ��ϴ�
	DeviceIoControl( handle, IOCTL_SIMPLE_CONTROL, WriteData, sizeof(WriteData), ReadData, sizeof(ReadData), &ret, NULL );	

	CloseHandle( handle );

	MessageBox( NULL, L"����̹��� ������ �����Ͽ����ϴ�", L"����", MB_OK );

	free( pDeviceName );
	return 0;
}

