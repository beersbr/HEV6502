/**********************
 * HEV6502 CPU Emulator
 * CPU.CPP
 * CPU function definitions
 **********************/
#include "cpu.h"


#define byte char

CPU::CPU(MemoryController* memory)
{
    cpuMem = memory;
    PC = cpuMem->getStartAddr();
    A = X = Y = ST = 0;
    updateStatusFlags();
    updateFlagReg();
    SP = 0xFF; //stack stars here, grows down.
    currentClocks = 0;
    //CPU initialized, but don't call execute yourself!
}

bool CPU::loadJumpTable()
{
    for(int i = 0; i < 0x100; i++)
    {
        opTable[i] = 0;
    }

    /* ADC */
    opTable[0x69] = &CPU::adci;
    opTable[0x65] = &CPU::adcz;
    opTable[0x75] = &CPU::adczx;
    opTable[0x6D] = &CPU::adca;
    opTable[0x7D] = &CPU::adcax;
    opTable[0x79] = &CPU::adcay;
    opTable[0x61] = &CPU::adcix;
    opTable[0x71] = &CPU::adciy;

    /* AND */
    opTable[0x29] = &CPU::andi;
    opTable[0x25] = &CPU::andz;
    opTable[0x35] = &CPU::andzx;
    opTable[0x2D] = &CPU::anda;
    opTable[0x3D] = &CPU::andax;
    opTable[0x39] = &CPU::anday;
    opTable[0x21] = &CPU::andix;
    opTable[0x31] = &CPU::andiy;

    /* ASL */
    opTable[0x0A] = &CPU::asla;
    opTable[0x06] = &CPU::aslz;
    opTable[0x16] = &CPU::aslzx;
    opTable[0x0E] = &CPU::asla;
    opTable[0x1E] = &CPU::aslax;


    /* BCx */
    opTable[0x90] = &CPU::bcc;
    opTable[0xB0] = &CPU::bcs;
    opTable[0xF0] = &CPU::beq;
    opTable[0x30] = &CPU::bmi;
    opTable[0xD0] = &CPU::bne;
    opTable[0x10] = &CPU::bpl;
    opTable[0x50] = &CPU::bvc;
    opTable[0x70] = &CPU::bvs;

    opTable[0x24] = &CPU::bitz;
    opTable[0x2C] = &CPU::bita;

    opTable[0x00] = &CPU::brk;

    /* CLx */
    opTable[0x18] = &CPU::clc;
    opTable[0xD8] = &CPU::cld;
    opTable[0x58] = &CPU::cli;
    opTable[0xB8] = &CPU::clv;

    /* CMP */
    opTable[0xC9] = &CPU::cmpi;
    opTable[0xC5] = &CPU::cmpz;
    opTable[0xD5] = &CPU::cmpzx;
    opTable[0xCD] = &CPU::cmpa;
    opTable[0xDD] = &CPU::cmpax;
    opTable[0xD9] = &CPU::cmpay;
    opTable[0xC1] = &CPU::cmpix;
    opTable[0xD1] = &CPU::cmpiy;

    /* CPX */
    opTable[0xE0] = &CPU::cpxi;
    opTable[0xE4] = &CPU::cpxz;
    opTable[0xEC] = &CPU::cpxa;

    /* CPY */
    opTable[0xC0] = &CPU::cpyi;
    opTable[0xC4] = &CPU::cpyz;
    opTable[0xCC] = &CPU::cpya;

    /* DEC */
    opTable[0xC6] = &CPU::decz;
    opTable[0xD6] = &CPU::deczx;
    opTable[0xCE] = &CPU::deca;
    opTable[0xDE] = &CPU::decax;

    opTable[0xCA] = &CPU::dex;

    opTable[0x88] = &CPU::dey;

    /* EOR */
    opTable[0x49] = &CPU::eori;
    opTable[0x45] = &CPU::eorz;
    opTable[0x55] = &CPU::eorzx;
    opTable[0x4D] = &CPU::eora;
    opTable[0x5D] = &CPU::eorax;
    opTable[0x59] = &CPU::eoray;
    opTable[0x41] = &CPU::eorix;
    opTable[0x51] = &CPU::eoriy;

    /* INC */
    opTable[0xE6] = &CPU::incz;
    opTable[0xF6] = &CPU::inczx;
    opTable[0xEE] = &CPU::inca;
    opTable[0xFE] = &CPU::incax;

    opTable[0xE8] = &CPU::inx;

    opTable[0xC8] = &CPU::iny;

    /* JMP */
    opTable[0x4C] = &CPU::jmpa;
    opTable[0x6C] = &CPU::jmpi;

    /* JSR */
    opTable[0x20] = &CPU::jsr;

    /* LDA */
    opTable[0xA9] = &CPU::ldai;
    opTable[0xA5] = &CPU::ldaz;
    opTable[0xB5] = &CPU::ldazx;
    opTable[0xAD] = &CPU::ldaa;
    opTable[0xBD] = &CPU::ldaax;
    opTable[0xB9] = &CPU::ldaay;
    opTable[0xA1] = &CPU::ldaix;
    opTable[0xB1] = &CPU::ldaiy;

    /* LDX */
    opTable[0xA2] = &CPU::ldxi;
    opTable[0xA6] = &CPU::ldxz;
    opTable[0xB6] = &CPU::ldxzy;
    opTable[0xAE] = &CPU::ldxa;
    opTable[0xBE] = &CPU::ldxay;

    /* LDY */
    opTable[0xA0] = &CPU::ldyi;
    opTable[0xA4] = &CPU::ldyz;
    opTable[0xB4] = &CPU::ldyzx;
    opTable[0xAC] = &CPU::ldya;
    opTable[0xBC] = &CPU::ldyax;

    /* LSR */
    opTable[0x4A] = &CPU::lsrac;
    opTable[0x46] = &CPU::lsrz;
    opTable[0x56] = &CPU::lsrzx;
    opTable[0x4E] = &CPU::lsra;
    opTable[0x5E] = &CPU::lsrax;

    /* NOP */
    opTable[0xEA] = &CPU::nop;

    /* ORA */
    opTable[0x09] = &CPU::orai;
    opTable[0x05] = &CPU::oraz;
    opTable[0x15] = &CPU::orazx;
    opTable[0x0D] = &CPU::oraa;
    opTable[0x1D] = &CPU::oraax;
    opTable[0x19] = &CPU::oraay;
    opTable[0x01] = &CPU::oraix;
    opTable[0x11] = &CPU::oraiy;

    /* Pxx */
    opTable[0x48] = &CPU::pha;
    opTable[0x08] = &CPU::php;
    opTable[0x68] = &CPU::pla;
    opTable[0x28] = &CPU::plp;

    /* ROL */
    opTable[0x2A] = &CPU::rolac;
    opTable[0x26] = &CPU::rolz;
    opTable[0x36] = &CPU::rolzx;
    opTable[0x2E] = &CPU::rola;
    opTable[0x3E] = &CPU::rolax;

    /* ROR */
    opTable[0x6A] = &CPU::rorac;
    opTable[0x66] = &CPU::rorz;
    opTable[0x76] = &CPU::rorzx;
    opTable[0x6E] = &CPU::rora;
    opTable[0x7E] = &CPU::rorax;

    /* RTI */
    opTable[0x40] = &CPU::rti;

    /* RTS */
    opTable[0x60] = &CPU::rts;

    /* SBC */
    opTable[0xE9] = &CPU::sbci;
    opTable[0xE5] = &CPU::sbcz;
    opTable[0xF5] = &CPU::sbczx;
    opTable[0xED] = &CPU::sbca;
    opTable[0xFD] = &CPU::sbcax;
    opTable[0xF9] = &CPU::sbcay;
    opTable[0xE1] = &CPU::sbcix;
    opTable[0xF1] = &CPU::sbciy;

    /* SEC */
    opTable[0x38] = &CPU::sec;

    /* SED */
    opTable[0xF8] = &CPU::sed;

    /* SEI */
    opTable[0x78] = &CPU::sei;

    /* STA */
    opTable[0x85] = &CPU::staz;
    opTable[0x95] = &CPU::stazx;
    opTable[0x9D] = &CPU::staa;
    opTable[0x9D] = &CPU::staax;
    opTable[0x99] = &CPU::staay;
    opTable[0x81] = &CPU::staix;
    opTable[0x91] = &CPU::staiy;

    /* STX */
    opTable[0x86] = &CPU::stxz;
    opTable[0x96] = &CPU::stxzy;
    opTable[0x8E] = &CPU::stxa;

    /* STY */
    opTable[0x84] = &CPU::styz;
    opTable[0x94] = &CPU::styzx;
    opTable[0x8C] = &CPU::stya;

    /* Txx */
    opTable[0xAA] = &CPU::tax;
    opTable[0xA8] = &CPU::tay;
    opTable[0xBA] = &CPU::tsx;
    opTable[0x8A] = &CPU::txa;
    opTable[0x9A] = &CPU::txs;
    opTable[0x98] = &CPU::tya;

}

