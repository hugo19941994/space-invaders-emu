#include <stdio.h>
#include <stdlib.h>
#include<SDL.h>
#include <memory.h>
#include "SDL_endian.h"
//TODO DISSASSEMBLER
void draw();
//Debugging purposes
int veces = 0;
int cycles = 0;

//Refresh rate
int refresh = (2000000/60)/2; // Interrupts
//Interrupts: $cf (RST 0x08) at the start of vblank, $d7 (RST 0x10) at the end of vblank.
int INT = 0; //Interrupts enabled flag

unsigned char A; //Accumulator 8bit
unsigned char B, C, D, E, H, L; //General purpose registers 8bits
short int sp, pc; //16 bit stack pointer, program counter
unsigned char memory[8192*4]; //64kilobytes of memory, each bank is 1 byte creo que es 8192
//unsigned char *memory; //64kilobytes of memory, each bank is 1 byte

//Ports
unsigned char Read0 = 0x0e;
unsigned char Read1 = 0x09;
unsigned char Read2 = 0x00;
short int ShiftRegister = 0x00;
short int noOfBitsToShift = 0x00;

//a ver si asi funciona
int shift0;
int shift1;

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

int parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1<<size)-1));
	for (i=0; i<size; i++)
	{
		if (x & 0x1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}

void emulateCycle(){

	unsigned char *opcode = &memory[pc];
	int result;

	switch (*opcode){ //ONLY SPACE INVADERS OPCODES IMPLEMENTED
		
	case (0x00) : //NOP
		pc += 1;
		cycles += 4;
		break;

	case (0x01) : //LXI B,D16
		C = opcode[1];
		B = opcode[2];
		pc += 3;
		cycles += 10;
		break;

	case (0x05) : //DCR B
		B--;
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
		P = parity(B, 8);
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
		result = ((H << 8) | L) + ((B << 8) | C);
		H = result >> 8;
		L = result;
		//Carry flag
		if ((result)> 0xFFFF)
			CY = 1;
		else
			CY = 0;
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
		P = parity(C, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case (0x0e) : //MVI C,D8
		C = opcode[1];
		pc += 2;
		cycles += 7;
		break;

	case(0x0f) : //RRC
		/*Carry flag
		unsigned char x;
		x = A;
		//OLD WAY
		x = x << 7; //Antes era x << 7
		if (x == 0x80)
			CY = 1;
		else
			CY = 0;
		A = A >> 1;
		//NEW WAY
		A = ((x & 1) << 7) | (x >> 1);
		if ((x & 1) == 1)
			CY = 1;
		else
			CY = 0;*/
		result = (A >> 1) | ((A & 0x1) << 7);
		CY = (A & 0x1);
		A = result & 0xff;
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
		result = ((D << 8) | E) + 1;
		E = result & 0xff;
		D = result >> 8;
		pc += 1;
		cycles += 5;
		break;

	case(0x19) ://DAD D
		//HL = HL + DE
		result = ((H << 8) | L) + ((D << 8) | E);
		H = result >> 8;
		L = result;
		//Carry flag
		if (result > 0xFFFF)
			CY = 1;
		else
			CY = 0;
		pc += 1;
		cycles += 10;
		break;

	case(0x1a) : //LDAX D
		//A <- (DE)
		A = memory[(D << 8) | E];
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
		result = ((H << 8) | L) + 1;
		L = result & 0x00FF;
		H = result >> 8;
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
		result = 2*((H << 8) | L);
		H = result >> 8;
		L = result;
		//Carry flag
		if (result > 0xFFFF)
			CY = 1;
		else
			CY = 0;
		pc += 1;
		cycles += 10;
		break;

	case(0x31) : //LXI SP, D16
		sp = (opcode[2] << 8) | opcode[1];
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
		memory[(H << 8) | L] = opcode[1];
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
		A &= A;
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
		P = parity(A, 8);
		//Carry flag
		CY = 0; //Carry bit is reset to zero
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0xaf) : //XRA A
		//A <-A ^ A
		A ^= A;
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
		P = parity(A, 8);
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
		pc = (opcode[1] | opcode[2] << 8);
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
		if (A > (0xFF - opcode[1]))
			CY = 1;
		else
			CY = 0;
		A += opcode[1];
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
		P = parity(A, 8);
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 2;
		cycles += 7;
		break;

	case(0xc9) : //RET
		//PC.lo <- (sp); PC.hi<-(sp+1); SP <- SP+2
		pc = (memory[(sp + 1)] << 8) | memory[sp];
		sp += 2;
		cycles += 10;
		break;

	case(0xcd) : //CALL
		//(SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP+2;PC=adr
		memory[sp - 1] = ((pc + 3) >> 8) & 0xff;
		memory[sp - 2] = ((pc + 3) & 0xff);
		sp -= 2;
		pc = (opcode[2] << 8) | opcode[1];
		cycles += 17;
		break;

	case (0xd1) : //POP D
		//E <- (sp); D <- (sp+1); sp <- sp+2
		E = memory[sp];
		D = memory[sp + 1];
		sp += 2;
		pc += 1;
		cycles += 10;
		break;

	case(0xd3) : //OUT D8
		//outputDevice[opcode[1]] = A;
		if (opcode[1] == 0x02){
			noOfBitsToShift = A & 0x7;
		}
		else if (opcode[1] == 0x04){
			/*int AA;
			AA = A << 8;
			ShiftRegister = ((ShiftRegister & 0xFF00) >> 8);
			ShiftRegister = ShiftRegister | (AA& 0xFF00);*/
			shift0 = shift1;
			shift1 = A;
		}
		pc += 2;
		cycles += 10;
		break;
		 
	case(0xd5) : //PUSH D
		memory[sp - 2] = E;
		memory[sp - 1] = D;
		sp -= 2;
		pc += 1;
		cycles += 11;
		break;

	case(0xe1) : //POP H
		L = memory[sp];
		H = memory[sp + 1];
		sp += 2;
		pc += 1;
		cycles += 10;
		break;

	case(0xe5) : //PUSH H
		//(sp-2)<-L; (sp-1)<-H; sp <- sp - 2
		memory[sp - 2] = L;
		memory[sp - 1] = H;
		sp -= 2;
		pc += 1;
		cycles += 11;
		break;

	case(0xe6) : //ANI D8
		//A <-A & data
		A &= opcode[1];
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
		P = parity(A, 8);
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
		S = (memory[sp] >> 7) & 0x01;
		Z = (memory[sp] >> 6) & 0x01;
		AC = (memory[sp] >> 4) & 0x01;
		P = (memory[sp] >> 2) & 0x01;
		CY = memory[sp] & 0x01;
		A = memory[sp + 1];
		sp += 2;
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
		sp -= 2;
		pc += 1;
		cycles += 11;
		break;

	case(0xfb) : //EI
		INT = 1;
		pc += 1;
		cycles += 4;
		break;

	case(0xfe) : //CPI D8
		/*A - data
		char unsigned res;
		//Carry flag
		if (A < (opcode[1]))
			CY = 1;
		else
			CY = 0;
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
		if ((res % 2) == 0) //If B has even parity
			P = 1;
		else
			P = 0;
		//Auxiliary flag - NOT IMPLEMENTED*/
		if (A < opcode[1])
			CY = 1;
		else
			CY = 0;
		if ((A & 0xF) < (opcode[1] & 0xF))
			AC = 1;
		else
			AC = 0;
		if (A == opcode[1])
			Z = 1;
		else
			Z = 0;
		if ((A - opcode[1]) >> 7)
			S = 1;
		else
			S = 0;
		P = parity(A-opcode[1], 8);
		pc += 2;
		cycles += 7;
		break;
//--------------------------------------------------------------------------------
	case(0x35) ://DCR M
		memory[(H << 8) | L] -= 1;
		//Zero flag
		if (memory[(H << 8) | L] == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((memory[(H << 8) | L] & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		P = parity(memory[(H << 8) | L], 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 10;
		break;

	case(0xdb) : //IN para input
		if (opcode[1] == 0x00)
			A = Read0;
		else if (opcode[1] == 0x01)
			A = Read1;
		else if (opcode[1] == 0x02)
			A = Read2;
		else if (opcode[1] == 0x03){
			//A = (ShiftRegister << noOfBitsToShift) >> 8;
			int dwval;
			dwval = (shift1 << 8) | shift0;
			A = ((dwval >> (8 - noOfBitsToShift)) & 0xff);
		}
		pc += 2;
		cycles += 10;
		break;

	case (0xc8) : //RZ
		if (Z == 1){
			pc = (memory[(sp + 1)] << 8) | memory[sp];
			cycles += 11;
			sp += 2;
		}
		else{
			cycles += 5;
			pc += 1;
		}
		break;

	case(0xda) : //JC
		if (CY == 1)
			pc = opcode[1] | (opcode[2] << 8);
		else
			pc += 3;
		cycles += 10;
		break;

	case(0xca) ://JZ
		if (Z == 1)
			pc = opcode[1] | (opcode[2] << 8);
		else
			pc += 3;
		cycles += 10;
		break;

	case (0x27) : //DAA
		if (((A & 0x0f) > 9) | AC == 1) {
			if ((A & 0x000f) > 9)
				AC = 1;
			else
				AC = 0;
			A += 6;
		}
		if (((A >> 4) > 9) | CY == 1) {
			int sigbits = A >> 4;
			if ((A >> 4) > 9)
				CY = 1;
			else
				CY = 0;
			sigbits += 6;
			A = (sigbits << 4) | (A & 0x0f);
		}
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
		P = parity(A, 8);
		pc += 1;
		cycles += 4;
		break;

	case (0x7d) : //MOV AL
		A = L;
		cycles += 5;
		pc += 1;
		break;

	case(0x3d): //DCR A
		A--;
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
		P = parity(A, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case(0x80) : //ADD B
		//Carry flag
		if (A > (0xFF - B))
			CY = 1;
		else
			CY = 0;
		A += B;
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
		P = parity(A, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case (0x22) ://SHLD
		memory[ ( opcode[1] | (opcode[2] << 8) )     ] = L;
		memory[ ( opcode[1] | (opcode[2] << 8) ) + 1 ] = H;
		pc += 3;
		cycles += 16;
		break;

	case(0xd2) : //JNC
		if (CY == 0)
			pc = (opcode[1] | opcode[2] << 8);
		else
			pc += 3;
		cycles += 10;
		break;

	case(0x82) ://ADD D
		//Carry flag
		if (A > (0xFF - D))
			CY = 1;
		else
			CY = 0;
		A += D;
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
		P = parity(A, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0x17) : //RAL
		/*int acc;
		acc= A;
		A = A << 1;
		A += CY;
		CY = acc >> 7;*/
		/*acc = A;
		A = A << 1;
		A = (A&0xfe) | CY;
		CY = acc >> 7;*/
		result = (A << 1) | CY;
		CY = ((A & 0x80) >> 7);
		A =  result & 0xff;
		pc += 1;
		cycles += 4;
		break;

	case(0x4e) : //MOV CM
		C = memory[(H << 8)| L];
		cycles += 7;
		pc += 1;
		break;

	case(0x2a) : //LHLD
		L = memory[ ( opcode[1] | (opcode[2] << 8) )    ];
		H = memory[ ( opcode[1] | (opcode[2] << 8) ) + 1];
		cycles += 16;
		pc += 3;
		break;
		
	case(0x0a) : //LDAX B
		//A <- (BC)
		A = memory[(B << 8) | C];
		pc += 1;
		cycles += 7;
		break;
		
	case(0x37) : //STC
		CY = 1;
		pc += 1;
		cycles += 4;
		break;
		
	case(0x03) : //INX B
		//BC <- BC + 1
		result = ((B << 8) | C) + 1;
		B = result >> 8;
		C = result & 0xff;
		pc += 1;
		cycles += 5;
		break;
		
	case(0x67) : //MOV HA
		H = A;
		cycles += 5;
		pc += 1;
		break;

	case(0x5f) : //MOV EA
		E = A;
		cycles += 5;
		pc += 1;
		break;

	case(0x57) : //MOV DA
		D = A;
		cycles += 5;
		pc += 1;
		break;

	case(0xd8) : //RC
		if (CY == 1){
			pc =  ( ( memory[(sp + 1)] << 8 ) | ( memory[sp] ) );
			sp += 2;
			cycles += 11;
		}
		else{
			pc += 1;
			cycles += 5;
		}
		break;
		
	case(0x4f) : //MOV CA
		C = A;
		pc + 1;
		cycles += 5;
		pc += 1;
		break;
		
	case(0x2e) : //MVI C,d8
		C = opcode[1];
		pc += 2;
		cycles += 7;
		break;

	case(0xb6) : //ORA M
		A |= memory[(H<<8) | L];
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
		P = parity(A, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		CY = 0; //Carry bit reset
		pc += 1;
		cycles += 7;
		break;

	case(0x46) ://MOV BM
		B = memory[(H << 8) | L];
		cycles += 7;
		pc += 1;
		break;

	case(0xb0) : //ORA B
		A |= B;
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
		P = parity(A, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		CY = 0; //Carry bit reset
		pc += 1;
		cycles += 7;
		break;

	case(0x79)://MOV AC
		A = C;
		cycles += 7;
		pc += 1;
		break;

	case(0xe3) : //XTHL
		//L <-> (sp); H <-> (sp+1)
		unsigned char temp;
		temp = L;
		L = memory[sp];
		memory[sp] = temp;
		temp = H;
		H = memory[sp + 1];
		memory[sp + 1] = temp;
		pc += 1;
		cycles += 18;
		break;

	case(0xe9) ://PCHL
		pc = (H<<8) | L;
		cycles += 5;
		break;

	case(0xa8) : //XRA B
		//A <-A ^ B
		A ^= B;
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
		P = parity(A, 8);
		//Carry flag
		CY = 0; //Carry bit is reset to zero
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0xc0) : //RNZ
		if (Z == 0){
			pc = (memory[(sp + 1)] << 8) | memory[sp];
			cycles += 11;
			sp += 2;
		}
		else{
			cycles += 5;
			pc += 1;
		}
		break;

	case(0xd0) : //RNC
		if (C == 0){
			pc = (memory[(sp + 1)] << 8) | memory[sp];
			cycles += 11;
			sp += 2;
		}
		else{
			cycles += 5;
			pc += 1;
		}
		break;

	case(0x2b) : //DCX H
		result = ((H << 8) | L) - 1;
		H = result >> 8;
		L = result & 0xFF;
		pc += 1;
		cycles += 5;
		break;

	case(0x78) : //MOV AB
		A = B;
		pc += 1;
		cycles += 5;
		break;

	case(0xd6) ://SUI d8
		//Carry flag
		if (A < opcode[1])
			CY = 1;
		else
			CY = 0;
		A -= opcode[1];
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
		P = parity(A, 8);
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 2;
		cycles += 7;
		break;

	case(0x07) : //RLC //MIRAR pq no se si esta bien
		/*CY = A >> 7;
		A = A << 1;
		A += CY;*/
		result = (A << 1) | ((A & 0x08) >> 7);
		CY = ((A & 0x80) >> 7);
		A = result & 0xFF;
		pc += 1;
		cycles += 4;
		break;

	case(0x16) : //MVI D d8
		D = opcode[1];
		pc += 2;
		cycles += 7;
		break;

	case(0xc4) ://CNZ a16
		if (Z == 0){
			memory[sp - 1] = ((pc + 3) >> 8) & 0xff;
			memory[sp - 2] = (pc + 3) & 0xff;
			sp -= 2;
			pc = (opcode[2] << 8) | opcode[1];
			cycles += 17;
		}
		else{
			pc += 3;
			cycles += 11;
		}
		break;

	case(0x1f) ://RAR //mirar pq quizas esta mal
		/*int CY2;
		CY2 = CY;
		CY = A & 1;
		A = A >> 1;
		//A += CY2;
		A = A | (CY2 << 7);*/
		result = (A >> 1) | (CY << 7);
		CY = (A & 0x1);
		A = result & 0xff;
		pc += 1;
		cycles += 4;
		break;

	case(0xf6) ://ORI d8
		//Carry flag
		if (A > (0xFF - opcode[1]))
			CY = 1;
		else
			CY = 0;
		A |= opcode[1];
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
		P = parity(A, 8);
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 2;
		cycles += 7;
		break;

	case(0x04) : //INR B
		B++;
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
		P = parity(B, 8);
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case(0x70): //MOV MB
		memory[(H << 8) | L] = B;
		pc += 1;
		cycles += 7;
		break;

	case(0xb4)://ORA H
		A |= H;
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
		P = parity(A, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		CY = 0; //Carry bit reset
		pc += 1;
		cycles += 7;
		break;

	case(0x3c): //INR A
		A++;
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
		P = parity(A, 8);
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case(0xcc)://CZ a16
		if (Z == 1){
			memory[sp - 1] = ((pc + 3) >> 8) & 0xff;
			memory[sp - 2] = (pc + 3) & 0xff;
			sp = sp - 2;
			pc = (opcode[2] << 8) | opcode[1];
			cycles += 17;
		}
		else{
			pc += 3;
			cycles += 11;
		}
		break;

	case(0xfa) ://JM a16
		if (S == 1)
			pc = opcode[1] | (opcode[2] << 8);
		else
			pc += 3;
		cycles += 10;
		break;

	case(0x68): //MOV LB
		L = B;
		pc += 1;
		cycles += 5;
		break;

	case(0x61): //MOV HC
		H = C;
		pc += 1;
		cycles += 5;
		break;

	case(0xde) : //SBI d8
		result = opcode[1] + CY;
		//Carry
		if (A < result)
			CY = 1;
		else
			CY = 0;
		A -= result;
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
		P = parity(A, 8);
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 2;
		cycles += 7;
		break;

	case(0x47) : //MOV BA
		B = A;
		pc += 1;
		cycles += 5;
		break;

	case(0x14): //INR D
		D++;
		//Zero flag
		if (D == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((D & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		P = parity(D, 8);
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case(0x15) ://DCR D
		D--;
		//Zero flag
		if (D == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((D & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		P = parity(D, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case(0x86) ://ADD M
		//Carry flag
		if (A > (0xFF - memory[(H<<8)|L]))
			CY = 1;
		else
			CY = 0;
		A += memory[(H << 8) | L];
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
		P = parity(A, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 7;
		break;

	case(0x69): //MOV LC
		L = C;
		pc += 1;
		cycles += 5;
		break;

	case(0x34): //INR M
		memory[(H << 8) | L]++;
		//Zero flag
		if (memory[(H << 8) | L] == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((memory[(H << 8) | L] & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		P = parity(memory[(H << 8) | L], 8);
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 10;
		break;

	case(0xb8) : //CMP B
		//Carry
		if (A < B)
			CY = 1;
		else
			CY = 0;
		result = A - B;
		//Zero flag
		if (result == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((result & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		P = parity(result, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0x85): //ADD L
		//Carry flag
		if (A > (0xFF - L))
			CY = 1;
		else
			CY = 0;
		A += L;
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
		P = parity(A, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0xa0): //ANA B
		//A <-A & B
		A &= B;
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
		P = parity(A, 8);
		//Carry flag
		CY = 0; //Carry bit is reset to zero
		//Auxiliary flag - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0xbe): //CMP M
		result = A - memory[(H << 8) | L];
		//Carry
		//if (A < memory[(H<<8)|L])
		if ((result&0x100) != 0)
			CY = 1;
		else
			CY = 0;
		//Zero flag
		if (result == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((result & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		P = parity(result, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0x1b): //DCX D
		result = ((D << 8) | E) - 1;
		D = result >> 8;
		E = result & 0xFF;
		pc += 1;
		cycles += 5;
		break;

	case(0x25): //DCR H
		H--;
		//Zero flag
		if (H == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((H & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		P = parity(H, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 5;
		break;

	case(0x12) : //STAX D
		memory[(D << 8) | E] = A;
		pc += 1;
		cycles += 7;
		break;

	case(0xbc): //CMP H
		result = A - H;
		//Carry
		//if (A < B)
		if ((result & 0x100) != 0)
			CY = 1;
		else
			CY = 0;
		//Zero flag
		if (result == 0)
			Z = 1;
		else
			Z = 0;
		//Sign flag
		if ((result & 0x80) == 0x80)
			S = 1;
		else
			S = 0;
		//Parity flag
		P = parity(result, 8);
		//Auxiliary Carry - NOT IMPLEMENTED
		pc += 1;
		cycles += 4;
		break;

	case(0xd4) : //CNC a16
		if (CY == 0){
			memory[sp - 1] = ((pc + 3) >> 8) & 0xff;
			memory[sp - 2] = (pc + 3) & 0xff;
			sp = sp - 2;
			pc = (opcode[2] << 8) | opcode[1];
			cycles += 17;
		}
		else{
			pc += 3;
			cycles += 11;
		}
		break;

	default:
		printf("ERROR");
		cycles += 4;
		//exit(0);
	}

}

void interruptExecute(int opcode){

	switch (opcode){
		case(0xcf) : //RST algo //11xxx111 001
			//short int ret = pc + 2;
			memory[sp - 1] = (pc >> 8) & 0xff;
			memory[sp - 2] = pc & 0xff;
			sp -= 2;
			pc = 0x08;
			cycles += 11;
			break;

		case(0xd7) : //RST algo //010 11xxx111
			//short int ret = pc + 2;
			memory[sp - 1] = (pc >> 8) & 0xff;
			memory[sp - 2] = pc & 0xff;
			sp -= 2;
			pc = 0x10;
			cycles += 11;
			break;
	}

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
		gWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

	Uint32 pixel;
	Uint16 i;
	Uint32 *bits;
	Uint8 j;

	int help;
	help = gScreenSurface->pitch;

	memset(gScreenSurface->pixels, 0, SCREEN_HEIGHT*gScreenSurface->pitch);
	pixel = SDL_MapRGB(gScreenSurface->format, 0xFF, 0xFF, 0xFF);

	SDL_LockSurface(gScreenSurface);
	for (i = 0x2400; i < 0x3fff; i++){
		if (memory[i] != 0){
			for (j = 0; j < 8; j++){
				if ((memory[i] & (1 << j)) != 0){
					bits = (Uint32 *)gScreenSurface->pixels + ((255 - ((((i-2400) % 0x20) << 3) +j )) * (gScreenSurface->w)) + ((i-2400) >> 5)+11;
					//bits = (Uint32 *)gScreenSurface->pixels + ((255 - 1 - (((i % 0x20) << 3) + j)) * (gScreenSurface->w)) + (i >> 5);
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
	loadRom("D:\\Users\\Hugo\\Downloads\\emulators\\invaders\\invaders.h", 0);
	loadRom("D:\\Users\\Hugo\\Downloads\\emulators\\invaders\\invaders.g", 0x800);
	loadRom("D:\\Users\\Hugo\\Downloads\\emulators\\invaders\\invaders.f", 0x1000);
	loadRom("D:\\Users\\Hugo\\Downloads\\emulators\\invaders\\invaders.e", 0x1800);
	pc = 0x0;
	sp = 0xf000;
	int hugo = 0;
	int totaldraw=0;
	init();
	int decideINT = 0;
	while (hugo == 0){
		emulateCycle();
		veces++;

		if ((INT == 1) && (cycles >= refresh)) {

			if ((decideINT == 0) && (INT == 1)) {
				interruptExecute(0xcf);
				decideINT = 1;
				INT = 0;
			}

			if ((decideINT == 1) && (INT == 1)) {
				interruptExecute(0xd7);
				decideINT = 0;
				INT = 0;
			}
			draw();
			SDL_Delay(10);
			cycles = 0;
		}

	}
	return 0;
}
	