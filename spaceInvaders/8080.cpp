#include <stdio.h>
#include <stdlib.h>
//TODO DISSASSEMBLER
unsigned char A; //Accumulator 8bit
unsigned char B, C, D, E, H, L; //General purpose registers 8bits
short int sp, pc; //16 bit stack pointer, program counter
unsigned char memory[8192]; //64kilobytes of memory, each bank is 1 byte
//unsigned char *memory; //64kilobytes of memory, each bank is 1 byte

//PSW
unsigned char Z;//Zero flag
unsigned char S;//Sign flag
unsigned char P;//Parity flag
unsigned char CY;//Carry flag
unsigned char AC;//Auxiliary carry flag NOT USED IN SPACE INVADERS

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
		memory[i+offset] = buffer[i];
	}
	fclose(ROM);
	free(buffer);
}

void emulateCycle(){

	unsigned char *opcode = &memory[pc];
	short int operando1, operando2, result;
	short int address;
	short int numero;

	switch (*opcode){
		//ONLY SPACE INVADERS OPCODES IMPLEMENTED
	case (0x00) : //NOP
		pc += 1;
		break;

	case (0x01) : //LXI B,D16
		C = opcode[1];
		B = opcode[2];
		pc += 3; //CREO QUE ES 3
		break;

	case (0x05) : //DCR B
		B = B--;
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
		break;

	case (0x06) : //MVI B, D8
		B = opcode[1];
		pc += 2;
		break;

	case(0x09) : //DAD B
		//HL = HL + BC
		//short int operando1, operando2, result; //16bit number
		operando1 = H << 8 | L;
		operando2 = B << 8 | C;
		result = operando1 + operando2;
		H = result;
		L = result >> 8;
		//Carry flag
		if (operando1 > (0xFFFF - operando2))
			CY = 1;
		pc += 1;
		break;

	case(0x0d) : //DCR C
		C--;
		//Zero flag
		if (C == 0)
			Z = 0;
		else
			Z = 1;
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
		break;

	case (0x0e) : //MVI C,D8
		C = opcode[1];
		pc += 2;
		break;

	case(0x0f) : //RRC TODO
		//Carry flag
		unsigned char x;
		x = A;
		x << 7; //MAL creo
		if (x == 0x80)
			CY = 1;
		else
			CY = 0;
		A = A >> 1;
		pc += 1;
		break;

	case(0x11) : //LXI D, D16
		E = opcode[1];
		D = opcode[2];
		pc += 3;
		break;

	case(0x13) ://INX D
		//DE <- DE + 1
		//short int numero;
		numero = D << 8 | E;
		++numero;
		E = numero;
		D = numero >>8;
		pc += 1;
		break;

	case(0x19) ://DAD D
		//HL = HL + DE
		//short int operando1, operando2, result; //16bit number
		operando1 = H << 8 | L;
		operando2 = D << 8 | E;
		result = operando1 + operando2;
		H = result;
		L = result >> 8;
		//Carry flag
		if (operando1 > (0xFFFF - operando2))
			CY = 1;
		pc += 1;
		break;

	case(0x1a) : //LDAX D
		//A <- (DE)
		short int direccion; //16 bit number
		direccion = D << 8 | E;
		A = memory[direccion];
		pc += 1;
		break;

	case(0x21) : //LXI H,D16
		L = opcode[1];
		H = opcode[2];
		pc += 3;
		break;

	case(0x23) : //INX H
		//HL <- HL + 1
		//short int numero;
		numero = H << 8 | L;
		++numero;
		L = numero;
		H = numero >> 8;
		pc += 1;
		break;

	case(0x26) : //MVI H,D8
		L = opcode[1];
		pc += 2;
		break;

	case(0x29) : //DAD H
		//HL = HL + HL
		short int operando1, result; //16bit number
		operando1 = H << 8 | L;
		result = operando1 + operando1;
		H = result;
		L = result >> 8;
		//Carry flag
		if (operando1 > (0xFFFF - operando1))
			CY = 1;
		pc += 1;
		break;

	case(0x31) : //LXI SP, D16
		unsigned char hi, lo;
		lo = opcode[1];
		hi = opcode[2];
		sp = hi << 8 | lo;
		pc += 3;
		break;

	case(0x32) : //STA adr
		//(adr) <- A
		memory[opcode[1] | (opcode[2] << 8)] = A;
		pc += 3;
		break;

	case(0x36) : //MVI M,D8
		//(HL) <- byte 2
		//short int address;
		address = H << 8 | L;
		memory [address]= opcode[1];
		pc += 2;
		break;

	case(0x3a) : //LDA adr
		//A <- (adr)
		A = memory[opcode[1] | (opcode[2] << 8)];
		pc += 3;
		break;

	case(0x3e) : //MVI A,D8
		//A <- byte 2
		A = opcode[1];
		pc += 2;
		break;

	case(0x56) : //MOV D,M
		//D <- (HL)
		D = memory[H | (L << 8)];
		pc += 1;
		break;

	case(0x5e) : //MOV E,M
		//E <- (HL)
		E = memory[H | (L << 8)];
		pc += 1;
		break;

	case(0x66) : //MOV H,M
		//H <-(HL)
		H = memory[H | (L << 8)];
		pc += 1;
		break;

	case(0x6f) : //MOV L,A
		L = A;
		pc += 1;
		break;

	case(0x77) : //MOV M,A
		//(HL) <- A
		memory[H | (L << 8)] = A;
		pc += 1;
		break;

	case(0x7a) : //MOV A,D
		A = D;
		pc += 1;
		break;

	case(0x7b) : //MOV A,E
		A = E;
		pc += 1;
		break;

	case(0x7c) : //MOV A,H
		A = H;
		pc += 1;
		break;

	case(0x7e) : //MOV A,M
		A = memory[H | (L << 8)];
		pc += 1;
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
		break;

	case(0xc1) : //POP B
		//C <- (sp); B <- (sp+1); sp <- sp+2
		memory[sp] = C;
		memory[sp + 1] = B;
		sp += 2;
		pc += 1;
		break;

	case(0xc2) : //JNZ adr
		//if NZ, PC <- adr
		if (Z == 0)
			pc = (opcode[1] | (opcode[2] << 8));
		else
			pc += 3;
		break;

	case(0xc3) ://JMP adr
		//short int address;
		//address = (opcode[1]<<4) | (opcode[2]);
		address = (opcode[1] | opcode[2]<<8);
		pc = address;
		break;

	case(0xc5) : //PUSH B
		//(sp-2)<-C; (sp-1)<-B; sp <- sp - 2
		C = sp - 2;
		B = sp - 1;
		sp = sp - 2;
		pc += 1;
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
		break;

	case(0xc9) : //RET LEER COMO SE HACE
		//PC.lo <- (sp); PC.hi<-(sp+1); SP <- SP+2
		pc = memory[sp] | (memory[sp + 1] << 8);
		sp += 2;
		break;

	case(0xcd) : //CALL adr TODO - CREO QUE ESTA SOLUCIONADO
		//(SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP+2;PC=adr
		//uint16_t    ret = state->pc+2;
		memory[sp - 1] = pc >> 8; //MAL state->memory[state->sp-1] = (ret >> 8) & 0xff;
		memory[sp - 2] = pc; //MAL state->memory[state->sp-2] = (ret & 0xff);
		sp = sp - 2;
		pc = (opcode[2] << 8) | opcode[1];
		break;

	case (0xd1) : //POP D
		//E <- (sp); D <- (sp+1); sp <- sp+2
		memory[sp] = E;
		memory[sp + 1] = D;
		sp = sp + 2;
		pc += 1;
		break;

	case(0xd3) : //OUT D8
		//outputDevice[opcode[1]] = A;
		pc += 2;
		break;

	case(0xd5): //PUSH D
		E = sp - 2;
		D = sp - 1;
		sp = sp - 2;
		pc += 1;
		break;

	case(0xe1): //POP H
		memory[sp] = L;
		memory[sp + 1] = H;
		sp = sp + 2;
		pc += 1;
		break;

	case(0xe5): //PUSH H
		L = sp - 2;
		H = sp - 1;
		sp = sp - 2;
		pc += 1;
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
		break;

	case(0xf1) : //POP PSW
		//flags <- (sp); A <- (sp+1); sp <- sp+2
		//FLAGS = memory[sp];
		S = (memory[sp] << 7) & 0x01;
		Z = (memory[sp] << 6) & 0x01;
		AC = (memory[sp] << 4) & 0x01;
		P = (memory[sp] << 2) & 0x01;
		CY = memory[sp] & 0x01;
		A = memory[sp + 1];
		sp = sp + 2;
		pc += 1;
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
		break;

	case(0xfb) : //EI TODO
		//Enable interrupts
		pc += 1;
		break;

	case(0xfe) : //CPI D8
		//A - data
		char unsigned res;
		//Carry flag
		if (A > (0xFFFF - opcode[1]))
			CY = 1;
		else
			CY = 0;
		CY = 0; //Carry bit is reset to zero
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
		break;
	}

}

void main(){
	//Load ROMs
	loadRom("D:\\Users\\Hugo\\Downloads\\invaders\\invaders.h", 0);
	loadRom("D:\\Users\\Hugo\\Downloads\\invaders\\invaders.g", 0x800);
	loadRom("D:\\Users\\Hugo\\Downloads\\invaders\\invaders.f", 0x1000);
	loadRom("D:\\Users\\Hugo\\Downloads\\invaders\\invaders.e", 0x1800);
	pc = 0x0;
	sp = 0xf000;
	int hugo = 0;
	int veces = 0;
	while (hugo == 0){ //Se rompe en el ciclo
		emulateCycle();
		veces++;
	}
}