# Convolution HW Accelerator Implementation
This project is an attempt to implement a convolution HW accelerator system.  
The code is writtend by Verilog and synthesized on Xilix FPGA using Vivado.  
This code is synthesizable. 

### Platform : Vivado 22.1v (Xilinx HW IDE)  
### Language : Verilog  
### Board : ZYBO Z7-20
### Code Version : 1.1
### Implement Resource Usage  
![Resource Usage](https://user-images.githubusercontent.com/75150975/229065503-f297dc23-037d-47f0-8a6f-870eb8717815.png)
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
This version adds Ethernet communication capabilities to previous versions.  
I think Ethernet Communication is very powerful about transmiiting Large amounts of data.  
Actually , CNN need to a lot of data for operation.(for example, GB or TB etc...)  
So high speed interface communication skills are one of the technologies required in cnn.  
This project have a purpose that implement FPGA-based 1Gigabit Ethernet which is high speed.  
Zynq Processor contains 1Gigabit Ethernet IP. 
