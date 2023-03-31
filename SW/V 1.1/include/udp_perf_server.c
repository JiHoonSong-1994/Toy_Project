/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

/** Connection handle for a UDP Server session */
/*
 * cnn header file
 */
#include <stdio.h>
#include "xil_types.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xtime_l.h"
#include "xaxicdma.h"
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform.h"
#include "platform_config.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "sleep.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "xil_cache.h"
#include "xgpio.h"
/*
 *
 */
/*
 * cnn define
 */
#include "lwip/ip_addr.h"
#define KX 3
#define KY 3
#define ICH 4
#define OCH 8

#define CORE_RUN_ADDR 	0x00 
#define F_ENABLE 		0x04
#define F_READ_DONE		0x08
#define F_VALUE 		0x0C
#define F_DONE 			0x10
#define W_ENABLE 		0x14
#define W_READ_DONE		0x18
#define W_VALUE 		0x1C
#define W_DONE 			0x20
#define B_ENABLE 		0x24
#define B_READ_DONE		0x28
#define B_VALUE 		0x2C
#define B_DONE 			0x30 
#define M_ENABLE 		0x34
#define M_ADDRESS 		0x38
#define M_VALUE 		0x3C
#define M_DONE			0x40 
#define CNN_RESULT_0	0x44 
#define CNN_RESULT_1	0x48
#define CNN_DONE		0x4C
#define CNN_RESULT_2	0x50
#define CNN_RESULT_3	0x54
#define CNN_RESULT_4	0x58
#define CNN_RESULT_5	0x5C
#define CNN_RESULT_6	0x60
#define CNN_RESULT_7	0x64 
#define M0_INIT 		0x68 
#define M1_INIT 		0x6C 
#define M2_INIT 		0x70 




#include "udp_perf_server.h"
#include "lwip/udp.h"
#include "xparameters.h"
#include "xil_io.h"
#include "udp_perf_client.h"
#include "string.h"

XAxiCdma CDMA_Init;

extern struct netif server_netif;
static struct udp_pcb *pcb;
static struct perf_stats server;


static int send_buf[UDP_SEND_BUFSIZE];


static void Callback(void *CallBackRef, u32 IrqMask, int *IgnorePtr);

/* Report interval in ms */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 1000)

void print_app_header(void)
{
	xil_printf("UDP server listening on port %d\r\n",
			UDP_CONN_PORT);
	xil_printf("On Host: Run $iperf -c %s -i %d -t 300 -u -b <bandwidth>\r\n",
			inet_ntoa(server_netif.ip_addr),
			INTERIM_REPORT_INTERVAL);
	xil_printf("UDP client connecting to %s on port %d\r\n",
			UDP_SERVER_IP_ADDRESS, CLIENT_UDP_CONN_PORT);
	xil_printf("On Host: Run $iperf -s -i %d -u\r\n\r\n",
			INTERIM_REPORT_INTERVAL);
}

static void print_udp_conn_stats(void)
{
	xil_printf("[%3d] local %s port %d connected with ",
			server.client_id, inet_ntoa(server_netif.ip_addr),
			UDP_CONN_PORT);
	xil_printf("%s port %d\r\n", inet_ntoa(pcb->remote_ip),
			pcb->remote_port);
	xil_printf("[ ID] Interval\t     Transfer     Bandwidth\t");
	xil_printf("    Lost/Total Datagrams\n\r");
}

static void stats_buffer(char* outString,
		double data, enum measure_t type)
{
	int conv = KCONV_UNIT;
	const char *format;
	double unit = 1024.0;

	if (type == SPEED)
		unit = 1000.0;

	while (data >= unit && conv <= KCONV_GIGA) {
		data /= unit;
		conv++;
	}

	/* Fit data in 4 places */
	if (data < 9.995) { /* 9.995 rounded to 10.0 */
		format = "%4.2f %c"; /* #.## */
	} else if (data < 99.95) { /* 99.95 rounded to 100 */
		format = "%4.1f %c"; /* ##.# */
	} else {
		format = "%4.0f %c"; /* #### */
	}
	sprintf(outString, format, data, kLabel[conv]);
}


