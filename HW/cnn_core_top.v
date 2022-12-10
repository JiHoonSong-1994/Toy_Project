/*******************************************************************************
Copyright (c) 2022  JiHoon Song, All Rights Reserved
AUTHOR: JiHoon Song 
AUTHOR'S EMAIL : jihoon20620@naver.com 
ASSOCIATED FILENAME : cnn_core.v
REVISION HISTORY : December 10, 2022 - initial release
*******************************************************************************/

`timescale 1 ns / 1 ps
`include "defines_cnn_core.vh"

	module cnn_core_top #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 7,
		
		// Parameters of Axi Master Bus Interface M00_AXI
		parameter  C_M00_AXI_START_DATA_VALUE	= 32'h00000000,
		parameter  C_M00_AXI_TARGET_SLAVE_BASE_ADDR	= 32'h42000000,
		parameter integer C_M00_AXI_ADDR_WIDTH	= 32,
		parameter integer C_M00_AXI_DATA_WIDTH	= 32,
		parameter integer C_M00_AXI_TRANSACTIONS_NUM	= 3*3*4 + 3*3*4*8 + 8,

		 // Parameters of Axi Master Bus Interface M01_AXI
		 parameter  C_M01_AXI_START_DATA_VALUE	= 32'h00000000,
		 parameter  C_M01_AXI_TARGET_SLAVE_BASE_ADDR	= 32'h42000000,
		 parameter integer C_M01_AXI_ADDR_WIDTH	= 32,
		 parameter integer C_M01_AXI_DATA_WIDTH	= 32,
		 parameter integer C_M01_AXI_TRANSACTIONS_NUM	= 7*4*2*2 + 1


	)
	(
		// Users to add ports here

		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S00_AXI
		input wire  s00_axi_aclk,
		input wire  s00_axi_aresetn,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [2 : 0] s00_axi_awprot,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [1 : 0] s00_axi_bresp,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [2 : 0] s00_axi_arprot,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready,

				// Ports of Axi Master Bus Interface M00_AXI
//		input wire  m00_axi_init_axi_txn,
//		output wire  m00_axi_error,
//		output wire  m00_axi_txn_done,
		input wire  m00_axi_aclk,
		input wire  m00_axi_aresetn,
		output wire [C_M00_AXI_ADDR_WIDTH-1 : 0] m00_axi_awaddr,
		output wire [2 : 0] m00_axi_awprot,
		output wire  m00_axi_awvalid,
		input wire  m00_axi_awready,
		output wire [C_M00_AXI_DATA_WIDTH-1 : 0] m00_axi_wdata,
		output wire [C_M00_AXI_DATA_WIDTH/8-1 : 0] m00_axi_wstrb,
		output wire  m00_axi_wvalid,
		input wire  m00_axi_wready,
		input wire [1 : 0] m00_axi_bresp,
		input wire  m00_axi_bvalid,
		output wire  m00_axi_bready,
		
		output wire [C_M00_AXI_ADDR_WIDTH-1 : 0] m00_axi_araddr,
		output wire [2 : 0] m00_axi_arprot,
		output wire  m00_axi_arvalid,
		input wire  m00_axi_arready,
		input wire [C_M00_AXI_DATA_WIDTH-1 : 0] m00_axi_rdata,
		input wire [1 : 0] m00_axi_rresp,
		input wire  m00_axi_rvalid,
		output wire  m00_axi_rready,

 		// Ports of Axi Master Bus Interface M01_AXI
 //		input wire  m01_axi_init_axi_txn,
 //		output wire  m01_axi_error,
 //		output wire  m01_axi_txn_done,
 		input wire  m01_axi_aclk,
 		input wire  m01_axi_aresetn,
 		output wire [C_M01_AXI_ADDR_WIDTH-1 : 0] m01_axi_awaddr,
 		output wire [2 : 0] m01_axi_awprot,
 		output wire  m01_axi_awvalid,
 		input wire  m01_axi_awready,
 		output wire [C_M01_AXI_DATA_WIDTH-1 : 0] m01_axi_wdata,
 		output wire [C_M01_AXI_DATA_WIDTH/8-1 : 0] m01_axi_wstrb,
 		output wire  m01_axi_wvalid,
 		input wire  m01_axi_wready,
 		input wire [1 : 0] m01_axi_bresp,
 		input wire  m01_axi_bvalid,
 		output wire  m01_axi_bready,
 		output wire [C_M01_AXI_ADDR_WIDTH-1 : 0] m01_axi_araddr,
 		output wire [2 : 0] m01_axi_arprot,
 		output wire  m01_axi_arvalid,
 		input wire  m01_axi_arready,
 		input wire [C_M01_AXI_DATA_WIDTH-1 : 0] m01_axi_rdata,
 		input wire [1 : 0] m01_axi_rresp,
 		input wire  m01_axi_rvalid,
 		output wire  m01_axi_rready

	);

	wire                              			w_in_valid  		;
	wire  [CI*KX*KY*I_F_BW-1 : 0]  				w_in_fmap    		;
	wire 	signed [CO*CI*KX*KY*W_BW-1 : 0]		w_in_weight 		;
	wire 	signed [CO*B_BW-1 : 0]				w_in_bias 			;


	wire                              			w_ot_valid  		;
	wire [CO*O_F_BW-1 : 0]  					w_ot_fmap    		;

	wire [W_BW-1 : 0]  							w_w_value 			;
	wire [B_BW-1 : 0]  							w_b_value   		;

	reg  [CI*KX*KY*I_F_BW-1 : 0]  				in_fmap    			;

	wire [I_F_BW-1:0]							in_f_value			;
	wire  										in_f_read_done		;
	wire 										in_f_enable			;
	wire 										in_f_done			;

	wire 										w_data_en			;
	wire 										w_result_en			;

    wire [32-1:0] 								w_result_0			;
	wire [32-1:0] 								w_result_1			;
	wire [32-1:0] 								w_result_2			;
	wire [32-1:0] 								w_result_3			;
	wire [32-1:0] 								w_result_4			;
	wire [32-1:0] 								w_result_5			;
	wire [32-1:0] 								w_result_6			;
	wire [32-1:0] 								w_result_7			;

    wire [32-1:0] 								maxPool_Result_0	;
    wire [32-1:0] 								maxPool_Result_1	;
    wire [32-1:0] 								maxPool_Result_2	;
    wire [32-1:0] 								maxPool_Result_3	;
    wire [32-1:0] 								maxPool_Result_4	;
    wire [32-1:0] 								maxPool_Result_5	;
    wire [32-1:0] 								maxPool_Result_6	;
    
    wire 										maxPool_Done		;
    
    wire [O_F_ACC_BW-1:0] 						w_result_acc		;

	wire 										in_max_valid		;		
    wire 	[MAX_BW-1:0]						in_maxPool_value	;
	wire	[31:0]								in_maxPool_address	;
	wire										in_maxPool_enable	;
	wire										in_maxPool_done		;

	wire [M_CO*M_CI*2*2*MAX_BW-1 : 0]			out_maxPool			;
	wire										maxPool_data_done	;
    wire 										o_ot_max_valid		;
    wire [M_CO*M_CI*MAX_BW-1 : 0]				o_ot_maxPool_fmap	;

	wire m00_axi_init_axi_txn;
	wire m00_axi_error;
	wire m00_axi_txn_done;
	
	wire m01_axi_init_axi_txn;
	wire m01_axi_error;
	wire m01_axi_txn_done;
	
