
`timescale 1 ns / 1 ps

	module cnn_core_ci3_co16 #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 6
	)
	(
		// Users to add ports here
		//AXI_DMA Port

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
		input wire  s00_axi_rready
	);
	`include "defines_cnn_core.vh"
	wire                              	w_in_valid  	;
	wire    [CI*KX*KY*I_F_BW-1:0]  		w_in_fmap    	;
	wire                              	w_ot_valid  	;
	wire    [CO*O_F_BW-1 : 0]  			w_ot_fmap    	;

	wire    [W_BW-1 : 0]  				w_w_value 		;
	wire    [B_BW-1 : 0]  				w_b_value   	;

	reg    [CI*KX*KY*I_F_BW-1 : 0]  	in_fmap    	;
	reg    [CO*CI*2*2*I_F_BW-1 : 0] 	in_max		;

	wire 					w_data_en;
	wire [I_F_BW-1 :0] 		w_f_value;
	wire 					w_result_en;
    wire [O_F_BW-1:0] 		w_result_0;
    wire [O_F_ACC_BW-1:0] 	w_result_acc;

	wire [I_F_BW-1 :0] 		w_max_value;

	wire [I_F_BW-1 : 0]		i_in_maxPool_fmap;
	wire 					o_max_en;
	wire 					i_in_max_valid;
	wire 					o_ot_max_valid;
	wire					maxPool_valid;
	wire [CO*CI*W_BW-1 : 0]	i_result_max;

// Instantiation of Axi Bus Interface S00_AXI
	axi4_lite # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) u_axi4_lite (
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
		.o_max_en		(o_max_en	 ),
		.o_f_value		(w_f_value	 ),
		.o_w_value		(w_w_value	 ),
		.o_b_value		(w_b_value	 ),
		.o_max_value	(w_max_value ),

		.i_result_en	(w_result_en ),
    	.i_result_0		(w_result_0	 ),
    	.i_result_acc	(w_result_acc),
		.i_result_max	(i_result_max),
		.i_maxPool_valid(maxPool_valid)

	);

	// Add user logic here
	wire clk 	 = s00_axi_aclk;
	wire reset_n = s00_axi_aresetn;
//==============================================================================
// Data In
//==============================================================================
	integer i;
	always @(*) begin
		for(i = 0; i < CI*KX*KY ; i = i+1) begin
			in_fmap[i*I_F_BW +: I_F_BW] = w_f_value;
			// w_f_value ?뜲?씠?꽣 諛붾?붾븣 留덈떎 ?떎?쓬 ?씪?씤?씠 ?떎?뻾?맖. always@(*) ?븣臾몄뿉
			// in_fmap[0*I_F_BW +: I_F_BW] = w_f_value;
			// in_fmap[1*I_F_BW +: I_F_BW] = w_f_value;
			// in_fmap[2*I_F_BW +: I_F_BW] = w_f_value;
			// in_fmap[3*I_F_BW +: I_F_BW] = w_f_value;
			// in_fmap[4*I_F_BW +: I_F_BW] = w_f_value;
		end
	end

	integer max_idx;
	always @(*) begin
		for(max_idx = 0; max_idx < CO*CI*2*2 ; imax_idx = max_idx+1) begin
			in_max[max_idx*I_F_BW +: I_F_BW] = w_max_value; //MAXpool 媛??뒫 ?슏?닔 : CO*CI*2*2
		end
	end
//==============================================================================
// Accum operation
//==============================================================================
	// Accum result for test
	reg    [O_F_ACC_BW-1 : 0]  		acc_result;
	always @(*) begin
		acc_result = {O_F_ACC_BW{1'b0}};
		for(i = 0; i < CO; i = i+1) begin
			acc_result = acc_result + w_ot_fmap[i*O_F_BW +: O_F_BW] ; // [CO*O_F_BW-1 : 0] w_ot_fmap?뿉 16*8 ?뜲?씠?꽣 ?뱾?뼱?엳?쓬. ?씠寃? conv 理쒖쥌 寃곌낵.
			// 23鍮꾪듃?씠湲? ?븣臾몄뿉 8鍮꾪듃濡? ?굹?닠?꽌 AXI LITE ?씠?슜?빐?꽌 DDR3濡? ?뜲?씠?꽣 ?쟾?넚?븷 寃?.
		end
	end
	
	reg    [O_F_ACC_BW-1 : 0]  		r_acc_result;
	reg								r_ot_valid;
	always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			r_ot_valid	 <= 1'b0;
			r_acc_result <= {O_F_ACC_BW{1'b0}};
	    end else begin
			r_ot_valid	 <= w_ot_valid;
			r_acc_result <= acc_result;
	    end
	end



//==============================================================================
// Cnn Core
//==============================================================================
	assign w_in_valid	= w_data_en;
	assign w_in_fmap 	= in_fmap  ; 

	assign w_result_en	= w_ot_valid && r_ot_valid ;
	assign w_result_0 	= w_ot_fmap[0+:O_F_BW];
	assign w_result_acc = r_acc_result;


	cnn_core u_cnn_core(
    	.clk             (clk    	  ),
    	.reset_n         (reset_n	  ),
    	.i_soft_reset    (1'b0		  ), // no use
    	.i_cnn_weight    (w_w_value	  ), 
    	.i_cnn_bias      (w_b_value   ),
    	.i_in_valid      (w_in_valid  ),
    	.i_in_fmap       (w_in_fmap   ),
    	.o_ot_valid      (w_ot_valid  ),
    	.o_ot_fmap       (w_ot_fmap   )      
    );

//==============================================================================
// Max Pooling
//==============================================================================
	assign i_in_max_valid = o_max_en;
	assign maxPool_valid = o_ot_max_valid;

	assign i_in_maxPool_fmap = in_max;
	assign i_result_max = maxP_result;

	maxPooling u_maxPooling(
		.clk					(clk			  ),
		.reset_n				(reset_n		  ),
		.i_in_max_valid			(i_in_max_valid	  ),
		.i_in_maxPool_fmap		(i_in_maxPool_fmap),
		.o_ot_valid				(o_ot_max_valid	  ),
		.o_ot_maxPool_fmap		(o_ot_maxPool_fmap) // [CO*CI*W_BW-1 : 0]

	);

	reg [CO*CI*W_BW-1 : 0] maxP_result;
	always @(posedge clk or negedge reset_n)
	if(!reset_n) begin
		maxP_result <= 1'b0;
	end
	else begin
		maxP_result <= o_ot_maxPool_fmap;
	end
	// User logic ends

	endmodule
