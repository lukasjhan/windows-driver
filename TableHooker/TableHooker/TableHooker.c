#include "tablehooker.h"

// --------------- ������������ �����Ѵ� ----------------------
PDRIVER_OBJECT		gpTargetDriverObject = NULL;
PDRIVER_DISPATCH	gfnOrgDriverDispatch[IRP_MJ_MAXIMUM_FUNCTION + 1] = { 0, };
PDRIVER_OBJECT		gMainDriverObject = NULL;
KSPIN_LOCK			gLogLock; // �α��۾��� ���Ǵ� ���ɶ��� �����Ѵ�
int					gLogCount = 0; // ���� ���Ǿ��ִ� �α׿�Ʈ���� ��
int					gSequenceNumber = 0; // �α��� ������ ��� ����
LIST_ENTRY			gLogList; // �α׸� ����ϴ� ����Ʈ���

// ---------------- ���� �Լ��� ------------------------
// �α׸� ��� ����� ���� �Լ�
VOID
internal_EmptyLog(
	VOID
)
{
	PRECORD_LIST pRecordList;
	KIRQL oldIrql;

	KeAcquireSpinLock(&gLogLock, &oldIrql);

	while (!IsListEmpty(&gLogList)) {

		pRecordList = (PRECORD_LIST)RemoveHeadList(&gLogList);
		KeReleaseSpinLock(&gLogLock, oldIrql);

		// ....

		ExFreePool(pRecordList);

		KeAcquireSpinLock(&gLogLock, &oldIrql);
	}

	gLogCount = 0;
	gSequenceNumber = 0;

	KeReleaseSpinLock(&gLogLock, oldIrql);
}

// ---------------- Utility �Լ��� ------------------------

PDRIVER_OBJECT SearchDriverObject(PUNICODE_STRING pUni)
{
	NTSTATUS st;
	PDRIVER_OBJECT Object;
	POBJECT_TYPE TypeObjectType = ObGetObjectType(gMainDriverObject);

	st = ObReferenceObjectByName(pUni, OBJ_CASE_INSENSITIVE, NULL, 0, TypeObjectType, KernelMode, NULL, &Object);

	if (st != STATUS_SUCCESS)
		return (PDRIVER_OBJECT)0;

	ObDereferenceObject(Object);
	return Object;
}

// �α��׸��� �ϳ� �����ϴ� �Լ�
BOOLEAN
InsertLog(
	PIRP	Irp,
	BOOLEAN bIsStatus)
{
	PRECORD_LIST pRecordList = NULL;
	KIRQL	oldIrql;
	ULONG	Length = 0;
	BOOLEAN	bRet = FALSE;
	PIO_STACK_LOCATION	pStack = NULL;
	void *	pTargetBuffer = NULL;
	
	if (gLogCount >= MAX_LOG_COUNT)
		goto exit;

	pStack = IoGetCurrentIrpStackLocation(Irp);

	Length = sizeof(RECORD_LIST);

	pRecordList = (PRECORD_LIST)ExAllocatePool(NonPagedPool, Length);
	if (pRecordList == NULL)
		goto exit;

	memset(pRecordList, 0, Length);

	InitializeListHead(&pRecordList->List);

	pRecordList->ListData.Length = Length - sizeof(LIST_ENTRY);
	pRecordList->ListData.bIsStatus = bIsStatus;

	pRecordList->ListData.MajorFunction = pStack->MajorFunction;

	switch (pStack->MajorFunction)
	{
	case IRP_MJ_READ:
		if (bIsStatus == FALSE)
		{
			pRecordList->ListData.extra.ReadRequest.Length = pStack->Parameters.Read.Length;
			pRecordList->ListData.extra.ReadRequest.Offset = pStack->Parameters.Read.ByteOffset.QuadPart;
		}
		else
		{
			pRecordList->ListData.extra.ReadStatus.ntStatus = Irp->IoStatus.Status;
			pRecordList->ListData.extra.ReadStatus.Information = Irp->IoStatus.Information;
			if (NT_SUCCESS(Irp->IoStatus.Status))
			{
				pRecordList->ListData.extra.ReadStatus.DataLength = (ULONG)Irp->IoStatus.Information;
				if (pRecordList->ListData.extra.ReadStatus.DataLength)
				{
					if (Irp->AssociatedIrp.SystemBuffer)
					{
						memcpy(pRecordList->ListData.extra.ReadStatus.DataBuffer, 
							Irp->AssociatedIrp.SystemBuffer, 
							pRecordList->ListData.extra.ReadStatus.DataLength);
					}
					else if (Irp->MdlAddress)
					{
						pTargetBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, HighPagePriority);
						if (pTargetBuffer)
						{
							memcpy(pRecordList->ListData.extra.ReadStatus.DataBuffer,
								pTargetBuffer,
								pRecordList->ListData.extra.ReadStatus.DataLength);
						}
					}
				}
			}
			pRecordList->ListData.extra.ReadStatus.ntStatus = Irp->IoStatus.Status;
		}
		break;
	}

	pRecordList->ListData.SequenceNumber = gSequenceNumber;
	gSequenceNumber++;

	KeAcquireSpinLock(&gLogLock, &oldIrql);

	InsertTailList(&gLogList, &pRecordList->List);
	gLogCount++;

	KeReleaseSpinLock(&gLogLock, oldIrql);

	bRet = TRUE;

