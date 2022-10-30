`include "timescale.vh"

module maxPooling (
    clk,
    reset_n,
    i_in_max_valid,
    i_in_maxPool_fmap,
    o_ot_max_valid,
    o_ot_maxPool_fmap

);
`include "defines_cnn_core.vh"
localparam LATENCY = 1;
//==============================================================================
// Input/Output declaration
//==============================================================================

input                           clk;
input                           reset_n;
input                           i_in_max_valid;
input  [CO*CI*2*2*W_BW-1 : 0]   i_in_maxPool_fmap;
output                          o_ot_max_valid;
output [CO*CI*W_BW-1 : 0]   o_ot_maxPool_fmap;

//==============================================================================
// Enable Signals 
//==============================================================================

reg     [LATENCY-1 : 0] 	r_valid;

always @(posedge clk or negedge reset_n) begin
    if(!reset_n) begin
        r_valid   <= {LATENCY{1'b0}};
    end else begin
       //      r_valid[LATENCY-2] <=  relu_valid;
             r_valid[LATENCY-1] <= i_in_max_valid; //[LATENCY-2];
    end
end

//==============================================================================
// Data In
//==============================================================================
    reg [CO*CI*2*2*W_BW-1 : 0] max_reg ;
    
    always @(posedge clk or negedge reset_n )
    begin
        if(!reset_n ) begin
            max_reg <= 1'b0;
        end
        else begin
            max_reg <= i_in_maxPool_fmap;
        end
    end

//==============================================================================
// operation
//==============================================================================

wire [CO*CI*W_BW-1 : 0] com1;
wire [CO*CI*W_BW-1 : 0] com2;
reg  [CO*CI*W_BW-1 : 0] com_rlt;

genvar i;
generate
    for (i = 0; i < (CO*CI); i=i+1) begin : gen_maxPool
    
        assign com1[i*W_BW +: W_BW] = ( max_reg[(i*4)*W_BW +: W_BW] >= max_reg[((i*4)+1)*W_BW +: W_BW]) ? max_reg[(i*4)*W_BW +: W_BW] : max_reg[((i*4)+1)*W_BW +: W_BW];
        assign com2[i*W_BW +: W_BW] = ( max_reg[((i*4)+2)*W_BW +: W_BW] >= max_reg[((i*4)+3)*W_BW +: W_BW]) ? max_reg[((i*4)+2)*W_BW +: W_BW] : max_reg[((i*4)+3)*W_BW +: W_BW];    
       
        always@( posedge clk or negedge reset_n) begin
            if(!reset_n || !i_in_max_valid) begin
                    com_rlt[i*W_BW +: W_BW] <= {W_BW{1'b0}};
            end
            else if (i_in_max_valid)begin
                if ( (com1[i*W_BW +: W_BW] > com2[i*W_BW +: W_BW]) ) begin
                    com_rlt[i*W_BW +: W_BW] <= com1[i*W_BW +: W_BW];
                end
                else begin
                    com_rlt[i*W_BW +: W_BW] <= com2[i*W_BW +: W_BW];
                end
            end   
            else begin
                com_rlt[i*W_BW +: W_BW] <= {W_BW{1'b0}};
            end
        end
        
    end    
endgenerate


assign o_ot_maxPool_fmap = com_rlt;
assign o_ot_max_valid = r_valid;

endmodule