//==============================================================================
// AXI LITE
//==============================================================================
// Instantiation of Axi Bus Interface S00_AXI
	axi4_lite_test # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) u_axi4_lite_test (
		.S_AXI_ACLK(s00_axi_aclk),
		.S_AXI_ARESETN(s00_axi_aresetn),
		.S_AXI_AWADDR(s00_axi_awaddr),
		.S_AXI_AWPROT(s00_axi_awprot),
		.S_AXI_AWVALID(s00_axi_awvalid),
		.S_AXI_AWREADY(s00_axi_awready),
		.S_AXI_WDATA(s00_axi_wdata),
		.S_AXI_WSTRB(s00_axi_wstrb),
		.S_AXI_WVALID(s00_axi_wvalid),
		.S_AXI_WREADY(s00_axi_wready),
		.S_AXI_BRESP(s00_axi_bresp),
		.S_AXI_BVALID(s00_axi_bvalid),
		.S_AXI_BREADY(s00_axi_bready),
		.S_AXI_ARADDR(s00_axi_araddr),
		.S_AXI_ARPROT(s00_axi_arprot),
		.S_AXI_ARVALID(s00_axi_arvalid),
		.S_AXI_ARREADY(s00_axi_arready),
		.S_AXI_RDATA(s00_axi_rdata),
		.S_AXI_RRESP(s00_axi_rresp),
		.S_AXI_RVALID(s00_axi_rvalid),
		.S_AXI_RREADY(s00_axi_rready),

		// User ports
		.o_data_en		(w_data_en	 ),

		.f_enable		(in_f_enable),
		.f_read_done	(in_f_read_done),
		.f_done			(in_f_done),

		.m_enable		(in_maxPool_enable),