exit:
	return bRet;
}

// �α��׸��� �ϳ� �������鼭, ����� �Լ�
ULONG
GetAndRemoveLog(
	void * OutputBuffer,
	ULONG  OutputBufferLength)
{
	PRECORD_LIST pRecordList = NULL;
	KIRQL	oldIrql;
	ULONG	Length = 0;

	if (OutputBuffer == NULL)
		goto exit;

	if (OutputBufferLength == 0)
		goto exit;

	KeAcquireSpinLock(&gLogLock, &oldIrql);

	if (gLogCount == 0)
	{
		KeReleaseSpinLock(&gLogLock, oldIrql);
		goto exit;
	}

	pRecordList = (PRECORD_LIST)RemoveHeadList(&gLogList);
	gLogCount--;

	KeReleaseSpinLock(&gLogLock, oldIrql);

	Length = pRecordList->ListData.Length;
	Length = (OutputBufferLength > Length) ? Length : OutputBufferLength;

	memcpy(OutputBuffer, &pRecordList->ListData, Length);

	ExFreePool(pRecordList);

exit:
	return Length;
}

// ��� �α׸� �����ϴ� �Լ�
void
SpyClearLog()
{
	internal_EmptyLog();
}

/*
doPreparePostProcessingIrp() �Լ��� ��ó���۾��� ���ؼ� �Ϸ��ƾ(CompletionRoutine)�� �����ϴ� �۾��� �����ϴ� �Լ��̴�
Hooker �Լ��� ȣ��Ǵ� �ñ�� �����ϰ� ���ؼ�, ����ä����ؿ��� ȣ��Ǿ��� ������, Hooker�� ����Ҹ��� �������� IO_STACK_LOCATION �� ����
�̰��� �������� CompletionRoutine�����۾��� �����ȴٴ� �ǹ��̱⵵ �ϱ� ������, Ʈ��(Trick)�� ����ؾ� �Ѵ�
*/
typedef struct _COMPLETION_CONTEXT
{
	IO_STACK_LOCATION	Stack;					// ���� Stack
}COMPLETION_CONTEXT, *PCOMPLETION_CONTEXT;

