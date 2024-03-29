# Convolution HW Accelerator Implementation
This project is an attempt to implement a convolution HW accelerator system.  
The code is writtend by Verilog and synthesized on Xilix FPGA using Vivado.  
This code is synthesizable. 

### Platform : Vivado 22.1v (Xilinx HW IDE)  
### Language : Verilog  
### Board : ZYBO Z7-20
### Code Version : 1.0
### Implement Resource Usage  
![resource](https://user-images.githubusercontent.com/75150975/206840615-cd1ca29c-a4b5-48c9-bf5c-82cabd783324.png)
# Architecture
 ┣ 📂cnn_core   
 ┃ ┣ 📜cnn_acc_ci.v   
 ┃ ┣ 📜cnn_core.v   
 ┃ ┣ 📜cnn_kernel.v   
 ┃ ┣ 📜defines_cnn_core.vh  
 ┃ ┣ 📜relu.v   
 ┃ ┗ 📜timescale.vh        
 ┣ 📜M00_AXI.v  
 ┣ 📜M01_AXI.v  
 ┣ 📜axi4_lite.v  
 ┣ 📜cnn_core_top.v   
 ┣ 📜cnn_data_combine.v   
 ┣ 📜maxPool_data_combine.v   
 ┣ 📜maxPooling.v   
 ┗ 📜README.md   
 
# Block Diagram
![block diagram](https://user-images.githubusercontent.com/75150975/215968653-f6d01b8c-dde4-4958-9c34-2e8658b7b125.png)
# Conclusion  
I think that pure RTL hardware design is not good choice for neural networks. So I try to design a part of the neural networks.  
Especially, Convolution is a core computation of neural networks and is a part which takes a lot of times.  
This is a reason why I try to design as hardware. 
I proceeded with all stages of planning, design, verification by myself.  
This special experience was a new challenge for me and I could learn a lot.   
I hope it will be an experience that goes one step further. 
