/*******************************************************************************
Copyright (c) 2022  JiHoon Song, All Rights Reserved
AUTHOR: JiHoon Song 
AUTHOR'S EMAIL : jihoon20620@naver.com 
ASSOCIATED FILENAME : cnn_core.v
REVISION HISTORY : December 10, 2022 - initial release
*******************************************************************************/

`include "defines_cnn_core.vh"

module maxPool_data_combine (
    clk               ,
    reset_n           ,

    in_maxPool_value        ,
    in_maxPool_address      ,
    in_maxPool_enable       ,
    in_maxPool_done         ,

    out_maxPool          ,

    data_done 
         
);

//==============================================================================
// Input/Output declaration
//==============================================================================
   	input   								  clk                     ;       
   	input   								  reset_n           	  ;

   	input   [MAX_BW-1:0]      			  in_maxPool_value        ;
   	input   [31:0]            			  in_maxPool_address      ;
   	input   								  in_maxPool_enable       ;
   	input   								  in_maxPool_done         ;

   	output  [M_CO*M_CI*2*2*MAX_BW-1 : 0]   out_maxPool          	  ;

   	output 								  data_done               ;


	reg [M_CO*M_CI*2*2*MAX_BW-1 + MAX_BW*2 : 0] maxPool_reg;
	reg [M_CO*M_CI*2*2*MAX_BW-1  : 0] maxPool_reg_out;
	reg [31:0] maxPool_address;
	wire in_maxPool_done1;
	reg [MAX_BW-1:0] in_maxpool_delay;
	reg [MAX_BW-1:0] in_maxpool_delay1;
	reg [MAX_BW-1:0] in_maxpool_delay2;
	reg [MAX_BW-1:0] in_maxpool_delay3;
	reg [MAX_BW-1:0] in_maxpool_delay4;

//==============================================================================
// Address 
//==============================================================================
    always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			maxPool_address	 <= 1'b0;
	    end else begin
			maxPool_address	 <= in_maxPool_address;
	    end
	    
	end
	
//==============================================================================
// Delay part
//==============================================================================
	 always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_maxpool_delay	 <= 1'b0;
		end else begin
			in_maxpool_delay <=  in_maxPool_value;
	    end
	end
	 always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_maxpool_delay1	 <= 1'b0;
		end else begin
			in_maxpool_delay1 <=  in_maxpool_delay;
	    end
	end
	 always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_maxpool_delay2	 <= 1'b0;
		end else begin
			in_maxpool_delay2 <=  in_maxpool_delay1;
	    end
	end
	always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_maxpool_delay3	 <= 1'b0;
		end else begin
			in_maxpool_delay3 <=  in_maxpool_delay2;
	    end
	end
	always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_maxpool_delay4	 <= 1'b0;
		end else begin
			in_maxpool_delay4 <=  in_maxpool_delay3;
	    end
	end

	always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			maxPool_reg	 <= 1'b0;
		end else if (in_maxPool_enable & ~in_maxPool_done1)begin
			maxPool_reg[maxPool_address*MAX_BW +: MAX_BW] <=  in_maxpool_delay4;
	    end
	end
	
	always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			maxPool_reg_out	 <= 1'b0;
		end else if (in_maxPool_done1)begin
			maxPool_reg_out <=  maxPool_reg[M_CO*M_CI*2*2*MAX_BW-1 + MAX_BW*2 : MAX_BW*2];
	    end
	end

//==============================================================================
// Output Assign
//==============================================================================

    assign out_maxPool = (in_maxPool_done1) ? maxPool_reg_out : 0;  
    assign in_maxPool_done1 = in_maxPool_done;

	assign data_done = in_maxPool_done1;

endmodule