// CNN Result
		.i_result_en	(w_result_en ),
    	.i_result_0		(w_result_0	 ),
		.i_result_1		(w_result_1	 ),
		.i_result_2		(w_result_2	 ),
		.i_result_3		(w_result_3	 ),
		.i_result_4		(w_result_4	 ),
		.i_result_5		(w_result_5	 ),
		.i_result_6		(w_result_6	 ),
		.i_result_7		(w_result_7	 ),
    	.i_result_acc	(w_result_acc),
// Maxpoll Result    	
        .maxPool_Result_0(maxPool_Result_0),
        .maxPool_Result_1(maxPool_Result_1),
        .maxPool_Result_2(maxPool_Result_2),
        .maxPool_Result_3(maxPool_Result_3),
        .maxPool_Result_4(maxPool_Result_4),
        .maxPool_Result_5(maxPool_Result_5),
        .maxPool_Result_6(maxPool_Result_6),
        
        .maxPool_Done(maxPool_Done),


		.in_f_read_done(in_f_read_done),
		
// AXI Master Enable Signal
		.m00_axi_init_axi_txn(m00_axi_init_axi_txn),
		.m01_axi_init_axi_txn(m01_axi_init_axi_txn)
	 	
	);

	// Add user logic here
	wire clk 	 = s00_axi_aclk;
	wire reset_n = s00_axi_aresetn;

//==============================================================================
// AXI MASTER
//==============================================================================
 
    wire [32-1 : 0]	M0_AXI_RDATA_OUT;
	wire [32-1 : 0] M0_AXI_RADDR_OUT;
	wire			M0_AXI_READ_DONE;
	wire            M0_AXI_TXN_DONE;
	
	wire [32-1 : 0] M1_AXI_RDATA_OUT;
	wire [32-1 : 0] M1_AXI_RADDR_OUT;
	wire			M1_AXI_READ_DONE;
	wire            M1_AXI_TXN_DONE;
	
