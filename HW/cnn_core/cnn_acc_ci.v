/*******************************************************************************
Copyright (c) 2022  JiHoon Song, All Rights Reserved
AUTHOR: JiHoon Song 
AUTHOR'S EMAIL : jihoon20620@naver.com 
ASSOCIATED FILENAME : cnn_core.v
REVISION HISTORY : December 10, 2022 - initial release
*******************************************************************************/

`include "timescale.vh"
`include "defines_cnn_core.vh"

module cnn_acc_ci (

    clk             ,
    reset_n         ,

    i_cnn_weight    ,
    i_in_valid      ,
    i_in_fmap       ,

    o_ot_valid      ,
    o_ot_ci_acc       

    );

localparam LATENCY = 1;

input                                       clk         	;
input                                       reset_n     	;
input                                       i_soft_reset	;
input     signed [CI*KX*KY*W_BW-1 : 0]  	i_cnn_weight 	;
input                                       i_in_valid  	;
input     [CI*KX*KY*I_F_BW-1 : 0]  	        i_in_fmap    	;
output                                      o_ot_valid  	;
output    signed [ACI_BW-1 : 0]  			o_ot_ci_acc 	;


//==============================================================================
// Data Enable Signals 
//==============================================================================

reg     [LATENCY-1 : 0] 	r_valid;
wire    [CI-1 : 0]          w_ot_valid;

always @(posedge clk or negedge reset_n) begin
    if(!reset_n) begin
        r_valid   <= {LATENCY{1'b0}};
    end else if(i_soft_reset) begin
        r_valid   <= {LATENCY{1'b0}};
    end else begin
        r_valid[LATENCY-1]  <= &w_ot_valid;
    end
end


//==============================================================================
// mul_acc kenel instance
//==============================================================================

wire    [CI-1 : 0]                  w_in_valid;
wire    signed [CI*AK_BW-1 : 0]  	w_ot_kernel_acc;
wire    signed [ACI_BW-1 : 0]  		w_ot_ci_acc;
reg     signed [ACI_BW-1 : 0]  		r_ot_ci_acc;

genvar mul_inst;
generate
	for(mul_inst = 0; mul_inst < CI; mul_inst = mul_inst + 1) begin : gen_mul_inst
		wire    signed [KX*KY*W_BW-1 : 0]  	w_cnn_weight 	= i_cnn_weight[mul_inst*KY*KX*W_BW +: KY*KX*W_BW];
		wire    [KX*KY*I_F_BW-1 : 0]  	w_in_fmap    	= i_in_fmap[mul_inst*KY*KX*I_F_BW +: KY*KX*I_F_BW]; 

		assign	w_in_valid[mul_inst] = i_in_valid; 

		cnn_kernel u_cnn_kernel(
    	.clk             (clk            ),
    	.reset_n         (reset_n        ),
    	.i_cnn_weight    (w_cnn_weight   ),
    	.i_in_valid      (w_in_valid[mul_inst]),
    	.i_in_fmap       (w_in_fmap      ),
    	.o_ot_valid      (w_ot_valid[mul_inst]),
    	.o_ot_kernel_acc (w_ot_kernel_acc[mul_inst*AK_BW +: AK_BW])             
    	);
	end
endgenerate

//==============================================================================
// Mul result accumulate
//==============================================================================

reg    signed [ACI_BW-1 : 0]  		ot_ci_acc; 
integer i;
always @(*) begin
	ot_ci_acc = {ACI_BW{1'b0}};
	for(i = 0; i < CI; i = i+1) begin
		    ot_ci_acc = $signed(ot_ci_acc) + $signed(w_ot_kernel_acc[i*AK_BW +: AK_BW]);
	end
end

assign w_ot_ci_acc = ot_ci_acc;

always @(posedge clk or negedge reset_n) begin
    if(!reset_n) begin
        r_ot_ci_acc[0 +: ACI_BW] <= {ACI_BW{1'b0}};
    end else if(&w_ot_valid)begin
        r_ot_ci_acc[0 +: ACI_BW] <= w_ot_ci_acc[0 +: ACI_BW];
    end
end

assign o_ot_valid = r_valid[LATENCY-1];
assign o_ot_ci_acc = r_ot_ci_acc;

endmodule
