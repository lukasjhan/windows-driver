#include "HJSTOR.h"

/*
Scsi Miniport�� �ٸ����� ���� �߿��Ѱ���!!!
������� ���۰� ���޵Ǵ� ����̴�. 
Stor Miniport������ ������� ���۰� Scatter Gather ����� �����ּ����·� �����ȴ�.
����ũ�� �����ϱ� ������, �츮�� �� Scatter Gather �ּҸ� ���ؿµ�, �̰��� �����ּҷ� ��ȯ�ؼ� ����ؾ� �Ѵ�
*/
BOOLEAN
HJSTOR_Hw_BuildIo(
	PVOID					pContext,
	PSCSI_REQUEST_BLOCK		srb
	)
{
	PSRB_EXTENSION			pSrbExtension		= NULL;
	PHW_DEVICE_EXTENSION	DeviceExtension	= NULL;
	PCDB					cdb					= NULL;
	ULONG					diskSectors;
	ULONG_PTR				SrbBuffer;
	LARGE_INTEGER			blk_offset;
	ULONG					blk_length;
	PSTOR_SCATTER_GATHER_LIST	pScatterGatherList	= NULL;
	UCHAR					status			= SRB_STATUS_SUCCESS;

	if ( (pContext == NULL) || (srb == NULL) )
	{
		return FALSE;
	}

	if ( srb->Function != SRB_FUNCTION_EXECUTE_SCSI )
	{
		return TRUE; // StartIo�� ����� ������� �ǹ�
	}

	cdb = (PCDB) srb->Cdb;

	// READ, WRITE ��ɾ BuildIo���� ó���ϴ� ����� ���ؼ� StartIo�� �����Ǵ� ����� �����ش�
	// ������ ����� ��� �� StartIO �� ������ ó���Ѵ�.	
	if ( cdb->CDB6GENERIC.OperationCode != SCSIOP_READ 
		&& cdb->CDB6GENERIC.OperationCode != SCSIOP_READ6 
		&& cdb->CDB6GENERIC.OperationCode != SCSIOP_READ12 
		&& cdb->CDB6GENERIC.OperationCode != SCSIOP_READ16 
		&& cdb->CDB6GENERIC.OperationCode != SCSIOP_WRITE 
		&& cdb->CDB6GENERIC.OperationCode != SCSIOP_WRITE6 
		&& cdb->CDB6GENERIC.OperationCode != SCSIOP_WRITE12 
		&& cdb->CDB6GENERIC.OperationCode != SCSIOP_WRITE16 
		)
	{
		return TRUE; // STOR ���� StartIo�� ��ɾ �����Ұ��� ��û�Ѵ�
	}

	DeviceExtension	= (PHW_DEVICE_EXTENSION) pContext;
	pSrbExtension		= (PSRB_EXTENSION) srb->SrbExtension;

/*
	pScatterGatherList = StorPortGetScatterGatherList( DeviceExtension, srb );
	if( pScatterGatherList == NULL )
	{
		return TRUE; // StartIo�� ����� ������� �ǹ�
	}
*/

	diskSectors	= DeviceExtension->TotalBlocks;

	cdb = (PCDB)srb->Cdb;

	if (cdb->CDB6GENERIC.OperationCode == SCSIOP_READ)
	{
#ifdef DBGOUT
		DbgPrint("[HJSTOR]HJSTOR_HwStartIo, SCSIOP_READ\n");
#endif

		blk_offset.u.HighPart = 0;
		blk_offset.u.LowPart = (ULONG)(cdb->CDB10.LogicalBlockByte3 |
						cdb->CDB10.LogicalBlockByte2 << 8 |
						cdb->CDB10.LogicalBlockByte1 << 16 |
						cdb->CDB10.LogicalBlockByte0 << 24);
	
		blk_length = (ULONG)(cdb->CDB10.TransferBlocksLsb |
						cdb->CDB10.TransferBlocksMsb << 8);

		/*
		blk_offset : ���� ����
		blk_length : ũ�� ����
		*/

#ifdef DBGOUT
		DbgPrint("[HJSTOR]param, blk_length = 0x%8X, blk_offset = 0x%u\n", blk_length, blk_offset.QuadPart  );
#endif

		StorPortGetSystemAddress( DeviceExtension, srb, (PVOID *)&SrbBuffer );
		StorPortMoveMemory( (PVOID)SrbBuffer, (PVOID)(DeviceExtension->Base + blk_offset.QuadPart * SIZE_LOGICAL_BLOCK), blk_length * SIZE_LOGICAL_BLOCK );
        status = SRB_STATUS_SUCCESS;
	}
	else if (cdb->CDB6GENERIC.OperationCode == SCSIOP_WRITE)
	{
#ifdef DBGOUT
		DbgPrint("[HJSTOR]HJSTOR_HwStartIo, SCSIOP_WRITE\n");
#endif
		blk_offset.u.HighPart = 0;
		blk_offset.u.LowPart = (ULONG)(cdb->CDB10.LogicalBlockByte3 |
						cdb->CDB10.LogicalBlockByte2 << 8 |
						cdb->CDB10.LogicalBlockByte1 << 16 |
						cdb->CDB10.LogicalBlockByte0 << 24);
	
		blk_length = (ULONG)(cdb->CDB10.TransferBlocksLsb |
						cdb->CDB10.TransferBlocksMsb << 8);

		/*
		blk_offset : ���� ����
		blk_length : ũ�� ����
		*/

#ifdef DBGOUT
		DbgPrint("[HJSTOR]param, blk_length = 0x%8X, blk_offset = 0x%u\n", blk_length, blk_offset.QuadPart  );
#endif

		StorPortGetSystemAddress( DeviceExtension, srb, (PVOID *)&SrbBuffer );
		StorPortMoveMemory((PVOID)(DeviceExtension->Base + blk_offset.QuadPart * SIZE_LOGICAL_BLOCK), (PVOID)SrbBuffer, blk_length * SIZE_LOGICAL_BLOCK );

        status = SRB_STATUS_SUCCESS;
	}
	else if(cdb->CDB6GENERIC.OperationCode == SCSIOP_READ16)
	{
#ifdef DBGOUT
		DbgPrint("[HJSTOR]HJSTOR_HwStartIo, SCSIOP_READ16\n");
#endif
		blk_offset.u.HighPart = (ULONG)(cdb->CDB16.LogicalBlock[3] |
						cdb->CDB16.LogicalBlock[2] << 8 |
						cdb->CDB16.LogicalBlock[1] << 16 |
						cdb->CDB16.LogicalBlock[0] << 24);
		
		blk_offset.u.LowPart = (ULONG)(cdb->CDB16.LogicalBlock[7] |
						cdb->CDB16.LogicalBlock[6] << 8 |
						cdb->CDB16.LogicalBlock[5] << 16 |
						cdb->CDB16.LogicalBlock[4] << 24);
	
		blk_length = (ULONG)(cdb->CDB16.TransferLength[3] |
						cdb->CDB16.TransferLength[2] << 8 |
						cdb->CDB16.TransferLength[1] << 16 |
						cdb->CDB16.TransferLength[0] << 24);
		/*
		blk_offset : ���� ����
		blk_length : ũ�� ����
		*/

#ifdef DBGOUT
		DbgPrint("[HJSTOR]param, blk_length = 0x%8X, blk_offset = 0x%u\n", blk_length, blk_offset.QuadPart  );
#endif

		StorPortGetSystemAddress( DeviceExtension, srb, (PVOID *)&SrbBuffer );
		StorPortMoveMemory((PVOID)SrbBuffer, (PVOID)(DeviceExtension->Base + blk_offset.QuadPart * SIZE_LOGICAL_BLOCK), blk_length * SIZE_LOGICAL_BLOCK );

        status = SRB_STATUS_SUCCESS;
	}
	else if(cdb->CDB6GENERIC.OperationCode == SCSIOP_READ12)
	{
#ifdef DBGOUT
		DbgPrint("[HJSTOR]HJSTOR_HwStartIo, SCSIOP_READ12\n");
#endif
		blk_offset.u.HighPart = 0;

		blk_offset.u.LowPart = (ULONG)(cdb->CDB12.LogicalBlock[3] |
						cdb->CDB12.LogicalBlock[2] << 8 |
						cdb->CDB12.LogicalBlock[1] << 16 |
						cdb->CDB12.LogicalBlock[0] << 24);
	
		blk_length = (ULONG)(cdb->CDB12.TransferLength[3] |
						cdb->CDB12.TransferLength[2] << 8 |
						cdb->CDB12.TransferLength[1] << 16 |
						cdb->CDB12.TransferLength[0] << 24);

		/*
		blk_offset : ���� ����
		blk_length : ũ�� ����
		*/

#ifdef DBGOUT
		DbgPrint("[HJSTOR]param, blk_length = 0x%8X, blk_offset = 0x%u\n", blk_length, blk_offset.QuadPart  );
#endif

		StorPortGetSystemAddress( DeviceExtension, srb, (PVOID *)&SrbBuffer );
		StorPortMoveMemory((PVOID)SrbBuffer, (PVOID)(DeviceExtension->Base + blk_offset.QuadPart * SIZE_LOGICAL_BLOCK), blk_length * SIZE_LOGICAL_BLOCK );

        status = SRB_STATUS_SUCCESS;
	}
	else if(cdb->CDB6GENERIC.OperationCode == SCSIOP_READ6)
	{
#ifdef DBGOUT
		DbgPrint("[HJSTOR]HJSTOR_HwStartIo, SCSIOP_READ6\n");
#endif
		blk_offset.u.HighPart = 0;

		blk_offset.u.LowPart = (ULONG)(cdb->CDB6READWRITE.LogicalBlockLsb |
						cdb->CDB6READWRITE.LogicalBlockMsb0 << 8 |
						cdb->CDB6READWRITE.LogicalBlockMsb1 << 16);
	
		blk_length = (ULONG)(cdb->CDB6READWRITE.TransferBlocks);

		/*
		blk_offset : ���� ����
		blk_length : ũ�� ����
		*/

		DbgPrint("[HJSTOR]param, blk_length = 0x%8X, blk_offset = 0x%u\n", blk_length, blk_offset.QuadPart  );

		StorPortGetSystemAddress( DeviceExtension, srb, (PVOID *)&SrbBuffer );
		StorPortMoveMemory((PVOID)SrbBuffer, (PVOID)(DeviceExtension->Base + blk_offset.QuadPart * SIZE_LOGICAL_BLOCK), blk_length * SIZE_LOGICAL_BLOCK );

        status = SRB_STATUS_SUCCESS;
	}
	else if (cdb->CDB6GENERIC.OperationCode == SCSIOP_WRITE16)
	{
#ifdef DBGOUT
		DbgPrint("[HJSTOR]HJSTOR_HwStartIo, SCSIOP_WRITE16\n");
#endif
		blk_offset.u.HighPart = (ULONG)(cdb->CDB16.LogicalBlock[3] |
						cdb->CDB16.LogicalBlock[2] << 8 |
						cdb->CDB16.LogicalBlock[1] << 16 |
						cdb->CDB16.LogicalBlock[0] << 24);
		
		blk_offset.u.LowPart = (ULONG)(cdb->CDB16.LogicalBlock[7] |
						cdb->CDB16.LogicalBlock[6] << 8 |
						cdb->CDB16.LogicalBlock[5] << 16 |
						cdb->CDB16.LogicalBlock[4] << 24);
	
		blk_length = (ULONG)(cdb->CDB16.TransferLength[3] |
						cdb->CDB16.TransferLength[2] << 8 |
						cdb->CDB16.TransferLength[1] << 16 |
						cdb->CDB16.TransferLength[0] << 24);

		/*
		blk_offset : ���� ����
		blk_length : ũ�� ����
		*/
#ifdef DBGOUT
		DbgPrint("[HJSTOR]param, blk_length = 0x%8X, blk_offset = 0x%u\n", blk_length, blk_offset.QuadPart  );
#endif

		StorPortGetSystemAddress( DeviceExtension, srb, (PVOID *)&SrbBuffer );
		StorPortMoveMemory((PVOID)(DeviceExtension->Base + blk_offset.QuadPart * SIZE_LOGICAL_BLOCK), (PVOID)SrbBuffer, blk_length * SIZE_LOGICAL_BLOCK );

        status = SRB_STATUS_SUCCESS;
	}
	else if (cdb->CDB6GENERIC.OperationCode == SCSIOP_WRITE12)
	{
#ifdef DBGOUT
		DbgPrint("[HJSTOR]HJSTOR_HwStartIo, SCSIOP_WRITE12\n");
#endif
		blk_offset.u.HighPart = 0;

		blk_offset.u.LowPart = (ULONG)(cdb->CDB12.LogicalBlock[3] |
						cdb->CDB12.LogicalBlock[2] << 8 |
						cdb->CDB12.LogicalBlock[1] << 16 |
						cdb->CDB12.LogicalBlock[0] << 24);
	
		blk_length = (ULONG)(cdb->CDB12.TransferLength[3] |
						cdb->CDB12.TransferLength[2] << 8 |
						cdb->CDB12.TransferLength[1] << 16 |
						cdb->CDB12.TransferLength[0] << 24);

		/*
		blk_offset : ���� ����
		blk_length : ũ�� ����
		*/

#ifdef DBGOUT
		DbgPrint("[HJSTOR]param, blk_length = 0x%8X, blk_offset = 0x%u\n", blk_length, blk_offset.QuadPart  );
#endif

		StorPortGetSystemAddress( DeviceExtension, srb, (PVOID *)&SrbBuffer );
		StorPortMoveMemory((PVOID)(DeviceExtension->Base + blk_offset.QuadPart * SIZE_LOGICAL_BLOCK), (PVOID)SrbBuffer, blk_length * SIZE_LOGICAL_BLOCK );

        status = SRB_STATUS_SUCCESS;
	}
	else if (cdb->CDB6GENERIC.OperationCode == SCSIOP_WRITE6)
	{
#ifdef DBGOUT
		DbgPrint("[HJSTOR]HJSTOR_HwStartIo, SCSIOP_WRITE6\n");
#endif
		blk_offset.u.HighPart = 0;

		blk_offset.u.LowPart = (ULONG)(cdb->CDB6READWRITE.LogicalBlockLsb |
						cdb->CDB6READWRITE.LogicalBlockMsb0 << 8 |
						cdb->CDB6READWRITE.LogicalBlockMsb1 << 16);
	
		blk_length = (ULONG)(cdb->CDB6READWRITE.TransferBlocks);

		/*
		blk_offset : ���� ����
		blk_length : ũ�� ����
		*/

#ifdef DBGOUT
		DbgPrint("[HJSTOR]param, blk_length = 0x%8X, blk_offset = 0x%u\n", blk_length, blk_offset.QuadPart  );
#endif

		StorPortGetSystemAddress( DeviceExtension, srb, (PVOID *)&SrbBuffer );
		StorPortMoveMemory((PVOID)(DeviceExtension->Base + blk_offset.QuadPart * SIZE_LOGICAL_BLOCK), (PVOID)SrbBuffer, blk_length * SIZE_LOGICAL_BLOCK );

        status = SRB_STATUS_SUCCESS;
	}
	else
	{
		// ��������ɾ���� ��� StartIo�� ������
		return TRUE;

	}

	srb->SrbStatus = (UCHAR)status;
	StorPortNotification(RequestComplete,   DeviceExtension, srb);
	StorPortNotification(NextRequest,  DeviceExtension, NULL);

	return FALSE; // StartIo�� ����� ������ ����� �ǹ�
}