NTSTATUS HookerCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID pContext)
{
	PCOMPLETION_CONTEXT	pNewCompletionContext = NULL;
	IO_STACK_LOCATION	TempStack;

	UCHAR OrgControl = 0;
	PIO_COMPLETION_ROUTINE OrgCompletionRoutine = NULL;
	PVOID OrgContext = NULL;

	BOOLEAN bMustCallOrgCompletionRoutine = FALSE;
	PIO_STACK_LOCATION	pLowerStackLocation = NULL;

	pNewCompletionContext = (PCOMPLETION_CONTEXT)pContext;
	if (pNewCompletionContext == NULL)
	{
		goto exit;
	}

	memcpy(&TempStack, &pNewCompletionContext->Stack, sizeof(IO_STACK_LOCATION));
	OrgControl = TempStack.Control;
	OrgCompletionRoutine = TempStack.CompletionRoutine;
	OrgContext = TempStack.Context;

	// ��ó�� �α׸� �����
	// �̶�, ���������� ���� StackLocation ��ġ�� Hooker�� �� ������ ������� ���̴�
	pLowerStackLocation = IoGetNextIrpStackLocation(Irp);
	memcpy(pLowerStackLocation, &TempStack, sizeof(IO_STACK_LOCATION));
	IoSetNextIrpStackLocation(Irp);
	InsertLog(Irp, TRUE);
	IoSkipCurrentIrpStackLocation(Irp);
	//

exit:
	if (pNewCompletionContext)
	{
		ExFreePool(pNewCompletionContext);
	}

	// Hooker ������ ȣ������ Completion Routine �� �����Ǿ������鼭, ȣ��� ������ �´����� Ȯ���Ѵ�
	if ((OrgControl & SL_INVOKE_ON_SUCCESS) && (Irp->IoStatus.Status == STATUS_SUCCESS))
	{
		bMustCallOrgCompletionRoutine = TRUE;
	}
	else if ((OrgControl & SL_INVOKE_ON_CANCEL) && (Irp->IoStatus.Status == STATUS_CANCELLED))
	{
		bMustCallOrgCompletionRoutine = TRUE;
	}
	else if ((OrgControl & SL_INVOKE_ON_ERROR) && (!NT_SUCCESS(Irp->IoStatus.Status)))
	{
		bMustCallOrgCompletionRoutine = TRUE;
	}

	if ((bMustCallOrgCompletionRoutine == TRUE) && OrgCompletionRoutine)
	{
		return OrgCompletionRoutine(DeviceObject, Irp, OrgContext);
	}

	if (Irp->PendingReturned)
		IoMarkIrpPending(Irp);

	return Irp->IoStatus.Status;
}

void doPreparePostProcessingIrp(PIRP Irp)
{
	PCOMPLETION_CONTEXT	pNewCompletionContext = NULL;
	PIO_STACK_LOCATION	pLowerStackLocation = NULL;

	pNewCompletionContext = (PCOMPLETION_CONTEXT)ExAllocatePool(NonPagedPool, sizeof(COMPLETION_CONTEXT));
	if (pNewCompletionContext == NULL)
		goto exit;

	memset(pNewCompletionContext, 0, sizeof(COMPLETION_CONTEXT));

	// ������� Irp->Tail.Overlay.CurrentIrpStackLocation �� Hooker�� �Ʒ������� �����̴�
	pLowerStackLocation = IoGetCurrentIrpStackLocation(Irp);

	// ���� Hooker�� �Ʒ������� ���ƾ��ϴ� ������ ���������� ����Ѵ�
	memcpy(&pNewCompletionContext->Stack, pLowerStackLocation, sizeof(IO_STACK_LOCATION));

	// ���ο� Completion Routine�� �����Ѵ�
	pLowerStackLocation->CompletionRoutine = HookerCompletionRoutine;
	pLowerStackLocation->Context = pNewCompletionContext;
	pLowerStackLocation->Control = (SL_INVOKE_ON_SUCCESS| SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL);

exit:
	return;
}

////////////////////////////////////////////////////
// ����ä���Լ�

// �߻��Ǵ� ������ �α׷� ����� �۾��� �����Ѵ�
NTSTATUS NewHookerDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS			ntStatus = STATUS_NOT_SUPPORTED;
	PIO_STACK_LOCATION	pStack = NULL;
	PDRIVER_DISPATCH	pOrgDispatchFunction = NULL;
	BOOLEAN				bInserted = FALSE;
	
	pStack = IoGetCurrentIrpStackLocation(Irp); // IRP�� �Ķ���͸� Ȯ���Ѵ�

	pOrgDispatchFunction = gfnOrgDriverDispatch[pStack->MajorFunction];

	if (pOrgDispatchFunction == NULL)
	{
		ntStatus = STATUS_NOT_SUPPORTED;
		goto exit;
	}

	switch (pStack->MajorFunction)
	{
	case IRP_MJ_READ:
		bInserted = InsertLog(Irp, FALSE);
		break;
	}

	if (bInserted == TRUE)
	{
		// Completion Routine�� �����ؼ�, ��ó�� �α׵� �����ؾ� �Ѵ�
		doPreparePostProcessingIrp(Irp);
	}

	goto bypass;

bypass:
	ntStatus = pOrgDispatchFunction(DeviceObject, Irp);
	return ntStatus;

exit:
	Irp->IoStatus.Status = ntStatus;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return ntStatus;
}

void HookCallback(unsigned char MajorFunction)
{
	if (gpTargetDriverObject == NULL)
		goto exit;

	if(gfnOrgDriverDispatch[MajorFunction]) // �̹� ����ä���ּҰ� ������ ���¶��,
		goto exit;

	gfnOrgDriverDispatch[MajorFunction] = gpTargetDriverObject->MajorFunction[MajorFunction];
	gpTargetDriverObject->MajorFunction[MajorFunction] = NewHookerDispatch;

exit:
	return;
}