/** The report function of a TCP server session */
static void udp_conn_report(u64_t diff,
		enum report_type report_type)
{
	u64_t total_len, cnt_datagrams, cnt_dropped_datagrams, total_packets;
	u32_t cnt_out_of_order_datagrams;
	double duration, bandwidth = 0;
	char data[16], perf[16], time[64], drop[64];

	if (report_type == INTER_REPORT) {
		total_len = server.i_report.total_bytes;
		cnt_datagrams = server.i_report.cnt_datagrams;
		cnt_dropped_datagrams = server.i_report.cnt_dropped_datagrams;
	} else {
		server.i_report.last_report_time = 0;
		total_len = server.total_bytes;
		cnt_datagrams = server.cnt_datagrams;
		cnt_dropped_datagrams = server.cnt_dropped_datagrams;
		cnt_out_of_order_datagrams = server.cnt_out_of_order_datagrams;
	}

	total_packets = cnt_datagrams + cnt_dropped_datagrams;
	/* Converting duration from milliseconds to secs,
	 * and bandwidth to bits/sec .
	 */
	duration = diff / 1000.0; /* secs */
	if (duration)
		bandwidth = (total_len / duration) * 8.0;

	stats_buffer(data, total_len, BYTES);
	stats_buffer(perf, bandwidth, SPEED);
	/* On 32-bit platforms, xil_printf is not able to print
	 * u64_t values, so converting these values in strings and
	 * displaying results
	 */
	sprintf(time, "%4.1f-%4.1f sec",
			(double)server.i_report.last_report_time,
			(double)(server.i_report.last_report_time + duration));
	sprintf(drop, "%4llu/%5llu (%.2g%%)", cnt_dropped_datagrams,
			total_packets,
			(100.0 * cnt_dropped_datagrams)/total_packets);
	xil_printf("[%3d] %s  %sBytes  %sbits/sec  %s\n\r", server.client_id,
			time, data, perf, drop);

	if (report_type == INTER_REPORT) {
		server.i_report.last_report_time += duration;
	} else if ((report_type != INTER_REPORT) && cnt_out_of_order_datagrams) {
		xil_printf("[%3d] %s  %u datagrams received out-of-order\n\r",
				server.client_id, time,
				cnt_out_of_order_datagrams);
	}
}


static void reset_stats(void)
{
	server.client_id++;
	/* Save start time */
	server.start_time = get_time_ms();
	server.end_time = 0; /* ms */
	server.total_bytes = 0;
	server.cnt_datagrams = 0;
	server.cnt_dropped_datagrams = 0;
	server.cnt_out_of_order_datagrams = 0;
	server.expected_datagram_id = 0;

	/* Initialize Interim report parameters */
	server.i_report.start_time = 0;
	server.i_report.total_bytes = 0;
	server.i_report.cnt_datagrams = 0;
	server.i_report.cnt_dropped_datagrams = 0;
	server.i_report.last_report_time = 0;
}

