#ifndef _APPINTERFACE_H_
#define _APPINTERFACE_H_

#define CUSTOM_NAME	L"CustomDevice"
#define CREATE_FILE_NAME	L"\\\\.\\" CUSTOM_NAME

// �������α׷��� ����ϴµ� ���Ǵ� ��ɾ �����Ѵ�
#define IOCTL_TABLEHOOKER_GET_LOG		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TABLEHOOKER_CLR_LOG		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

// --------------- �α׸� �����ϱ� ���ؼ� �����Ѵ� -----------------
#define MAX_BUFFER_SIZE	(0x10000)	// 64KBytes�� �ִ밪���� ����Ѵ�
#define MAX_LOG_COUNT	1000		// �α׿�Ʈ���� �ִ���� �����Ѵ�

typedef struct _READ_REQUEST_DATA {
	ULONG_PTR		Offset;
	ULONG			Length;
} READ_REQUEST_DATA, *PREAD_REQUEST_DATA;

typedef struct _READ_STATUS_DATA {
	NTSTATUS		ntStatus;
	ULONG_PTR		Information;
	ULONG			DataLength;
	unsigned char	DataBuffer[MAX_BUFFER_SIZE];
} READ_STATUS_DATA, *PREAD_STATUS_DATA;

typedef struct _RECORD_LIST_APP {
	ULONG							Length;
	ULONG							SequenceNumber;
	unsigned char					MajorFunction;
	BOOLEAN							bIsStatus;
	union
	{
		READ_REQUEST_DATA			ReadRequest;
		READ_STATUS_DATA			ReadStatus;
	}extra;
} RECORD_LIST_APP, *PRECORD_LIST_APP;

#endif //_APPINTERFACE_H_