// Instantiation of Axi Bus Interface M00_AXI
	M00_AXI # ( 
		.C_M_START_DATA_VALUE(C_M00_AXI_START_DATA_VALUE),
		.C_M_TARGET_SLAVE_BASE_ADDR(C_M00_AXI_TARGET_SLAVE_BASE_ADDR),
		.C_M_AXI_ADDR_WIDTH(C_M00_AXI_ADDR_WIDTH),
		.C_M_AXI_DATA_WIDTH(C_M00_AXI_DATA_WIDTH),
		.C_M_TRANSACTIONS_NUM(C_M00_AXI_TRANSACTIONS_NUM)
	) M00_AXI_inst (
		.INIT_AXI_TXN(m00_axi_init_axi_txn),
		.ERROR(m00_axi_error),
		.TXN_DONE(m00_axi_txn_done),
		.M_AXI_ACLK(m00_axi_aclk),
		.M_AXI_ARESETN(m00_axi_aresetn),
		.M_AXI_AWADDR(m00_axi_awaddr),
		.M_AXI_AWPROT(m00_axi_awprot),
		.M_AXI_AWVALID(m00_axi_awvalid),
		.M_AXI_AWREADY(m00_axi_awready),
		.M_AXI_WDATA(m0_axi_wdata),
		.M_AXI_WSTRB(m00_axi_wstrb),
		.M_AXI_WVALID(m00_axi_wvalid),
		.M_AXI_WREADY(m00_axi_wready),
		.M_AXI_BRESP(m00_axi_bresp),
		.M_AXI_BVALID(m00_axi_bvalid),
		.M_AXI_BREADY(m00_axi_bready),
		.M_AXI_ARADDR(m00_axi_araddr),
		.M_AXI_ARPROT(m00_axi_arprot),
		.M_AXI_ARVALID(m00_axi_arvalid),
		.M_AXI_ARREADY(m00_axi_arready),
		.M_AXI_RDATA(m00_axi_rdata),
		.M_AXI_RRESP(m00_axi_rresp),
		.M_AXI_RVALID(m00_axi_rvalid),
		.M_AXI_RREADY(m00_axi_rready),
		
        .M0_AXI_READ_DONE(M0_AXI_READ_DONE),
		.read_data_out(M0_AXI_RDATA_OUT),
		.read_idx_out(M0_AXI_RADDR_OUT)
		

	);
	
	assign in_f_read_done = M0_AXI_READ_DONE;
	assign M0_AXI_TXN_DONE = m00_axi_txn_done;
	
 // Instantiation of Axi Bus Interface M01_AXI
 	M01_AXI # ( 
 		.C_M_START_DATA_VALUE(C_M01_AXI_START_DATA_VALUE),
 		.C_M_TARGET_SLAVE_BASE_ADDR(C_M01_AXI_TARGET_SLAVE_BASE_ADDR),
 		.C_M_AXI_ADDR_WIDTH(C_M01_AXI_ADDR_WIDTH),
 		.C_M_AXI_DATA_WIDTH(C_M01_AXI_DATA_WIDTH),
 		.C_M_TRANSACTIONS_NUM(C_M01_AXI_TRANSACTIONS_NUM)
 	) M01_AXI_inst (
 		.INIT_AXI_TXN(m01_axi_init_axi_txn),
 		.ERROR(m01_axi_error),
 		.TXN_DONE(m01_axi_txn_done),
 		.M_AXI_ACLK(m01_axi_aclk),
 		.M_AXI_ARESETN(m01_axi_aresetn),
 		.M_AXI_AWADDR(m01_axi_awaddr),
 		.M_AXI_AWPROT(m01_axi_awprot),
 		.M_AXI_AWVALID(m01_axi_awvalid),
 		.M_AXI_AWREADY(m01_axi_awready),
 		.M_AXI_WDATA(m01_axi_wdata),
 		.M_AXI_WSTRB(m01_axi_wstrb),
 		.M_AXI_WVALID(m01_axi_wvalid),
 		.M_AXI_WREADY(m01_axi_wready),
 		.M_AXI_BRESP(m01_axi_bresp),
 		.M_AXI_BVALID(m01_axi_bvalid),
 		.M_AXI_BREADY(m01_axi_bready),
 		.M_AXI_ARADDR(m01_axi_araddr),
 		.M_AXI_ARPROT(m01_axi_arprot),
 		.M_AXI_ARVALID(m01_axi_arvalid),
 		.M_AXI_ARREADY(m01_axi_arready),
 		.M_AXI_RDATA(m01_axi_rdata),
 		.M_AXI_RRESP(m01_axi_rresp),
 		.M_AXI_RVALID(m01_axi_rvalid),
 		.M_AXI_RREADY(m01_axi_rready),

        // user add
        
 		.M1_AXI_READ_DONE(M1_AXI_READ_DONE),
 		.read_data_out(M1_AXI_RDATA_OUT),
 		.read_idx_out(M1_AXI_RADDR_OUT)

 	);
	
 	assign in_m_read_done = M1_AXI_READ_DONE;
    assign in_maxPool_done = m01_axi_txn_done;

//==============================================================================
// CNN_DATA COMBINE
//==============================================================================

	wire [CI*KX*KY*I_F_BW-1 : 0]  		 out_fmap;
	wire signed [CO*CI*KX*KY*W_BW-1 : 0] out_weight;
	wire signed [CO*B_BW-1 : 0]          out_bias;
	wire 								data_done;

    wire [31:0]     					in_f_address;
    
    wire            					in_f_value_done;
   
 	cnn_data_combine u_cnn_data_combine(
    .clk(clk)          				 ,
    .reset_n(reset_n)           	 ,

    .in_fmap(in_f_value)         	 ,
    .in_f_address(in_f_address)      ,
    .in_f_enable(in_f_enable)        ,
    .in_f_done(in_f_done)            ,
    .in_f_value_done(in_f_value_done),

    .out_fmap(out_fmap)         	 ,
    .out_weight(out_weight)    	     ,
    .out_bias(out_bias)        	     ,
    .data_done(data_done) 
);

   assign 	in_f_value = M0_AXI_RDATA_OUT[0 +: I_F_BW];
   assign	in_f_address = M0_AXI_RADDR_OUT;
   assign in_f_value_done = M0_AXI_TXN_DONE;

