
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
#define ICH 3
#define OCH 8

#define CORE_RUN_ADDR 	0x00
#define F_VAL_ADDR 		0x04
#define W_VAL_ADDR 		0x08
#define B_VAL_ADDR 		0x0C
#define CORE_DONE_ADDR 	0x10
#define RESULT_0_ADDR 	0x14
#define RESULT_ACC_ADDR 0x18
#define CNN_DATA    	0x1C
#define m_axi_en 		0x20
#define m_axi_error 	0x24
#define m_axi_done 		0x28

// 0x00 : Data_en 	   / [0]  data_en
// 0x04 : feature map  / [31:0] f_value
// 0x08 : weight 	   / [31:0] w_value
// 0x0C : bias 	       / [31:0] b_value
// 0x10 : result_en    / [0]  result_en
// 0x14 : result_0     / [31:0] result_0
// 0x18 : result_acc   / [31:0] result_acc



#define XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR 0x40000000
#define XPAR_PS7_DDR_0_S_AXI_BASEADDR 0x00100000

//#define XPAR_PS7_DDR_0_S_AXI_BASEADDR 0x00100000
//#define XPAR_PS7_DDR_0_S_AXI_HIGHADDR 0x3FFFFFFF

//#define DDR_FMAP_ADDR XPAR_PS7_DDR_0_S_AXI_BASEADDR //0x00100000
//#define DDR_WEIGHT_ADDR 0x00101200
//#define DDR_BIAS_ADDR 0x00106800

static void Callback(void *CallBackRef, u32 IrqMask, int *IgnorePtr);