int CPU::execute()
{
    //PC == current opcode, call the function at the jump table.
    int res = 0;
    byte tmp = 0;
    while(res != -1) //while we haven't been ordered to HALT
    {
        tmp = cpuMem->loadByte(PC);
        ++PC;
        (this->*opTable[tmp])(); //calling necessary function;
    }
}

short CPU::relative()
{
    short addr = cpuMem->loadWord(PC);
    (addr < 0x80)? addr+= PC : addr += (PC - 256); 
    return addr;
}
 
short CPU::zeroPageX()
{
    short addr = cpuMem->loadByte(PC);
    addr = (( addr + X ) & 0xFF);
    return addr;
}

short CPU::zeroPageY()
{
    short addr = cpuMem->loadByte(PC);
    addr = (( addr + Y ) & 0xFF);
    return addr;
}

short CPU::absolute()
{
    //full address
    short addr = ((*++in) << 8); //high byte
    addr = (addr ^ (*in));       //low  byte
    //byte res = cpuMem->loadByte(addr);
    return addr;
}
short CPU::absoluteX()
{
    //full address + X with wrapping..maybe
    byte low = (*in);
    (low + X > 255) ? low -= X : low += X;
    short addr = ((*++in) << 8); //high byte
    addr = (addr ^ low);
    //byte res = cpuMem->loadByte(addr);
    return addr;
}
short CPU::absoluteY()
{
    //full address + Y with wrapping..maybe
    byte low = (*in);
    (low + Y > 255) ? low -= Y : low += Y;
    short addr = ((*++in) << 8); //high byte
    addr = (addr ^ low);
    //byte res = cpuMem->loadByte(addr);
    return addr;
}
short CPU::indirect()
{
    //used for jump, load address from address.
    short addr = (((*++in) << 8) & 0xFF00);
    addr = (addr ^ (*in)); //sweet, now load this byte and the next one
    short target = (((cpuMem->loadByte(addr)) << 8) & 0xFF00);
    target = (target ^ cpuMem->loadByte(++addr));
    return target;

}
//returning the address to work with.
short CPU::indexedIndirect()
{
    //we're loading the address at ($00(x + *in))
    short addr = (*in) + X;
    addr &= 0xFF00; //load the zero page address here and +1
    short target = cpuMem->loadByte(addr) << 8 & 0xFF00;
    target = target ^ cpuMem->loadByte(++addr);
    //now load the target
    return target; 
}
//returning the address to work with
short CPU::indirectIndexed()
{
    //rol ($2A), Y
    //The value $03 in Y is added to the address $C235 at addresses $002A and $002B for a sum of $C238. 
    //The value $2F at $C238 is shifted right (yielding $17) and written back to $C238.
    short target = cpuMem->loadByte(*in) << 8 & 0xFF00;
    target = target ^ cpuMem->loadByte(++(*in));
    target += Y;
    return target;
}

