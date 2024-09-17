# ECE385-Final-Project

This is the final project by Albert Jiang and Rachel Abraham for [ECE385-Digital Systems Laboratory](https://ece.illinois.edu/academics/courses/ece385). It is a tile-matching game, similar to Candy Crush, built on FPGA using SystemVerilog. 

# Features
- An 8x8 gameboard is displayed by HDMI signal with 640x480 resolution at 30Hz, 12 bit RGB colors
- The player can select a tile on the 8x8 gameboard and switch with a neighbor tile using an USB keyboard
- Tiles', start screen's, and background's sprites are all AI-generated and stored in SRAM by COE file
- The game logic and state machine is coded in the microprocessor by C, which manipulate the display signal using memory mapped I/O
- Patterns of three tiles of the same color in a line will disappear and new tiles will be generated from the top of the gameboard with animation
- A special tile will be generated for any pattern of more than three in a line. The special tile will eliminate the whole row with special animation

# Code
Some source code is omitted as they are based on previous assignments.

# Material
All original art material

# Documents
Proposal, final reports, presentation video

