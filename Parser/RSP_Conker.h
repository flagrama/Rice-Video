/*
Copyright (C) 2002-2009 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef RSP_CONKER_H__
#define RSP_CONKER_H__

uint32 dwConkerVtxZAddr=0;

extern void ProcessVertexDataConker(uint32 dwAddr, uint32 dwV0, uint32 dwNum);
void RSP_Vtx_Conker(MicroCodeCommand command)
{
	uint32 address = RSPSegmentAddr((command.inst.cmd1));
	uint32 len   = ((command.inst.cmd0 >> 1 )&0x7F);
	uint32 n     = ((command.inst.cmd0 >> 12)&0xFF);
	uint32 v0	 = len - n;

	LOG_UCODE("    Vtx: Address 0x%08x, vEnd: %d, v0: %d, Num: %d", address, len, v0, n);

	ProcessVertexDataConker(address, v0, n);

#ifdef _DEBUG
	status.dwNumVertices += dwN;
	DisplayVertexInfo(dwAddr, dwV0, dwN);
#endif
}

//*****************************************************************************
//
//*****************************************************************************
void RSP_Tri1_Conker(MicroCodeCommand command)
{

    u32 pc = gDlistStack.address[gDlistStackPointer];
    u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

    // If Off screen rendering is true then just skip the whole list of tris //Corn
    // Skip shadow as well
    if (g_CI.dwFormat != TXT_FMT_RGBA)
    {
        do
        {
            command.inst.cmd0 = *pCmdBase++;
            command.inst.cmd1 = *pCmdBase++;
            pc += 8;
        } while (command.inst.cmd == RSP_ZELDATRI1);
        gDlistStack.address[gDlistStackPointer] = pc - 8;
        return;
    }

    bool tris_added = false;

    do
    {
        //DL_PF("    0x%08x: %08x %08x %-10s", pc-8, command.inst.cmd0, command.inst.cmd1, "G_GBI2_TRI2");

        // Vertex indices already divided in ucodedef
        u32 v0_idx = command.gbi2tri1.v0 >> 1;
        u32 v1_idx = command.gbi2tri1.v1 >> 1;
        u32 v2_idx = command.gbi2tri1.v2 >> 1;

        tris_added |= AddTri(v0_idx, v1_idx, v2_idx);

        command.inst.cmd0 = *pCmdBase++;
        command.inst.cmd1 = *pCmdBase++;
        pc += 8;
    } while (command.inst.cmd == RSP_ZELDATRI1);

    gDlistStack.address[gDlistStackPointer] = pc - 8;

    if (tris_added)
    {
        CRender::g_pRender->DrawTriangles();
    }
}

//*****************************************************************************
//
//*****************************************************************************
void RSP_Tri2_Conker(MicroCodeCommand command)
{

    u32 pc = gDlistStack.address[gDlistStackPointer];
    u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

    // If Off screen rendering is true then just skip the whole list of tris //Corn
    // Skip shadow as well
    if (g_CI.dwFormat != TXT_FMT_RGBA)
    {
        do
        {
            command.inst.cmd0 = *pCmdBase++;
            command.inst.cmd1 = *pCmdBase++;
            pc += 8;
        } while (command.inst.cmd == RSP_ZELDATRI2);
        gDlistStack.address[gDlistStackPointer] = pc - 8;
        return;
    }

    bool tris_added = false;

    do
    {
        //DL_PF("    0x%08x: %08x %08x %-10s", pc-8, command.inst.cmd0, command.inst.cmd1, "G_GBI2_TRI2");

        // Vertex indices already divided in ucodedef
        u32 v0_idx = command.gbi2tri2.v0;
        u32 v1_idx = command.gbi2tri2.v1;
        u32 v2_idx = command.gbi2tri2.v2;

        tris_added |= AddTri(v0_idx, v1_idx, v2_idx);

        u32 v3_idx = command.gbi2tri2.v3;
        u32 v4_idx = command.gbi2tri2.v4;
        u32 v5_idx = command.gbi2tri2.v5;

        tris_added |= AddTri(v3_idx, v4_idx, v5_idx);

        command.inst.cmd0 = *pCmdBase++;
        command.inst.cmd1 = *pCmdBase++;
        pc += 8;
    } while (command.inst.cmd == RSP_ZELDATRI2);

    gDlistStack.address[gDlistStackPointer] = pc - 8;

    if (tris_added)
    {
        CRender::g_pRender->DrawTriangles();
    }
}

//*****************************************************************************
//
//*****************************************************************************
void RSP_Tri4_Conker(MicroCodeCommand command)
{

	// While the next command pair is Tri2, add vertices
	uint32 dwPC = gDlistStack.address[gDlistStackPointer];

    // If Off screen rendering is true then just skip the whole list of tris //Corn
    if (g_CI.dwFormat != TXT_FMT_RGBA)
    {
        do
        {
            command.inst.cmd0 = *(u32 *)(g_pu8RamBase + dwPC + 0);
            command.inst.cmd1 = *(u32 *)(g_pu8RamBase + dwPC + 4);
            dwPC += 8;
        } while ((command.inst.cmd0 >> 28) == 1);
        gDlistStack.address[gDlistStackPointer] = dwPC - 8;
        return;
    }

	bool bTrisAdded = false;

	do {
		LOG_UCODE("    Conker Tri4: 0x%08x 0x%08x", command.inst.cmd0, command.inst.cmd1);
		uint32 idx[12];
		idx[0] = (command.inst.cmd1      )&0x1F;
		idx[1] = (command.inst.cmd1 >>  5)&0x1F;
		idx[2] = (command.inst.cmd1 >> 10)&0x1F;

		bTrisAdded |= AddTri(idx[0], idx[1], idx[2]);

		idx[3] = (command.inst.cmd1 >> 15)&0x1F;
		idx[4] = (command.inst.cmd1 >> 20)&0x1F;
		idx[5] = (command.inst.cmd1 >> 25)&0x1F;

		bTrisAdded |= AddTri(idx[3], idx[4], idx[5]);

		idx[6] = (command.inst.cmd0    )&0x1F;
		idx[7] = (command.inst.cmd0 >> 5)&0x1F;
		idx[8] = (command.inst.cmd0 >> 10)&0x1F;

		bTrisAdded |= AddTri(idx[6], idx[7], idx[8]);

		idx[ 9] = ((((command.inst.cmd0 >> 15)&0x7)<<2)|(command.inst.cmd1>>30));
		idx[10] = (command.inst.cmd0>>18)&0x1F;
		idx[11] = (command.inst.cmd0>>23)&0x1F;

		bTrisAdded |= AddTri(idx[9], idx[10], idx[11]);

        command.inst.cmd0 = *(uint32 *)(g_pu8RamBase + dwPC+0);
        command.inst.cmd1 = *(uint32 *)(g_pu8RamBase + dwPC+4);
		dwPC += 8;

#ifdef _DEBUG
	} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (w0>>28) == 1 );
#else
	} while ((command.inst.cmd0>>28) == 1);
#endif

	gDlistStack.address[gDlistStackPointer] = dwPC-8;

	if (bTrisAdded)	
	{
		CRender::g_pRender->DrawTriangles();
	}

	DEBUG_TRIANGLE(TRACE0("Pause at Conker Tri4"));
}



void RSP_MoveMem_Conker(MicroCodeCommand command)
{
	uint32 dwType = command.inst.cmd0 & 0xFE;
	uint32 dwAddr = RSPSegmentAddr(command.inst.cmd1);

	switch (dwType)
	{
	case RSP_GBI2_MV_MEM__MATRIX:
		{
			LOG_UCODE("    DLParser_MoveMem_Conker");
			dwConkerVtxZAddr = dwAddr;
		}
		break;
	case RSP_GBI2_MV_MEM__LIGHT:
		{
			LOG_UCODE("    MoveMem Light Conker");
			uint32 dwOffset2 = ((command.inst.cmd0) >> 5) & 0x3FFF;
			uint32 light_index = (dwOffset2 / 48);

			if (light_index < 2)
			{
				return;
			}
			light_index -= 2;

			N64Light *light = (N64Light*)(g_pu8RamBase + dwAddr);
			RSP_MoveMemLight(light_index, light);
	
			SetLightPosition(light_index, light->x, light->y, light->z, light->w);
			SetLightCBFD(light_index, light->nonzero);

			DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_SET_LIGHT, 
			{
				DebuggerAppendMsg("RSP_MoveMemLight: Addr=%08X, cmd0=%08X", dwAddr, (command.inst.cmd0));
				TRACE0("Pause after MoveMemLight");
			});
		}
		break;
	default:
		RSP_GBI2_MoveMem(command);
		break;
	}
}

void RSP_MoveWord_Conker(MicroCodeCommand command)
{
	uint32 dwType   = ((command.inst.cmd0) >> 16) & 0xFF;
	switch (dwType)
	{
		case RSP_MOVE_WORD_NUMLIGHT:
		{
			uint32 dwNumLights = command.inst.cmd1 / 48;
			SetNumLights(dwNumLights);
		}
		break;

		case RSP_MOVE_WORD_SEGMENT:
		{
			uint32 dwSeg = command.mw2.offset >> 2;
			uint32 dwAddr = command.mw2.value & 0x00FFFFFF;			// Hack - convert to physical

			LOG_UCODE("      RSP_MOVE_WORD_SEGMENT Segment[%d] = 0x%08x", dwSeg, dwAddr);

			gRSP.segments[dwSeg] = dwAddr;
		}
		break;

		case 0x10:
		{
			if ((command.inst.cmd0 & 8) == 0)
			{
				uint32 idx = (command.inst.cmd0 >> 1) & 3;
				uint32 pos = command.inst.cmd0 & 0x30;

				switch (pos)
				{
				case 0:
					gRDP.CoordMod[0 + idx] = (s16)(command.inst.cmd1 >> 16);
					gRDP.CoordMod[1 + idx] = (s16)(command.inst.cmd1 & 0xFFFF);
					break;
				case 0x10:
					gRDP.CoordMod[4 + idx] = (command.inst.cmd1 >> 16) / 65536.0f;
					gRDP.CoordMod[5 + idx] = (command.inst.cmd1 & 0xFFFF) / 65536.0f;
					gRDP.CoordMod[12 + idx] = gRDP.CoordMod[0 + idx] + gRDP.CoordMod[4 + idx];
					gRDP.CoordMod[13 + idx] = gRDP.CoordMod[1 + idx] + gRDP.CoordMod[5 + idx];
					break;
				case 0x20:
					gRDP.CoordMod[8 + idx] = (s16)(command.inst.cmd1 >> 16);
					gRDP.CoordMod[9 + idx] = (s16)(command.inst.cmd1 & 0xFFFF);
					break;
				}
			}
		}
		break;
	}
}
#endif //RSP_CONKER_H__