void CPU::updateFlagReg()
{
    //compress flags into ST
    carryFlag = (carryFlag ? 1 : 0);
    if(carryFlag)
        ST &= FLAG_CARRY;
    zeroFlag = (zeroFlag ? 1 : 0);
    if(zeroFlag)
        ST &= FLAG_ZERO;
    intFlag = (intFlag ? 1 : 0);
    if(intFlag)
        ST &= FLAG_INT;
    decFlag = (decFlag ? 1 : 0);
    if(decFlag)
        ST &= FLAG_DEC;
    brkFlag = (brkFlag ? 1 : 0);
    if(brkFlag)
        ST &= FLAG_BRK;
    overFlag = (overFlag ? 1 : 0);
    if(overFlag)
        ST &= FLAG_OVER;
    signFlag = (signFlag ? 1 : 0);
    if(signFlag)
        ST &= FLAG_SIGN;

}

void CPU::updateStatusFlags()
{
    //extract ST into flag vars
    if(ST & FLAG_CARRY)
        carryFlag = 1;
    if(ST & FLAG_ZERO)
        zeroFlag = 1;
    if(ST & FLAG_INT)
        intFlag = 1;
    if(ST & FLAG_DEC)
        decFlag = 1;
    if(ST & FLAG_BRK)
        brkFlag = 1;
    if(ST & FLAG_OVER)
        overFlag = 1;
    if(ST & FLAG_SIGN)
        signFlag = 1;
}

void CPU::push(byte toPush)
{
    //stack grows down
    SP--;
    if(!SP)
    {
        //STACK OVERFLOW!
    }
    cpuMem->writeByte(toPush, (0x100 + SP));
}

byte CPU::pull()
{
    //stack pops up
    byte retVal = cpuMem->loadByte(SP + 0x100);
    SP++;
    return retVal;
}

byte CPU::addCOp(byte toAdd)
{
    short res = A + toAdd + carryFlag;
    overFlag = (!((A ^ toAdd) & 0x80) && ((A ^ res) & 0x80)) ? 1 : 0; //signed overflow
    carryFlag = res > 0xFF ? 1 : 0;  //unsigned overflow
    signFlag  = (res >> 7) & 1;      //last bit set to 1
    zeroFlag  = !(res);              //Did we get a zero?
    return (byte)res;
}

