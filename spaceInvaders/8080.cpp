#include <stdio.h>
#include <stdlib.h>
#include<SDL.h>
#include <memory.h>
#include "SDL_endian.h"
//TODO DISSASSEMBLER

//Debugging purposes
int veces = 0;
int cycles = 0;

//Refresh rate
int refresh = 2000000/60; //TODO Interrupts
//Interrupts: $cf (RST 0x08) at the start of vblank, $d7 (RST 0x10) at the end of vblank.
int INT = 0; //Interrupts enabled flag

unsigned char A; //Accumulator 8bit
unsigned char B, C, D, E, H, L; //General purpose registers 8bits
short int sp, pc; //16 bit stack pointer, program counter
unsigned char memory[8192*3]; //64kilobytes of memory, each bank is 1 byte creo que es 8192
//unsigned char *memory; //64kilobytes of memory, each bank is 1 byte

//PSW
//F - Status register ... Not used in Space Invaders
unsigned char Z;//Zero flag
unsigned char S;//Sign flag
unsigned char P;//Parity flag
unsigned char CY;//Carry flag
unsigned char AC;//Auxiliary carry flag ... Not used in Space Invaders

//Screen dimension constants
const int SCREEN_WIDTH = 224;
const int SCREEN_HEIGHT = 256;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//The window surface
SDL_Surface* gScreenSurface = NULL;

void loadRom(char * file, int offset){
	FILE *ROM;
	fopen_s(&ROM, file, "rb");
	fseek(ROM, 0, SEEK_END);
	long size = ftell(ROM);
	rewind(ROM);

	//Allocate memory
	unsigned char *buffer = (unsigned char *)malloc(sizeof(char) * size);

	//Copy file to buffer
	size_t result = fread(buffer, 1, size, ROM);
	for (int i = 0; i < size; i++){
		memory[i + offset] = buffer[i];
	}
	fclose(ROM);
	free(buffer);
}

