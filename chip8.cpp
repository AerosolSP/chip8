#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*This array defines the CHIP-8's fontset. Recall that the system draws entire sprites to pixels on the screen in a given location
 * and you may realize what this array is doing. Each character is 4 pixels across, and 5 pixels down. Look at the very first entry,
 * 0xF0, and recall that in binary, 0xF0 is 1111 0000. The CHIP-8 ignores the last 4 bits, so when it sees 1111, it knows to fill the first
 * line of pixels for a character in. The next entry, 0x90, is 1001 0000 in binary. So it knows to fill the first pixel, skip the next two,
 * and then fill the last pixel.*/
unsigned char fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

emu::emu() {
	//Empty constructor
}

emu::~emu() {
	//Empty destructor
}

/*Now we're getting to the meat of the emulator, where we implement the functions we defined in the header file. So, before we can do anything
 * with the CHIP-8 rom we're going to emulate...we must load it!*/

//First, a little bit of house cleaning. We'll be using this in the loadRom function, but we need to define it here. Alternatively, we could
//just define it in the loadRom function, but it helps if you think of this as a separate step.
void emu::initialize_chip8() {
	pc = 0x200; 	//We set the program counter to 0x200, because that's where the ROM address starts on the system
	opcode = 0;	//Reset the opcode...
	index = 0;	//...the index register...
	sp = 0;		//...and the stack pointer
	
	for(int i = 0; i < 2048; i++) {
		graphics[i] = 0;	//Clearing the graphics array. Just make it all 0...there are 2048 items in the array, so we loop that many times
	}
	
	for(int i = 0; i < 16; i++) {
		stack[i] = 0;		//Clear the stack. 16 items, 16 loops.
	}
	
	for(int i = 0; i < 16; i++) {
		input[i] = registers[i] = 0;	//Clear the input and cpu registers too!
	}
	
	for(int i = 0; i < 4096; i++) {
		mem[i] = 0;			//Cleaning cleaning....
	}
	
	for(int i = 0; i < 80; i++) {
		mem[i] = fontset[i];	//We'll load the fontset as part of the initialization process. It needs to be here for the system to use!
	}
	
	//Resetting system timers
	delayTimer = 0;
	soundTimer = 0;
	
	//And finally, clear the screen! Just the once is fine.
	drawFlag = true;
	
	srand(time(NULL));
}
 
bool emu::loadRom(const char * fileName) {
	initialize_chip8(); 					//Picture this as turning the CHIP-8 on, if it helps.
	printf("Starting %s...\n", fileName); 	// Just a little debug line, pretty self-explanatory
	
	FILE * pFile = fopen(fileName, "rb"); 	//Look up fopen if you need more help here, but briefly, fopen returns a stream that can be 
											//referred to with pFile. It takes two parameters, the path of the file in question and
											//the mode. "rb" here means "read and binary mode". That's what we need!
	if(pFile == NULL) {
		fputs("Error loading rom. ", stderr); 	//Mostly self-explanatory. Just remember that fputs prints to a file stream, and stderr
													//is a file stream that's part of the stdlib.
		return false;								//If something screws up loading the rom, we're gonna want to abort the whole thing.
	}
	
	//Next up, we're gonna check the size of the rom. We'll need to do this to make sure it will fit in the emulated system's memory!
	
	fseek(pFile, 0, SEEK_END);	//fseek just moves the file position indicator. Read this as "move the indicator 0 bytes from the end of the
								//file pointed to by pFILE".
	long romSize = ftell(pFile);//Since we're at the end of the file, ftell will tell us how many bytes there are from the end of the file
								//to the beginning. We'll dump that info in a long called romSize and use it later.
	rewind(pFile);				//Now go back to the beginning of pFile. You rewind VHS tapes when you finish the movie right? :)
	printf("Rom size: %d\n", (int)romSize); //Self explanatory. Note the type cast for romSize. We want it as a long, but we don't need to
											//print it as one, right?
	
	char * buffer = (char*)malloc(sizeof(char) * romSize);	//malloc returns a void* pointer to the beginning of a block of memory. We'll
															//be needing to dereference this block of memory later (in order to fill the
															//emulated system memory with the same data) so what we want as a char* pointer.
															//Next, we want the size of the block of memory to be the size of a char
															//multiplied by how many bytes are in the the rom. Or in otherwords...the romsize.
															//So malloc's parameter becomes "sizeof(char) * romSize". Easy!
	if(buffer == NULL) {
		fputs("Memory error!", stderr);						//Quick sanity check. If malloc failed to make a buffer for us...
		return false;										//We want to abort the rest of what we're doing.
	}
	
	/*So we've loaded the file, we've recorded it's size, and we've allocated memory for it. Next up is to copy it to the emulated system's
	 * memory*/
	 
	size_t result = fread(buffer, 1, romSize, pFile);		//fread takes, in this order, a pointer to a block of memory to write to, the size
															//in bytes for each read, the number of reads to perform, and the file stream to
															//read from. fread returns the number of successfully read elements.
	
	if(result != romSize) { 								//In our code, result and romSize should be the same size. If they're not, something's wrong.
		fputs("Reading error", stderr);
		return false;										//So abort!
	}
	
	if((4096-512) > romSize) {								//ROM goes into system memory from location 0x200 (512). We don't want to do
		for(int i = 0; i < romSize; i++) {
			mem[i+512] = buffer[i];
		}
	}
	else {
		printf("ROM too large for memory!");
	}
	
	//Okay, we're done with the file now. It should be in program memory and emulated system memory at this point. Let's clean up
	//and close the file and free the buffer. And return true at the end, since everything should've been successful
	fclose(pFile);
	free(buffer);
	
	return true;
	
}

