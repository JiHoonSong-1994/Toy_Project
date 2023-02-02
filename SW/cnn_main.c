/*******************************************************************************
Copyright (c) 2022  JiHoon Song, All Rights Reserved
AUTHOR: JiHoon Song 
AUTHOR'S EMAIL : jihoon20620@naver.com 
ASSOCIATED FILENAME : cnn_main.c
REVISION HISTORY : December 10, 2022 - initial release
*******************************************************************************/

#include <stdio.h>
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xtime_l.h"
#include "xaxicdma.h"
#include "sleep.h"
#include "xil_cache.h"

#define KX 3
#define KY 3
#define ICH 4
#define OCH 8

#define CORE_RUN_ADDR 	0x00 // 0
#define F_ENABLE 		0x04
#define F_READ_DONE		0x08
#define F_VALUE 		0x0C
#define F_DONE 			0x10

#define MAXPOOL_RESULT_0	0x14 // 5
#define MAXPOOL_RESULT_1	0x18
#define MAXPOOL_RESULT_2 	0x1C
#define MAXPOOL_RESULT_3 	0x20
#define MAXPOOL_RESULT_4 	0x24
#define MAXPOOL_RESULT_5	0x28
#define MAXPOOL_RESULT_6 	0x2C // 11
#define MAXPOOL_DONE		0x30 // 12

#define M_ENABLE 		0x34
#define M_ADDRESS 		0x38
#define M_VALUE 		0x3C
#define M_DONE			0x40 // 16
#define CNN_RESULT_0		0x44 // 17
#define CNN_RESULT_1		0x48
#define CNN_DONE		0x4C
#define CNN_RESULT_2		0x50
#define CNN_RESULT_3		0x54
#define CNN_RESULT_4		0x58
#define CNN_RESULT_5		0x5C
#define CNN_RESULT_6		0x60
#define CNN_RESULT_7		0x64 // 25
#define M0_INIT 		0x68 //26 // 42000000
#define M1_INIT 		0x6C //		 42000100


static void Callback(void *CallBackRef, u32 IrqMask, int *IgnorePtr);

