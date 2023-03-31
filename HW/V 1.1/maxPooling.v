/*******************************************************************************
Copyright (c) 2022  JiHoon Song, All Rights Reserved
AUTHOR: JiHoon Song 
AUTHOR'S EMAIL : jihoon20620@naver.com 
ASSOCIATED FILENAME : cnn_core.v
REVISION HISTORY : December 10, 2022 - initial release
*******************************************************************************/

`include "timescale.vh"
`include "defines_cnn_core.vh"

module maxPooling (
    clk,
    reset_n,
    i_in_max_valid,
    i_in_maxPool_fmap,
    o_ot_max_valid,
    o_ot_maxPool_fmap

);

localparam LATENCY = 1;
//==============================================================================
// Input/Output declaration
//==============================================================================

input                                   clk                 ;
input                                   reset_n             ;
input                                   i_in_max_valid      ;
input  [M_CO*M_CI*2*2*MAX_BW-1 : 0]     i_in_maxPool_fmap   ;
output                                  o_ot_max_valid      ;
output [M_CO*M_CI*MAX_BW-1 : 0]         o_ot_maxPool_fmap   ;

//==============================================================================
// Enable Signals 
//==============================================================================

reg     [LATENCY-1 : 0] 	r_valid;

always @(posedge clk or negedge reset_n) begin
    if(!reset_n) begin
        r_valid   <= {LATENCY{1'b0}};
    end else begin
             r_valid[LATENCY-1] <= i_in_max_valid; //[LATENCY-2];
    end
end

//==============================================================================
// Data In
//==============================================================================
    reg [M_CO*M_CI*2*2*MAX_BW-1 : 0] max_reg ;
    
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
// Operation
//==============================================================================

wire [M_CO*M_CI*MAX_BW-1 : 0] com1;
wire [M_CO*M_CI*MAX_BW-1 : 0] com2;
reg  [M_CO*M_CI*MAX_BW-1 : 0] com_rlt;

genvar i;
generate
    for (i = 0; i < (M_CO*M_CI); i=i+1) begin : gen_maxPool
    
        assign com1[i*MAX_BW +: MAX_BW] = ( max_reg[(i*4)*MAX_BW +: MAX_BW] >= max_reg[((i*4)+1)*MAX_BW +: MAX_BW]) ? max_reg[(i*4)*MAX_BW +: MAX_BW] : max_reg[((i*4)+1)*MAX_BW +: MAX_BW];
        assign com2[i*MAX_BW +: MAX_BW] = ( max_reg[((i*4)+2)*MAX_BW +: MAX_BW] >= max_reg[((i*4)+3)*MAX_BW +: MAX_BW]) ? max_reg[((i*4)+2)*MAX_BW +: MAX_BW] : max_reg[((i*4)+3)*MAX_BW +: MAX_BW];    
       
        always@( posedge clk or negedge reset_n) begin
            if(!reset_n || !i_in_max_valid) begin
                    com_rlt[i*MAX_BW +: MAX_BW] <= {MAX_BW{1'b0}};
            end
            else if (i_in_max_valid)begin
                if ( (com1[i*MAX_BW +: MAX_BW] > com2[i*MAX_BW +: MAX_BW]) ) begin
                    com_rlt[i*MAX_BW +: MAX_BW] <= com1[i*MAX_BW +: MAX_BW];
                end
                else begin
                    com_rlt[i*MAX_BW +: MAX_BW] <= com2[i*MAX_BW +: MAX_BW];
                end
            end   
            else begin
                com_rlt[i*MAX_BW +: MAX_BW] <= {MAX_BW{1'b0}};
            end
        end
    end    
endgenerate

//==============================================================================

assign o_ot_maxPool_fmap = com_rlt;
assign o_ot_max_valid = r_valid;

endmodule