/** Receive data on a udp session */
static void udp_recv_perf_traffic(void *arg, struct udp_pcb *tpcb,
		struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	static u8_t first = 1;
	u32_t drop_datagrams = 0;
	s32_t recv_id;

	/* first, check if the datagram is received in order */
#ifdef __MICROBLAZE__
	/* For Microblaze, word access are at 32 bit boundaries.
	 * To read complete 4 byte of UDP ID from data payload,
	 * we should read upper 2 bytes from current word boundary
	 * of payload and lower 2 bytes from next word boundary of
	 * payload.
	 */
	s16_t *payload;
	payload = (s16_t *) (p->payload);
	recv_id = (ntohs(payload[0]) << 16) | ntohs(payload[1]);

//	xil_printf("received packet length : %d \r\n" , p->len);

//	for (int i=0;i<(p->len)/4; i++){
//		xil_printf("%08x \n" , *(payload++));
//	}

#else
	recv_id = ntohl(*((int *)(p->payload)));
#endif
	if (first && (recv_id == 0)) {
		/* First packet should always start with recv id 0.
		 * However, If Iperf client is running with parallel
		 * thread, then this condition will also avoid
		 * multiple print of connection header
		 */
		pcb->remote_ip = *addr;
		pcb->remote_port = port;
		reset_stats();
		/* Print connection statistics */
		print_udp_conn_stats();
		first = 0;
	} else if (first) {
		/* Avoid rest of the packets if client
		 * connection is already terminated.
		 */
		return;
	}

	if (recv_id < 0) {
		u64_t now = get_time_ms();
		u64_t diff_ms = now - server.start_time;
		/* Send Ack */
		udp_sendto(tpcb, p, addr, port);
		udp_conn_report(diff_ms, UDP_DONE_SERVER);
		xil_printf("UDP test passed Successfully\n\r");
		first = 1;
		pbuf_free(p);
		return;
	}

	/* Update dropped datagrams statistics */
	if (server.expected_datagram_id != recv_id) {
		if (server.expected_datagram_id < recv_id) {
			drop_datagrams =
				recv_id - server.expected_datagram_id;
			server.cnt_dropped_datagrams += drop_datagrams;
			server.expected_datagram_id = recv_id + 1;
		} else if (server.expected_datagram_id > recv_id) {
			server.cnt_out_of_order_datagrams++;
		}
	} else {
		server.expected_datagram_id++;
	}

	server.cnt_datagrams++;

	/* Record total bytes for final report */
	server.total_bytes += p->tot_len;

	if (REPORT_INTERVAL_TIME) {
		u64_t now = get_time_ms();

		server.i_report.cnt_datagrams++;
		server.i_report.cnt_dropped_datagrams += drop_datagrams;

		/* Record total bytes for interim report */
		server.i_report.total_bytes += p->tot_len;
		if (server.i_report.start_time) {
			u64_t diff_ms = now - server.i_report.start_time;

			if (diff_ms >= REPORT_INTERVAL_TIME) {
				udp_conn_report(diff_ms, INTER_REPORT);
				/* Reset Interim report counters */
				server.i_report.start_time = 0;
				server.i_report.total_bytes = 0;
				server.i_report.cnt_datagrams = 0;
				server.i_report.cnt_dropped_datagrams = 0;
			}
		} else {
			/* Save start time for interim report */
			server.i_report.start_time = now;
		}
	}

	pbuf_free(p);
	return;
}

