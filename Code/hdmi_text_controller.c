

/***************************** Include Files *******************************/
#include "hdmi_text_controller.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sleep.h"

/************************** Function Definitions ***************************/

void hdmiSet(int background, int foreground)
{
	hdmi_ctrl->CTRL = 0x00;
}

void hdmiClr()
{
	for (int i = 0; i<72; i++)
	{
		hdmi_ctrl->VRAM[i] = 0x00;
	}
	hdmi_ctrl->CTRL = 0x08;
	hdmi_ctrl->SCORE = 0x00;
	hdmi_ctrl->LASER = 0x00;
	hdmi_ctrl->START = 0x00;



}

void printTiles(uint8_t* arr)
{
	for(uint8_t i = 0; i <8; i++)
	{
		for(uint8_t j = 0; j<8; j++)
		{
			xil_printf("%x ", arr[i*WIDTH + j]);
		}
		xil_printf("\n\r");
	}
}

void iniTile()
{
	static uint8_t tiles[64];
	for(uint8_t j = 0; j < 64; j++)
	{
		tiles[j] = rand()%NUMELEMENTS;
//		xil_printf("%x \n\r", &hdmi_ctrl);
		xil_printf("%x \n\r", hdmi_ctrl->VRAM[j]);
	}


	uint8_t match = 1;

	while(match != 0)
	{
		match = checkMatchesSimple(tiles);
		xil_printf("match= %x \n\r", match);
		fillTiles(tiles);
		//aniFill(tiles);
		printTiles(tiles);
		//pushTiles(tiles);
	}
	pushTiles(tiles);

	//aniFill(tiles);
	xil_printf("steady state");


}

void setTile(uint8_t tile, uint8_t offset, uint8_t color)
{
	hdmi_ctrl->VRAM[tile] = (offset << 3 )+ color%8;
}

void setLaser(uint8_t on, uint8_t row, uint32_t offset, uint8_t spin)
{
	uint32_t orig = hdmi_ctrl->LASER;
	uint8_t off;
	if(offset>254)
	{
		off= 255;
	}
	else
	{
		off = offset;
	}
	if(offset>254)
	{

	}
	xil_printf("laser_orig= %x \n\r", orig);
	if(on != 0)
	{
		orig = 0b00010000;
		orig = orig + row % 8 + (off<<24) + ((spin%8)<<8);
		hdmi_ctrl->LASER = orig;
	}
	else
	{
		hdmi_ctrl->LASER = 0;
	}


}

void switchTile(uint8_t a, uint8_t b, uint8_t* arr)
{
	if(a != b)
	{
	uint8_t temp = arr[a];
	arr[a] = arr[b];
	arr[b] = temp;
	pushTiles(arr);
	uint8_t match;
	if(arr[a] == LTILE|| arr[b] == LTILE)
	{
		match += 8;
		uint8_t row = 0;
		if(arr[a] == LTILE)
		{
			row = a/8;
			arr[a] = REMOVED;
		}
		else
		{
			row = b/8;
			arr[b] = REMOVED;
		}

		for(uint32_t i = 0; i <256; i+=8)
		{

			setLaser(1, row, i, i/8);
			while(hdmi_ctrl->FRAME !=0)
			{

			}
		}
		setLaser(0, 0, 0, 0);

		for(uint8_t i=0; i<8; i++)
		{
			arr[row*8+i] = REMOVED;
		}
		pushTiles(arr);
	}
	else
	{
		match = checkMatches(arr);
	}

	xil_printf("match= %x \n\r", match);
	if(match != 0)
	{
		addScore(match);
		aniFill(arr);
		match = checkMatches(arr);
		while(match!= 0)
		{
			addScore(match);
			aniFill(arr);
			match = checkMatches(arr);
		}

	}
	else
	{
		arr[b] = arr[a];
		arr[a] = temp;
	}

	pushTiles(arr);
	}
}

void getTiles(uint8_t * tiles)
{
	for(uint8_t i = 8; i < 72; i++)
	{
		tiles[i-8] = hdmi_ctrl->VRAM[i];
	}
}

