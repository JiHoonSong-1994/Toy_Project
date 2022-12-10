
parameter   CNN_PIPE    = 5;
parameter   CI          = 4;
parameter   CO          = 8;
parameter   M_CI        = 4;
parameter   M_CO        = 7;

parameter	KX			= 3;
parameter	KY			= 3;

parameter   I_F_BW      = 8;
parameter   W_BW        = 8; // Weight BW
parameter   B_BW        = 8; // Bias BW
parameter   MAX_BW      = 8;

parameter   M_BW        = 20; //8*9 17b
parameter   AK_BW       = 24; // M_BW + log(KY*KX) accum kernel +4 = 21b
parameter   ACI_BW		= 24;//22; // AK_BW + log (CI) ,23b
parameter   AB_BW       = 24;
parameter   O_F_BW      = 24; // No Activation, So O_F_BW == AB_BW

parameter   O_F_ACC_BW  = 27; // for demo
