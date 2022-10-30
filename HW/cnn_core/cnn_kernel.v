
`include "timescale.vh"
module cnn_kernel (
    // Clock & Reset
    clk             ,
    reset_n         ,
    i_soft_reset    ,
    i_cnn_weight    ,
    i_in_valid      ,
    i_in_fmap       ,
    o_ot_valid      ,
    o_ot_kernel_acc              
    );
`include "defines_cnn_core.vh"
localparam LATENCY = 2;

input                               clk         	;
input                               reset_n     	;
input                               i_soft_reset	;
input     signed [KX*KY*W_BW-1 : 0]  		i_cnn_weight 	;
input                               i_in_valid  	;
input     signed [KX*KY*I_F_BW-1 : 0]  	i_in_fmap    	;
output                              o_ot_valid  	;
output    signed [AK_BW-1 : 0]  			o_ot_kernel_acc ;


//==============================================================================
// Data Enable Signals 
//==============================================================================
wire    [LATENCY-1 : 0] 	ce;
reg     [LATENCY-1 : 0] 	r_valid;
always @(posedge clk or negedge reset_n) begin
    if(!reset_n) begin
        r_valid   <= {LATENCY{1'b0}};
    end else if(i_soft_reset) begin
        r_valid   <= {LATENCY{1'b0}};
    end else begin
        r_valid[LATENCY-2]  <= i_in_valid;
        r_valid[LATENCY-1]  <= r_valid[LATENCY-2];
    end
end

assign	ce = r_valid;

//==============================================================================
// mul = fmap * weight
//==============================================================================

wire      signed [KY*KX*M_BW-1 : 0]    mul  ;
reg       signed [KY*KX*M_BW-1 : 0]    r_mul;

genvar mul_idx;
generate
	for(mul_idx = 0; mul_idx < KY*KX; mul_idx = mul_idx + 1) begin : gen_mul
		assign  mul[mul_idx * M_BW +: M_BW]	= $signed(i_in_fmap[mul_idx * I_F_BW +: I_F_BW]) * $signed(i_cnn_weight[mul_idx * W_BW +: W_BW]);
	
		always @(posedge clk or negedge reset_n) begin
		    if(!reset_n) begin
		        r_mul[mul_idx * M_BW +: M_BW] <= {M_BW{1'b0}};
		    end else if(i_soft_reset) begin
		        r_mul[mul_idx * M_BW +: M_BW] <= {M_BW{1'b0}};
		    end else if(i_in_valid)begin
		        r_mul[mul_idx * M_BW +: M_BW] <= mul[mul_idx * M_BW +: M_BW];
		    end
		end
	end
endgenerate

reg       signed [AK_BW-1 : 0]    acc_kernel 	;
reg       signed [AK_BW-1 : 0]    r_acc_kernel   ;

integer acc_idx;
generate
	always @ (*) begin
		acc_kernel[0 +: AK_BW]= {AK_BW{1'b0}};
		for(acc_idx =0; acc_idx < KY*KX; acc_idx = acc_idx +1) begin
			acc_kernel[0 +: AK_BW] = $signed(acc_kernel[0 +: AK_BW]) + $signed(r_mul[acc_idx*M_BW +: M_BW]); 
		end
	end
	always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
	        r_acc_kernel[0 +: AK_BW] <= {AK_BW{1'b0}};
	    end else if(i_soft_reset) begin
	        r_acc_kernel[0 +: AK_BW] <= {AK_BW{1'b0}};
	    end else if(ce[LATENCY-2])begin
	        r_acc_kernel[0 +: AK_BW] <= acc_kernel[0 +: AK_BW];
	    end
	end
endgenerate

assign o_ot_valid = r_valid[LATENCY-1];
assign o_ot_kernel_acc = r_acc_kernel;

endmodule