void pushTiles(uint8_t * tiles)
{
	for(uint8_t i = 8; i < 72; i++)
	{
		hdmi_ctrl->VRAM[i] = tiles[i-8] ;
	}
}

void addScore(uint8_t score)
{
	uint32_t orig = hdmi_ctrl->SCORE;
	xil_printf("orig= %x \n\r", orig);
	uint8_t num[4];
	xil_printf("score= %x \n\r", score);
	for(uint8_t i = 4; i>0; i--)
	{
		num[i-1] = orig % 256 ;
		orig = orig >>8;
	}

	num[0] += score;
	uint32_t final = 0;
	for(uint8_t i = 0; i<3; i++)
	{
		xil_printf("num%x= %x  \n\r",i, num[i]);
		if(num[i]>= 10)
		{
			//xil_printf("num%x= %x  \n\r",i, num[i]);
			num[i+1] +=1;
			num[i] -=10;
		}
	}

	for(uint8_t i = 0; i<3; i++)
	{
		final += num[i];
		final = final << 8;
	}
		final += num[3];
	xil_printf("final= %x \n\r", final);

	hdmi_ctrl->SCORE = final;

}

uint8_t empty(uint8_t* arr)
{
	for(uint8_t i =0; i<64; i++)
	{
		if(arr[i] == REMOVED)
		{
			return 1;
		}
	}
	return 0;
}

void aniFill(uint8_t* arr)
{
	uint8_t counter = 1;
	while(empty(arr) != 0)
	{
		for(uint8_t i = 0; i<8; i++)
		{
			setTile(i, 0, rand()%NUMELEMENTS);
		}

		while(counter != 32)
		{
			for(uint8_t j = 0; j<8; j++)
			{
				uint8_t flag = 0;
				for(uint8_t i = 8; i>0; i--)
				{

					if(arr[(i-1)*WIDTH + j] == REMOVED)
					{
						flag = 1;
					}

					if(flag!=0)
					{
						//setTile(i*WIDTH +j, counter, hdmi_ctrl->VRAM[(i-1)*WIDTH +j]);
						setTile((i-1)*WIDTH +j, counter, hdmi_ctrl->VRAM[(i-1)*WIDTH +j]);
					}

				}
			}

			while(hdmi_ctrl->FRAME != 0)
			{

			}
			counter+=1;
		}

		for(uint8_t j = 0; j<8; j++)
		{
			uint8_t flag = 0;
			for(uint8_t i = 8; i>0; i--)
			{

				if(arr[(i-1)*WIDTH + j] == REMOVED)
				{
					flag = 1;
				}

				if(flag!=0)
				{
					arr[(i-1)*WIDTH + j] = hdmi_ctrl->VRAM[(i-1)*WIDTH +j]%8;
				}

			}
		}

		pushTiles(arr);
		counter = 1;
	}


}

void fillTiles(uint8_t* arr)
{
    int write_idx;
       // Iterate through each column
       for (int j = 0; j < WIDTH; j++) {
         write_idx = WIDTH - 1;
         for (int i = WIDTH - 1; i >= 0; i--) {
           if (arr[i * WIDTH + j] != REMOVED) {
             arr[write_idx * WIDTH + j] = arr[i * WIDTH + j];
             write_idx--;
           }
         }

         // Fill empty spaces with random values (up to n elements)
         for (int i = write_idx; i >= 0; i--) {
           arr[i * WIDTH + j] = rand() % NUMELEMENTS;
         }
         xil_printf("\n");
       }
}