int main()
{
	int inbyte_in;
	int val;
	signed int kx, ky; 					// Kernel
	signed int ich, och; 					// in / ouput channel
	signed int fmap [ICH][KY][KX]; 		// 16b
	signed int weight[OCH][ICH][KY][KX]; 	// 16b
	signed int bias [OCH]; 					// 16b
	signed int mac_result[OCH]; 			// 36b = 32 bit + 4bit ( log (KY*KX 9) ) + 2 bit ( log (ICH 3) )
	signed int result[OCH]; 				// 37b = 36 bit + 1bit (bias)
	signed int result_for_demo=0; 		// 41b = 37 bit + 4 b ( log (OCH 16) )

	int64_t result_0_rtl; 				// 37b = 22 bit + 1bit (bias)
	int64_t result_for_demo_rtl; 		// 41b = 23 bit + 4 b ( log (OCH 16) )

	signed int weight_rand_val = 0;
	signed int bias_rand_val = 0;
	unsigned int fmap_rand_val = 0;

	double ref_c_run_time;
	double ref_v_run_time;
	XTime ref_c_run_cycle;
	XTime ref_v_run_cycle;

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
			case '1': // Show all registers
				printf("====  CNN Core. seed_num %d ====\n",tStart);
				srand(tStart);
/////////////////// Random Gen /////////////////////////////
				//generated same value for input each param.
				weight_rand_val = rand()%256-128;
				bias_rand_val = rand()%256-128;
				fmap_rand_val = rand()%256;

				// Initial Setting fmap, weight, bias value.
				for (och = 0 ; och < OCH; och ++){
					for(ich = 0; ich < ICH; ich ++){
						for(ky = 0; ky < KY; ky++){
							for(kx = 0; kx < KX; kx++){
								if(och == 0) {
									fmap[ich][ky][kx] = fmap_rand_val;
								}
								weight[och][ich][ky][kx] = weight_rand_val;
							}
						}
					}
					bias[och] = bias_rand_val;
					mac_result[och] = 0;
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
				// added bias, no activation function
				for (och = 0 ; och < OCH; och ++){
					result[och] = mac_result[och] + bias[och];
					//printf("[och:%d] result : %d\n",och, result[och]);
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
				printf("[REF_C] result[0] : %d\n", result[0]);
				printf("[REF_C] result_acc_for_demo : %d\n", result_for_demo);

/////////////////// CNN Run in PS /////////////////////////////
				printf("============[RTL_V] CNN Run in PL .=============\n");
//				// Xil_Out32
//
//				Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR + F_VAL_ADDR), (u32) fmap_rand_val);
//				Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR + W_VAL_ADDR), (u32) weight_rand_val);
//				Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR + B_VAL_ADDR), (u32) bias_rand_val);
//
//				XTime_GetTime(&tStart);
//				Xil_Out32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR + CORE_RUN_ADDR), (u32) 1); // run
//
//				while(1) {
//					val = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR + CORE_DONE_ADDR));
//					if(val == 1)
//						break;
//				}
//				result_0_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR + RESULT_0_ADDR));
//				result_for_demo_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR + RESULT_ACC_ADDR));
//				XTime_GetTime(&tEnd);
//				ref_v_run_cycle = 2*(tEnd - tStart);
//				ref_v_run_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
//				printf("[RTL_V] Output took %llu clock cycles.\n", ref_v_run_cycle);
//				printf("[RTL_V] Output took %.2f us.\n", ref_v_run_time);
//				printf("[RTL_V] result[0] : %d\n", result_0_rtl);
//				printf("[RTL_V] result_acc_for_demo : %d\n", result_for_demo_rtl);


/////////////////////////////////// DMA TRANSFER///////////////////////////////////////////////////////////////////////
				printf("\n\n");
				xil_printf("DMA DATA transfer start\n");


				XAxiCdma_Config *CDMA_Config;
				XAxiCdma *CDMA_Init;
				int status;
	//			XAxiCdma_CallBackFn *CallbackFn;
				volatile static u32 DDR_BUF[2*4] __attribute__ ((aligned (64)));
				int DDR_SIZE = sizeof(DDR_BUF);


				printf("============fmap , weight, bias init============ \n\n");

				for (int i=0;i<3;i++){
					fmap_rand_val = rand()%256;
					DDR_BUF[i] = fmap_rand_val;
				}


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
				u32 *cdma_src_weight;
				u32 *cdma_src_bias;

				cdma_src = (u32*)DDR_BUF;
				printf("ddr_buf addr : %#x\n\n",DDR_BUF);
				cdma_dsc = XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR;

//				cdma_src_weight = DDR_WEIGHT_ADDR;
//				cdma_src_bias = DDR_BIAS_ADDR;

				printf("Bram Address : %#x  \n" , cdma_dsc );
				printf("DDR Address  : %#x  \n" , cdma_src );

				printf("Previous ddr , bram data \n\n");
				for (int i = 0; i < 10; i++){
				printf(" DDR value : %#x  , Bram value : %#x\n" ,((u32)*(cdma_src+i)),((u32)*(cdma_dsc+i)));}

				Xil_DCacheFlushRange((INTPTR)&DDR_BUF,DDR_SIZE);
				printf("Data Transfer ongoing...\n ");
				XTime_GetTime(&tStart);
				status = XAxiCdma_SimpleTransfer( &CDMA_Init ,
						(u32) cdma_src,
						(u32) cdma_dsc,
						(u32)(6*sizeof(cdma_src)),
						Callback,
						(void *)&CDMA_Init);
				XTime_GetTime(&tEnd);
				ref_v_run_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
				printf("Take time : %.2f us.\n", ref_v_run_time);
//				status = XAxiCdma_SimpleTransfer( &CDMA_Init , (u32) cdma_src_weight,
//						(u32) cdma_dsc_weight,
//						(u32)BRAM_SIZE,
//						CallbackFn,
//						(void *)&CDMA_Init);
//
//				status = XAxiCdma_SimpleTransfer( &CDMA_Init , (u32) cdma_src_bias,
//						(u32) cdma_dsc_bias,
//						(u32)BRAM_SIZE,
//						CallbackFn,
//						(void *)&CDMA_Init);
				//u32 XAxiCdma_SimpleTransfer(XAxiCdma *InstancePtr, UINTPTR SrcAddr, UINTPTR DstAddr,int Length, XAxiCdma_CallBackFn SimpleCallBack, void *CallBackRef)
				//typedef void (*XAxiCdma_CallBackFn)(void *CallBackRef, u32 IrqMask,int *NumBdPtr);

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

				xil_printf("DMA DATA transfer end\n");

				printf("======================================Data Check \n");
				for (int i = 0; i < 10; i++){
				printf(" DDR value : %#x  , Bram value : %#x\n" ,((u32)*(cdma_src+i)),((u32)*(cdma_dsc+i)));
				}

				printf("\n\n");

				printf("======================================Data check finish!\n\n");

				printf("======================================Data Transfer To Cnn Core.\n\n");

//				int *cnn_buf __attribute__ ((aligned (64)));
//				cnn_buf = XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR;
                int m_axi_done_val;
                int m_axi_error_val;
                int m_axi_enable_sig;
                int m_axi_enable_val;


				m_axi_enable_sig = 0;
                Xil_Out32((u32)(XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR + m_axi_en), (u32)(m_axi_enable_sig));
                m_axi_enable_val = Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR+m_axi_en));
                m_axi_done_val = Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR+m_axi_done));
                m_axi_error_val = Xil_In32((u32)(XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR+m_axi_error));
                printf("m_axi read enable code code : %#x\n" , m_axi_enable_val);
                printf("m_axi read done code : %#x \n" , m_axi_done_val);
                printf("m_axi read error code : %#x\n" , m_axi_error_val);

                int (*CNN_VALUE)[3];
                int *ptr = NULL;
                CNN_VALUE = XPAR_CNN_CORE_TEST_CI3_CO32_V1_0_BASEADDR;
                ptr = &CNN_VALUE;
                XTime_GetTime(&tStart);
                  for (int i=0;i<3;i++){
                	  *(ptr+i) = Xil_In32((u32)(cdma_dsc+i));
                  }
                XTime_GetTime(&tEnd);
				ref_v_run_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
				printf("Take time : %.2f us.\n", ref_v_run_time);
                for (int i=0;i<3;i++){
                	printf("bram address : %#x , bram value : %#x\n", cdma_dsc+i, ((u32)*(cdma_dsc+i)));
                }

                //                while(1){
