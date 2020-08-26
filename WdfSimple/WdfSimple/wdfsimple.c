#include "wdfsimple.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, SIMPLEEvtDeviceAdd)
#pragma alloc_text (PAGE, SIMPLEEvtIoRead)
#pragma alloc_text (PAGE, SIMPLEEvtIoWrite)
#pragma alloc_text (PAGE, SIMPLEEvtIoDeviceControl)
#endif

// ����̹��� �޸𸮿� �����Ҷ� ȣ��˴ϴ�
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG   config;

    WDF_DRIVER_CONFIG_INIT(
        &config,
        SIMPLEEvtDeviceAdd
        );

// WDFDEVICE ������Ʈ�� �����մϴ�
    status = WdfDriverCreate(
        DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES, // Driver Attributes
        &config,          // Driver Config Info
        WDF_NO_HANDLE
        );

    if (!NT_SUCCESS(status)) {
    }

    return status;
}

// ����̽������� �����Ҷ� ȣ��˴ϴ�
NTSTATUS
SIMPLEEvtDeviceAdd(
    IN WDFDRIVER       Driver,
    IN PWDFDEVICE_INIT DeviceInit
    )
{
    NTSTATUS               status = STATUS_SUCCESS;
    PFDO_DATA              fdoData;
    WDF_IO_QUEUE_CONFIG    queueConfig;
    WDF_OBJECT_ATTRIBUTES  fdoAttributes;
    WDFDEVICE              hDevice;
    WDFQUEUE               queue;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&fdoAttributes, FDO_DATA);

// WDFDEVICE ������Ʈ�� �����մϴ�
    status = WdfDeviceCreate(&DeviceInit, &fdoAttributes, &hDevice);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    fdoData = SIMPLEFdoGetData(hDevice);

// �������α׷��� ����� �� �ֵ��� �մϴ�. CreateFile() API
    status = WdfDeviceCreateDeviceInterface(
                 hDevice,
                 (LPGUID) &GUID_SIMPLE,
                 NULL 
             );

    if (!NT_SUCCESS (status)) {
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig,  WdfIoQueueDispatchParallel);

/*
	ReadFile() API, WriteFile API, DeviceIoControl() API �Լ��� ����� �̺�Ʈ�Լ��� ����մϴ�
*/
    queueConfig.EvtIoRead = SIMPLEEvtIoRead;
    queueConfig.EvtIoWrite = SIMPLEEvtIoWrite;
    queueConfig.EvtIoDeviceControl = SIMPLEEvtIoDeviceControl;

// WDFQUEUE ������Ʈ�� �����մϴ�
    status = WdfIoQueueCreate(
        hDevice,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &queue
        );

    if (!NT_SUCCESS (status)) {

        return status;
    }

    return status;
}

// ReadFile() API�Լ��� ����˴ϴ�
VOID
SIMPLEEvtIoRead (
    WDFQUEUE      Queue,
    WDFREQUEST    Request,
    size_t        Length
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG_PTR bytesCopied =0;
    WDFMEMORY memory;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Length);

    PAGED_CODE();

    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if(NT_SUCCESS(status) ) {
    }

    WdfRequestCompleteWithInformation(Request, status, bytesCopied);
}

// WriteFile() API�Լ��� ����˴ϴ�
VOID
SIMPLEEvtIoWrite (
    WDFQUEUE      Queue,
    WDFREQUEST    Request,
    size_t        Length
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG_PTR bytesWritten =0;
    WDFMEMORY memory;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Length);

    PAGED_CODE();

    status = WdfRequestRetrieveInputMemory(Request, &memory);
    if(NT_SUCCESS(status) ) {
        bytesWritten = Length;
    }

    WdfRequestCompleteWithInformation(Request, status, bytesWritten);
}


// DeviceIoControl() API�Լ��� ����˴ϴ�
VOID
SIMPLEEvtIoDeviceControl(
    IN WDFQUEUE     Queue,
    IN WDFREQUEST   Request,
    IN size_t       OutputBufferLength,
    IN size_t       InputBufferLength,
    IN ULONG        IoControlCode
    )
{
    NTSTATUS  status= STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    PAGED_CODE();

    switch (IoControlCode) {
		case IOCTL_SIMPLE_CONTROL: // �������α׷��� ����� IOControlCode�� Ȯ���մϴ�
			break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    WdfRequestCompleteWithInformation(Request, status, (ULONG_PTR) 0);
}