#define RX_BUF_SIZE 1024
int * int_to_bin(uint32_t num){
   uint32_t int_num = num;
  static int ddr_buf_1[4] = {0};
  ddr_buf_1[0] = int_num << 16;
  ddr_buf_1[0] = ddr_buf_1[0] << 8;
  ddr_buf_1[0] = ddr_buf_1[0] >> 24;

  ddr_buf_1[1] =  int_num << 16;
  ddr_buf_1[1] = ddr_buf_1[1] >> 24;

  ddr_buf_1[2] = int_num >> 16;
  ddr_buf_1[2] = ddr_buf_1[2] << 24;
  ddr_buf_1[2] = ddr_buf_1[2] >> 24;

  ddr_buf_1[3] = int_num >> 16;
  ddr_buf_1[3] = ddr_buf_1[3] >> 8;

  return  ddr_buf_1;
}
int * cnn_core()
{


	int val;
	XTime tStart, tEnd;
	signed short mac_result[8] = {0}; 			
	signed short result[8] = {0}; 
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

	Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + M0_INIT), (u32) 0);
	Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + M1_INIT), (u32) 0);
	Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + M2_INIT), (u32) 0);

	unsigned char fmap [3*3*4];
	signed char weight[8][3*3*4];
	signed char bias [8];



	// Initial Setting fmap, weight, bias value.
	int DDR_BUF[(3*3)+(3*3*8)+2] ;
	int DDR_SIZE = sizeof(DDR_BUF);


	for(int ich = 0; ich < 3*3*4; ich ++){
			fmap[ich] = Xil_In32(XPAR_PS7_DDR_0_S_AXI_BASEADDR +sizeof(char)*(ich));
			//printf("fmap[%d] : %X\n" ,ich,fmap[ich]);
	}

	printf("\n\n");

	for (int i=0;i<8;i++){
		for (int och = 0 ; och <3*3*4 ; och ++){
					weight[i][och] = Xil_In32(XPAR_PS7_DDR_0_S_AXI_BASEADDR +sizeof(char)*36 + sizeof(char)*36*i + sizeof(char)*(och));
					//printf("weight[%d][%d] : %X\n" ,i,weight[i][och]);
		}
	}

	for (int och = 0 ; och < 2*4; och ++){
		bias[och] = Xil_In32(XPAR_PS7_DDR_0_S_AXI_BASEADDR +sizeof(char)*(3*3*4) + sizeof(char)*(3*3*4*8) + sizeof(char)*(och));
		//printf("bias[%d] : %X\n" , och,bias[och]);
	}

	for (int och = 0 ; och < 8; och ++){
		for(int ich = 0; ich < 3*3*4; ich ++){
					mac_result[och] += (fmap[ich] * weight[och][ich]);
					//printf("%d , mac_result : %d\n" , (fmap[ich] * weight[och][ich]),mac_result[och] );
				}
		mac_result[och] = mac_result[och] + bias[och];
			}

	// added bias,  activation function ReLu
	for (int och = 0 ; och < 8; och ++){
			printf("[och:%d] PS CNN result : %d\n",och, mac_result[och]);
	}

	/*
	 * DDR Data print
	 */
	for (int i=0;i<(3*3+3*3*8+2);i++){
		DDR_BUF[i] = Xil_In32(XPAR_PS7_DDR_0_S_AXI_BASEADDR +sizeof(int)*(i));

	}


    				printf("DATA combine done \n");
    				printf("init done\n");

    				//////////////////////////////////////////////////////////////// DMA TRANSFER///////////////////////////////////////////////////////////////////////
    							xil_printf("============ DMA DATA TRANSFER START ============\n\n");

    							XAxiCdma_Config *CDMA_Config;

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
    							cdma_src = (int*)DDR_BUF;
    							cdma_dsc = (int*)XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR;
    				//			printf("Bram Address : %08x  \n" , cdma_dsc );
    				//			printf("DDR Address  : %08x  \n" , cdma_src );

    							Xil_DCacheFlushRange((INTPTR)&DDR_BUF,DDR_SIZE);
    							printf("Data Transfer ongoing...\n ");
    							XTime_GetTime(&tStart);
    							status = XAxiCdma_SimpleTransfer( &CDMA_Init ,
    									(u32) cdma_src,
    									(u32) cdma_dsc,
    									(u32)(((3*3)+(3*3*8)+2)*sizeof(DDR_BUF)),
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

    							/*
    							 *  bram print
    							 */

//    							for (int i=0;i<3*3+3*3*8+2;i++){
//    								printf("bram data : %08x \n", Xil_In32(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR +sizeof(int)*i));
//    							}

    							/////////////////////////////////////////////////////////////////////////////

    							int f_read_done ;
    							int w_read_done ;
    							int b_read_done ;

    							XTime_GetTime(&tStart);
    										Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR+ F_DONE), (u32) 0);
    										Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + M0_INIT), (u32) 1);
    										Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + F_ENABLE), (u32) 1);
    										while(1){
    											f_read_done = (int) Xil_In32((u32)(XPAR_CNN_CORE_0_BASEADDR+F_READ_DONE));
    											if (f_read_done==1){
    												XTime_GetTime(&tEnd);
    												Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR+ F_ENABLE), (u32) 0);
    												Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR+ F_DONE), (u32) 1);
    												Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + M0_INIT), (u32) 0);
    												break;
    											}
    										}
    										double bram_transfer_time;
    										XTime bram_transfer_cycle;
    										bram_transfer_cycle = 2*(tEnd - tStart);
    										bram_transfer_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
    										printf("bram_transfer took %llu clock cycles.\n", bram_transfer_cycle);
    										printf("bram_transfer took %.2f us.\n", bram_transfer_time);
    										printf("DATA_READ_DONE \n" );
    										printf("=========== DMA DATA TRANSFER DONE ====================\n\n");

    										printf("=========== CNN CORE RUN ====================\n");

    													printf("cnn core running....\n");
    													XTime_GetTime(&tStart);
    														Xil_Out32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CORE_RUN_ADDR), (u32) 1); // run

    														while(1) {
    															val = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_DONE));
    															if(val == 1)
    																break;
    														}
    													XTime_GetTime(&tEnd);
    													printf("=========== CNN CORE DONE ====================\n\n");

    													ref_v_run_cycle = 2*(tEnd - tStart);
    																ref_v_run_time = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
    																printf("[RTL_V] Output took %llu clock cycles.\n", ref_v_run_cycle);
    																printf("[RTL_V] Output took %.2f us.\n\n", ref_v_run_time);

    																result_0_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_RESULT_0));
    																result_1_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_RESULT_1));
    																result_2_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_RESULT_2));
    																result_3_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_RESULT_3));
    																result_4_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_RESULT_4));
    																result_5_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_RESULT_5));
    																result_6_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_RESULT_6));
    																result_7_rtl = (int) Xil_In32 ((u32) (XPAR_CNN_CORE_0_BASEADDR + CNN_RESULT_7));

    																printf("PL VS PS MISMATCH CHECK\n");
    																printf("[PL] result[0] : %08X , %d\n", result_0_rtl,result_0_rtl);//,mac_result[0]);// Xil_In32((u32) (0x10000000+0*sizeof(int))));
    																printf("[PL] result[1] : %08X , %d\n", result_1_rtl,result_1_rtl);//,mac_result[1]);// Xil_In32((u32) (0x10000000+1*sizeof(int))));
    																printf("[PL] result[2] : %08X , %d\n", result_2_rtl,result_2_rtl);//,mac_result[2]);// Xil_In32((u32) (0x10000000+2*sizeof(int))));
    																printf("[PL] result[3] : %08X , %d\n", result_3_rtl,result_3_rtl);//,mac_result[3]);// Xil_In32((u32) (0x10000000+3*sizeof(int))));
    																printf("[PL] result[4] : %08X , %d\n", result_4_rtl,result_4_rtl);//,mac_result[4]);// Xil_In32((u32) (0x10000000+4*sizeof(int))));
    																printf("[PL] result[5] : %08X , %d\n", result_5_rtl,result_5_rtl);//,mac_result[5]);// Xil_In32((u32) (0x10000000+5*sizeof(int))));
    																printf("[PL] result[6] : %08X , %d\n", result_6_rtl,result_6_rtl);//,mac_result[6]);// Xil_In32((u32) (0x10000000+6*sizeof(int))));
    																printf("[PL] result[7] : %08X , %d\n\n", result_7_rtl,result_7_rtl);//,mac_result[7]);// Xil_In32((u32) (0x10000000+7*sizeof(int))));
    													
    													//////////////////////////////////////////////////////////////////////////////////////////////////////
    															double total_time;
    															double data_move;
    															total_time = ref_v_run_time+bram_transfer_time+dma_run_time;
    															printf("=========== RESULT =================== \n");
    															printf("[Match] REF_C vs RTL_V \n");
    															double perf_ratio = ref_c_run_cycle / ref_v_run_cycle;
    															printf("[Match] RTL_V is  %.2f times faster than REF_C  \n", perf_ratio);
    															printf("[Match] The difference between RTL_V and REF_C is %.2f us.  \n\n", ref_c_run_time - ref_v_run_time);
    															printf("=========== RUN TIME =================== \n");
    															printf("PS CORE RUN TIME : %.2f us.  VS  PL CORE RUM TIME : %.2f us.\n", ref_c_run_time,ref_v_run_time);
    															printf(" core_run_time : %.2f us. ,dma_run_time : %.2f us. , bram_to cnn_core data transfer_time : %.2f us.\n"
    																	,ref_v_run_time, dma_run_time,bram_transfer_time);



																static int CNN_CORE_result[8];
																CNN_CORE_result[0] = result_0_rtl;
																CNN_CORE_result[1] = result_1_rtl;
																CNN_CORE_result[2] = result_2_rtl;
																CNN_CORE_result[3] = result_3_rtl;
																CNN_CORE_result[4] = result_4_rtl;
																CNN_CORE_result[5] = result_5_rtl;
																CNN_CORE_result[6] = result_6_rtl;
																CNN_CORE_result[7] = result_7_rtl;
				return CNN_CORE_result;
}