int CPU::adci()
{
    //add with carry immediate
    A = (addCOp(*in) & 0xFF);
    return 2;
}

int CPU::adcz()
{
    A = (addCOp(cpuMem->loadByte(*in)) & 0xFF);
    return 3;
}

int CPU::adczx()
{
    byte val = cpuMem->loadByte(zeroPageX(in));
    A = (addCOp(val) & 0xFF);
    return 4;
}

int CPU::adca()
{
    byte val = cpuMem->loadByte(absolute(in));
    A = (addCOp(val) & 0xFF);
    return 4;
}

int CPU::adcax()
{
    byte val = cpuMem->loadByte(absoluteX(in));
    A = (addCOp(val) & 0xFF);
    return 4;   //could be +1
}

int CPU::adcay()
{
    byte val = cpuMem->loadByte(absoluteY(in));
    A = (addCOp(val) & 0xFF);
    return 4;   //could be +1
}

int CPU::adcix()
{
    byte val = cpuMem->loadByte(indexedIndirect(in));
    A = (addCOp(val) & 0xFF);
    return 6;
}


int CPU::adciy()
{
    byte val = cpuMem->loadByte(indirectIndexed(in));
    A = (addCOp(val) & 0xFF);
    return 5;
}

byte CPU::andOp(byte toAnd)
{
    //Used to perform common AND operations
    byte res = A & toAnd;
    signFlag = (A >> 7 & 1);
    (A > 0) ? zeroFlag = 0 : zeroFlag = 1;
    
    return res;
    
}

int CPU::andi()
{
    A = andOp((*in));
    return 2;
}

int CPU::andz()
{
    A = andOp(cpuMem->loadByte((*in)));
    return 3;
}

int CPU::andzx()
{
    byte res = cpuMem->loadByte(zeroPageX(in));
    A = andOp(res);
    return 4;
}

int CPU::anda()
{
    byte res = cpuMem->loadByte(absolute(in));
    A = andOp(res);
    return 4;
}

int CPU::andax()
{
    byte res = cpuMem->loadByte(absoluteX(in));
    A = andOp(res);
    return 4; //could be + 1
}

int CPU::anday()
{
    byte res = cpuMem->loadByte(absoluteY(in));
    A = andOp(res);
    return 4; //could be + 1
}

int CPU::andix()
{
    byte res = cpuMem->loadByte(indexedIndirect(in));
    A = andOp(res);
    return 6;    
}

int CPU::andiy()
{
    byte res = cpuMem->loadByte(indirectIndexed(in));
    A = andOp(res);
    return 5; //could be + 1
}

byte CPU::aslOp(byte toShift)
{
    //arithmetic shift left
    byte res  = toShift;
    carryFlag = (res >> 7) & 1;
    toShift   = (res << 1) & 0xFF;
    signFlag  = (res >> 7) &1;
    zeroFlag  = !(res);
    return res;
}

int CPU::aslac() //accumulator
{
    A = aslOp(A);
    return 2;
}

int CPU::aslz()
{
    cpuMem->writeByte(aslOp(cpuMem->loadByte(*in)), *in);
    return 5;
}

int CPU::aslzx()
{
    cpuMem->writeByte(aslOp(cpuMem->loadByte(zeroPageX(in))), zeroPageX(in));
    return 6;
}

int CPU::asla()
{
    cpuMem->writeByte(aslOp(cpuMem->loadByte(absolute(in))), absolute(in));
    return 6;
}

int CPU::aslax()
{
    cpuMem->writeByte(aslOp(cpuMem->loadByte(absoluteX(in))), absoluteX(in));
    return 7;
}

int CPU::bcc(byte *in)
{
    //if carry is clear, branch
    if(!carryFlag)
        PC = relative(in);
    return 2; //could be + 1 or 2
}

int CPU::bcs(byte *in)
{
    //if carry is set, branch
    if(carryFlag)
        PC = relative(in);
    return 2;
}

int CPU::beq(byte *in)
{
    //branch if equal
    if(zeroFlag)
        PC = relative(in);
    return 2;
}


int CPU::bitz(byte *in)
{
    byte val = cpuMem->loadByte(*in);
    signFlag = (val >> 7) & 1;
    overFlag = (val >> 6) & 1;
    zeroFlag = A & val;
    return 3;
}

int CPU::bita(byte *in)
{
    byte val = cpuMem->loadByte(absolute(in));
    signFlag = (val >> 7) & 1;
    overFlag = (val >> 6) & 1;
    zeroFlag = A & val;
    return 4;
}

int CPU::bmi()
{
    if(signFlag)
        PC = relative(in);
    return 2;
}

