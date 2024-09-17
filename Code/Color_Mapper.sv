//-------------------------------------------------------------------------
//    Color_Mapper.sv                                                    --
//    Stephen Kempf                                                      --
//    3-1-06                                                             --
//                                                                       --
//    Modified by David Kesler  07-16-2008                               --
//    Translated by Joe Meng    07-07-2013                               --
//    Modified by Zuofu Cheng   08-19-2023                               --
//                                                                       --
//    Fall 2023 Distribution                                             --
//                                                                       --
//    For use with ECE 385 USB + HDMI                                    --
//    University of Illinois ECE Department                              --
//-------------------------------------------------------------------------


module  color_mapper ( input  logic [9:0] DrawX, DrawY, 
                       input  logic frame_clk,
                       input  logic clk_100MHz,
                       input  logic clk_25MHz,
                       
                       
                       input  logic        Reset,
                       
                       
                       input logic [31:0]up_reg,
                       input logic [31:0]cur_reg,
                       input logic [31:0]score,
                       input logic [31:0] select,
                       input logic [31:0] laser,
                       input logic [31:0] start,
                       
                       output logic [6:0] upTile,
                       output logic [6:0] curTile,
                       output logic [3:0]  Red, Green, Blue );
    
    logic field_on, score_on, rainbow_on;
    logic [4:0] blink_c;
    logic [5:0] tile_x, tile_y;
    logic [6:0] score_x, score_y;
    logic [7:0] numbits;
    logic [10:0] num_addr;
    logic [2:0] cur_bit;
    logic [3:0] rainb_read;
    logic [3:0] bgd_read;
    logic [3:0] start_read; 
    logic [16:0] bgd_addr;
    

    logic [5:0] row, col;
    int offsetX, offsetY;
    logic[10:0] offScoreX, offScoreY;
    assign offsetX = 192;
    assign offsetY = 112;
//    assign offsetX = 0;
//    assign offsetY = 0;	 
    assign offScoreX = 448;
    assign offScoreY = 64;
