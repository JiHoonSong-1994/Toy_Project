
`include "timescale.vh"

module cnn_core (
    // Clock & Reset
    clk             ,
    reset_n         ,
    i_soft_reset    ,
    i_cnn_weight    ,
    i_cnn_bias      ,
    i_in_valid      ,
    i_in_fmap       ,
    o_ot_valid      ,
    o_ot_fmap             
    );
`include "defines_cnn_core.vh"

localparam LATENCY = 2; 

//==============================================================================
// Input/Output declaration
//==============================================================================
input                               clk         	;
input                               reset_n     	;
input                               i_soft_reset	;
//input     [CO*CI*KX*KY*W_BW-1 : 0]  i_cnn_weight 	;
//input     [CO*B_BW-1    : 0]  		i_cnn_bias   	;
input     signed [W_BW-1 : 0]  			i_cnn_weight 	;
input     signed [B_BW-1 : 0]  			i_cnn_bias   	;
input                               i_in_valid  	;
input     signed [CI*KX*KY*I_F_BW-1:0]  	i_in_fmap    	;
output                              o_ot_valid  	;
output    signed [CO*O_F_BW-1 : 0]  		o_ot_fmap    	;

//==============================================================================
// Data combine
//==============================================================================

wire    signed [CO*CI*KX*KY*W_BW-1 : 0]  	w_cnn_weight 	;
wire    signed [CO*B_BW-1    : 0]  		    w_cnn_bias   	;
reg     signed [CO*CI*KX*KY*W_BW-1 : 0]  	cnn_weight 	;
reg     signed [CO*B_BW-1    : 0]  		    cnn_bias   	;

integer i;
always @(*) begin
	for(i = 0; i < CO*CI*KX*KY ; i = i+1) begin
		cnn_weight[i*W_BW +: W_BW] = i_cnn_weight;
	end
end
always @(*) begin
	for(i = 0; i < CO ; i = i+1) begin
		cnn_bias[i*B_BW +: B_BW] = i_cnn_bias;
	end
end

assign w_cnn_weight = cnn_weight;
assign w_cnn_bias 	= cnn_bias;
//==============================================================================
// Data Enable Signals 
//==============================================================================
wire    [LATENCY-1 : 0] 	ce;
reg     [LATENCY-1 : 0] 	r_valid;
wire    [CO-1 : 0]          w_ot_valid;

wire relu_valid;

always @(posedge clk or negedge reset_n) begin
    if(!reset_n) begin
        r_valid   <= {LATENCY{1'b0}};
    end else if(i_soft_reset) begin
        r_valid   <= {LATENCY{1'b0}};
    end else begin
             r_valid[LATENCY-2] <=  relu_valid;
             r_valid[LATENCY-1] <= r_valid[LATENCY-2];
    end
end


assign	ce = r_valid;

//==============================================================================
// acc ci instance
//==============================================================================

wire    [CO-1 : 0]                      w_in_valid;
wire    signed [CO*(ACI_BW)-1 : 0]  	w_ot_ci_acc;
genvar ci_inst;
generate
	for(ci_inst = 0; ci_inst < CO; ci_inst = ci_inst + 1) begin : gen_ci_inst
		wire    [CI*KX*KY*W_BW-1 : 0]  	w_cnn_weight_ci	= w_cnn_weight[ci_inst*CI*KY*KX*W_BW +: CI*KY*KX*W_BW];
		wire    [CI*KX*KY*I_F_BW-1 : 0] w_in_fmap    	= i_in_fmap[0 +: CI*KX*KY*I_F_BW];
		assign	w_in_valid[ci_inst] = i_in_valid; 
		// Preserve the hierarchy of instance cnn_acc_ci
		(* DONT_TOUCH = "TRUE" *) cnn_acc_ci u_cnn_acc_ci(
	    .clk             (clk         ),
	    .reset_n         (reset_n     ),
	    .i_soft_reset    (i_soft_reset),
	    .i_cnn_weight    (w_cnn_weight_ci),
	    .i_in_valid      (w_in_valid[ci_inst]),
	    .i_in_fmap       (w_in_fmap),
	    .o_ot_valid      (w_ot_valid[ci_inst]),
	    .o_ot_ci_acc     (w_ot_ci_acc[ci_inst*(ACI_BW) +: (ACI_BW)])         
	    );
	end
endgenerate

//==============================================================================
// add_bias = acc + bias
//==============================================================================
wire      signed [CO*AB_BW-1 : 0]   add_bias  ;
reg       signed [CO*AB_BW-1 : 0]   r_add_bias;
genvar  add_idx;
generate
    for (add_idx = 0; add_idx < CO; add_idx = add_idx + 1) begin : gen_add_bias
        assign  add_bias[add_idx*AB_BW +: AB_BW] = $signed(w_ot_ci_acc[add_idx*(ACI_BW) +: ACI_BW]) + $signed(w_cnn_bias[add_idx*B_BW +: B_BW]);

        always @(posedge clk or negedge reset_n) begin
            if(!reset_n) begin
                r_add_bias[add_idx*AB_BW +: AB_BW]   <= {AB_BW{1'b0}};
            end else if(i_soft_reset) begin
                r_add_bias[add_idx*AB_BW +: AB_BW]   <= {AB_BW{1'b0}};
            end else if(&w_ot_valid) begin
                r_add_bias[add_idx*AB_BW +: AB_BW]   <= add_bias[add_idx*AB_BW +: AB_BW];
            end
        end
    end
endgenerate

//==============================================================================
// ReLu Activation
//==============================================================================

wire signed [CO*AB_BW-1 : 0]   x;
wire signed [CO*AB_BW-1 : 0]  out;
reg signed  [CO*AB_BW-1 : 0]  out_result;


assign x = r_add_bias;

relu u_relu (
    .clk(clk),
    .reset_n(reset_n),
    .w_ot_valid(w_ot_valid),
    .x(x),
    .relu_valid(relu_valid),
    .out(out)
);

genvar  out_idx;
generate
    for (out_idx = 0; out_idx < CO; out_idx = out_idx + 1) begin : gen_out_bias

        always @(posedge clk or negedge reset_n) begin
            if(!reset_n) begin
                out_result[out_idx*AB_BW +: AB_BW]   <= {AB_BW{1'b0}};
            end else if(i_soft_reset) begin
                out_result[out_idx*AB_BW +: AB_BW]   <= {AB_BW{1'b0}};
            end else if(&w_ot_valid) begin
                out_result[out_idx*AB_BW +: AB_BW]   <= out[out_idx*AB_BW +: AB_BW];
                // $display("o_ot_fmap relu out : %d " , o_ot_fmap[out_idx*AB_BW +: AB_BW]);
                // $display("r_valid : %b" , r_valid );
            end
        end
    end
endgenerate

assign o_ot_valid = &r_valid;
assign o_ot_fmap  = out_result; //r_add_bias;

endmodule