int CPU::bne()
{
    if(!zeroFlag)
        PC = relative(in);
    return 2;
}

int CPU::bpl()
{
    if(!signFlag)
        PC = relative(in);
    return 2;
}

int CPU::brk()
{
    //forces interrupt
    PC += 2; //increment counter
    push(((PC >> 8) & 0xFF));
    push((PC & 0xFF));
    brkFlag = 1;
    updateFlagReg();
    push(ST);
    
    intFlag = 1;
    PC = cpuMem->loadWord(0xFFFE);
    PC--;
    return 7;
}

int CPU::bvc()
{
    //branch if overflow clear
    if(!overFlag)
        PC = relative(in);
    return 2;
}

int CPU::bvs()
{
    if(overFlag)
        PC = relative(in);
    return 2;
} 

int CPU::clc()
{
    carryFlag = 0;
    return 2;
}

int CPU::cld()
{
    decFlag = 0;
    return 2;
}

int CPU::cli()
{
    intFlag = 0;
    return 2;
}

int CPU::clv()
{
    overFlag = 0;
    return 2;
}

byte CPU::cmpOp(byte toComp)
{
    byte res = A - toComp;
    carryFlag = (res > 0? 1 : 0);
    signFlag = (res >> 7) &1;
    zeroFlag = res & 0xFF;
    return res;
}

int CPU::cmpi()
{
    //add with carry immediate
    cmpOp(*in);
    return 2;
}

int CPU::cmpz()
{
    cmpOp(cpuMem->loadByte(*in));
    return 3;
}

int CPU::cmpzx()
{
    cmpOp(cpuMem->loadByte(zeroPageX(in)));
    return 4;
}

int CPU::cmpa()
{
    cpuMem->loadByte(absolute(in));
    return 4;
}

int CPU::cmpax()
{
    cpuMem->loadByte(absoluteX(in));
    return 4;   //could be +1
}

int CPU::cmpay()
{
    cpuMem->loadByte(absoluteY(in));
    return 4;   //could be +1
}

int CPU::cmpix()
{
    cpuMem->loadByte(indexedIndirect(in));
    return 6;
}


int CPU::cmpiy()
{
    cpuMem->loadByte(indirectIndexed(in));
    return 5;
}

byte CPU::cpxOp(byte toComp)
{
    byte res = X - toComp;
    carryFlag = (res >= 0? 1: 0);
    signFlag = (res >> 7)&1;
    zeroFlag = res&0xFF;
    return res;
}

int CPU::cpxi()
{
    cpxOp(*in);
    return 2;
}

int CPU::cpxz()
{
    cpxOp(cpuMem->loadByte(*in));
    return 3;
}

int CPU::cpxa()
{
    cpxOp(cpuMem->loadByte(absolute(in)));
    return 4;
}

byte CPU::cpyOp(byte toComp)
{
    byte res = Y - toComp;
    carryFlag = (res >= 0? 1: 0);
    signFlag = (res >> 7)&1;
    zeroFlag = res&0xFF;
    return res;
}

int CPU::cpyi()
{
    cpyOp(*in);
    return 2;
}

int CPU::cpyz()
{
    cpyOp(cpuMem->loadByte(*in));
    return 3;
}

int CPU::cpya()
{
    cpyOp(cpuMem->loadByte(absolute(in)));
    return 4;
}

byte CPU::decOp(byte toDec)
{
    byte res = (toDec -1)&0xFF;
    signFlag = (res >> 7)&1;
    zeroFlag = res;
    return res;
}

int CPU::decz()
{
    cpuMem->writeByte(cpuMem->loadByte(*in), *in);
    return 5;
}

int CPU::deczx()
{
    cpuMem->writeByte(cpuMem->loadByte(zeroPageX(in)), zeroPageX(in));
    return 6;
}

int CPU::deca()
{
    cpuMem->writeByte(cpuMem->loadByte(absolute(in)), absolute(in));
    return 6;
}

int CPU::decax()
{
    cpuMem->writeByte(cpuMem->loadByte(absoluteX(in)), absoluteX(in));
    return 7;
}

int CPU::dex()
{
    X -= 1;
    zeroFlag = X;
    signFlag = (X >> 7&1);
    return 2;
}

int CPU::dey()
{
    Y -= 1;
    zeroFlag = Y;
    signFlag = (Y >> 7&1);
    return 2;
}


byte CPU::eorOp(byte toeor)
{
    //Used to perform common eor operations
    byte res = A ^ toeor;
    signFlag = (A >> 7 & 1);
    (A > 0) ? zeroFlag = 0 : zeroFlag = 1;

    return res;

}

int CPU::eori()
{
    A = eorOp((*in));
    return 2;
}

