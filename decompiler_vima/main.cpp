#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <iomanip>

#pragma warning( disable: 4996)

enum op {
	DLOAD = 1,		//01
	ILOAD, 		//02
	SLOAD,		//03
	DLOAD0,		//04
	ILOAD0,		//05
	SLOAD0,		//06
	DLOAD1,		//07
	ILOAD1,		//08
	DLOADM1,		//09
	ILOADM1,		//0A
	DADD,			//0B
	IADD,			//0C
	DSUB,			//0D
	ISUB,			//0E
	DMUL,			//0F
	IMUL,			//10
	DDIV,			//11
	IDIV,			//12
	IMOD,			//13
	DNEG,			//14
	INEG,			//15
	IAOR,			//16
	IAAND,		//17
	IAXOR,		//18
	IPRINT,		//19
	DPRINT,		//1A
	SPRINT,		//1B
	I2D,			//1C
	D2I,			//1D
	S2I,			//1E
	SWAP,			//1F
	POP,			//20
	LOADDVAR0,	//21
	LOADDVAR1,	//22
	LOADDVAR2,	//23
	LOADDVAR3,	//24
	LOADIVAR0,	//25
	LOADIVAR1,	//26
	LOADIVAR2,	//27
	LOADIVAR3,	//28
	LOADSVAR0,	//29
	LOADSVAR1,	//2A
	LOADSVAR2,	//2B
	LOADSVAR3,	//2C
	STOREDVAR0,	//2D
	STOREDVAR1,	//2E
	STOREDVAR2,	//2F
	STOREDVAR3,	//30
	STOREIVAR0,	//31
	STOREIVAR1,	//32
	STOREIVAR2,	//33
	STOREIVAR3,	//34
	STORESVAR0,	//35
	STORESVAR1,	//36
	STORESVAR2,	//37
	STORESVAR3,	//38
	LOADDVAR,		//39
	LOADIVAR,		//3A
	LOADSVAR,		//3B
	STOREDVAR,	//3C
	STOREIVAR,	//3D
	STORESVAR,	//3E
	LOADCTXDVAR,	//3F
	LOADCTXIVAR,	//40
	LOADCTXSVAR,	//41
	STORECTXDVAR,	//42
	STORECTXIVAR,	//43
	STORECTXSVAR,	//44
	DCMP,			//45
	ICMP,			//46
	JA,			//47
	IFICMPNE,		//48
	IFICMPE,		//49
	IFICMPG,		//4A
	IFICMPGE,		//4B
	IFICMPL,		//4C
	IFICMPLE,		//4D
	DUMP,			//4E
	STOP,			//4F
	CALL,			//50
	CALLNATIVE,	//51
	RETURN,		//52
	BREAK 		//53
};

int getSize(char* filename)
{
	int size;
	FILE *fileptr;
	fileptr = fopen(filename, "rb");
	if (!fileptr) return -1;
	fseek(fileptr, 0L, SEEK_END);
	size = ftell(fileptr);
	fclose(fileptr);
	return size;
}

char* getByteCode(char* filename)
{
	char *bytecode;
	FILE *fileptr;
	int size;
	fileptr = fopen(filename, "rb");
	if (!fileptr) return NULL;
	size = getSize(filename);
	bytecode = (char*)malloc(size + 1);
	fread(bytecode, size, 1, fileptr);
	fclose(fileptr);
	bytecode[size] = 0;
	return bytecode;
}

short read16(char* bytecode)
{
	short res;
	res = *(bytecode + 1);
	res <<= 8;
	res |= *bytecode;
	return res;
}

int read32(char* bytecode)
{
	int res = 0;
	int i, j;
	for (i = 1, j = 0; j != 4; ++j, i *= 0x100)
	{
		res += i * *(bytecode + j);
	}
	return res;
}

long long read64(char* bytecode)
{
	long long res = 0;
	int i, j;
	for (i = 1, j = 0; j != 8; ++j, i *= 0x100)
	{
		res += i * *(bytecode + j);
	}
	return res;
}

double readd64(char* bytecode)
{
	double res = 0;
	long long a = 0;
	int i, j;
	for (i = 1, j = 0; j != 8; ++j, i *= 0x100)
	{
		a += i * *(bytecode + j);
	}
	res = *((double*)&a);
	return res;
}


