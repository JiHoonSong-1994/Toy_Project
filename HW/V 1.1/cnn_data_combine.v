module data_combine (
    clk               ,
    reset_n           ,

    in_fmap           ,
    in_f_address      ,
    in_f_enable       ,
    in_f_done         ,
    in_f_value_done   ,

    out_fmap          ,
    out_weight        ,
    out_bias          ,

    data_done 
         
);
`include "defines_cnn_core.vh"

//==============================================================================
// Input/Output declaration
//==============================================================================
   input   clk               ;       
   input   reset_n           ;

   input  [32-1:0]      in_fmap           ;
   input  [31:0]            in_f_address      ;
   input   in_f_enable       ;
   input   in_f_done         ;
   input   in_f_value_done  ;
   
   output wire [CI*KX*KY*I_F_BW-1 : 0]  out_fmap          ;
   output  wire signed [CO*CI*KX*KY*W_BW-1 : 0] out_weight        ;
   output  wire signed  [CO*B_BW-1 : 0]          out_bias          ;

   output data_done;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

  reg [((CI*KX*KY*I_F_BW) + (CO*CI*KX*KY*W_BW) + (CO*B_BW))-1 +32 : 0] fmap_reg;
  reg [((CI*KX*KY*I_F_BW) + (CO*CI*KX*KY*W_BW) + (CO*B_BW))-1 : 0] out_data;
  reg [31:0] f_address;
  reg [32-1:0] in_fmap_delay;
  reg [32-1:0] in_fmap_delay1;
  reg [32-1:0] in_fmap_delay2;
  reg in_fmap_txn_done;

 reg [CI*KX*KY*I_F_BW-1 : 0] out_fmap_reg;
 reg [CO*CI*KX*KY*W_BW-1 : 0]  out_weight_reg;
 reg [CO*B_BW-1 : 0] out_bias_reg;

 wire data_end;
assign data_end = in_f_done ;
    always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			f_address	 <= 1'b0;
	    end else begin
			f_address	 <= in_f_address;
	    end
	end
	////////////////////////////////////////////////////////////////////////////////////////////////////// read_done
	 always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_fmap_txn_done	 <= 1'b0;
	    end else begin
			in_fmap_txn_done	 <= in_f_value_done;
	    end
	end
	////////////////////////////////////////////////////////////////////////////////////////////////////// delay part
	 always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_fmap_delay	 <= 1'b0;
		end else if (in_f_enable)begin
			in_fmap_delay <=  in_fmap;
	    end
	end
	 always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_fmap_delay1	 <= 1'b0;
		end else if (in_f_enable)begin
			in_fmap_delay1 <=  in_fmap_delay;
	    end
	end
	 always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			in_fmap_delay2	 <= 1'b0;
		end else if (in_f_enable)begin
			in_fmap_delay2 <=  in_fmap_delay1;
	    end
	end
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	  always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			fmap_reg	 <= 1'b0;
		end else if (in_f_enable & ~in_fmap_txn_done)begin
			fmap_reg[f_address*32 +: 32] <=  in_fmap_delay2;
	    end
	end

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	
    always @(posedge clk or negedge reset_n) begin
	    if(!reset_n) begin
			out_data	 <= 1'b0;
		end else if (in_f_enable)begin
			out_data <=  fmap_reg[((CI*KX*KY*I_F_BW) + (CO*CI*KX*KY*W_BW) + (CO*B_BW))-1 +32 : 32] ;
	    end
	end

////////////////////////////////////////////////////////////////////////////////////////////////////////
	  always @(posedge clk or negedge reset_n) begin
            if(!reset_n) begin
                out_fmap_reg	 <= 1'b0;
                out_weight_reg	 <= 1'b0;
                out_bias_reg	 <= 1'b0;
            end 
            else begin
                out_fmap_reg <=   out_data[ CI*KX*KY*I_F_BW -1 : 0];
                out_weight_reg <=  out_data[CI*KX*KY*I_F_BW +: (CO*CI*KX*KY*W_BW)];
                out_bias_reg <=  out_data[(CO*CI*KX*KY*W_BW+CI*KX*KY*I_F_BW) +: (CO*B_BW)];
            end
	   end

     
    assign data_done = in_f_done ;

    assign out_fmap =  (data_done) ? out_fmap_reg : 0 ; 
    assign out_weight = (data_done) ? out_weight_reg : 0; 
    assign out_bias =   (data_done) ? out_bias_reg : 0; 
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////


endmodule