int CPU::eorz()
{
    A = eorOp(cpuMem->loadByte((*in)));
    return 3;
}

int CPU::eorzx()
{
    byte res = cpuMem->loadByte(zeroPageX(in));
    A = eorOp(res);
    return 4;
}

int CPU::eora()
{
    byte res = cpuMem->loadByte(absolute(in));
    A = eorOp(res);
    return 4;
}

int CPU::eorax()
{
    byte res = cpuMem->loadByte(absoluteX(in));
    A = eorOp(res);
    return 4; //could be + 1
}

int CPU::eoray()
{
    byte res = cpuMem->loadByte(absoluteY(in));
    A = eorOp(res);
    return 4; //could be + 1
}

int CPU::eorix()
{
    byte res = cpuMem->loadByte(indexedIndirect(in));
    A = eorOp(res);
    return 6;
}

int CPU::eoriy()
{
    byte res = cpuMem->loadByte(indirectIndexed(in));
    A = eorOp(res);
    return 5; //could be + 1
}

byte CPU::incOp(byte toinc)
{
    byte res = (toinc + 1)&0xFF;
    signFlag = (res >> 7)&1;
    zeroFlag = res;
    return res;
}

int CPU::incz()
{
    cpuMem->writeByte(cpuMem->loadByte(*in), *in);
    return 5;
}

int CPU::inczx()
{
    cpuMem->writeByte(cpuMem->loadByte(zeroPageX(in)), zeroPageX(in));
    return 6;
}

int CPU::inca()
{
    cpuMem->writeByte(cpuMem->loadByte(absolute(in)), absolute(in));
    return 6;
}

int CPU::incax()
{
    cpuMem->writeByte(cpuMem->loadByte(absoluteX(in)), absoluteX(in));
    return 7;
}

int CPU::inx()
{
    X += 1;
    zeroFlag = X;
    signFlag = (X >> 7&1);
    return 2;
}

int CPU::iny()
{
    Y += 1;
    zeroFlag = Y;
    signFlag = (Y >> 7&1);
    return 2;
}

int CPU::jmpa()
{
    //set PC to absolute address
    PC = absolute(in) -1;
    return 3;
}

int CPU::jmpi()
{
    PC = indirect(in) -1;
    return 5;
}

int CPU::jsr()
{
    push((PC >> 8) & 0xFF);
    push((PC & 0xFF));
    PC = absolute(in)-1;
    return 6;
}

int CPU::ldai()
{
    A = *in;
    signFlag = (A >> 7)&1;
    zeroFlag = A;
    return 2;
}

int CPU::ldaz()
{
    A = cpuMem->loadByte(*in);
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 3;
}

