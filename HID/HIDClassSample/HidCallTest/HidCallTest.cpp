// HidCallTest.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include <Windows.h>
#include <initguid.h>
#include <SetupAPI.h>
extern "C"
{
#include <hidsdi.h>
}

#include <stdio.h>
#include <conio.h>

#pragma comment(lib,"SetupApi")
#pragma comment(lib,"hid")

#define MAXDEVICENUMBER 10 // ���� Interface GUID�� ����ϴ� ����̽��� �ִ� 10������ �˻��Ϸ��� �ǵ��� �����ϴ�
#define MYDEVICE	_T("HID\\VID_1234&PID_5678")

/*
GetDeviceStackNameCount()�Լ��� GetDeviceStackName()�Լ��� ���� ���Ƿ� �ۼ��� �����Լ��Դϴ�.
�̰������� � ������� InterfaceGUID�� ����Ͽ� ����̽������� �̸��� ���ϴ����� �� �� �ֽ��ϴ�
*/

// ���� �����Ǵ� GUID�� �����ϴ� DeviceStack�� ������ �˷��ݴϴ�
int GetDeviceStackNameCount(struct _GUID * pGuid)
{
	SP_DEVINFO_DATA dinfo;
	SP_INTERFACE_DEVICE_DATA interfaceData;
	int index = 0;
	int count = 0;
	HDEVINFO Info = SetupDiGetClassDevs(pGuid, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (Info == (HDEVINFO)-1)
		return 0; // �ý����� �̷� ����� �������� ���Ѵ�

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

	while (1)
	{
		BOOLEAN bl;
		DWORD ret;
		DWORD PropertyRegDataType;
		WCHAR PropertyData[1024];

		bl = SetupDiEnumDeviceInterfaces(Info, 0, pGuid, index, &interfaceData);
		if (bl == FALSE)
			break;

		dinfo.cbSize = sizeof(dinfo);
		bl = SetupDiEnumDeviceInfo(Info, index, &dinfo);
		if (bl == FALSE)
		{
			index++;
			continue;
		}
		bl = SetupDiGetDeviceRegistryProperty(Info, &dinfo, SPDRP_HARDWAREID, &PropertyRegDataType, (BYTE *)PropertyData, sizeof(WCHAR) * 1024, &ret);
		if (bl == FALSE)
		{
			index++;
			continue;
		}
		if (wcsncmp(PropertyData, MYDEVICE, wcslen(MYDEVICE)) == 0)
		{
			count++;
		}
		index++;
	}

	SetupDiDestroyDeviceInfoList(Info);

	return count;
}

// ���� �����Ǵ� GUID�� �����ϴ� DeviceStack�� �̸����� �˷��ݴϴ�
BOOLEAN GetDeviceStackName(struct _GUID * pGuid, WCHAR ** ppDeviceName, WCHAR ** ppHardwareId, int deviceindex)
// index : ���� Interface GUID�� ���� ��ġ�� �������ΰ��, ���� ���� �˻��Ǵ� ��ġ�� �����Ϸ��� �̰��� 0�� ����մϴ�
{
	SP_DEVINFO_DATA dinfo;
	DWORD size;
	SP_INTERFACE_DEVICE_DATA interfaceData;
	PSP_INTERFACE_DEVICE_DETAIL_DATA pData;
	HDEVINFO Info = SetupDiGetClassDevs(pGuid, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	WCHAR *pDeviceName;
	WCHAR *pHardwareId;
	int index = 0;
	int count = 0;

	*ppDeviceName = (WCHAR *)0;

	if (Info == (HANDLE)-1)
		return FALSE;

	interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

	while (1)
	{
		BOOLEAN bl;
		DWORD ret;
		DWORD PropertyRegDataType;
		WCHAR PropertyData[1024];

		bl = SetupDiEnumDeviceInterfaces(Info, 0, pGuid, index, &interfaceData);
		if (bl == FALSE)
			break;

		dinfo.cbSize = sizeof(dinfo);
		bl = SetupDiEnumDeviceInfo(Info, index, &dinfo);
		if (bl == FALSE)
		{
			index++;
			continue;
		}
		bl = SetupDiGetDeviceRegistryProperty(Info, &dinfo, SPDRP_HARDWAREID, &PropertyRegDataType, (BYTE *)PropertyData, sizeof(WCHAR) * 1024, &ret);
		if (bl == FALSE)
		{
			index++;
			continue;
		}
		if (wcsncmp(PropertyData, MYDEVICE, wcslen(MYDEVICE)) == 0)
		{
			if (deviceindex == count)
			{
				SetupDiGetDeviceInterfaceDetail(Info, &interfaceData, 0, 0, &size, 0);
				pData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
				pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
				SetupDiGetDeviceInterfaceDetail(Info, &interfaceData, pData, size, 0, 0);

				pDeviceName = (WCHAR *)malloc((wcslen(pData->DevicePath) + 1) * sizeof(WCHAR));
				memset(pDeviceName, 0, (wcslen(pData->DevicePath) + 1) * sizeof(WCHAR));
				wcscpy(pDeviceName, pData->DevicePath);
				pHardwareId = (WCHAR *)malloc((wcslen(PropertyData) + 1) * sizeof(WCHAR));
				memset(pHardwareId, 0, (wcslen(PropertyData) + 1) * sizeof(WCHAR));
				wcscpy(pHardwareId, PropertyData);

				free(pData);

				SetupDiDestroyDeviceInfoList(Info);
				*ppDeviceName = pDeviceName;
				*ppHardwareId = pHardwareId;
				return TRUE;
			}

			count++;
		}
		index++;
	}

	SetupDiDestroyDeviceInfoList(Info);
	return FALSE;
}

void dumpDeviceInfo(int index, WCHAR *pHardwareId, WCHAR *pDeviceName, HIDP_CAPS * pCaps, PHIDP_PREPARSED_DATA	Ppd, HANDLE handle)
{
	USHORT					numValues;
	USHORT                  numCaps;
	PHIDP_BUTTON_CAPS       buttonCaps;
	PHIDP_VALUE_CAPS        valueCaps;

	buttonCaps = (PHIDP_BUTTON_CAPS)malloc(sizeof(HIDP_BUTTON_CAPS));
	valueCaps = (PHIDP_VALUE_CAPS)malloc(sizeof(HIDP_VALUE_CAPS));

	wprintf(_T("Device Index = %d"), index);
	wprintf(_T("HardwareId : %ws\n"), pHardwareId);
	wprintf(_T("DeviceName : %ws\n"), pDeviceName);

	wprintf(_T("Usage				: 0x%4x\n"), pCaps->Usage);
	wprintf(_T("UsagePage			: 0x%4x\n"), pCaps->UsagePage);
	if (pCaps->InputReportByteLength)
		wprintf(_T("InputReportByteLength		: 0x%4x\n"), pCaps->InputReportByteLength);
	if (pCaps->OutputReportByteLength)
		wprintf(_T("OutputReportByteLength		: 0x%4x\n"), pCaps->OutputReportByteLength);
	if (pCaps->FeatureReportByteLength)
		wprintf(_T("FeatureReportByteLength		: 0x%4x\n"), pCaps->FeatureReportByteLength);

	if (pCaps->NumberInputButtonCaps)
	{
		wprintf(_T("pCaps->NumberInputButtonCaps	: 0x%4x\n"), pCaps->NumberInputButtonCaps);
		numCaps = pCaps->NumberInputButtonCaps;
		HidP_GetButtonCaps(HidP_Input,
			buttonCaps,
			&numCaps,
			Ppd);
		if (numCaps)
		{
			if (buttonCaps->ReportID)
				wprintf(_T("ReportId		: 0x%4x\n"), buttonCaps->ReportID);

			wprintf(_T("	UsagePage	: 0x%4x\n"), buttonCaps->UsagePage);
			if (buttonCaps->IsRange)
			{
				for (USHORT usage = buttonCaps->Range.UsageMin; usage <= buttonCaps->Range.UsageMax; usage++)
				{
					wprintf(_T("	Usage		: 0x%4x\n"), usage);
				}
			}
			else
			{
				wprintf(_T("	Usage		: 0x%4x\n"), buttonCaps->NotRange.Usage);
			}
		}
		wprintf(_T("\n"));
	}
	if (pCaps->NumberInputValueCaps)
	{
		wprintf(_T("pCaps->NumberInputValueCaps	: 0x%4x\n"), pCaps->NumberInputValueCaps);
		numValues = pCaps->NumberInputValueCaps;
		HidP_GetValueCaps(HidP_Input,
			valueCaps,
			&numValues,
			Ppd);
		if (numValues)
		{
			if (valueCaps->ReportID)
				wprintf(_T("ReportId		: 0x%4x\n"), valueCaps->ReportID);

			wprintf(_T("	UsagePage	: 0x%4x\n"), valueCaps->UsagePage);
			if (valueCaps->IsRange)
			{
				for (USHORT usage = valueCaps->Range.UsageMin; usage <= valueCaps->Range.UsageMax; usage++)
				{
					wprintf(_T("	Usage		: 0x%4x\n"), usage);
				}
			}
			else
			{
				wprintf(_T("	Usage		: 0x%4x\n"), valueCaps->NotRange.Usage);
			}
		}
		wprintf(_T("\n"));
	}

	if (pCaps->NumberOutputButtonCaps)
	{
		wprintf(_T("pCaps->NumberOutputButtonCaps	: 0x%4x\n"), pCaps->NumberOutputButtonCaps);
		numCaps = pCaps->NumberOutputButtonCaps;
		HidP_GetButtonCaps(HidP_Output,
			buttonCaps,
			&numCaps,
			Ppd);
		if (numCaps)
		{
			if (buttonCaps->ReportID)
				wprintf(_T("ReportId		: 0x%4x\n"), buttonCaps->ReportID);

			wprintf(_T("	UsagePage	: 0x%4x\n"), buttonCaps->UsagePage);
			if (buttonCaps->IsRange)
			{
				for (USHORT usage = buttonCaps->Range.UsageMin; usage <= buttonCaps->Range.UsageMax; usage++)
				{
					wprintf(_T("	Usage		: 0x%4x\n"), usage);
				}
			}
			else
			{
				wprintf(_T("	Usage		: 0x%4x\n"), buttonCaps->NotRange.Usage);
			}
		}
		wprintf(_T("\n"));
	}
	if (pCaps->NumberOutputValueCaps)
	{
		wprintf(_T("pCaps->NumberOutputValueCaps	: 0x%4x\n"), pCaps->NumberOutputValueCaps);
		numValues = pCaps->NumberOutputValueCaps;
		HidP_GetValueCaps(HidP_Output,
			valueCaps,
			&numValues,
			Ppd);
		if (numValues)
		{
			if (valueCaps->ReportID)
				wprintf(_T("ReportId		: 0x%4x\n"), valueCaps->ReportID);

			wprintf(_T("	UsagePage	: 0x%4x\n"), valueCaps->UsagePage);
			if (valueCaps->IsRange)
			{
				for (USHORT usage = valueCaps->Range.UsageMin; usage <= valueCaps->Range.UsageMax; usage++)
				{
					wprintf(_T("	Usage		: 0x%4x\n"), usage);
				}
			}
			else
			{
				wprintf(_T("	Usage		: 0x%4x\n"), valueCaps->NotRange.Usage);
			}
		}
		wprintf(_T("\n"));
	}

	if (pCaps->NumberFeatureButtonCaps)
	{
		wprintf(_T("pCaps->NumberFeatureButtonCaps	: 0x%4x\n"), pCaps->NumberFeatureButtonCaps);
		numCaps = pCaps->NumberFeatureButtonCaps;
		HidP_GetButtonCaps(HidP_Feature,
			buttonCaps,
			&numCaps,
			Ppd);
		if (numCaps)
		{
			if (buttonCaps->ReportID)
				wprintf(_T("ReportId		: 0x%4x\n"), buttonCaps->ReportID);

			wprintf(_T("	UsagePage	: 0x%4x\n"), buttonCaps->UsagePage);
			if (buttonCaps->IsRange)
			{
				for (USHORT usage = buttonCaps->Range.UsageMin; usage <= buttonCaps->Range.UsageMax; usage++)
				{
					wprintf(_T("	Usage		: 0x%4x\n"), usage);
				}
			}
			else
			{
				wprintf(_T("	Usage		: 0x%4x\n"), buttonCaps->NotRange.Usage);
			}
		}
		wprintf(_T("\n"));
	}
	if (pCaps->NumberFeatureValueCaps)
	{
		wprintf(_T("pCaps->NumberFeatureValueCaps	: 0x%4x\n"), pCaps->NumberFeatureValueCaps);
		numValues = pCaps->NumberFeatureValueCaps;
		HidP_GetValueCaps(HidP_Feature,
			valueCaps,
			&numValues,
			Ppd);
		if (numValues)
		{
			if (valueCaps->ReportID)
				wprintf(_T("ReportId		: 0x%4x\n"), valueCaps->ReportID);

			wprintf(_T("	UsagePage	: 0x%4x\n"), valueCaps->UsagePage);
			if (valueCaps->IsRange)
			{
				for (USHORT usage = valueCaps->Range.UsageMin; usage <= valueCaps->Range.UsageMax; usage++)
				{
					wprintf(_T("	Usage		: 0x%4x\n"), usage);
				}
			}
			else
			{
				wprintf(_T("	Usage		: 0x%4x\n"), valueCaps->NotRange.Usage);
			}
		}
		wprintf(_T("\n"));
	}

	free(buttonCaps);
	free(valueCaps);
}

int main()
{
	SP_DEVINFO_DATA			DevInfoData;
	GUID                    hidGuid;
	HANDLE					handle;
	int						count, index;
	WCHAR					*pDeviceName;
	WCHAR					*pHardwareId;
	BOOLEAN					bl;
	PHIDP_PREPARSED_DATA	Ppd;
	HIDD_ATTRIBUTES			Attributes;
	HIDP_CAPS				Caps;

	memset(&DevInfoData, 0, sizeof(SP_DEVINFO_DATA));
	DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	HidD_GetHidGuid(&hidGuid);

	count = GetDeviceStackNameCount((struct _GUID *)&hidGuid);
	if (count == 0)
		return 0; // �ý����� SampleGuid�� �����ϴ� ��ġ�� ��ġ���� �ʾҽ��ϴ�

	for (index = 0; index < count; index++)
	{
		bl = GetDeviceStackName((struct _GUID *)&hidGuid, &pDeviceName, &pHardwareId, index);

		if (bl == FALSE)
			continue;

		// pDeviceName�� ����̽����ÿ� �����ϱ� ���� �̸��� �������ϴ�
		handle = CreateFile(pDeviceName, GENERIC_READ | GENERIC_WRITE
			, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL
			, 0);
		// �̶� ����̹������� IRP_MJ_CREATE��ɾ ���޵˴ϴ�
		if (handle == (HANDLE)-1)
		{
			free(pHardwareId);
			free(pDeviceName);
			continue;
		}

		//MessageBox(NULL, _T("�����۾��� �����մϴ�"), _T("HID Call Test"), MB_OK );

		// Hid Report���������� ���Ѵ�
		if (!HidD_GetPreparsedData(handle, &Ppd)) {
			free(pHardwareId);
			free(pDeviceName);
			continue;
		}

		if (!HidD_GetAttributes(handle, &Attributes)) {
			HidD_FreePreparsedData(Ppd);
			free(pHardwareId);
			free(pDeviceName);
			continue;
		}

		if (!HidP_GetCaps(Ppd, &Caps)) {
			HidD_FreePreparsedData(Ppd);
			free(pHardwareId);
			free(pDeviceName);
			continue;
		}

		// ������ġ�� ���� ��ġ������ ����Ѵ�
		dumpDeviceInfo(index, pHardwareId, pDeviceName, &Caps, Ppd, handle);

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// �׽�Ʈ�� �غ���.
		// ���� ReadFile()�Լ��� ����ؼ� Input Report�� �о��. �״���, HidP_GetXXX()�Լ��� ����Ѵ�.
		// �Ʒ��ڵ�� ����̹��� ������������ �ڵ��̹Ƿ�, ����̹��� �����Ǹ� �ݵ�� �Բ� �����Ǿ����� �Ѵ�
		// �׽�Ʈ��ġ�� ������ Collection ���� ����Ÿ�� �о���� �׽�Ʈ�̴�
		// Input Report�� Feature Report�� ���� ���� ����Ÿ�� �о���� ������ �����Ѵ�
		// ���� ����̹��� Input Report, Feature Report ��� ������ ReportId �� ������ ��������
		// dumpDeviceInfo()�Լ��� ���ؼ� ��µ� ����� ��ġ�ؾ� �Ѵ�
		// Col01's ReportId = 0x01 (Input Report)
		// Col02's ReportId = 0x02 (Input Report)
		// Col03's ReportId = 0x03 (Feature Report)
		// ����, ������ Collection�� ���� ���� ����Ÿ�� �������� �˰��ִٰ� �����Ѵ�
		BYTE *reportDataBuffer = NULL;
		int reportSize;
		NTSTATUS status;
		ULONG usageData;

		// ReadFile() Test
#if 0
		reportSize = Caps.InputReportByteLength;
		reportDataBuffer = (BYTE *)malloc(reportSize);
		ReadFile(handle, reportDataBuffer, reportSize, &ret, NULL);
		ReadFile(handle, reportDataBuffer, reportSize, &ret, NULL);
		free(reportDataBuffer);
#endif 
		// HidP_GetXXX() Test
		switch (index)
		{
		case 0: // Col01
			reportSize = Caps.InputReportByteLength;
			reportDataBuffer = (BYTE *)malloc(reportSize);
			reportDataBuffer[0] = 0x01; // ù��° ����Ʈ�� ReportId�� �־��־�� �Ѵ�

			HidD_GetInputReport(handle, reportDataBuffer, reportSize);
			// ���� ����Ÿ�� Usage�� �����Ͽ� �ؼ��Ϸ��� �Ʒ��� ���� �۾��� �߰��� �ʿ��ϴ�
			// �ʿ��� �Ķ���ʹ� dumpDeviceInfo()�Լ����� ���ŵȴ�
			// ���⼭�� ���������ϵ��� �Ѵ�
			// ���� ����̹��� UsageMin���� UsageMax������ �����Ƿ�, ���⼭�� 0x0001 �� �м��غ���
			status = HidP_GetUsageValue(HidP_Input,
				0xFF00, // UsagePage
				0,
				0x0001, // Usage
				&usageData,
				Ppd,
				(PCHAR)reportDataBuffer,
				reportSize);

			// debug
			/*
			wprintf( _T("0x%8x\n"), HIDP_STATUS_SUCCESS);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_NULL);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_INVALID_PREPARSED_DATA);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_INVALID_REPORT_TYPE);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_INVALID_REPORT_LENGTH);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_USAGE_NOT_FOUND);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_VALUE_OUT_OF_RANGE);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_BAD_LOG_PHY_VALUES);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_BUFFER_TOO_SMALL);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_INTERNAL_ERROR);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_INCOMPATIBLE_REPORT_ID);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_NOT_VALUE_ARRAY);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_IS_VALUE_ARRAY);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_DATA_INDEX_NOT_FOUND);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_DATA_INDEX_OUT_OF_RANGE);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_BUTTON_NOT_PRESSED);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_REPORT_DOES_NOT_EXIST);
			wprintf( _T("0x%8x\n"), HIDP_STATUS_NOT_IMPLEMENTED);
			*/
			if (status == HIDP_STATUS_SUCCESS)
			{
				// ���� ���� ����Ѵ�
				wprintf(_T("		* Usage 0x0001 value = 0x%4x\n\n"), usageData);
			}
			free(reportDataBuffer);

			break;
		case 1: // Col02
			reportSize = Caps.InputReportByteLength;
			reportDataBuffer = (BYTE *)malloc(reportSize);
			reportDataBuffer[0] = 0x02; // ù��° ����Ʈ�� ReportId�� �־��־�� �Ѵ�

			HidD_GetInputReport(handle, reportDataBuffer, reportSize);
			// ���� ����Ÿ�� Usage�� �����Ͽ� �ؼ��Ϸ��� �Ʒ��� ���� �۾��� �߰��� �ʿ��ϴ�
			// �ʿ��� �Ķ���ʹ� dumpDeviceInfo()�Լ����� ���ŵȴ�
			// ���⼭�� ���������ϵ��� �Ѵ�
			// ���� ����̹��� UsageMin���� UsageMax������ �����Ƿ�, ���⼭�� 0x0001 �� �м��غ���
			status = HidP_GetUsageValue(HidP_Input,
				0xFF00, // UsagePage
				0,
				0x0001, // Usage
				&usageData,
				Ppd,
				(PCHAR)reportDataBuffer,
				reportSize);
			if (status == HIDP_STATUS_SUCCESS)
			{
				// ���� ���� ����Ѵ�
				wprintf(_T("		* Usage 0x0001 value = 0x%4x\n\n"), usageData);
			}
			free(reportDataBuffer);

			break;
		case 2: // Col03
			reportSize = Caps.FeatureReportByteLength;
			reportDataBuffer = (BYTE *)malloc(reportSize);
			reportDataBuffer[0] = 0x03; // ù��° ����Ʈ�� ReportId�� �־��־�� �Ѵ�

			HidD_GetFeature(handle, reportDataBuffer, reportSize);

			// ���� ����Ÿ�� Usage�� �����Ͽ� �ؼ��Ϸ��� �Ʒ��� ���� �۾��� �߰��� �ʿ��ϴ�
			// �ʿ��� �Ķ���ʹ� dumpDeviceInfo()�Լ����� ���ŵȴ�
			// ���⼭�� ���������ϵ��� �Ѵ�
			// ���� ����̹��� UsageMin���� UsageMax������ �����Ƿ�, ���⼭�� 0x0001 �� �м��غ���
			status = HidP_GetUsageValue(HidP_Feature,
				0xFF00, // UsagePage
				0,
				0x0001, // Usage
				&usageData,
				Ppd,
				(PCHAR)reportDataBuffer,
				reportSize);

			if (status == HIDP_STATUS_SUCCESS)
			{
				// ���� ���� ����Ѵ�
				wprintf(_T("		* Usage 0x0001 value = 0x%4x\n\n"), usageData);
			}
			free(reportDataBuffer);

			break;
		}

		//
		//////////////////////////////////////////////////////////////////////////////////////////////////////

		CloseHandle(handle);

		free(pHardwareId);
		free(pDeviceName);
		// pDeviceName�� GetDeviceStackName()�Լ������� �Ҵ�Ǵ� �޸��̹Ƿ�, ����� ������ �ݵ�� �����Ͽ��� �մϴ�
		wprintf(_T("-----------------------------------------------------------------\n"));

	}
	MessageBox(NULL, _T("�۾��� �������ϴ�"), _T("HID Call Test"), MB_OK);
	return 0;
}