int main()
{
	int inbyte_in;
	int val;
	unsigned int kx, ky; 					
	unsigned int ich, och; 					
	unsigned int fmap [ICH][KY][KX]; 		
	signed int weight[OCH][ICH][KY][KX]; 	
	signed int bias [OCH]; 					
	signed int mac_result[OCH]; 			
	signed int result[OCH]; 
	signed int result_for_demo=0; 			

	signed int result_0_rtl;
	signed int result_1_rtl;
	signed int result_2_rtl;
	signed int result_3_rtl;
	signed int result_4_rtl;
	signed int result_5_rtl;
	signed int result_6_rtl;
	signed int result_7_rtl;

	signed int weight_rand_val = 0;
	signed int bias_rand_val = 0;
	signed int fmap_rand_val = 0;

	double ref_c_run_time;
	double ref_v_run_time;
	XTime ref_c_run_cycle;
	XTime ref_v_run_cycle;

	Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + M0_INIT), (u32) 0);
	Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + M1_INIT), (u32) 0);


	while (1)
	{
		print ("********************** CNN Core TEST Start *********************** \r\n ");
		print ("TeraTerm: Please Set Local Echo Mode. \r\n");
		print ("Press '1' Start Demo \r\n");
		print ("Press '2' to exit \r\n");
		print ("Selection:");
		inbyte_in = inbyte ();
		print ("\r\n");
		print ("\r\n");

		XTime tStart, tEnd;

		switch (inbyte_in)
		{
			case '1': 
				srand(tStart);

/////////////////// Random Gen /////////////////////////////

				// Initial Setting fmap, weight, bias value.
				for(ich = 0; ich < ICH; ich ++){
						for(ky = 0; ky < KY; ky++){
							for(kx = 0; kx < KX; kx++){
								fmap_rand_val = rand()%256;
								fmap[ich][ky][kx] = fmap_rand_val;
								//printf("fmap[%d][%d][%d] : %d\n" ,ich,ky,kx,fmap[ich][ky][kx]);
							}
						}
				}

				for (och = 0 ; och < OCH; och ++){
					for(ich = 0; ich < ICH; ich ++){
						for(ky = 0; ky < KY; ky++){
							for(kx = 0; kx < KX; kx++){
								weight_rand_val = rand()%256-128;
								weight[och][ich][ky][kx] = weight_rand_val;
								//printf("weight[%d][%d][%d][%d] : %d\n" ,och,ich,ky,kx,weight[och][ich][ky][kx]);
							}
						}
					}
					bias_rand_val = rand()%256-128;
					bias[och] = bias_rand_val;
					mac_result[och] = 0;
				}
				for (och = 0 ; och < OCH; och ++){
				//printf("bias[%d] : %d\n" , och,bias[och]);
				}
				result_for_demo =0;

/////////////////// CNN Run in PS /////////////////////////////
				printf("============[REF_C] CNN Run in PS .=============\n");
				XTime_GetTime(&tStart);
				// multiply and accumulate
				for (och = 0 ; och < OCH; och ++){
					for(ich = 0; ich < ICH; ich ++){
						for(ky = 0; ky < KY; ky++){
							for(kx = 0; kx < KX; kx++){
								mac_result[och] += (fmap[ich][ky][kx] * weight[och][ich][ky][kx]);
							}
						}
					}
				}
				// added bias,  activation function ReLu
				for (och = 0 ; och < OCH; och ++){
					result[och] = mac_result[och] + bias[och];
					if (result[och] <= 0){
						result[och] = 0;
					}
					printf("[och:%d] result : %08X\n",och, result[och]);
				}
				for  (och = 0 ; och < OCH; och ++){
				Xil_Out32 ((u32) (0x10000000+och*sizeof(int)), (u32) result[och]);
				}
				// to check result between ref_c vs rtl_v
				for (och = 0 ; och < OCH; och ++){
					result_for_demo += result[och];
				}
				XTime_GetTime(&tEnd);
				ref_c_run_cycle = 2*(tEnd - tStart);
				ref_c_run_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
				printf("[REF_C] Output took %llu clock cycles.\n", ref_c_run_cycle);
				printf("[REF_C] Output took %.2f us.\n", ref_c_run_time);
				printf("============[REF_C] DONE .=============\n");

///////////////////////////////////////////////////////////////////////////// INPUT DATA_Init //////////////////////////////////////////////////////////
		#define fmap_size (3*3*4)
		#define	weight_size (3*3*4*8)
		#define	bias_size 8

		int DDR_BUF_fmap[fmap_size] ; // 3*120
		signed int DDR_BUF_weight[weight_size] ;
		signed int DDR_BUF_bias[bias_size] ;
		signed int DDR_BUF[fmap_size+weight_size+bias_size];
		int DDR_SIZE = sizeof(DDR_BUF);

		printf("============fmap , weight, bias init============ \n");
		printf("fmap init\n");

		int buf_address = 0;
		for(ich = 0; ich < ICH; ich ++){
				for(ky = 0; ky < KY; ky++){
					for(kx = 0; kx < KX; kx++) {
						DDR_BUF_fmap[buf_address] = fmap[ich][ky][kx];
						buf_address++;
			//printf("DDR_BUF_fmap[%d] : %d , fmap_ps[%d] : %d \n" ,buf_address,DDR_BUF_fmap[buf_address],i,fmap_ps[i]);
					}
				}
		}//
		buf_address = 0;
		printf("weight init & bias init\n");
		for (och = 0 ; och < OCH; och ++){
			for(ich = 0; ich < ICH; ich ++){
				for(ky = 0; ky < KY; ky++){
					for(kx = 0; kx < KX; kx++){
						DDR_BUF_weight[buf_address] = weight[och][ich][ky][kx];
						buf_address++;
						//printf("DDR_BUF_weight[%d] : %d , weight_ps[%d] : %d \n",i,DDR_BUF_weight[l*fmap_size+i],i,weight_ps[l][i]);
					}
				}
			}
			DDR_BUF_bias[och] = bias[och];
		}

		// DDR_BUF Combine/.
		int i;
		for(i = 0; i < fmap_size; i++)
		{
			DDR_BUF[i] = DDR_BUF_fmap[i];
		}
		for(i = 0; i < weight_size; i++)
		{
			DDR_BUF[i+fmap_size] = DDR_BUF_weight[i];
		}
		for(i =0 ; i < bias_size; i++)
		{
			DDR_BUF[i+(fmap_size+weight_size)] = DDR_BUF_bias[i];
		}
		for(i =0 ; i < 112; i++)
		{
			printf("DDR_BUF[%d] : %x \n" , i,DDR_BUF[i]);
		}

		printf("DATA combine done \n");
		printf("init done\n");
		printf("============================================================\n");

//////////////////////////////////////////////////////////////// DMA TRANSFER///////////////////////////////////////////////////////////////////////
			xil_printf("============ DMA DATA transfer start ============\n\n");

			XAxiCdma_Config *CDMA_Config;
			XAxiCdma *CDMA_Init;
			//============DDR_BUF => BRAM Data transfer
			int status;
			int Interrupt;
			CDMA_Config = XAxiCdma_LookupConfig(XPAR_AXI_CDMA_0_DEVICE_ID);
			status = XAxiCdma_CfgInitialize(&CDMA_Init, CDMA_Config , CDMA_Config->BaseAddress );
			if (status){
				printf("CDMA_Init Error\n ");
				return 0;
			}
			printf("CDMA_Init Done \n ");
			int *cdma_src;
			int *cdma_dsc;
			cdma_src = (u32*)DDR_BUF;
			cdma_dsc = XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR;
			printf("Bram Address : %08x  \n" , cdma_dsc );
			printf("DDR Address  : %08x  \n" , cdma_src );

			Xil_DCacheFlushRange((INTPTR)&DDR_BUF,DDR_SIZE);
			printf("Data Transfer ongoing...\n ");
			XTime_GetTime(&tStart); // DMA Data trasfer runtime measurement
			status = XAxiCdma_SimpleTransfer( &CDMA_Init ,
					(u32) cdma_src,
					(u32) cdma_dsc,
					(u32)((fmap_size+weight_size+bias_size)*sizeof(DDR_BUF)),
					Callback,
					(void *)&CDMA_Init
					);
			XTime_GetTime(&tEnd);
			if (status == XST_FAILURE){
				printf("CDMA ERROR \n");
				printf("CDMA Engine is busy or Simple transfer ongoing \n");
				return 0;
			}
			if (status == XST_INVALID_PARAM){
				printf("Length out of valid range [1: XAXICDMA_MAX_TRANSFER_LEN]  \n");
				return 0;
			}
			status = XAxiCdma_GetError(&CDMA_Init);
			if (status != 0x00){
				XAxiCdma_Reset(&CDMA_Init);
				printf("Error Code : %#x  \n" , status);
				return 0;
			}
			XAxiCdma_IntrEnable(&CDMA_Init, XAXICDMA_XR_IRQ_ALL_MASK);
			Interrupt = XAxiCdma_IntrGetEnabled(&CDMA_Init);
			printf("Interrupt Code : %#x \n" , Interrupt);
			xil_printf("DMA DATA transfer end (DDR_BUF => BRAM Data transfer Done)\n");

			double dma_run_time;
			XTime dma_cycle;
			dma_cycle = 2*(tEnd - tStart);
			dma_run_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
			printf("dma took %llu clock cycles.\n", dma_cycle);
			printf("dma took %.2f us.\n", dma_run_time);

/////////////////////////////////////////	AXI Master Data Read	////////////////////////////////////
			signed int fmap_buf;
			signed int fmap_bram;
			signed int weight_buf;
			signed int weight_bram;
			signed int bias_buf;
			signed int bias_bram;

			int f_read_done ;
			int w_read_done ;
			int b_read_done ;

			XTime_GetTime(&tStart);  //AXI Master Read time measurement

			Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR+ F_DONE), (u32) 0);
			Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + M0_INIT), (u32) 1);
			Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + F_ENABLE), (u32) 1);
			while(1){
				f_read_done = (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR+F_READ_DONE));
				if (f_read_done==1){
					Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR+ F_ENABLE), (u32) 0);
					Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR+ F_DONE), (u32) 1);
					Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + M0_INIT), (u32) 0);
					break;
				}
			}
			XTime_GetTime(&tEnd);

			printf("data_read_done : %d\n" ,f_read_done );
			printf("===========data transfer done====================\n");

			double bram_transfer_time;
			XTime bram_transfer_cycle;
			bram_transfer_cycle = 2*(tEnd - tStart);
			bram_transfer_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
			printf("bram_transfer took %llu clock cycles.\n", bram_transfer_cycle);
			printf("bram_transfer took %.2f us.\n", bram_transfer_time);
			printf("\n\n");