void UnhookCallback(unsigned char MajorFunction)
{
	if (gpTargetDriverObject == NULL)
		goto exit;

	if (gfnOrgDriverDispatch[MajorFunction] == NULL) // ����ä���ּҰ� �������� ���� ���¶��,
		goto exit;

	gpTargetDriverObject->MajorFunction[MajorFunction] = gfnOrgDriverDispatch[MajorFunction];
	gfnOrgDriverDispatch[MajorFunction] = NULL;

exit:
	return;
}

// --------------- DriverEntry ----------------------

NTSTATUS
DriverEntry(
	PDRIVER_OBJECT pDriverObject,
	PUNICODE_STRING pRegistryPath
)
{
	UNICODE_STRING		sddlString;
	UNICODE_STRING		DeviceName;
	UNICODE_STRING		Win32Name;
	PDEVICE_OBJECT		pCustomDeviceObject = NULL;
	UNICODE_STRING		TargetDriverUniName;
	NTSTATUS			ntStatus = STATUS_UNSUCCESSFUL;
	TABLEHOOKER_DEVICE_EXTENSION	* pDeviceExtension = NULL;

	UNREFERENCED_PARAMETER(pRegistryPath);

	gMainDriverObject = pDriverObject;

	// ����ä���۾��� �����Ѵ�
	RtlInitUnicodeString(&TargetDriverUniName, TARGET_DRIVER_NAME);

	gpTargetDriverObject = SearchDriverObject(&TargetDriverUniName);
	if (gpTargetDriverObject == NULL)
	{
		goto exit;// ����̸��� �߰ߵ��� ������ �׳� �����Ѵ�
	}

	RtlInitUnicodeString(&DeviceName, CUSTOM_DEVICE_NAME);
	RtlInitUnicodeString(&Win32Name, CUSTOM_WIN32_NAME);
	RtlInitUnicodeString(&sddlString, L"D:P(A;;GA;;;BA)(A;;GA;;;SY)");

	// Custom ����̹��� ���� Custom Device Object�� �����Ѵ�
	ntStatus = IoCreateDeviceSecure(pDriverObject, sizeof(TABLEHOOKER_DEVICE_EXTENSION),
		&DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, 
		FALSE, &sddlString, 0, &pCustomDeviceObject);

	if (!NT_SUCCESS(ntStatus))
	{
		goto exit; // DEVICE_OBJECT�� ������ �� ���ٸ�, �����Ѵ�
	}

	// �������� ���α׷��� ����ϴ� ä���� ���� �ɺ�����ũ�� �����Ѵ�
	ntStatus = IoCreateSymbolicLink(&Win32Name, &DeviceName);
	if (!NT_SUCCESS(ntStatus))
	{ // �ɺ�����ũ�� ������ �� ���ٸ�, �����Ѵ�
		IoDeleteDevice(pCustomDeviceObject);
		goto exit;
	}
									
	pDeviceExtension = (TABLEHOOKER_DEVICE_EXTENSION *)pCustomDeviceObject->DeviceExtension;
	memset(pDeviceExtension, 0, sizeof(TABLEHOOKER_DEVICE_EXTENSION));

	pDeviceExtension->CustomSignature = 'HAJE'; // �ñ״��ĸ� ����Ѵ�
	/*
	������ TableHooker ����̹��� �����ϴ� �۾��� Ư������̹��� ������ �����ϴ� �����̹Ƿ�, �ñ״��İ� �� �ʿ��Ѱ��� �ƴϴ�.
	�ñ״��Ĵ� ���� ����̹��� �̴���Ʈ����̹����� ����ϴ� ��츦 ���ؼ� �����صд�
	*/

	// Custom ����̹��� ���� �ݹ��Լ��� ����Ѵ�
	pDriverObject->DriverUnload = TABLEHOOKER_Unload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = TABLEHOOKER_Create; // for Win32 API CreateFile()
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = TABLEHOOKER_Close; // for Win32 API CloseHandle()
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = TABLEHOOKER_Cleanup; // for Win32 API CloseHandle()
	// Win32 API CloseHandle() -> IRP_MJ_CLEANUP -> IRP_MJ_CLOSE

	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = TABLEHOOKER_DeviceIoControl; // for Win32 API DeviceIoControl()

	KeInitializeSpinLock(&gLogLock);
	InitializeListHead(&gLogList);

	pCustomDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING; // �������α׷����� ���� ������ ����Ѵ�

	ntStatus = STATUS_SUCCESS;

exit:
	return ntStatus;
}