//                    if (m_axi_done_val == 1){
//                        printf("m_axi read done\n");
//                        break;
//                    }
//                    else if (m_axi_error_val){
//                        printf("m_axi read error code : %#x..\n" , m_axi_error_val);
//                    }
//                    else {
//                        printf("m_axi read onging..\n");
//                    }
//                }

//				status = XAxiCdma_SimpleTransfer( &CDMA_Init ,
//						(u32) cdma_dsc,
//						(u32) cnn_dsc,
//						(u32)(6*sizeof(cdma_src)),
//						Callback,
//						(void *)&CDMA_Init);
                for(int i=0;i<3;i++){
				printf("CNN_DATA address : %#x , CNN_DATA value : %#x\n" ,CNN_VALUE+i, CNN_VALUE[i]);
                }


				return 0;
				// 0x00 : Data_en 	   / [0]  data_en
				// 0x04 : feature map  / [31:0] f_value
				// 0x08 : weight 	   / [31:0] w_value
				// 0x0C : bias 	       / [31:0] b_value
				// 0x10 : result_en    / [0]  result_en
				// 0x14 : result_0     / [31:0] result_0
				// 0x18 : result_acc   / [31:0] result_acc
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				if(result[0] != result_0_rtl) {
					printf("[Mismatch] result[0] : %d vs result_0_rtl : %d\n", result[0], result_0_rtl);
					print ("exit \r\n");
					return 0;
				}
				if(result_for_demo != result_for_demo_rtl) {
					printf("[Mismatch] result_for_demo : %d vs result_for_demo_rtl : %d\n", result_for_demo, result_for_demo_rtl);
					print ("exit \r\n");
					return 0;
				}
				printf("[Match] REF_C vs RTL_V \n");
				double perf_ratio = ref_c_run_cycle / ref_v_run_cycle;
				printf("[Match] RTL_V is  %.2f times faster than REF_C  \n", perf_ratio);
				printf("[Match] The difference between RTL_V and REF_C is %.2f us.  \n", ref_c_run_time - ref_v_run_time);
				break;
			case '2': // exit
				print ("exit \r\n");
				return 0;
		}
		print ("\r\n");
	}
    return 0;
}

static void Callback(void *CallBackRef, u32 IrqMask, int *IgnorePtr){

	printf("Done\n");
}