/////////////////////////////////////////	CNN PL Run	////////////////////////////////////

			XTime_GetTime(&tStart);
				Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CORE_RUN_ADDR), (u32) 1); // CNN run

				while(1) {
					val = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_DONE));
					if(val == 1)
						break;
				}
			XTime_GetTime(&tEnd);

			ref_v_run_cycle = 2*(tEnd - tStart);
			ref_v_run_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
			printf("[RTL_V] Output took %llu clock cycles.\n", ref_v_run_cycle);
			printf("[RTL_V] Output took %.2f us.\n", ref_v_run_time);

			result_0_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_RESULT_0));
			result_1_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_RESULT_1));
			result_2_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_RESULT_2));
			result_3_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_RESULT_3));
			result_4_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_RESULT_4));
			result_5_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_RESULT_5));
			result_6_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_RESULT_6));
			result_7_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + CNN_RESULT_7));

			printf("PL VS PS \n");
			printf("[PL] result[0] : %08X | [PS] result[0] : %08X\n", result_0_rtl, Xil_In32((u32) (0x10000000+0*sizeof(int))));
			printf("[PL] result[1] : %08X | [PS] result[1] : %08X\n", result_1_rtl, Xil_In32((u32) (0x10000000+1*sizeof(int))));
			printf("[PL] result[2] : %08X | [PS] result[2] : %08X\n", result_2_rtl, Xil_In32((u32) (0x10000000+2*sizeof(int))));
			printf("[PL] result[3] : %08X | [PS] result[3] : %08X\n", result_3_rtl, Xil_In32((u32) (0x10000000+3*sizeof(int))));
			printf("[PL] result[4] : %08X | [PS] result[4] : %08X\n", result_4_rtl, Xil_In32((u32) (0x10000000+4*sizeof(int))));
			printf("[PL] result[5] : %08X | [PS] result[5] : %08X\n", result_5_rtl, Xil_In32((u32) (0x10000000+5*sizeof(int))));
			printf("[PL] result[6] : %08X | [PS] result[6] : %08X\n", result_6_rtl, Xil_In32((u32) (0x10000000+6*sizeof(int))));
			printf("[PL] result[7] : %08X | [PS] result[7] : %08X\n\n", result_7_rtl, Xil_In32((u32) (0x10000000+7*sizeof(int))));

