#include <stdio.h>
#include <stdlib.h>
#include "head.h"


//acresenta PC
unsigned char busca(unsigned char instruction)
{

    return instruction + 1;

}

//para jumps
unsigned char jump(unsigned char instruct)
{

    return instruct;

}


//para branchs
unsigned char branch(unsigned char instruct,unsigned char imm)
{

    return instruct + imm +1;

}
