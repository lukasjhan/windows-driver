#include <wdm.h>
#include <wdmsec.h>
#include <stdio.h>

#include "devioctl.h"

#include ".\\..\common\appinterface.h"

#ifndef _TABLEHOOKER_H_
#define _TABLEHOOKER_H_

// --------------- ����� �����Ѵ� ----------------------
// ����ä�⸦ ������ ����� �̸��� �����Ѵ�
#define TARGET_DRIVER_NAME	L"\\Driver\\KBDCLASS" // Ű����Ŭ��������̹��� ����ä���غ���

// �������α׷��� ����Ǳ� ���ؼ� �����ϴ� �̸�
#define CUSTOM_DEVICE_NAME	L"\\Device\\" CUSTOM_NAME
#define CUSTOM_WIN32_NAME	L"\\DosDevices\\" CUSTOM_NAME

typedef struct _TABLEHOOKER_DEVICE_EXTENSION
{
	ULONG		CustomSignature; // 'HAJE'
} TABLEHOOKER_DEVICE_EXTENSION, *PTABLEHOOKER_DEVICE_EXTENSION;

NTSTATUS DriverEntry(
	PDRIVER_OBJECT		pDriverObject,
	PUNICODE_STRING		pRegistryPath
);

void TABLEHOOKER_Unload(
	PDRIVER_OBJECT	pDriverObject
);

NTSTATUS
TABLEHOOKER_Cleanup(
	IN PDEVICE_OBJECT	pFDO,
	IN PIRP				pIrp
);

NTSTATUS
TABLEHOOKER_Create(
	IN PDEVICE_OBJECT	pFDO,
	IN PIRP				pIrp
);

NTSTATUS
TABLEHOOKER_Close(
	IN PDEVICE_OBJECT	pFDO,
	IN PIRP				pIrp
);

NTSTATUS
TABLEHOOKER_DeviceIoControl(
	IN PDEVICE_OBJECT	pFDO,
	IN PIRP				pIrp
);

// --------------- ���� ��ƿ��Ƽ�Լ��� �����Ѵ� ----------------------
void HookCallback(unsigned char MajorFunction);
void UnhookCallback(unsigned char MajorFunction);
PDRIVER_OBJECT SearchDriverObject(PUNICODE_STRING pUni);
NTSTATUS NewHookerDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

BOOLEAN
InsertLog(
	PIRP	Irp,
	BOOLEAN bIsStatus);

ULONG
GetAndRemoveLog(
	void * OutputBuffer,
	ULONG  OutputBufferLength);

void
SpyClearLog();



NTSTATUS NTAPI ObReferenceObjectByName(PUNICODE_STRING ObjectName,
	ULONG Attributes,
	PACCESS_STATE AccessState,
	ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType,
	KPROCESSOR_MODE AccessMode,
	PVOID ParseContext OPTIONAL,
	PVOID* Object);

POBJECT_TYPE ObGetObjectType(PVOID Object);

typedef struct _RECORD_LIST {
	LIST_ENTRY						List;
	RECORD_LIST_APP					ListData;
} RECORD_LIST, *PRECORD_LIST;


#endif //_TABLEHOOKER_H_