uint8_t checkMatches(uint8_t* arr)
{
	//set updatiles to arr
	 uint8_t updatedTiles[WIDTH * WIDTH];
	    uint8_t match = 0;
	    for (int i = 0; i < WIDTH; i++) {
	        for (int j = 0; j < WIDTH; j++) {
	            updatedTiles[i * WIDTH + j] = arr[i * WIDTH + j];
	        }
	    }
	//check horizontal matches
	    for (int i = 0; i < WIDTH; i++) {
	        for (int j = 0; j< WIDTH-2; j ++) {
	        	if(j < WIDTH - 4 && arr[i * WIDTH + j] == arr[i * WIDTH + j+1] && arr[i * WIDTH + j] == arr[i * WIDTH + j+2]
					&& arr[i * WIDTH + j+2] == arr[i * WIDTH + j+3] && arr[i * WIDTH + j+3] == arr[i * WIDTH + j+4] )
	        	{
					for(uint8_t k=0; k<5; k++)
					{
						if(updatedTiles[i* WIDTH + j+k] != LTILE)
						{
						 updatedTiles[i* WIDTH + j+k] = REMOVED;
						}

						if(k==2)
						{
							updatedTiles[i* WIDTH + j+k] = LTILE;
						}
					}


	        	}

	        	if(j < WIDTH - 3 && arr[i * WIDTH + j] == arr[i * WIDTH + j+1]
					&& arr[i * WIDTH + j] == arr[i * WIDTH + j+2] && arr[i * WIDTH + j+2] == arr[i * WIDTH + j+3])
	        	{
					for(uint8_t k=0; k<4; k++)
					{
						if(updatedTiles[i* WIDTH + j+k] != LTILE)
						{
						 updatedTiles[i* WIDTH + j+k] = REMOVED;
						}

						if(k==2)
						{
							updatedTiles[i* WIDTH + j+k] = LTILE;
						}
					}


	        	}

	            if (arr[i * WIDTH + j] == arr[i * WIDTH + j+1] && arr[i * WIDTH + j] == arr[i * WIDTH + j+2]) {
	                xil_printf("horizontal match at %d %d \n", i, j);
					for(uint8_t k=0; k<3; k++)
					{
						if(updatedTiles[i* WIDTH + j+k] != LTILE)
						{
						 updatedTiles[i* WIDTH + j+k] = REMOVED;
						}
					}
	                match += 1;
	            }
	        }
	    }
	//check for vertical matches
	    for (int i = 0; i < WIDTH-2; i++) {
	        for (int j = 0; j< WIDTH; j ++) {
	        	if (i<WIDTH -4 &&arr[i * WIDTH + j] == arr[(i+1) * WIDTH + j]
					&& arr[i * WIDTH + j] == arr[(i+2)* WIDTH + j] && arr[i * WIDTH + j] == arr[(i+3)* WIDTH + j]
					&&arr[i * WIDTH + j] == arr[(i+4)* WIDTH + j]) {
	        		for(uint8_t k=0; k<5; k++)
	        		{
	        			if(updatedTiles[(i+k) * WIDTH + j] != LTILE)
	        			{
	        				updatedTiles[(i+k) * WIDTH + j] = REMOVED;
	        			}

	        			if(k==3)
	        			{
	        				updatedTiles[(i+k) * WIDTH + j] = LTILE;
	        			}
	        		}

	        	}

	        	if (i<WIDTH -3 &&arr[i * WIDTH + j] == arr[(i+1) * WIDTH + j]
	        	&& arr[i * WIDTH + j] == arr[(i+2)* WIDTH + j] && arr[i * WIDTH + j] == arr[(i+3)* WIDTH + j])
	        	{
					for(uint8_t k=0; k<4; k++)
					{
						if(updatedTiles[(i+k) * WIDTH + j] != LTILE)
						{
						 updatedTiles[(i+k) * WIDTH + j] = REMOVED;
						}

						if(k==2)
						{
							updatedTiles[(i+k) * WIDTH + j] = LTILE;
						}
					}

	        	}

	            if (arr[i * WIDTH + j] == arr[(i+1) * WIDTH + j] && arr[i * WIDTH + j] == arr[(i+2)* WIDTH + j]) {
	                xil_printf("vertical match at %d %d \n", i, j);
	                for(uint8_t k=0; k<3; k++)
	                {
	                	if(updatedTiles[(i+k) * WIDTH + j] != LTILE)
	                	{
	                		updatedTiles[(i+k) * WIDTH + j] = REMOVED;
	                	}


	                }
	                match += 1;
	            }
	        }
	    }

	    for (int i = 0; i < WIDTH; i++) {
	        for (int j = 0; j < WIDTH; j++) {
	           arr[i * WIDTH + j] = updatedTiles[i * WIDTH + j];
	        }
	    }
	    pushTiles(arr);
	    //sleep(1);
	    return match;
}

