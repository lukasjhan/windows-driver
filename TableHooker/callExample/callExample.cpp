// callExample.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <winioctl.h>
#include ".\..\common\appinterface.h"

#define IRP_MJ_READ	(0x03)

void DataDump(void * pDataBuffer_T, ULONG Size)
{
	unsigned char * pDataBuffer = NULL;
	ULONG Count = 0;

	pDataBuffer = (unsigned char *)pDataBuffer_T;

	while (Count < Size)
	{
		if (Count && (Count % 4 == 0))
			printf("\n");

		printf("(0x%2X) ", pDataBuffer[Count]);
		Count++;
	}
	printf("\n");
}

int main()
{
	HANDLE hDevice;
	ULONG dwRet;
	PRECORD_LIST_APP	pLogData = NULL;
	BOOLEAN bRet = FALSE;

	hDevice = CreateFile(CREATE_FILE_NAME, 
		FILE_READ_DATA | FILE_WRITE_DATA,
		NULL, NULL, OPEN_EXISTING, 0, NULL);

	if (hDevice == (HANDLE)-1)
		return 0;

	// ��������� �α׸� �����
	DeviceIoControl(hDevice, IOCTL_TABLEHOOKER_CLR_LOG, NULL, 0, NULL, 0, &dwRet, NULL);

	pLogData = (PRECORD_LIST_APP)malloc(sizeof(RECORD_LIST_APP)); 

	// �α׸� �����´�
#define POLLINGTIME_MSEC	(100) // 100ms

	while (1)
	{
		/*
		PENDING�� �����ϴ� ���°� �������� ���簡 �ǰ�����, ���÷� �ۼ��Ǿ��� ������ �ֱ������� �����ϴ� ���·� �ۼ��Ǿ���
		�� �κ��� �����ڰ� ���� �����غ��°����� ���ܵΰڴ�
		*/
		Sleep(POLLINGTIME_MSEC);
		dwRet = 0;
		memset(pLogData, 0, sizeof(RECORD_LIST_APP));
		bRet = DeviceIoControl(hDevice, IOCTL_TABLEHOOKER_GET_LOG, NULL, 0, pLogData, sizeof(RECORD_LIST_APP), &dwRet, NULL);
		if (bRet == FALSE)
			break;

		if (dwRet == 0)
			continue;

		// ������ �α׸� ����Ѵ�
		switch (pLogData->MajorFunction)
		{
		case IRP_MJ_READ:
			if (pLogData->bIsStatus == FALSE)
			{
				printf("MajorFunction(IRP_MJ_READ), Request. Request Length(0x%X), Offset(0x%X)\n"
					, pLogData->extra.ReadRequest.Length, (ULONG)pLogData->extra.ReadRequest.Offset);
			}
			else
			{
				printf("MajorFunction(IRP_MJ_READ), Complete. Status(0x%X), Result Length(0x%X) \n"
					, pLogData->extra.ReadStatus.ntStatus, (ULONG)pLogData->extra.ReadStatus.Information);
				DataDump(pLogData->extra.ReadStatus.DataBuffer, (ULONG)pLogData->extra.ReadStatus.Information);
			}
			break;
		}
	}
	CloseHandle(hDevice);

    return 0;
}

