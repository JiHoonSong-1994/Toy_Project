/*******************************************************************************
Copyright (c) 2022  JiHoon Song, All Rights Reserved
AUTHOR: JiHoon Song 
AUTHOR'S EMAIL : jihoon20620@naver.com 
ASSOCIATED FILENAME : cnn_core.v
REVISION HISTORY : December 10, 2022 - initial release
*******************************************************************************/

`include "timescale.vh"
`include "defines_cnn_core.vh"

module relu (
        clk,
        reset_n,
        w_ot_valid,
        x,
        relu_valid,
        out
);


    input                                           clk       ;
    input                                           reset_n   ;
    input               [CO-1 : 0]                  w_ot_valid;
    input   signed      [CO*AB_BW-1 : 0]            x         ;  // relu input value
    output  reg                                     relu_valid; 
    output  reg signed  [CO*AB_BW-1 : 0]            out       ;


    
//==============================================================================
// Valid Signal
//==============================================================================
    

    always@ (posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            relu_valid <= 1'b0;
        end
        else if (&w_ot_valid)begin
            relu_valid <= 1'b1;
        end
        else begin
            relu_valid <= 1'b0;
        end

    end

genvar  relu_idx;
generate
    for (relu_idx = 0; relu_idx < CO; relu_idx = relu_idx + 1) begin : gen_relu

        always @(posedge clk or negedge reset_n) begin
            if(!reset_n) begin
                    out[relu_idx*AB_BW +: AB_BW]   <= {AB_BW{1'b0}};
            end else if(&w_ot_valid) begin
                if ( $signed(x[relu_idx*AB_BW +: AB_BW ]) >= 0 ) begin
                    out[relu_idx*AB_BW +: AB_BW]   <= x[relu_idx*AB_BW +: AB_BW];
                end
                else begin
                    out[relu_idx*AB_BW +: AB_BW]   <= {AB_BW{1'b0}} ;
                end
            end
        end
    end
endgenerate

endmodule

