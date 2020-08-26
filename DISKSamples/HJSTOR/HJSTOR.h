
#ifndef _HJSTOR_H_
#define _HJSTOR_H_


#include <ntddk.h>
#include <storport.h>
#include <ntddscsi.h>
#include <stdio.h>

//#define USE_INTERRUPT_MODE	1

#define	SIZE_LOGICAL_BLOCK				(0x200UL)  // 512
//#define PAGE_SIZE						(0x1000UL) // PAGE_SIZE == 4K		

#define MAX_DMA_BLOCK_SIZE 				(0x80000UL)

#define MAX_PHYSICAL_PAGES				(0x400UL )//0x1000UL //
#define MAX_DMA_TRANSFER_SIZE			(MAX_PHYSICAL_PAGES * PAGE_SIZE)		
#define DEFAULT_DISK_SIZE				(((ULONGLONG)1024*1024*16)) // �� ����Ʈ��
#define NUM_TOTAL_BLOCKS				(((ULONG)2*1024*16)) // �� ���ͼ�

// HW_DEVICE_EXTENSION ����ü ���� 
typedef struct _HW_DEVICE_EXTENSION
{
    PSCSI_REQUEST_BLOCK CurrentSrb;   
	ULONGLONG			DiskLength;
	ULONG				TotalBlocks;
	PUCHAR				Base;
	UCHAR				DataBuff[512];
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

// ���� ����� ���ɼ��� ���� ����ü(Stor������ ����̹������� BuildIo�� StartIo���� Ư�����̷� ����)
typedef struct _SRB_EXTENSION
{
	PUCHAR				Reserved;
} SRB_EXTENSION, *PSRB_EXTENSION;

#pragma pack()
int
HJSTOR_HwFindAdapter(
	IN PVOID  pContext,
	IN PVOID  HwContext,
	IN PVOID  BusInformation,
	IN PVOID  LowerDevice,
	IN PCHAR  ArgumentString,
	IN OUT PPORT_CONFIGURATION_INFORMATION  pConfigInfo,
	OUT PBOOLEAN  Again
);

BOOLEAN
HJSTOR_HwInitialize(
	PVOID pContext
);

BOOLEAN HJSTOR_HwStartIo(
	IN PVOID HwDeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	);

BOOLEAN HJSTOR_HwInterrupt(
	IN PVOID HwDeviceExtension
	);

BOOLEAN HJSTOR_HwResetBus(
	IN PVOID HwDeviceExtension,
	IN ULONG PathId
	);

SCSI_ADAPTER_CONTROL_STATUS HJSTOR_HwAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    );

BOOLEAN
HJSTOR_Hw_BuildIo(
	PVOID				pContext,
	PSCSI_REQUEST_BLOCK	pSrb
	);


#endif // _HJSTOR_H_
