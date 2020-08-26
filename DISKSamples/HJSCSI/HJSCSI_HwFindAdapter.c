#include "HJSCSI.h"

/***********************************************************************************
�Լ��� : HJSCSI_HwFindAdapter
��  �� :
			IN PVOID HwDeviceExtension,
			IN PVOID Context,
			IN PVOID BusInformation,
			IN PCHAR ArgumentString,
			IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
			OUT PBOOLEAN Again
���ϰ� : ULONG
��  �� : �о���� �ϵ���� ���������� ��� ����͸� �����մϴ�.
***********************************************************************************/
ULONG HJSCSI_HwFindAdapter(
    IN		PVOID							HwDeviceExtension,
    IN		PVOID							Context,
    IN		PVOID							BusInformation,
    IN		PCHAR							ArgumentString,
    IN OUT	PPORT_CONFIGURATION_INFORMATION	ConfigInfo,
    OUT		PBOOLEAN						Again
    )
{	
 	PHW_DEVICE_EXTENSION	DeviceExtension = HwDeviceExtension;

	UNREFERENCED_PARAMETER(ArgumentString);
	UNREFERENCED_PARAMETER(BusInformation);
	UNREFERENCED_PARAMETER(Context);


#ifdef DBGOUT
	DbgPrint("HJSCSI_HwFindAdapter\n");
#endif

	*Again = FALSE;						

/*
    accessRange = &((*(ConfigInfo->AccessRanges))[1]);
    if ( ScsiPortValidateRange(HwDeviceExtension,
        ConfigInfo->AdapterInterfaceType,
        ConfigInfo->SystemIoBusNumber,
        accessRange->RangeStart,
        accessRange->RangeLength,
        FALSE) ){
        pciAddress = (ULONG_PTR) ScsiPortGetDeviceBase(HwDeviceExtension,
            ConfigInfo->AdapterInterfaceType,
            ConfigInfo->SystemIoBusNumber,
            accessRange->RangeStart,
            accessRange->RangeLength,
            FALSE);

		DeviceExtension->Base1 = pciAddress;
    }
	���� �ϵ��� ����ϴ� ��쿡 �ʿ��ϴ�
*/
	DeviceExtension->Base = ExAllocatePool( NonPagedPool, DEFAULT_DISK_SIZE );
	DeviceExtension->DiskLength		= DEFAULT_DISK_SIZE;			
	DeviceExtension->TotalBlocks	= NUM_TOTAL_BLOCKS;
	// ����ũ�� ���ؼ� �ʿ��� ����޸𸮸� �غ��Ѵ�

	ConfigInfo->AdapterInterfaceType				= PCIBus;
	ConfigInfo->InterruptMode						= LevelSensitive; 
	ConfigInfo->MaximumTransferLength				= MAX_DMA_TRANSFER_SIZE;
	ConfigInfo->NumberOfPhysicalBreaks				= SCSI_MAXIMUM_PHYSICAL_BREAKS;
	ConfigInfo->DmaChannel							= SP_UNINITIALIZED_VALUE;
	ConfigInfo->DmaPort								= SP_UNINITIALIZED_VALUE;
	ConfigInfo->DmaWidth							= MaximumDmaWidth; //Width32Bits;
	ConfigInfo->DmaSpeed							= MaximumDmaSpeed; //Compatible;
	ConfigInfo->AlignmentMask						= 3;	// DWORD Alignment	// scatter & gather option
	ConfigInfo->NumberOfBuses						= 1;
	ConfigInfo->ScatterGather						= FALSE;
	ConfigInfo->Master								= TRUE;						// SCSI Host Controller => Master / SCSI Device => Slave
	ConfigInfo->CachesData							= FALSE;
	ConfigInfo->AdapterScansDown					= FALSE;					// scan device number up(++)/down(--) direction
	ConfigInfo->Dma32BitAddresses					= TRUE;
	ConfigInfo->MaximumNumberOfTargets				= 1;
	ConfigInfo->MaximumNumberOfLogicalUnits			= 1;
	DeviceExtension->CurrentSrb			= NULL;

    return SP_RETURN_FOUND;
}