//==============================================================================
// CNN_CORE
//==============================================================================

	assign w_in_valid	= w_data_en & data_done;
	assign w_in_fmap 	= out_fmap;
	assign w_in_weight 	= out_weight;
	assign w_in_bias 	= out_bias;

	assign w_result_en	= w_ot_valid;


	cnn_core u_cnn_core(
    	.clk             (clk    	  ),
    	.reset_n         (reset_n	  ),

    	.i_cnn_weight    (w_in_weight ),
    	.i_cnn_bias      (w_in_bias   ),
    	.i_in_valid      (w_in_valid  ),
    	.i_in_fmap       (w_in_fmap   ),
    	.o_ot_valid      (w_ot_valid  ),
    	.o_ot_fmap       (w_ot_fmap   )      
    );

assign w_result_0 	= w_ot_fmap[0*O_F_BW+:O_F_BW];
assign w_result_1 	= w_ot_fmap[1*O_F_BW+:O_F_BW];
assign w_result_2 	= w_ot_fmap[2*O_F_BW+:O_F_BW];
assign w_result_3 	= w_ot_fmap[3*O_F_BW+:O_F_BW];
assign w_result_4 	= w_ot_fmap[4*O_F_BW+:O_F_BW];
assign w_result_5 	= w_ot_fmap[5*O_F_BW+:O_F_BW];
assign w_result_6 	= w_ot_fmap[6*O_F_BW+:O_F_BW];
assign w_result_7 	= w_ot_fmap[7*O_F_BW+:O_F_BW];


//==============================================================================
// MAX_POOLING_DATA_COMBINE
//==============================================================================
	wire [M_CO*M_CI*2*2*MAX_BW-1 : 0] i_in_maxPool_fmap;

 	maxPool_data_combine u_maxPool_data_combine (
    .clk					(clk				)           ,
    .reset_n				(reset_n			)           ,

    .in_maxPool_value		(in_maxPool_value	)           ,
    .in_maxPool_address		(in_maxPool_address	)   	    ,
    .in_maxPool_enable		(in_maxPool_enable	)    	    ,
    .in_maxPool_done		(in_maxPool_done	)     	    ,

    .out_maxPool			(out_maxPool		)           ,

    .data_done				(maxPool_data_done	) 
         
);

assign 	in_maxPool_value = M1_AXI_RDATA_OUT[0 +: MAX_BW];
assign     in_maxPool_address = M1_AXI_RADDR_OUT;
 
assign i_in_max_valid = maxPool_data_done;
assign i_in_maxPool_fmap = out_maxPool;
//==============================================================================
// MAX_POOLING
//==============================================================================


maxPooling u_maxPooling (
    .clk					(clk				),
    .reset_n				(reset_n			),
    .i_in_max_valid			(i_in_max_valid		),
    .i_in_maxPool_fmap		(i_in_maxPool_fmap	),
    .o_ot_max_valid			(o_ot_max_valid		),
    .o_ot_maxPool_fmap		(o_ot_maxPool_fmap	)

);

assign maxPool_Result_0 = o_ot_maxPool_fmap[32*0 +: 32];
assign maxPool_Result_1 = o_ot_maxPool_fmap[32*1 +: 32];
assign maxPool_Result_2 = o_ot_maxPool_fmap[32*2 +: 32];
assign maxPool_Result_3 = o_ot_maxPool_fmap[32*3 +: 32];
assign maxPool_Result_4 = o_ot_maxPool_fmap[32*4 +: 32];
assign maxPool_Result_5 = o_ot_maxPool_fmap[32*5 +: 32];
assign maxPool_Result_6 = o_ot_maxPool_fmap[32*6 +: 32];

assign maxPool_Done = o_ot_max_valid;

	// User logic ends

	endmodule