// --------------- TABLEHOOKER_Unload ----------------------

void
TABLEHOOKER_Unload(
	PDRIVER_OBJECT pDriverObject
)
{
	UNICODE_STRING		Win32Name;

	UNREFERENCED_PARAMETER(pDriverObject);

	// �����Ǿ��ִ� ��� �α׸� �����
	SpyClearLog();

	// �ɺ�����ũ�� �����Ѵ�
	RtlInitUnicodeString(&Win32Name, CUSTOM_WIN32_NAME);
	IoDeleteSymbolicLink(&Win32Name);

	// Custom Device Object�� �����Ѵ�
	IoDeleteDevice(pDriverObject->DeviceObject); 
	// Custom Driver�� ������ DEVICE_OBJECT�� ���� Custom Device Object �ۿ� �����Ƿ�,

	return;
}

// --------------- TABLEHOOKER_Create ----------------------

NTSTATUS
TABLEHOOKER_Create(
	IN PDEVICE_OBJECT pFDO,
	IN PIRP pIrp
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(pFDO);

	//////////////////////////////////////////////////////////////
	// ������ ��� ����̹��� �ֿ� �ݹ��Լ��� ����ä���Ѵ�
	HookCallback(IRP_MJ_READ);

	goto exit;

exit:
	pIrp->IoStatus.Status = ntStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return ntStatus;
}

// --------------- TABLEHOOKER_Cleanup ----------------------

NTSTATUS
TABLEHOOKER_Cleanup(
	IN PDEVICE_OBJECT pFDO,
	IN PIRP pIrp
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(pFDO);

	goto exit;

exit:
	pIrp->IoStatus.Status = ntStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return ntStatus;
}

// --------------- TABLEHOOKER_Close ----------------------

NTSTATUS
TABLEHOOKER_Close(
	IN PDEVICE_OBJECT pFDO,
	IN PIRP pIrp
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(pFDO);

	// �� �ñ⿡ ����ä�⸦ �����ϴ� ���� ����
	UnhookCallback(IRP_MJ_READ);

	goto exit;

exit:
	pIrp->IoStatus.Status = ntStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return ntStatus;
}

// --------------- TABLEHOOKER_DeviceIoControl ----------------------

NTSTATUS
TABLEHOOKER_DeviceIoControl(
	IN PDEVICE_OBJECT pFDO,
	IN PIRP pIrp
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PIO_STACK_LOCATION pStack = NULL;
	ULONG IoctlCode = 0;
	int OutputBufferLength = 0;
	int InputBufferLength = 0;
	ULONG_PTR Information = 0;

	UNREFERENCED_PARAMETER(pFDO);

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	if (pStack == NULL)
	{
		ntStatus = STATUS_UNSUCCESSFUL;
		goto exit;
	}

	OutputBufferLength = pStack->Parameters.DeviceIoControl.OutputBufferLength;
	InputBufferLength = pStack->Parameters.DeviceIoControl.InputBufferLength;

	IoctlCode = pStack->Parameters.DeviceIoControl.IoControlCode;

	switch (IoctlCode)
	{
		case IOCTL_TABLEHOOKER_GET_LOG:
		{
			if (OutputBufferLength < sizeof(RECORD_LIST_APP))
			{
				ntStatus = STATUS_INVALID_PARAMETER;
				break;
			}
			if (pIrp->AssociatedIrp.SystemBuffer == NULL)
			{
				ntStatus = STATUS_INVALID_PARAMETER;
				break;
			}
			Information = (ULONG_PTR)GetAndRemoveLog(pIrp->AssociatedIrp.SystemBuffer, OutputBufferLength);
			ntStatus = STATUS_SUCCESS;
			break;
		}
		case IOCTL_TABLEHOOKER_CLR_LOG:
		{
			SpyClearLog();
			Information = 0;
			ntStatus = STATUS_SUCCESS;
			break;
		}
		default:
		{
			ntStatus = STATUS_UNSUCCESSFUL;
			break;
		}
	}

exit:
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = Information;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return ntStatus;
}