int CPU::ldazx()
{
    A = cpuMem->loadByte(zeroPageX(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4;
}

int CPU::ldaa()
{
    A = cpuMem->loadByte(absolute(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4;
}

int CPU::ldaax()
{
    A = cpuMem->loadByte(absoluteX(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4; //could be +1
}

int CPU::ldaay()
{
    A = cpuMem->loadByte(absoluteY(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4; //could be + 1
}

int CPU::ldaix()
{
    A = cpuMem->loadByte(indexedIndirect(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 6;
}

int CPU::ldaiy()
{
    A = cpuMem->loadByte(indirectIndexed(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 5; //could be + 1
}

int CPU::ldxi()
{
    X = *in;
    signFlag = (A >> 7)&1;
    zeroFlag = A;
    return 2;
}

int CPU::ldxz()
{
    X = cpuMem->loadByte(*in);
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 3;
}

int CPU::ldxzy()
{
    X = cpuMem->loadByte(zeroPageY(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4;
}

int CPU::ldxa()
{
    X = cpuMem->loadByte(absolute(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4;
}

int CPU::ldxay()
{
    X = cpuMem->loadByte(absoluteY(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4; //could be + 1
}

int CPU::ldyi()
{
    Y = *in;
    signFlag = (A >> 7)&1;
    zeroFlag = A;
    return 2;
}

int CPU::ldyz()
{
    Y = cpuMem->loadByte(*in);
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 3;
}

int CPU::ldyzx()
{
    Y = cpuMem->loadByte(zeroPageX(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4;
}

int CPU::ldya()
{
    Y = cpuMem->loadByte(absolute(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4;
}

int CPU::ldyax()
{
    Y = cpuMem->loadByte(absoluteX(in));
    signFlag = (A >> 7) &1;
    zeroFlag = A;
    return 4; //could be + 1
}

byte CPU::lsrOp(byte toShift)
{
    //arithmetic shift left
    byte res  = toShift;
    carryFlag = (res) & 1;
    toShift   = (res >> 1) & 0xFF;
    signFlag  = 0;
    zeroFlag  = !(res);
    return res;
}

int CPU::lsrac() //accumulator
{
    A = lsrOp(A);
    return 2;
}

int CPU::lsrz()
{
    cpuMem->writeByte(lsrOp(cpuMem->loadByte(*in)), *in);
    return 5;
}

int CPU::lsrzx()
{
    cpuMem->writeByte(lsrOp(cpuMem->loadByte(zeroPageX(in))), zeroPageX(in));
    return 6;
}

int CPU::lsra()
{
    cpuMem->writeByte(lsrOp(cpuMem->loadByte(absolute(in))), absolute(in));
    return 6;
}

int CPU::lsrax()
{
    cpuMem->writeByte(lsrOp(cpuMem->loadByte(absoluteX(in))), absoluteX(in));
    return 7;
}


byte CPU::rolOp(byte toShift)
{
    //arithmetic shift left
    byte res  = toShift;
    carryFlag = (res >> 7) & 1;
    toShift   = (res >> 1) & 0xFF;
    signFlag  = (res >> 7) &1;
    zeroFlag  = !(res);
    return res;
}

int CPU::rolac() //accumulator
{
    A = rolOp(A);
    return 2;
}

int CPU::rolz()
{
    cpuMem->writeByte(rolOp(cpuMem->loadByte(*in)), *in);
    return 5;
}

int CPU::rolzx()
{
    cpuMem->writeByte(rolOp(cpuMem->loadByte(zeroPageX(in))), zeroPageX(in));
    return 6;
}

int CPU::rola()
{
    cpuMem->writeByte(rolOp(cpuMem->loadByte(absolute(in))), absolute(in));
    return 6;
}

int CPU::rolax()
{
    cpuMem->writeByte(rolOp(cpuMem->loadByte(absoluteX(in))), absoluteX(in));
    return 7;
}

int CPU::nop()
{
    //Oh, it's a nop
    return 2;
}

byte CPU::oraOp(byte toOra)
{
    //Used to perform common ora operations
    byte res = A | toOra;
    signFlag = (A >> 7 & 1);
    (A > 0) ? zeroFlag = 0 : zeroFlag = 1;

    return res;

}

int CPU::orai()
{
    A = oraOp((*in));
    return 2;
}

int CPU::oraz()
{
    A = oraOp(cpuMem->loadByte((*in)));
    return 3;
}

int CPU::orazx()
{
    byte res = cpuMem->loadByte(zeroPageX(in));
    A = oraOp(res);
    return 4;
}

int CPU::oraa()
{
    byte res = cpuMem->loadByte(absolute(in));
    A = oraOp(res);
    return 4;
}

int CPU::oraax()
{
    byte res = cpuMem->loadByte(absoluteX(in));
    A = oraOp(res);
    return 4; //could be + 1
}

int CPU::oraay()
{
    byte res = cpuMem->loadByte(absoluteY(in));
    A = oraOp(res);
    return 4; //could be + 1
}

int CPU::oraix()
{
    byte res = cpuMem->loadByte(indexedIndirect(in));
    A = oraOp(res);
    return 6;
}

int CPU::oraiy()
{
    byte res = cpuMem->loadByte(indirectIndexed(in));
    A = oraOp(res);
    return 5; //could be + 1
}

int CPU::pha()
{
    push(A);
    return 3;
}

int CPU::php()
{
    updateFlagReg();
    push(ST);
    return 3;
}

int CPU::pla()
{
    A = pull();
    signFlag = (A >> 7)&1;
    zeroFlag = A;
    return 4;
}

int CPU::plp()
{
    updateFlagReg();
    ST = pull();
    updateStatusFlags();
    return 4;

}

byte CPU::rorOp(byte toShift)
{
    //arithmetic shift left
    byte res  = toShift;
    byte tmp = carryFlag << 7;
    carryFlag = toShift & 1;
    toShift   = ((res >> 1) & 0xFF) + tmp;
    signFlag = (toShift >>7)&1;
    zeroFlag  = !(res);
    return res;
}

int CPU::rorac() //accumulator
{
    A = rorOp(A);
    return 2;
}

int CPU::rorz()
{
    cpuMem->writeByte(rorOp(cpuMem->loadByte(*in)), *in);
    return 5;
}

int CPU::rorzx()
{
    cpuMem->writeByte(rorOp(cpuMem->loadByte(zeroPageX(in))), zeroPageX(in));
    return 6;
}

int CPU::rora()
{
    cpuMem->writeByte(rorOp(cpuMem->loadByte(absolute(in))), absolute(in));
    return 6;
}

int CPU::rorax()
{
    cpuMem->writeByte(rorOp(cpuMem->loadByte(absoluteX(in))), absoluteX(in));
    return 7;
}

int CPU::rti()
{
    //return from function
    byte tmp = pull();
    ST = tmp;
    updateStatusFlags();

    PC = pull(); //get PC back
    PC += (pull() << 8);
    if(PC == 0xFFFF)
        return -1;
    PC--;
    return 6;
}

int CPU::rts()
{
    //returning from subroutine
    PC = pull();
    PC += (pull() << 8);

    if(PC == 0xFFFF)
        return -1;
    return 6;
}

byte CPU::sbcOp(byte toAdd)
{
    short res = A - toAdd - (1 - (carryFlag ? 1 : 0));
    overFlag = (!((A ^ toAdd) & 0x80) && ((A ^ res) & 0x80)) ? 1 : 0; //signed overflow
    carryFlag = res > 0xFF ? 1 : 0;  //unsigned overflow
    signFlag  = (res >> 7) & 1;      //last bit set to 1
    zeroFlag  = !(res);              //Did we get a zero?
    return (byte)res;
}

int CPU::sbci()
{
    //add with carry immediate
    A = (addCOp(*in) & 0xFF);
    return 2;
}

int CPU::sbcz()
{
    A = (addCOp(cpuMem->loadByte(*in)) & 0xFF);
    return 3;
}

int CPU::sbczx()
{
    byte val = cpuMem->loadByte(zeroPageX(in));
    A = (addCOp(val) & 0xFF);
    return 4;
}

int CPU::sbca()
{
    byte val = cpuMem->loadByte(absolute(in));
    A = (addCOp(val) & 0xFF);
    return 4;
}

int CPU::sbcax()
{
    byte val = cpuMem->loadByte(absoluteX(in));
    A = (addCOp(val) & 0xFF);
    return 4;   //could be +1
}

int CPU::sbcay()
{
    byte val = cpuMem->loadByte(absoluteY(in));
    A = (addCOp(val) & 0xFF);
    return 4;   //could be +1
}

int CPU::sbcix()
{
    byte val = cpuMem->loadByte(indexedIndirect(in));
    A = (addCOp(val) & 0xFF);
    return 6;
}


int CPU::sbciy()
{
    byte val = cpuMem->loadByte(indirectIndexed(in));
    A = (addCOp(val) & 0xFF);
    return 5; //could be + 1
}

int CPU::sec()
{
    carryFlag = 1;
    return 2;
}

int CPU::sed()
{
    decFlag = 1;
    return 2;
}

int CPU::sei()
{
    intFlag = 1;
    return 2;
}

int CPU::staz()
{
    cpuMem->writeByte(A, *in);
    return 3;
}

int CPU::stazx()
{
    cpuMem->writeByte(A, zeroPageX(in));
    return 4;
}

int CPU::staa()
{
    cpuMem->writeByte(A, absolute(in));
    return 4;
}

int CPU::staax()
{
    cpuMem->writeByte(A, absoluteX(in));
    return 4;
}

int CPU::staay()
{
    cpuMem->writeByte(A, absoluteY(in));
    return 4;
}

int CPU::staix()
{
    cpuMem->writeByte(A, cpuMem->loadWord(indexedIndirect(in)));
    return 6;
}

int CPU::staiy()
{
    cpuMem->writeByte(A, cpuMem->loadWord(indirectIndexed(in)));
    return 6;
}

int CPU::stxz()
{
    cpuMem->writeByte(X, *in);
    return 3;
}

int CPU::stxzy()
{
    cpuMem->writeByte(X, zeroPageY(in));
}

int CPU::stxa()
{
    cpuMem->writeByte(X, absolute(in));
    return 4;
}

int CPU::styz()
{
    cpuMem->writeByte(Y, *in);
    return 3;
}

int CPU::styzx()
{
    cpuMem->writeByte(Y, zeroPageX(in));
    return 4;
}

int CPU::stya()
{
    cpuMem->writeByte(Y, absolute(in));
    return 4;
}

int CPU::tax()
{
    X = A;
    signFlag = (A >> 7) & 1;
    zeroFlag =  A;
    return 2;
}

int CPU::tay()
{
    Y = A;
    signFlag = (A >> 7) & 1;
    zeroFlag =  A;
    return 2;
}

int CPU::tsx()
{
    X = SP; //stack pointer register
    signFlag = (SP >> 7) & 1;
    zeroFlag =  X;
    return 2;
}

int CPU::txa()
{
    A = X;
    signFlag = (X >> 7) & 1;
    zeroFlag =  X;
    return 2;
}

int CPU::txs()
{
    SP = X;
    signFlag = (X >> 7) & 1;
    zeroFlag =  SP;
    return 2;
}

int CPU::tya()
{
    A = Y;
    signFlag = (Y >> 7) & 1;
    zeroFlag =  Y;
    return 2;
}
