// CallExample.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include <windows.h>
#include <setupapi.h>

#include <winioctl.h>

#pragma comment(lib,"SetupApi")

#include <initguid.h>

DEFINE_GUID(GUID_DEVINTERFACE_PCISAMPLE,
	0x946A7B1A, 0xEBBC, 0x422A, 0xA8, 0x1F, 0xF0, 0x7C, 0x8E, 0x41, 0xD1, 0x11);

typedef struct {
	USHORT  VendorID;
	USHORT  DeviceID;
	USHORT  Command;
	USHORT  Status;
	UCHAR   RevisionID;
	UCHAR   ProgIf;
	UCHAR   SubClass;
	UCHAR   BaseClass;
	UCHAR   CacheLineSize;
	UCHAR   LatencyTimer;
	UCHAR   HeaderType;
	UCHAR   BIST;

	union {
		struct _PCIDEF_HEADER_TYPE_0 {
			ULONG   BaseAddresses[6];
			ULONG   CIS;
			USHORT  SubVendorID;
			USHORT  SubSystemID;
			ULONG   ROMBaseAddress;
			UCHAR   CapabilitiesPtr;
			UCHAR   Reserved1[3];
			ULONG   Reserved2;
			UCHAR   InterruptLine;
			UCHAR   InterruptPin;
			UCHAR   MinimumGrant;
			UCHAR   MaximumLatency;
		} type0;
	} u;
} PCIDEF_COMMON_HEADER, *PPCIDEF_COMMON_HEADER;

typedef struct _PCISAMPLE_MAPPING_MEMORY
{
	ULONGLONG				Reserved1;
	ULONGLONG				Reserved2;
	ULONGLONG				PhysicalAddress;
	ULONGLONG				Reserved3;
	ULONGLONG				UserAddress;
	ULONGLONG				Size;
}PCISAMPLE_MAPPING_MEMORY, *PPCISAMPLE_MAPPING_MEMORY;

#define IOCTL_PCISAMPLE_REGISTER_SHARED_EVENT				CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA00, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PCISAMPLE_MAPPING_MEMORY						CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA01, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PCISAMPLE_UNMAPPING_MEMORY					CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA02, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PCISAMPLE_ALLOCATE_CONTIGUOUS_MEMORY_FOR_DMA	CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA03, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PCISAMPLE_FREE_CONTIGUOUS_MEMORY_FOR_DMA		CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA04, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PCISAMPLE_GET_CONFIGURATION_REGISTER			CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA05, METHOD_BUFFERED, FILE_ANY_ACCESS)

// ------------- ����̽� ������ �̸��� ���ϴ� �Լ��� �����Ѵ�

static int GetDeviceStackNameCount()
{
	SP_INTERFACE_DEVICE_DATA interfaceData;
	int index = 0;
	HDEVINFO Info = SetupDiGetClassDevs((struct _GUID *)&GUID_DEVINTERFACE_PCISAMPLE, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (Info == (HDEVINFO)-1)
		return 0; 

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

	while (1)
	{
		BOOLEAN bl;
		bl = SetupDiEnumDeviceInterfaces(Info, 0, (struct _GUID *)&GUID_DEVINTERFACE_PCISAMPLE, index, &interfaceData);
		if (bl == FALSE)
			break;
		index++;
	}

	SetupDiDestroyDeviceInfoList(Info);

	return index;
}

static BOOLEAN GetDeviceStackName(WCHAR * pDeviceName, int index)
{
	DWORD size;
	BOOLEAN bl;
	SP_INTERFACE_DEVICE_DATA interfaceData;
	PSP_INTERFACE_DEVICE_DETAIL_DATA pData;
	HDEVINFO Info = SetupDiGetClassDevs((struct _GUID *)&GUID_DEVINTERFACE_PCISAMPLE, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (Info == (HANDLE)-1)
		return FALSE;

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

	bl = SetupDiEnumDeviceInterfaces(Info, 0, (struct _GUID *)&GUID_DEVINTERFACE_PCISAMPLE, index, &interfaceData);
	if (bl == FALSE)
		return bl;

	SetupDiGetDeviceInterfaceDetail(Info, &interfaceData, 0, 0, &size, 0);
	pData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
	pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	SetupDiGetDeviceInterfaceDetail(Info, &interfaceData, pData, size, 0, 0);

	wcscpy(pDeviceName, pData->DevicePath);
	free(pData);

	SetupDiDestroyDeviceInfoList(Info);
	return TRUE;
}

// ------------- Main

int main()
{
	BOOLEAN bRet = FALSE;
	ULONG dwRet;
	HANDLE hInterruptEvent = NULL;
	HANDLE hFile = (HANDLE)-1;
	WCHAR DeviceStackName[MAX_PATH] = { 0, };
	PCIDEF_COMMON_HEADER PciConfigurationRegister = { 0, };
	PCISAMPLE_MAPPING_MEMORY	PCIMappingInformation = { 0, };
	PCISAMPLE_MAPPING_MEMORY	PCIDMAMappingInformation = { 0, };

	bRet = GetDeviceStackName(DeviceStackName, 0); // ù��°�� �߰ߵǴ� ����̽� ������ �̸��� ���Ѵ�
	if (bRet == FALSE)
		goto exit; // ����̽��� �߰ߵ��� �ʴ´�

	hFile = CreateFile(DeviceStackName, GENERIC_READ | GENERIC_WRITE
		, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0); // ����̽� ����̹��� �����Ѵ�

	if(hFile == (HANDLE)-1)
		goto exit; // ����̽��� �߰ߵ��� �ʴ´�

	/*
	IOCTL ��ɾ Ȱ���ؼ� �پ��� �׽�Ʈ�� �����غ� �� �ִ�
	�ٸ� �����ڰ� ����ϴ� ���� �ϵ��� ���� �ڵ忡 ����� �� �ִ� �κа� �׷��� ���� �κ��� �����ϰ� �������ƾ� �Ѵ�
	*/

	// ���ͷ�Ʈ�� �߻��ϸ� �ñ׳ε� �̺�Ʈ�� ����Ѵ�
	hInterruptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hInterruptEvent == (HANDLE)-1)
	{
		goto exit; 
	}
	bRet = DeviceIoControl(hFile, IOCTL_PCISAMPLE_REGISTER_SHARED_EVENT, 
		&hInterruptEvent, sizeof(HANDLE), 
		NULL, 0, 
		&dwRet, NULL);

	// PCI Configuration Register�� �о��
	bRet = DeviceIoControl(hFile, IOCTL_PCISAMPLE_GET_CONFIGURATION_REGISTER, 
		NULL, 0, 
		&PciConfigurationRegister, sizeof(PCIDEF_COMMON_HEADER), 
		&dwRet, NULL);

	// PCI Configuration Register���� ���� �����ּ�(BAR)�� ����ؼ� �������α׷��� �����ϴ� ������ �����ּҸ� �����Ѵ�
	// ������ �κ��̹Ƿ� ��Ȯ�ϰ� ����� �������� ���� ���ڸ� ���� �ּ�ó���Ѵ�
	/*
	PCIMappingInformation.PhysicalAddress = 0xXXXXXXXX; // �����ϰ��� �ϴ� �����ּҸ� ����Ѵ�
	PCIMappingInformation.Size = 0xXXXXXXXX; // �����Ϸ��� �����ּ��� ũ�⸦ ����Ѵ�

	bRet = DeviceIoControl(hFile, IOCTL_PCISAMPLE_MAPPING_MEMORY, 
		&PCIMappingInformation, sizeof(PCISAMPLE_MAPPING_MEMORY), 
		&PCIMappingInformation, sizeof(PCISAMPLE_MAPPING_MEMORY), 
		&dwRet, NULL);

	PCIMappingInformation.UserAddress; // ���ε� �����ּҰ� �������
	*/

	// �ݵ�� ����� ���� �����ּҸ� �ݳ��ؾ� �Ѵ�
	/*
	bRet = DeviceIoControl(hFile, IOCTL_PCISAMPLE_UNMAPPING_MEMORY, 
		&PCIMappingInformation, sizeof(PCISAMPLE_MAPPING_MEMORY),
		NULL, 0, 
		&dwRet, NULL);
	*/

	// DMA�� ����ϴ� ���, �������� ũ�⸦ ������ ȣ��Ʈ �����޸𸮰����� Ȯ���Ǿ����� �Ѵ�
	// �̷� �κе� ����̹��� ���ؼ� ���� �� �ִ�
	// ������ �κ��̹Ƿ� ��Ȯ�ϰ� ����� �������� ���� ���ڸ� ���� �ּ�ó���Ѵ�
	/*
	ULONGLONG Size = 0xXXXXXX; // �Ҵ��ϰ��� �ϴ� �������� �����޸�ũ�⸦ �����Ѵ�.�� ũ��� �ּ� 1MB�̻��̾�� �Ѵ�

	bRet = DeviceIoControl(hFile, IOCTL_PCISAMPLE_ALLOCATE_CONTIGUOUS_MEMORY_FOR_DMA,
		&Size, sizeof(ULONGLONG), 
		&PCIDMAMappingInformation, sizeof(PCISAMPLE_MAPPING_MEMORY),
		&dwRet, NULL);
	*/

	// �ݵ�� ����� ���� �����ּҸ� �ݳ��ؾ� �Ѵ�
	/*
	bRet = DeviceIoControl(hFile, IOCTL_PCISAMPLE_FREE_CONTIGUOUS_MEMORY_FOR_DMA,
		&PCIDMAMappingInformation, sizeof(PCISAMPLE_MAPPING_MEMORY),
		NULL, 0, 
		&dwRet, NULL);
	*/

exit:
	if(hInterruptEvent)
		CloseHandle(hInterruptEvent);

	if(hFile != (HANDLE)-1)
		CloseHandle(hFile); // ����̽� ����̹��� ������ �����Ѵ�
	
	return 0;
}

