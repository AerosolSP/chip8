class emu {
	public:								//Other parts of our emulator may need to access these functions, so we'll put these under public methods
		emu();
		~emu();

		bool drawFlag;

		void debugRender(); 			//Renders sprites as '0's and ' 's without a gfx backend to actually draw on the screen


		void emuCycle(); 				// A full cycle fetches the opcode, decodes it, and executes it. This function will be responsible for
										// all three of these tasks.
		bool loadRom(const wchar_t * fileName);// We also need a function to load the ROM into the program memory, and fill the emulated memory's
										   // array with the data. This function achieves that, and requires the filepath of the rom we're
										   // emulating in order to function, hence the parameter. Why a const *char? Because we will get
										   // this information from the command line when the program is run, and thus it will not change
										   // at runtime and is required to be a pointer.

	/*The definition for a CHIP-8 system is below.*******************************************************************************************/

		unsigned char graphics[64*32]; //The CHIP-8 uses a 64x32 sprite grid for drawing. This array facilitates that. Each sprite on the
										//grid is defined by an 8-bit value. The 'char' datatype is 8-bits (1 byte), so we can use an array of
										//chars to represent our sprite grid.

		unsigned char input[16]; 		//The CHIP-8 has a keyboard with 16 values, and each value is 8-bits in size. Another array of chars works
										//for that purpose.

	private: 							//Everything from here onwards is part of the internal working of the CPU core. No other parts of
										//our application need to modify anything here. Things like the variables to hold opcodes,
										//arrays to represent emulator memory, and internal system timers go here.

		/*All of these values are 2-byte values on the CHIP-8 system. They can't be negative, either, so
		  we will represent them all as unsigned short variables. Many will be self-explanatory, but I will
		  describe what each one is for nonetheless.*****************************************************************************************/

		short pixel;             //uses unicode characters to set characters for pixel on/off states

		unsigned short opcode;			//The variable to hold the current opcode being operated on
		unsigned short index;			//This variable represents the index register of the CHIP-8, which supports 2-byte values also.
		unsigned short pc;				//This variable represents the program counter of the CHIP-8, which supports 2-byte values.
		unsigned short stack[16];		//This represents the stack you'll need to implement to support program jumps. You use it to store
										//the current value of the program counter so the program will remember where it's supposed to jump
										//back to once the jump or subroutine has finished. Since the program counter is 2-bytes, you'll
										//need an array of 2-byte values to represent the stack. Ergo, another unsigned short.
		unsigned short sp;				//Finally, this variable represents the stack pointer of the CHIP-8. Used to remember which
										//of the stack to reference to determine what program counter value to use. Again, 2-byte values.

		/*The rest of the CHIP-8 system uses 8-bit (1 byte) values, most easily represented as unsigned char datatypes***********************/

		unsigned char registers[16];	//The CHIP-8 has 15 general purpose registers that use 1-byte values. The 16th is used as a carry flag
		unsigned char mem[4096];		//The CHIP-8 has 4096K of memory. In other words, 0x1000 memory locations. We'd like as much control
										//over the size of data accessed as possible, so rather than using a larger datatype that'll read
										//in bigger chunks, we go for the smallest possible, which is a char.

		unsigned char delayTimer;		//Both of these timers can have a value from 0 to F, or in otherwords, 0-15. They count down at 60hz,
		unsigned char soundTimer;		//so once a second. As part of emulator implementation, you'll want to find a way to slow down the
										//such that it only executes 60 opcodes a second. That's outside the scope of this class, however.

		/*Lastly, you'll need to initialize the virtual system. We'll create a function here for that purpose. It's not going to need to
		 *return any values either, seeing as how the CHIP-8 is such a simple system, so a void return type will do fine.*/

		 void initialize_chip8();
};
//All done describing the CHIP-8! Now move onto chip8.cpp, where we'll define all of the functions we've briefly described here.