uint8_t checkMatchesSimple(uint8_t* arr)
{
	//set updatiles to arr
	 uint8_t updatedTiles[WIDTH * WIDTH];
	    uint8_t match = 0;
	    for (int i = 0; i < WIDTH; i++) {
	        for (int j = 0; j < WIDTH; j++) {
	            updatedTiles[i * WIDTH + j] = arr[i * WIDTH + j];
	        }
	    }
	//check horizontal matches
	    for (int i = 0; i < WIDTH; i++) {
	        for (int j = 0; j< WIDTH-2; j ++) {
	            if (arr[i * WIDTH + j] == arr[i * WIDTH + j+1] && arr[i * WIDTH + j] == arr[i * WIDTH + j+2]) {
	                xil_printf("horizontal match at %d %d \n", i, j);
	                updatedTiles[i * WIDTH + j] = REMOVED;
	                updatedTiles[i * WIDTH + j+1] = REMOVED;
	                updatedTiles[i * WIDTH + j+2] = REMOVED;
	                match += 1;
	            }
	        }
	    }
	//check for vertical matches
	    for (int i = 0; i < WIDTH-2; i++) {
	        for (int j = 0; j< WIDTH; j ++) {
	            if (arr[i * WIDTH + j] == arr[(i+1) * WIDTH + j] && arr[i * WIDTH + j] == arr[(i+2)* WIDTH + j]) {
	                xil_printf("vertical match at %d %d \n", i, j);
	                updatedTiles[i * WIDTH + j] = REMOVED;
	                updatedTiles[(i+1)* WIDTH + j] = REMOVED;
	                updatedTiles[(i+2)* WIDTH + j] = REMOVED;
	                match += 1;
	            }
	        }
	    }

	    for (int i = 0; i < WIDTH; i++) {
	        for (int j = 0; j < WIDTH; j++) {
	           arr[i * WIDTH + j] = updatedTiles[i * WIDTH + j];
	        }
	    }
	    pushTiles(arr);
	    //sleep(1);
	    return match;
}

void refresh(uint8_t* arr)
{
	for(uint8_t i = 8; i < 72; i++)
		{
			arr[i-8] = REMOVED;
			hdmi_ctrl->VRAM[i] = REMOVED;
		}
	uint8_t match = 1;

		while(match != 0)
		{
			match = checkMatchesSimple(arr);
			aniFill(arr);
			//pushTiles(tiles);
		}
}

void hdmiTestWeek1(uint8_t key)
{
	xil_printf("%x \n\r", key);
	static uint8_t select = 0;
	static uint8_t tiles[64];

	if(key != 0)
	{
		hdmi_ctrl->START = 1;
	}

	getTiles(tiles);

	uint8_t move;
	uint8_t row = select & 0b11111000;
	uint8_t col = select & 0b00000111;
	switch(key){
	case 26:
		// w
		if(row == 0b0)
		{
			select = 0b00111000 + col;
		}
		else
		{
			select -=8;
		}
		move = select;
		break;
	case 4:
		//a
		select -= 1;
		col = select & 0b00000111;
		select = row + col;
		move = select;
		break;
	case 22:
		//s
		if(row == 0b00111000)
		{
			select = col;
		}
		else
		{
			select +=8;
		}
		move = select;
		break;
	case 7:
		//d
		select += 1;
		col = select & 0b00000111;
		select = row + col;
		move = select;
		break;

	case 79:
		//right
		if(select != row + 0b00000111)
			move = select + 1;
		else
			move = select;
		break;
	case 80:
		//left
		if(select != row )
			move = select -1;
		else
			move = select;
		break;
	case 81:
		//down
		if(row != 0b00111000)
			move = select + 8;
		else
			move = select;
		break;
	case 82:
		//up
		if(row != 0b00000000)
			move = select - 8;
		else
			move = select;
		break;
	case 21:
		//r
		move = select;
		refresh(tiles);
		break;
	default:
			move = select;

	}

	hdmi_ctrl->CTRL = select+8;
	xil_printf("%x \n\r", select);
	xil_printf("%x \n\r", hdmi_ctrl->CTRL);


	switchTile(select, move, tiles);
//
//	checkMatches(tiles);
//	pushTiles(tiles);



}