//    assign offScoreX = 0;
//    assign offScoreY = 0;
    
    logic [10:0] DistScoreX, DistScoreY;
    assign DistScoreX = DrawX - offScoreX;
    assign DistScoreY = DrawY - offScoreY;
    
    logic [10:0] DistX, DistY;
    assign DistX = DrawX - offsetX;
    assign DistY = DrawY - offsetY;
    
    assign score_on = (DrawX >= offScoreX && DrawY >= offScoreY 
                        && DrawX < offScoreX + 64 && DrawY < offScoreY + 32);
    
    assign score_x = DistScoreX[6:0];
    assign score_y = DistScoreY[6:0];
                        
    assign field_on = (DrawX >= offsetX && DrawY >= offsetY 
                           && DrawX < offsetX + 256 && DrawY < offsetY + 256); 
                           
    assign tile_x = DistX[4:0];
    assign tile_y = DistY[4:0];
    
    assign col = DistX[9:5];
    assign row = DistY[9:5];      
    
    assign bgd_addr = DrawY[9:1]* 320 + DrawX[9:1];            
                           
             // sprite
    localparam SP_WIDTH  = 32;  // sprite width in pixels
    localparam SP_HEIGHT = 224;  // sprite width in pixels
    localparam SP_PIXELS = SP_WIDTH * SP_HEIGHT;  // total pixels in buffer
    localparam SP_ADDRW  = $clog2(SP_PIXELS);  // address width
    localparam SP_DATAW  = 4;  // colour bits per pixel
   
    localparam CHANW = 4;        // colour channel width (bits)
    localparam COLRW = CHANW*3;  // colour width: three channels (bits)
    localparam CIDXW = 4;        // colour index width (bits)
    // pixel read address and colour
    logic [SP_ADDRW:0] SP_addr_read;
    logic [SP_DATAW-1:0] SP_colr_read;
    logic [3:0] tile_r, tile_g, tile_b;
    logic [3:0] clr_r, clr_g, clr_b;
    logic [3:0] rainb_r, rainb_g, rainb_b;
    logic [3:0] bgd_r, bgd_g, bgd_b;
    logic [3:0] start_r, start_g, start_b;
	logic [7:0] up_r[4];
	logic [7:0] cur_r[4];
	logic [3:0] num_r[4];
	logic [1:0]updown; //0 for back, 1 for up, 2 for cur
	
    assign upTile = row*4'd8 + col;
    assign curTile = upTile +4'd8;
    
     always_comb 
     begin
        up_r[0] = up_reg[7:0];
        up_r[1] = up_reg[15:8];
        up_r[2] = up_reg[23:16]; 
        up_r[3] = up_reg[31:24];       
    end
    
    always_comb 
    begin
        cur_r[0] = cur_reg[7:0];
        cur_r[1] = cur_reg[15:8];
        cur_r[2] = cur_reg[23:16]; 
        cur_r[3] = cur_reg[31:24];       
    end
    
    always_comb 
    begin
        num_r[0] = score[3:0];
        num_r[1] = score[11:8];
        num_r[2] = score[19:16]; 
        num_r[3] = score[27:24];               
    end
    
    assign num_addr = 16*(8'h30+ num_r[score_x[5:4]]) + score_y[4:1];
    
    always_comb 
    begin
        //laser[2:0] chose row/col number
        //laser[3] 0/1 horizontal/vertical
        //laser[4] on/off
        //laser[10:8] color offset
        //laser[31:24] pixel
        rainbow_on = laser[4] && ((row) == laser[2:0])&& (DistX <= laser[31:24]);
        rainb_read = DistY[4:2]+laser[10:8] + DistX[4:2];
    end
    
    always_comb begin    
        if(tile_y < up_r[upTile[1:0]][7:3])
            begin
                if(up_r[upTile[1:0]][2:0] == 3'd7)
                    begin
                        updown = 0;
                        SP_addr_read = 0;
                    end
                else
                    begin
                        updown = 1;      
                        SP_addr_read = tile_x + (6'd32- up_r[upTile[1:0]][7:3] + tile_y )*6'd32 + 12'd1024 *up_r[upTile[1:0]][2:0];                 
                    end
            end
        else if(tile_y >= cur_r[curTile[1:0]][7:3])
            begin
                if(cur_r[curTile[1:0]][2:0] == 3'd7)
                    begin
                        updown = 0;
                        SP_addr_read = 0;
                    end
                else
                    begin
                        updown = 2;
                        SP_addr_read = tile_x + (tile_y - cur_r[curTile[1:0]][7:3])*6'd32 + 12'd1024 *cur_r[curTile[1:0]][2:0];  
                    end
            end
        else
            begin
                updown = 0;
                SP_addr_read = 0;
            end
            
    end
    
    font_rom num_rom(
        .addr(num_addr),
        .data(numbits)
    );
    

    blk_mem_gen_0 tiles(
        .addra(SP_addr_read),
        .clka(clk_100MHz),
        .douta(SP_colr_read),
        .ena(1'b1)
    );
    
    blk_mem_gen_1 background(
        .addra(bgd_addr),
        .clka(clk_100MHz),
        .douta(bgd_read),
        .ena(1'b1)
    );
    
    blk_mem_gen_2 startscreen(
        .addra(bgd_addr),
        .clka(clk_100MHz),
        .douta(start_read),
        .ena(1'b1)
    );    
    
    rainbow_palette rainbow(
        .index(rainb_read[2:0]),
        .red(rainb_r),
        .green(rainb_g),
        .blue(rainb_b)
    ); 
    
    background_palette bgd_palette(
        .index(bgd_read),
        .red(bgd_r),
        .green(bgd_g),
        .blue(bgd_b)
    );   
    
    newStartScreen_palette start_palette(
        .index(start_read),
        .red(start_r),
        .green(start_g),
        .blue(start_b)
    );   
     
    tiles_424_palette clut(
        .index(SP_colr_read),
        .red(tile_r),
        .green(tile_g),
        .blue(tile_b)
    );                         
                           
    always_ff @(posedge frame_clk)
    begin: select_blink
        if(Reset)
        blink_c <= 5'b0;
        else
        blink_c <= blink_c+1;
    end

    always_comb
    begin
        if(tile_r == 4'd0 && tile_g == 4'd0 && tile_b == 4'd0)
            begin
                clr_r =  bgd_r;
                clr_g =  bgd_g;
                clr_b =  bgd_b;
            end
        else
            begin
                clr_r =  tile_r;
                clr_g =  tile_g;
                clr_b =  tile_b;
            end
         
    end
       
    always_comb
    begin:RGB_Display
        if(start[0] == 1'b0)
        begin
            Red = start_r;
            Green = start_g;
            Blue = start_b;
        end
        else if (field_on) 
        begin
            if(rainbow_on)
            begin
                Red = rainb_r;
                Green = rainb_g;
                Blue = rainb_b;
            end
            else if(tile_x == 6'd0 || tile_x == 6'd31 || tile_y == 6'd0 || tile_y == 6'd31 ||
                    tile_x == 6'd1 || tile_x == 6'd30 || tile_y == 6'd1 || tile_y == 6'd30)
            begin
                if(blink_c && (curTile == select[7:0]))
                begin
                    Red = 4'hA;
                    Green = 4'h2;
                    Blue = 4'hF;                   
                end
                else
                begin
                    Red = clr_r; 
                    Green = clr_g;
                    Blue = clr_b;
                end
            end 
            else if(updown != 2'b0)
                begin
                    Red = clr_r;
                    Green = clr_g;
                    Blue = clr_b;
                end
                else begin
                    Red = bgd_r;
                    Green = bgd_g;
                    Blue = bgd_b;
                end
        end       
        else if(score_on)
        begin
            if(score_x == 6'd0 || score_x == 6'd63 || score_y == 6'd0 || score_y == 6'd31)
            begin
                        Red = 4'h2;
                        Green = 4'h4;
                        Blue = 4'hF;
            end 
            else 
            begin
                cur_bit = 3'b111 - score_x[3:1];
                if(numbits[cur_bit])
                    begin
                        Red = 4'hA;
                        Green = 4'h2;
                        Blue = 4'hF; 
                    end  
                else begin
                    Red = bgd_r;
                    Green = bgd_g;
                    Blue = bgd_b;   
                end    
            end                    
        end          
        else begin 
           Red = bgd_r;
           Green = bgd_g;
           Blue = bgd_b;
        end      
    end 
    
 
    
    
endmodule
