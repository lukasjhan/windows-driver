#include "HJSCSI.h"

/***********************************************************************************
�Լ��� : HJSCSI_HwInterrupt
��  �� : 
			HwDeviceExtension
���ϰ� : BOOLEAN
��  �� : Interrupt routine
***********************************************************************************/

BOOLEAN HJSCSI_HwInterrupt(IN PVOID HwDeviceExtension)
{
	UNREFERENCED_PARAMETER(HwDeviceExtension);

	return FALSE;
/*
	���� �ϵ��� �ִٸ� ���ͷ�Ʈ�� �߻���Ų �ϵ��� �����ؾ� �Ѵ�
	Srb��ɾ �����ؾ� �Ѵ�
	DeviceExtension->CurrentSrb->SrbStatus = (UCHAR)SRB_STATUS_SUCCESS;    
	ScsiPortNotification(RequestComplete,   DeviceExtension, DeviceExtension->CurrentSrb);
    ScsiPortNotification(NextRequest,  DeviceExtension, NULL);
	DeviceExtension->CurrentSrb = NULL;		
*/
}