void emulateCycle(){

	unsigned char *opcode = &memory[pc];
	short int operando1, operando2, result;
	short int address;
	short int numero;
	int ref;

	switch (*opcode){
		//ONLY SPACE INVADERS OPCODES IMPLEMENTED
	case (0x00) : //NOP
		pc += 1;
		cycles += 4;
		break;

	case (0x01) : //LXI B,D16
		C = opcode[1];
		B = opcode[2];
		pc += 3; //CREO QUE ES 3
		cycles += 10;
		break;

	case (0x05) : //DCR B
		B = --B;
		//Zero flag
		if (B == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((B & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		if (B % 2 == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case (0x06) : //MVI B, D8
		B = opcode[1];
		pc += 2;
		cycles += 7;
		break;

	case(0x09) : //DAD B
		//HL = HL + BC
		//short int operando1, operando2, result; //16bit number
		operando1 = H << 8 | L;
		operando2 = B << 8 | C;
		result = operando1 + operando2;
		H = result >> 8;
		L = result;
		//Carry flag
		if (operando1 > (0xFFFF - operando2))
			CY = 1;
		pc += 1;
		cycles += 10;
		break;

	case(0x0d) : //DCR C
		C--;
		//Zero flag
		if (C == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((C & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		if (C % 2 == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case (0x0e) : //MVI C,D8
		C = opcode[1];
		pc += 2;
		cycles += 7;
		break;

	case(0x0f) : //RRC MAL
		//Carry flag
		unsigned char x;
		x = A;
		//OLD WAY
		//x = A;
		//x = x << 7; //Antes era x << 7
		//if (x == 0x80)
			//CY = 1;
		//else
			//CY = 0;
		//A = A >> 1;
		A = ((x & 1) << 7) | (x >> 1);
		if ((x & 1) == 1)
			CY = 1;
		else
			CY = 0;
		pc += 1;
		cycles += 4;
		break;

	case(0x11) : //LXI D, D16
		E = opcode[1];
		D = opcode[2];
		pc += 3;
		cycles += 10;
		break;

	case(0x13) ://INX D
		//DE <- DE + 1
		//short int numero;
		numero = D << 8 | E;
		++numero;
		E = numero;
		D = numero >> 8;
		pc += 1;
		cycles += 5;
		break;

	case(0x19) ://DAD D
		//HL = HL + DE
		//short int operando1, operando2, result; //16bit number
		operando1 = H << 8 | L;
		operando2 = D << 8 | E;
		result = operando1 + operando2;
		H = result >> 8;
		L = result;
		//Carry flag
		if (operando1 > (0xFFFF - operando2))
			CY = 1;
		pc += 1;
		cycles += 10;
		break;

	case(0x1a) : //LDAX D
		//A <- (DE)
		short int direccion; //16 bit number
		direccion = D << 8 | E;
		A = memory[direccion];
		pc += 1;
		cycles += 7;
		break;

	case(0x21) : //LXI H,D16
		L = opcode[1];
		H = opcode[2];
		pc += 3;
		cycles += 10;
		break;

	case(0x23) : //INX H
		//HL <- HL + 1
		//short int numero;
		//numero = H << 8 | L;
		numero = (H << 8 | L) & 0xFFFF;
		++numero;
		//L = numero;
		L = numero & 0xFFFF;
		H = numero >> 8;
		pc += 1;
		cycles += 5;
		break;

	case(0x26) : //MVI H,D8
		H = opcode[1];
		pc += 2;
		cycles += 7;
		break;

	case(0x29) : //DAD H
		//HL = HL + HL
		short int operando1, result; //16bit number
		operando1 = H << 8 | L;
		result = operando1 + operando1;
		H = result >> 8;
		L = result;
		//Carry flag
		if (operando1 > (0xFFFF - operando1))
			CY = 1;
		pc += 1;
		cycles += 10;
		break;

	case(0x31) : //LXI SP, D16
		unsigned char hi, lo;
		lo = opcode[1];
		hi = opcode[2];
		sp = hi << 8 | lo;
		pc += 3;
		cycles += 10;
		break;

	case(0x32) : //STA adr
		//(adr) <- A
		memory[opcode[1] | (opcode[2] << 8)] = A;
		pc += 3;
		cycles += 13;
		break;

	case(0x36) : //MVI M,D8
		//(HL) <- byte 2
		//short int address;
		address = (H << 8) | L;
		memory[address] = opcode[1];
		pc += 2;
		cycles += 10;
		break;

	case(0x3a) : //LDA adr
		//A <- (adr)
		A = memory[opcode[1] | (opcode[2] << 8)];
		pc += 3;
		cycles += 13;
		break;

	case(0x3e) : //MVI A,D8
		//A <- byte 2
		A = opcode[1];
		pc += 2;
		cycles += 7;
		break;

	case(0x56) : //MOV D,M
		//D <- (HL)
		D = memory[(H << 8) | L];
		pc += 1;
		cycles += 7;
		break;

	case(0x5e) : //MOV E,M
		//E <- (HL)
		E = memory[(H << 8) | L];
		pc += 1;
		cycles += 7;
		break;

	case(0x66) : //MOV H,M
		//H <-(HL)
		H = memory[(H << 8) | L];
		pc += 1;
		cycles += 7;
		break;

	case(0x6f) : //MOV L,A
		L = A;
		pc += 1;
		cycles += 5;
		break;

	case(0x77) : //MOV M,A
		//(HL) <- A
		memory[(H << 8) | L] = A;
		pc += 1;
		cycles += 7;
		break;

	case(0x7a) : //MOV A,D
		A = D;
		pc += 1;
		cycles += 5;
		break;

	case(0x7b) : //MOV A,E
		A = E;
		pc += 1;
		cycles += 5;
		break;

	case(0x7c) : //MOV A,H
		A = H;
		pc += 1;
		cycles += 5;
		break;

	case(0x7e) : //MOV A,M
		A = memory[(H << 8) | L];
		pc += 1;
		cycles += 7;
		break;

	case(0xa7) : //ANA A
		//A <-A & A
		A = A & A;
		//Zero flag
		if (A == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((A & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		if (A % 2 == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Carry flag
		CY = 0; //Carry bit is reset to zero
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0xaf) : //XRA A
		//A <-A ^ A
		A = A ^ A;
		//Zero flag
		if (A == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((A & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		if (A % 2 == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Carry flag
		CY = 0; //Carry bit is reset to zero
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0xc1) : //POP B
		//C <- (sp); B <- (sp+1); sp <- sp+2
		C = memory[sp];
		B = memory[sp + 1];
		sp += 2;
		pc += 1;
		cycles += 10;
		break;

	case(0xc2) : //JNZ adr
		//if NZ, PC <- adr
		if (Z == 0)
			pc = (opcode[1] | (opcode[2] << 8));
		else
			pc += 3;
		cycles += 10;
		break;

	case(0xc3) ://JMP adr
		//short int address;
		address = (opcode[1] | opcode[2] << 8);
		pc = address;
		cycles += 10;
		break;

	case(0xc5) : //PUSH B
		//(sp-2)<-C; (sp-1)<-B; sp <- sp - 2
		memory[sp - 2] = C;
		memory[sp - 1] = B;
		sp = sp - 2;
		pc += 1;
		cycles += 11;
		break;

	case(0xc6) : //ADI D8
		//A <- A + byte
		//Carry flag
		if (A > (0xFFFF - opcode[1]))
			CY = 1;
		else
			CY = 0;
		A = A + opcode[1];
		//Zero flag
		if (A == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((A & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		if (A % 2 == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 2;
		cycles += 7;
		break;

	case(0xc9) : //RET
		//PC.lo <- (sp); PC.hi<-(sp+1); SP <- SP+2
		pc = memory[(sp + 1)] << 8 | memory[sp];
		sp += 2;
		cycles += 10;
		break;

	case(0xcd) : //CALL
		//(SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP+2;PC=adr
		short int ret;
		ret = pc + 3;
		memory[sp - 1] = (ret >> 8) & 0xff;
		memory[sp - 2] = (ret & 0xff);
		sp = sp - 2;
		pc = (opcode[2] << 8) | opcode[1];
		cycles += 17;
		break;

	case (0xd1) : //POP D
		//E <- (sp); D <- (sp+1); sp <- sp+2
		E = memory[sp];
		D = memory[sp + 1];
		sp = sp + 2;
		pc += 1;
		cycles += 10;
		break;

	case(0xd3) : //OUT D8
		//outputDevice[opcode[1]] = A;
		pc += 2;
		cycles += 10;
		break;
		 
	case(0xd5) : //PUSH D
		memory[sp - 2] = E;
		memory[sp - 1] = D;
		sp = sp - 2;
		pc += 1;
		cycles += 11;
		break;

	case(0xe1) : //POP H
		L = memory[sp];
		H = memory[sp + 1];
		sp = sp + 2;
		pc += 1;
		cycles += 10;
		break;

	case(0xe5) : //PUSH H
		//(sp-2)<-L; (sp-1)<-H; sp <- sp - 2
		memory[sp - 2] = L;
		memory[sp - 1] = H;
		sp = sp - 2;
		pc += 1;
		cycles += 11;
		break;

	case(0xe6) : //ANI D8
		//A <-A & data
		A = A & opcode[1];
		//Zero flag
		if (A == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((A & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		if (A % 2 == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Carry flag
		CY = 0; //Carry bit is reset to zero
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 2;
		cycles += 7;
		break;

	case(0xeb) ://XCHG
		//H <->D; L <->E
		unsigned char hold;
		hold = H;
		H = D;
		D = hold;
		hold = L;
		L = E;
		E = hold;
		pc += 1;
		cycles += 5;
		break;

	case(0xf1) : //POP PSW
		//flags <- (sp); A <- (sp+1); sp <- sp+2
		//FLAGS = memory[sp]; //TODO creo que aqui falla pone el sp mal
		S = (memory[sp] << 7) & 0x01;
		Z = (memory[sp] << 6) & 0x01;
		AC = (memory[sp] << 4) & 0x01;
		P = (memory[sp] << 2) & 0x01;
		CY = memory[sp] & 0x01;
		A = memory[sp + 1];
		sp = sp + 2;
		pc += 1;
		cycles += 10;
		break;

	case(0xf5) : //PUSH PSW
		//(sp-2)<-flags; (sp-1)<-A; sp <- sp - 2
		//memory[sp-2] = FLAGS
		unsigned int psw;
		psw = 0x02;
		if (S == 1)
			psw += 0x80;
		if (Z == 1)
			psw += 0x40;
		if (AC == 1)
			psw += 0x10;
		if (P == 1)
			psw += 0x04;
		if (CY == 1)
			psw += 0x01;
		memory[sp - 2] = psw;
		memory[sp - 1] = A;
		sp = sp - 2;
		pc += 1;
		cycles += 11;
		break;

	case(0xfb) : //EI TODO
		//Enable interrupts
		INT = 1;
		pc += 1;
		cycles += 4;
		break;

	case(0xfe) : //CPI D8
		//A - data
		char unsigned res;
		//Carry flag
		if (A < (opcode[1]))
			CY = 1;
		else
			CY = 0;
		//CY = 0; //Carry bit is reset to zero
		res = A - opcode[1];
		//Zero flag
		if (res == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((res & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		if (res % 2 == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 2;
		cycles += 7;
		break;

	case(0x35) ://NO DEBERIA OCURRIR
		//DCR M
		ref = memory[H << 8 | L];
		ref = ref-1;
		//ref2 = ref2 - 1;
		memory[H << 8 | L] = ref;
		//Zero flag
		if (memory[H << 8 | L] == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((memory[H << 8 | L] & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		if (memory[H << 8 | L] % 2 == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 10;
		break;

	case(0xdb) : //IN para input
		cycles += 10;
		pc += 1;
		break;

	default:
		printf("ERROR");
		cycles += 4;
		//pc = pc + 1;
		//exit(0);
	}

}

int interruptExecute(int opcode){

	short int rethugo = pc + 2;

	switch (opcode){
		case(0xcf) : //RST algo //11xxx111 001
			//short int ret = pc + 2;
			memory[sp - 1] = (rethugo >> 8) & 0xff;
			memory[sp - 2] = rethugo & 0xff;
			//memory[sp - 2] = pc >> 8;
			//memory[sp - 1] = pc & 0xFF;
			sp = sp - 2;
			pc = 0x08;
			cycles += 11;
			break;

		case(0xd7) : //RST algo //010 11xxx111
			//short int ret = pc + 2;
			memory[sp - 1] = (rethugo >> 8) & 0xff;
			memory[sp - 2] = rethugo & 0xff;
			//memory[sp - 2] = pc >> 8;
			//memory[sp - 1] = pc & 0xFF;
			sp = sp - 2;
			pc = 0x10;
			cycles += 11;
			break;
	}

	return 0;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("CHIP8-Emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Custom
				gScreenSurface = SDL_GetWindowSurface(gWindow);
			}
		}
	}

	return success;
}

void draw(){

	static Uint32 lastframe = 0;
	//static Uint8 frames = FRAMESKIP;
	Uint32 pixel;
	//Uint32 curframe;
	Uint16 i;
	Uint32 *bits;
	Uint8 j;

	//memset(gScreenSurface->pixels, 0, SCREEN_HEIGHT*gScreenSurface->pitch);
	pixel = SDL_MapRGB(gScreenSurface->format, 0xFF, 0xFF, 0xFF);

	SDL_LockSurface(gScreenSurface);
	for (i = 0x2400; i < 0x3fff; i++){
		if (memory[i] != 0){
			for (j = 0; j < 8; j++){
				if ((memory[i] & (1 << j)) != 0){
					bits = (Uint32 *)gScreenSurface->pixels + ((256 - (((i % 0x20) << 3) + j)) * (gScreenSurface->w)) + (i >> 5)-64;
					*bits = pixel;
				}
			}
		}
	}
	SDL_UnlockSurface(gScreenSurface);
	//SDL_RenderPresent(gRenderer);
	SDL_UpdateWindowSurface(gWindow);
}

int main(int argc, char* argv[]){
	//Load ROMs
	loadRom("C:\\Users\\Hugo\\Downloads\\emulators\\invaders.h", 0);
	loadRom("C:\\Users\\Hugo\\Downloads\\emulators\\invaders.g", 0x800);
	loadRom("C:\\Users\\Hugo\\Downloads\\emulators\\invaders.f", 0x1000);
	loadRom("C:\\Users\\Hugo\\Downloads\\emulators\\invaders.e", 0x1800);
	pc = 0x0;
	sp = 0xf000;
	int hugo = 0;
	int vecestotal = 0;
	init();
	while (hugo == 0){ //Falla entre 42434 falla pc creo - ESTAN MAL LOS CARRY CREO //37513
		emulateCycle();
			veces++;

		if (veces == 42430) //despues delinterrupt va al f5 -> BIEN. Pero a es 00 y deberia ser 40 y sp es 23fe y deberia ser 23fc
			printf("Hugo");

		if ((INT == 1) && (cycles >= refresh)){
			interruptExecute(0x3f);
			draw();
			interruptExecute(0xd7);
			cycles = 0;
			//veces = 0;
			//INT = 0;
		}
			
	}
	return 0;
}
	