//Moment of truth. The system is initialized. The file is loaded and in the emulated system's memory. Now we fetch, decode, and execute.
//Exciting!

void emu::emuCycle()
{
	//Remember the first step? Fetching the opcode!
	
	opcode = mem[pc] << 8 | mem[pc+1]; 	//What happened here? Recall that an opcode is two bytes long, but each element of our memory array
										//is only one byte long. We need both bytes to know what the opcode is. So we get the first byte at
										//location indicated by our program counter (pc), and we shift the bits 8 bits to the left. Meaning,
										//if what we got was 0x00FF (0000 0000 1111 1111), we now have 0xFF00 (1111 1111 0000 0000). Then, we
										//OR it against the next element (pc+1). So if it's 0x00A2 (0000 0000 1010 0010) we now have, in
										//total, 1111 1111 1010 0010 stored in our opcode variable. Remember, OR means if either bit is SET,
										//the result is a SET bit. Look at the last 8 bits of both examples. See how it works now? Look up
										//bitwise OR for more help later.
										
	/*In any event, we've fetched our opcode now. Now we have to decode it. The CHIP-8 has 35 opcodes. You can find the opcode table
	 * at https://en.wikipedia.org/wiki/CHIP-8#Opcode_table. Once we have the opcode, we need to know which specific opcode it is in 
	 * order to know what we should do with it. The way we do this is by using bitwise AND. Take the first instruction in the table
	 * for instance. 0x0NNN means we should execute the RCA 1802 program at address NNN. So if our opcode is 0x0FFF, the first four bits are
	 * 0b0000. If we AND 0x0FFF against 0xF000, we'll end up with 0x0000, meaning the first four bits are 0b000. Look at the table again and
	 * you'll notice that 3 instructions will fit that criteria. Not a problem, we'll just do another switch test that will identify any one 
	 * of these 3 instructions. Another example? ANNN sets the index register to the address NNN. 0xANNN AND 0xF000 will give you 0xA000, so
	 * now you know which instruction it is! Since this is an interpreter, this is basically going to be our pattern. Test for a certain case
	 * and, if it's satisfied, execute that opcode. Fetch, decode, execute!*/
	 
	switch(opcode & 0xF000)
	{
		case(0x0000):													//Three commands satisfy this test, so we need more tests.
			switch(opcode & 0x0FFF)
			{
				case(0x00E0):
					//SCREEN CLEAR!
					pc+= 2;
					break;
				
				case(0x00EE):
					//RETURN FROM SUBROUTINE
					pc = stack[--sp];
					pc += 2;					
					break;
					
				default:
					printf("RUN MACHINE CODE AT ADDRESS 0x0NNN (DEPRECATED\n");				
			}
		break;
		
		case(0x1000):
			//JUMP TO SUBROUTINE AT ADDRESS 0x1NNN
			pc = opcode & 0x0FFF;
			break;
		
		case(0x2000):
			//CALL SUBROUTINE AT ADDRESS 0x2NNN
			stack[sp++] = pc;
			pc = opcode & 0x0FFF;			
			break;
		
		case(0x3000):
			//SKIP THE NEXT INSTRUCTION IF VX EQUALS NN (0x3XNN)
			if(registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		
		case(0x4000):
			//SKIP THE NEXT INSTRUCTION IF VX DOES NOT EQUAL NN (0x4XNN)
			if(registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		
		case(0x5000):
			//SKIP THE NEXT INSTRUCTION IF VX = VY (0x5XY0)
			if(registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4]){
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		
		case(0x6000):
			//SET VX TO NN (0x6XNN)
			registers[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			pc += 2;
			break;
		
		case(0x7000):
			//ADD NN TO VX WITHOUT SETTING CARRY FLAG (0x7XNN)
			registers[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			pc += 2;
			break;
			
		case(0x8000):													//Several instructions satisfy this test, so we need another
			switch(opcode & 0x000F)										//It's only the last hex digit that changes between them, so this
			{
				case(0x0000): //0x8000...								//test covers them.
					//SET VX to VY (0x8XY0)
					registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				case(0x0001): //0x8001...and so on
					//SET VX to VX|=VY (0x8XY1)
					registers[(opcode & 0x0F00) >> 8] |= registers[(opcode & 0x00F0) >> 4]; 
					pc += 2;
					break;
				
				case(0x0002):
					//SET VX to VX&=VY
					registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
					
				case(0x0003):
					//SET VX to VX^=VY
					registers[(opcode & 0x0F00) >> 8] ^= registers[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				case(0x0004):
					//ADD VY TO VX (0x8XY4). SET CARRY FLAG IF REQUIRED
					if(registers[(opcode & 0x00F0) >> 4] > (0xFF - registers[opcode & 0x0F00])){
						registers[0xF] = 1;
					} else {
						registers[0xF] = 0;
					}
					registers[(opcode & 0x0F00) >> 8] += registers[(opcode & 0x00F0) >> 4];
					pc += 2;					
					break;
					
				case(0x0005):
					//SUBTRACT VY FROM VX. UNSET CARRY FLAG IF REQUIRED
					if(registers[(opcode & 0x00F0) >> 4] > registers[(opcode & 0x0F00) >> 8]) {
						registers[0xF] = 0;
					} else {
						registers[0xF] = 1;
					}					
					registers[(opcode & 0x0F00) >> 8] -= registers[(opcode & 0x00F0)];
					pc += 2;
					break;
				
				case(0x0006):
					//STORE LEAST SIGNIFICANT BIT OF VX IN VF, THEN SHIFT VX >> 1
					registers[0xF] = registers[(opcode & 0x0F00) >> 8] & 0x1;
					registers[(opcode & 0x0F00) >> 8] >>= 1;
					pc+=2;
					break;
				
				case(0x0007):
					//SUBTRACT VX FROM VY AND STORE RESULT IN VX. VF CLEARED IF THERE'S A BORROW, SET IF NOT
					if(registers[(opcode & 0x0F00) >> 8] > registers[(opcode & 0x00F0) >> 4]) {
						registers[0xF] = 0;
					} else {
						registers[0xF] = 1;
					}
					
					registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] - registers[(opcode & 0x0F00) >> 4];
					pc += 2;
					break;
					
				case(0x000E):
					//STORE THE MOST SIGNIFICANT BIT OF VX in VF, THEN SHIFT VX << 1
					registers[0xF] = registers[(opcode & 0x0F00) >> 8] >> 7;
					registers[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
					break;
					
				default:
					printf("UNIMPLEMENTED OPCODE: %0.4X\n", opcode);
				}
				break;
					
		case(0x9000):
			//SKIPS NEXT INSTRUCTION IF VX != VY
			if(registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4]) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
			
		case(0xA000):
			//SETS I TO ADDRESS NNN
			index = opcode & 0x0FFF;
			pc += 2;
			break;
			
		case(0xB000):
			//JUMP TO THE ADDRESS NNN PLUS V0
			pc = (opcode & 0x0FFF) + registers[0];
			break;
			
		case(0xC000):
			//SET VX to "rand() & NN"
			registers[(opcode & 0x0F00) >> 8] = (rand()%0xFF) & (opcode & 0x00FF);
			pc += 2;
			break;
			
		case(0xD000):
			//DXYN. DRAW INSTRUCTIONS. A DOOZY. DO IT LATER.
			{
				unsigned short x = registers[(opcode & 0x0F00) >> 8];
				unsigned short y = registers[(opcode & 0x00F0) >> 4];
				unsigned short height = opcode & 0x000F;
				unsigned short pixel;
				
				registers[0xF] = 0;
				for(int yline = 0; yline < height; yline++)
				{
					pixel = mem[index + yline];
					for(int xline = 0; xline < 8; xline++)
					{
						if((pixel & (0x80 >> xline)) != 0)
						{
							if(graphics[(x+xline+((y+yline) * 64))] == 1)
							{
								registers[0xF] = 1;
							}
							graphics[(x+xline+((y+yline) * 64))] ^= 1;
						}
					}
				}
			}
				drawFlag = true;
				pc += 2;
				break;
			
		case(0xE000):
			switch(opcode & 0x00FF)
			{
				case(0x009E):
					//EX9E SKIP THE NEXT INSTRUCTION IF KEY IN VX IS PRESSED
					if(input[registers[(opcode & 0x0F00) >> 8]] != 0)
					{
						pc += 4;
					} else
					{
						pc += 2;
					}
					break;
				case(0x00A1):
					//EXA1 SKIP THE NEXT INSTRUCTION IF THE KEY IN VX ISN'T PRESSED
					if(input[registers[(opcode & 0x0F00) >> 8]] = 0)
					{
						pc += 4;
					} else
					{
						pc += 2;
					}
					pc += 2;
					break;
				default:
					printf("Something broke. Bad.\n");
				}
				break;
					
		case(0xF000):
			switch(opcode & 0x00FF)
			{
				case(0x0007):
					//FX07 SET VX TO VALUE OF DELAY TIMER
					registers[(opcode & 0x0F00) >> 8] = delayTimer;
					pc += 2;
					break;
					
				case(0x000A):
				{
					//FX0A WAITING FOR A KEY PRESS TO STORE IN VX
					//INSTRUCTIONS HALTED UNTIL KEY PRESS, SO DO NOT ADVANCE PC UNTIL KEY PRESS EXISTS
					
					bool keyPress = false;
					
					for(int i = 0; i < 16; i++)
					{
						if(input[i] != 0)
						{
							registers[(opcode & 0x0F00) >> 8] = i;		//We need to set the right register to the value of the keypress
							keyPress = true;							//And then flag that key has been pressed.
						}
					}
					
					if(!keyPress)										//If after looping through the entire input array no key has been
					{													//pressed...
						return;											//...we need to jump back and try again.
					}
					
					pc += 2;											//Only after a successful test can we increment the PC
					break;
				}
					
				case (0x0015):
					//FX15 SET DELAY TIMER TO VX
					delayTimer = registers[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
					
				case(0x0018):
					//FX18 SET SOUND TIMER TO VX
					soundTimer = registers[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
					
				case(0x001E):
					//FX1E ADD VX TO I
					index += registers[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
					
				case(0x0029):
					//FX29 SET INDEX TO LOCATION OF SPRITE FOR THE CHARACTER IN VX
					index = registers[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
					break;
					
				case(0x0033):
					//FX33 STORE THE BCD REPRESENTATION OF VX IN I
					mem[index]  = registers[(opcode & 0x0F00) >> 8] / 100;
					mem[index+1]= (registers[(opcode & 0x0F00) >> 8] / 10) % 10;
					mem[index+2]= (registers[(opcode & 0x0F00) >> 8] % 100) % 10;
					pc += 2;
					break;
					
				case(0x0055):
					//FX55 STORE V0 TO VX IN MEMORY STARTING AT ADDRESS AT INDEX
					
					for(int i = 0; i < registers[(opcode & 0x0F00) >> 8]; i++)
					{
						mem[index+i] = registers[i];
					}
					pc += 2;
					break;
					
				case(0x0065):
					//FX65 COPY MEMORY VALUES STARTING AT INDEX INTO REGISTERS V0 THRU VX
					
					for(int i = 0; i < registers[(opcode & 0x0F00) >> 8]; i++)
					{
						registers[i] = mem[index+i];
					}
					pc += 2;
					break;
					
				default:
					printf("UNIMPLEMENTED OPCODE: %0.4X\n", opcode);
				}
				break;
					
		default:
			printf("UNIMPLEMENTED OPCODE: %0.4X\n", opcode);
		
	}		
	//UPDATE TIMERS
	if(delayTimer > 0)
	{
		--delayTimer;
	}
	
	if(soundTimer > 0)
	{
		if(soundTimer == 1)
		{
			--soundTimer;
			printf("HONK\n");
		}
	}
}

void emu::debugRender()
{
	// Draw
	for(int y = 0; y < 32; ++y)
	{
		for(int x = 0; x < 64; ++x)
		{
			if(graphics[(y*64) + x] == 0) 
				printf("O");
			else 
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}

		
	