//////////////////////////////////////	PS PL MISMATCH CHECK 	///////////////////////////////////////////////////// 

		if(result[0] != result_0_rtl) {
			printf("[Mismatch] result[0] : %d vs result_0_rtl : %d\n", result[0], result_0_rtl);
			print ("exit \r\n");
			return 0;
		}
		if(result[1] != result_1_rtl) {
			printf("[Mismatch] result[1] : %d vs result_1_rtl : %d\n", result[1], result_1_rtl);
			print ("exit \r\n");
			return 0;
		}
		if(result[2] != result_2_rtl) {
			printf("[Mismatch] result[2] : %d vs result_2_rtl : %d\n", result[2], result_2_rtl);
			print ("exit \r\n");
			return 0;
		}
		if(result[3] != result_3_rtl) {
			printf("[Mismatch] result[3] : %d vs result_3_rtl : %d\n", result[3], result_3_rtl);
			print ("exit \r\n");
			return 0;
		}
		if(result[4] != result_4_rtl) {
			printf("[Mismatch] result[4] : %d vs result_4_rtl : %d\n", result[4], result_4_rtl);
			print ("exit \r\n");
			return 0;
		}
		if(result[5] != result_5_rtl) {
			printf("[Mismatch] result[5] : %d vs result_5_rtl : %d\n", result[5], result_5_rtl);
			print ("exit \r\n");
			return 0;
		}
		if(result[6] != result_6_rtl) {
			printf("[Mismatch] result[6] : %d vs result_6_rtl : %d\n", result[6], result_6_rtl);
			print ("exit \r\n");
			return 0;
		}
		if(result[7] != result_7_rtl) {
			printf("[Mismatch] result[7] : %d vs result_7_rtl : %d\n", result[7], result_7_rtl);
			print ("exit \r\n");
			return 0;
		}
