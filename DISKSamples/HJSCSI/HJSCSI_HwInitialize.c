
#include "HJSCSI.h"

/***********************************************************************************
�Լ��� : HJSCSI_HwInitialize
��  �� : 
			HwDeviceExtension
���ϰ� : BOOLEAN
��  �� : �ϵ��� �ʱ�ȭ�մϴ�. 
***********************************************************************************/

BOOLEAN HJSCSI_HwInitialize(IN PVOID HwDeviceExtension)
{
	UNREFERENCED_PARAMETER(HwDeviceExtension);

#ifdef DBGOUT
	DbgPrint("HJSCSI_HwInitialize\n");
#endif
	return TRUE;
}