int main(int argc, char **argv)
{
	if (argc == 1) return 1;
	char* code;
	if ((code = getByteCode(argv[1])) == NULL) return 1;
	std::ofstream output("output.txt");
	output << "Version: " << (int)code[2] << "." << (int)code[3] << "." << (int)code[4] << "." << (int)code[5] << std::endl;
	output << "Number of constants: " << read32(code + 6) << std::endl;
	output << "Size of constants: " << read32(code + 10) << std::endl;
	output << "\nStrings:\n" << "0. \"\"" << std::endl;
	int offset = 14;
	for (int i = read32(code + 6), j = 1; i != 0; --i, ++j, offset += strlen(code + offset) + 1)
	{
		output << j << ". \"";
		for (unsigned int q = 0; q < strlen(code + offset); ++q)
			if (code[offset + q] != 0xA)
				output << code[offset + q];
			else
				output << "\\n";
		output << "\"\n";
	}
	output << std::endl;
	output << "Entry ID: " << std::hex << read16(code + offset) << "\n" << std::endl;
	offset += 2;
	output << "Number of functions: " << std::dec << read32(code + offset) << "\n" << std::endl;
	offset += 4;
	for (int i = read32(code + offset - 4); i != 0; --i, offset += read32(code + offset))
	{
		output << "Function ID: " << std::hex << read16(code + offset + 12 + read32(code + offset + 8)) << std::endl;
		output << "Size of function: " << std::dec << read32(code + offset) << std::endl;
		output << "Size of bytecode: " << read32(code + offset + 4) << std::endl;
		output << "Size of signature: " << read32(code + offset + 8) << std::endl;
		output << "Signature: " << code + offset + 12 << std::endl;
		output << "Number of local variables: " << read32(code + 12 + offset + read32(code + offset + 8) + 2) << std::endl;
		output << "Number of arguments: " << read32(code + 12 + offset + read32(code + offset + 8) + 6) << std::endl;
		output << "Code:" << std::endl;
		int tmp_off = offset + read32(code + offset + 8) + 10 + 12, size = read32(code + offset + 4);
		int* pos = &tmp_off;
		char* bytecode = code;
		for (; size != 0; --size)
		{
#define SHOW(X) std::dec << (X) << ":" << std::hex << "0x" << (X) << std::dec
#define MEM(X) for(int k = 0; k < X; ++k) { output << std::setfill('0') << std::setw(2) << ((code[tmp_off + k] & 0xFF))<< " "; }
			output << std::hex << tmp_off << ": " << std::setfill('0') << std::setw(2) << (int)code[tmp_off] << " ";
			switch (code[tmp_off++])
			{
			default:
				return 1;
			case DLOAD:
				MEM(8)
				output << "\t\tDLOAD " << readd64(code + *pos) << ":" << std::hex << (long long)readd64(code + *pos) << std::endl;
				*pos = *pos + 8;
				size -= 8;
				break;
			case ILOAD:
				MEM(8)
				size -= 8;
				output << "\t\tILOAD " << SHOW(read64(code + *pos)) << std::endl;
				*pos = *pos + 8;
				break;
			case SLOAD:
				MEM(2)
				size -= 2;
				output << "\t\t\t\tSLOAD " << SHOW(read16(code + *pos)) << std::endl;
				*pos = *pos + 2;
				break;
			case DLOAD0:
			case SLOAD0:
			case ILOAD0:
				break;
			case DLOAD1:
				output << "\t\t\t\t\tDLOAD1" << std::endl;
				break;
			case ILOAD1:
				output << "\t\t\t\t\tILOAD1" << std::endl;
				break;
			case DLOADM1:
				output << "\t\t\t\t\tDLOADM1" << std::endl;
				break;
			case ILOADM1:
				output << "\t\t\t\t\tILOADM1" << std::endl;
				break;
			case DADD:
				output << "\t\t\t\t\tDADD" << std::endl;
				break;
			case IADD:
				output << "\t\t\t\t\tIADD" << std::endl;
				break;
			case DSUB:
				output << "\t\t\t\t\tDSUB" << std::endl;
				break;
			case ISUB:
				output << "\t\t\t\t\tISUB" << std::endl;
				break;
			case DMUL:
				output << "\t\t\t\t\tDMUL" << std::endl;
				break;
			case IMUL:
				output << "\t\t\t\t\tIMUL" << std::endl;
				break;
			case DDIV:
				output << "\t\t\t\t\tDDIV" << std::endl;
				break;
			case IDIV:
				output << "\t\t\t\t\tIDIV" << std::endl;
				break;
			case IMOD:
				output << "\t\t\t\t\tIMOD" << std::endl;
				break;
			case DNEG:
				output << "\t\t\t\t\tDNEG" << std::endl;
				break;
			case INEG:
				output << "\t\t\t\t\tINEG" << std::endl;
				break;
			case IAOR:
				output << "\t\t\t\t\tIAOR" << std::endl;
				break;
			case IAAND:
				output << "\t\t\t\t\tIAAND" << std::endl;
				break;
			case IAXOR:
				output << "\t\t\t\t\tIAXOR" << std::endl;
				break;
			case DPRINT:
				output << "\t\t\t\t\tDPRINT" << std::endl;
				break;
			case IPRINT:
				output << "\t\t\t\t\tIPRINT" << std::endl;
				break;
			case SPRINT:
				output << "\t\t\t\t\tSPRINT" << std::endl;
				break;
			case I2D:
				output << "\t\t\t\t\tI2D" << std::endl;
				break;
			case D2I:
				output << "\t\t\t\t\tD2I" << std::endl;
				break;
			case S2I:
				output << "\t\t\t\t\tS2I" << std::endl;
				break;
			case SWAP:
				output << "\t\t\t\t\tSWAP" << std::endl;
				break;
			case POP:
				output << "\t\t\t\t\tPOP" << std::endl;
				break;
			case LOADDVAR0:
				output << "\t\t\t\t\tLOADDVAR0" << std::endl;
				break;
			case LOADDVAR1:
				output << "\t\t\t\t\tLOADDVAR1" << std::endl;
				break;
			case LOADDVAR2:
				output << "\t\t\t\t\tLOADDVAR2" << std::endl;
				break;
			case LOADDVAR3:
				output << "\t\t\t\t\tLOADDVAR3" << std::endl;
				break;
			case LOADIVAR0:
				output << "\t\t\t\t\tLOADIVAR0" << std::endl;
				break;
			case LOADIVAR1:
				output << "\t\t\t\t\tLOADIVAR1" << std::endl;
				break;
			case LOADIVAR2:
				output << "\t\t\t\t\tLOADIVAR2" << std::endl;
				break;
			case LOADIVAR3:
				output << "\t\t\t\t\tLOADIVAR3" << std::endl;
				break;
			case LOADSVAR0:
				output << "\t\t\t\t\tLOADSVAR0" << std::endl;
				break;
			case LOADSVAR1:
				output << "\t\t\t\t\tLOADSVAR1" << std::endl;
				break;
			case LOADSVAR2:
				output << "\t\t\t\t\tLOADSVAR2" << std::endl;
				break;
			case LOADSVAR3:
				output << "\t\t\t\t\tLOADSVAR3" << std::endl;
				break;
			case STOREDVAR0:
				output << "\t\t\t\t\tSTOREDVAR0" << std::endl;
				break;
			case STOREDVAR1:
				output << "\t\t\t\t\tSTOREDVAR1" << std::endl;
				break;
			case STOREDVAR2:
				output << "\t\t\t\t\tSTOREDVAR2" << std::endl;
				break;
			case STOREDVAR3:
				output << "\t\t\t\t\tSTOREDVAR3" << std::endl;
				break;
			case STOREIVAR0:
				output << "\t\t\t\t\tSTOREIVAR0" << std::endl;
				break;
			case STOREIVAR1:
				output << "\t\t\t\t\tSTOREIVAR1" << std::endl;
				break;
			case STOREIVAR2:
				output << "\t\t\t\t\tSTOREIVAR2" << std::endl;
				break;
			case STOREIVAR3:
				output << "\t\t\t\t\tSTOREIVAR3" << std::endl;
				break;
			case STORESVAR0:
				output << "\t\t\t\t\tSTORESVAR0" << std::endl;
				break;
			case STORESVAR1:
				output << "\t\t\t\t\tSTORESVAR1" << std::endl;
				break;
			case STORESVAR2:
				output << "\t\t\t\t\tSTORESVAR2" << std::endl;
				break;
			case STORESVAR3:
				output << "\t\t\t\t\tSTORESVAR3" << std::endl;
				break;
			case LOADDVAR:
				MEM(2)
				output << "\t\t\tLOADDVAR " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case LOADIVAR:
				MEM(2)
				output << "\t\t\tLOADIVAR " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case LOADSVAR:
				MEM(2)
				output << "\t\t\tLOADSVAR " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case STOREDVAR:
				MEM(2)
				output << "\t\t\tSTOREDVAR " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case STOREIVAR:
				MEM(2)
				output << "\t\t\tSTOREIVAR " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case STORESVAR:
				MEM(2)
				output << "\t\t\tSTORESVAR " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case LOADCTXDVAR:
				MEM(4)
				output << "\t\t\tLOADCTXDVAR " << SHOW(read16(bytecode + *pos)) << " " << SHOW(read16(bytecode + *pos + 2)) << std::endl;
				*pos += 4;
				size -= 2;
				size -= 2;
				break;
			case LOADCTXIVAR:
				MEM(4)
				output << "\t\t\tLOADCTXIVAR " << SHOW(read16(bytecode + *pos)) << " " << SHOW(read16(bytecode + *pos + 2)) << std::endl;
				*pos += 4;
				size -= 2;
				size -= 2;
				break;
			case LOADCTXSVAR:
				MEM(4)
				output << "\t\t\tLOADCTXSVAR " << SHOW(read16(bytecode + *pos)) << " " << SHOW(read16(bytecode + *pos + 2)) << std::endl;
				*pos += 4;
				size -= 2;
				size -= 2;
				break;
			case STORECTXDVAR:
				MEM(4)
				output << "\t\t\tSTORECTXDVAR " << SHOW(read16(bytecode + *pos)) << " " << SHOW(read16(bytecode + *pos + 2)) << std::endl;
				*pos += 4;
				size -= 2;
				size -= 2;
				break;
			case STORECTXIVAR:
				MEM(4)
				output << "\t\t\tSTORECTXIVAR " << SHOW(read16(bytecode + *pos)) << " " << SHOW(read16(bytecode + *pos + 2)) << std::endl;
				*pos += 4;
				size -= 2;
				size -= 2;
				break;
			case STORECTXSVAR:
				MEM(4)
				output << "\t\t\tSTORECTXSVAR " << SHOW(read16(bytecode + *pos)) << " " << SHOW(read16(bytecode + *pos + 2)) << std::endl;
				*pos += 4;
				size -= 2;
				size -= 2;
				break;
			case DCMP:
				output << "\t\t\tDCMP" << std::endl;
				break;
			case ICMP:
				output << "\t\t\tICMP" << std::endl;
				break;
			case JA:
				//*pos += read16(bytecode + *pos);
				//MEM(2)
				for (int k = 0; k < 2; ++k) { 
					unsigned short a = (unsigned char)code[tmp_off + k];
					output << std::setfill('0') << std::setw(2) << ((code[tmp_off + k]) & 0xFF) << " "; 
				}
				output << "\t\t\t\tJA " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case IFICMPNE:
				MEM(2)
				output << "\t\t\tIFICMPNE " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case IFICMPE:
				MEM(2)
				output << "\t\t\tIFICMPE " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case IFICMPG:
				MEM(2)
				output << "\t\t\tIFICMPG " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case IFICMPGE:
				MEM(2)
				output << "\t\t\tIFICMPGE " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case IFICMPL:
				MEM(2)
				output << "\t\t\tIFICMPL " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case IFICMPLE:
				MEM(2)
				output << "\t\t\tIFICMPLE " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
			case DUMP:
				output << "\t\t\t\t\tDUMP" << std::endl;
				break;
			case STOP:
				output << "\t\t\t\t\tSTOP" << std::endl;
				break;
			case CALL:
				MEM(2)
				output << "\t\t\t\tCALL " << SHOW(read16(bytecode + *pos)) << std::endl;
				*pos += 2;
				size -= 2;
				break;
				/*todo*/
			case CALLNATIVE:
				output << "\t\t\t\t\tCALLNATIVE " << std::endl;
				break;
			case RETURN:
				output << "\t\t\t\t\tRETURN" << std::endl;
				break;
				/*todo*/
			case BREAK:
				break;
			}
		}
		output << std::endl;
	}
	output.close();
	free(code);
	return 0;
}