///////////////////////////////////////////		RESULT PRINT 	///////////////////////////////////////////////////////////
		double total_time;
		double data_move;
		total_time = ref_v_run_time+bram_transfer_time+dma_run_time;
		printf("[Match] REF_C vs RTL_V \n");
		double perf_ratio = ref_c_run_cycle / ref_v_run_cycle;
		printf("[Match] RTL_V is  %.2f times faster than REF_C  \n", perf_ratio);
		printf("[Match] The difference between RTL_V and REF_C is %.2f us.  \n\n", ref_c_run_time - ref_v_run_time);
		printf("===============================================================\n\n");
		printf("PS RUN TIME : %.2f us.  VS  PL RUM TIME : %.2f us.\n", ref_c_run_time,total_time);
		printf(" core_run_time : %.2f us. ,dma_run_time : %.2f us. , bram_transfer_time : %.2f us.\n"
				,ref_v_run_time, dma_run_time,bram_transfer_time);

///////////////////////////////////////MAX POOLING Run ///////////////////////////////////////////////////////////////
		Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR+ F_DONE), (u32) 0);
		Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + M0_INIT), (u32) 0);
		Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + M1_INIT), (u32) 1);
		Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + M_ENABLE), (u32) 1);
		Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + F_ENABLE), (u32) 1);
		while(1){
			int maxpool_done;
			maxpool_done = (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR + MAXPOOL_DONE));
			if (maxpool_done == 1)
				break;
		}

		printf("maxpool_result_0 : %x \n" , (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR
				+ MAXPOOL_RESULT_0)));
		printf("maxpool_result_1 : %x \n" , (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR
				+ MAXPOOL_RESULT_1)));
		printf("maxpool_result_2 : %x \n" , (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR
				+ MAXPOOL_RESULT_2)));
		printf("maxpool_result_3 : %x \n" , (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR
				+ MAXPOOL_RESULT_3)));
		printf("maxpool_result_4 : %x \n" , (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR
				+ MAXPOOL_RESULT_4)));
		printf("maxpool_result_5 : %x \n" , (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR
				+ MAXPOOL_RESULT_5)));
		printf("maxpool_result_6 : %x \n" , (int) Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO_0_BASEADDR
				+ MAXPOOL_RESULT_6)));
		printf("Test Done \n");
		break;
				
		case '2': // exit
				print ("exit \r\n");
				return 0;
		}
		print ("\r\n");
	}
}

static void Callback(void *CallBackRef, u32 IrqMask, int *IgnorePtr){

	printf("Done\n");
}