void udp_recv_callback1(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    char rx_buf[RX_BUF_SIZE];
    char add_buf[10];
    int rx_buf_int[RX_BUF_SIZE];
    struct pbuf *packet;
    int packet_buf_size;
    err_t err;
    int rx_len = p->len > RX_BUF_SIZE ? RX_BUF_SIZE : p->len;


    // Copy received data to rx_buf
    memcpy(rx_buf, p->payload, rx_len);
    memcpy(rx_buf_int, p->payload, rx_len);
    memcpy(send_buf ,p->payload, rx_len);

    // Add null-terminator to rx_buf
    rx_buf[rx_len] = '\0';
    rx_buf_int[rx_len] = '\0';
    send_buf[UDP_SEND_BUFSIZE] = '\0';

    for (int i=0; i<rx_len; i++){
    	Xil_Out32(XPAR_PS7_DDR_0_S_AXI_BASEADDR +sizeof(int)*i , rx_buf_int[i]);
    }

    ///////////////////////////////////////////////////// //////////////////Print received data

    xil_printf("\n");
    xil_printf("received Data :");
    for (int i=0; i < 3 ; i++){
    	xil_printf("%x  " ,rx_buf_int[i]);
    }
    xil_printf("\n");
    xil_printf("ddr memry data :");
    for (int i=0 ; i<10; i++){
    	xil_printf("%x   " , Xil_In32(XPAR_PS7_DDR_0_S_AXI_BASEADDR +sizeof(int)*i) );
    }
    ////////////////////////////////////////////////////////////////////
    XGpio dip;
    int dip_check;

    XGpio_Initialize(&dip,XPAR_XGPIOPS_0_DEVICE_ID );
    XGpio_SetDataDirection(&dip, 1, 0xffffffff);
    dip_check = 0;

    printf("allow to Move Switch \n");
    while(!dip_check){
    	dip_check = XGpio_DiscreteRead(&dip, 1);
    	//printf("select mode : %d \n" , dip_check );
    	//sleep(1);
    }

    signed int* cnn_result;
    if (dip_check == 1){
    	cnn_result = cnn_core();
		for (int i=0;i<8;i++){
			printf("cnn result : %08X \n" , cnn_result[i]);
		}
    }


    for (int i=0; i<8; i++){
    	 send_buf[i] =  (int*)(cnn_result[i]);
    }
//    /////////////////////////////////////////////////////
//
//
    xil_printf("\n");
    for (int i=0;i<8;i++){
    xil_printf("send buf data : %08X \n", send_buf[i]);
    }
    packet_buf_size = sizeof(send_buf);
    xil_printf("send buf lenthg : %d\n" , packet_buf_size);

//    /////////////////////////// /////////////////////////////////////////////packet send part
    packet = pbuf_alloc(PBUF_TRANSPORT, packet_buf_size, PBUF_POOL);
	pbuf_take(packet, (int*)send_buf, packet_buf_size);

	ip_addr_t remote_addr_1;


	// ip_addr_t value assign "xxx.xxx.xxx.xxx" value.
	inet_aton("169.254.83.194", &remote_addr_1);

	// pcb assign new ip value remote_ip.
	pcb->remote_ip = remote_addr_1;

	err = udp_send(pcb, packet);
	if (err != ERR_OK) {
					xil_printf("Error on udp_send: %d\r\n", err);
					usleep(100);
				} else {

					xil_printf("udp send no error \n");

				}
	//////////////////////////////////////////////////////
	///// send_buf clear part

    pbuf_free(p);
    pbuf_free(packet);
    printf("cnn done\n");
}


void start_application(void)
{
	err_t err;

	/* Create Server PCB */
	pcb = udp_new();
	if (!pcb) {
		xil_printf("UDP server: Error creating PCB. Out of Memory\r\n");
		return;
	}

	err = udp_bind(pcb, IP_ADDR_ANY, UDP_CONN_PORT);
	if (err != ERR_OK) {
		xil_printf("UDP server: Unable to bind to port");
		xil_printf(" %d: err = %d\r\n", UDP_CONN_PORT, err);
		udp_remove(pcb);
		return;
	}

	/* specify callback to use for incoming connections */
	udp_recv(pcb, udp_recv_perf_traffic, NULL);

	udp_recv(pcb, (udp_recv_fn)udp_recv_callback1, NULL);

	return;
}

static void Callback(void *CallBackRef, u32 IrqMask, int *IgnorePtr){

	printf("Done\n");
}

