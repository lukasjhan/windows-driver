#include "HJSCSI.h"

/***********************************************************************************
�Լ��� : HJSCSI_ResetController
��  �� :
			IN PVOID HwDeviceExtension,
			IN ULONG PathId
���ϰ� : BOOLEAN
��  �� : Reset
***********************************************************************************/

BOOLEAN HJSCSI_HwResetBus(IN PVOID HwDeviceExtension,IN ULONG PathId)
{
	UNREFERENCED_PARAMETER(HwDeviceExtension);
	UNREFERENCED_PARAMETER(PathId);

    return TRUE;
}
