#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "head.h"

#ifdef _WIN32
    #include <windows.h>
    #include <commdlg.h>
#endif

void selecionar_arquivo(char *caminho)
{
#ifdef _WIN32

    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = caminho;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 100;
    ofn.lpstrFilter = "Arquivos MEM\0*.mem\0Todos\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        printf("Arquivo selecionado: %s\n", caminho);
    }

#else

    printf("Digite o caminho do arquivo (.mem): ");
    scanf("%s", caminho);

    strcat(caminho, ".mem");

#endif
}

void selecionar_arquivo2(char *caminho)
{
#ifdef _WIN32

    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = caminho;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 260;
    ofn.lpstrFilter = "Arquivos DAT\0*.dat\0Todos\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        printf("Arquivo selecionado: %s\n", caminho);
    }

#else

    printf("Digite o caminho do arquivo (.dat): ");
    scanf("%s", caminho);

    strcat(caminho, ".dat");

#endif
}


//primeiro estagio do pipeline
static void estagio_IF(instro *mem, unsigned char *PC, RegIF_ID *if_id)
{



    if_id->PC= busca(*PC);

    if_id->RI = ler(mem,*PC);

    if_id->valid = 1;

    *PC = if_id->PC;



    printf("  [IF ] PC=%d |", *PC);

    printf(" instrução atual:");

    print_bin(if_id->RI);

    printf(" -> ");

    print_asm(if_id->RI);


}


//segundo estagio do pipeline
static void estagio_ID(signed char *reg, RegIF_ID *if_id, RegID_EX *id_ex)
{
    //carrega a instrução do primeiro pipe
    unsigned short instr = if_id->RI;

    //separa os bits
    unsigned char opcode = (instr >> 12) & 0xF;
    unsigned char rs     = (instr >> 9)  & 0x7;
    unsigned char rt     = (instr >> 6)  & 0x7;
    unsigned char rd     = (instr >> 3)  & 0x7;
    unsigned char imm6   = instr & 0x3F;

    //decodifica instrução

    Decodifica_controle(opcode,&id_ex->RegDst,&id_ex->ULAOp,&id_ex->ULAFonte,&id_ex->beq,&id_ex->jump,&id_ex->EscMem,&id_ex->EscReg,&id_ex->MemParaReg);



    //passa a proxima instrução para o terceiro pipeline
    id_ex->PC= if_id->PC;

    id_ex->rt= rt;

    id_ex->rd= rd;

    read(reg,(signed char)rs,(signed char)rt, &id_ex->A, &id_ex->B);

    Estender(imm6, &id_ex->immx);

    id_ex->valid = 1;



    // impressões

    printf("[ID ] rs=R%d(%d) rt=R%d(%d) rd=R%d imm=%d \n",rs, id_ex->A, rt, id_ex->B, rd, id_ex->immx);

    print_sinais(id_ex->RegDst,id_ex->ULAOp,id_ex->ULAFonte,id_ex->beq,id_ex->jump,id_ex->EscMem,id_ex->EscReg,id_ex->MemParaReg);





    //printf(" [ID ] op=0x%X rs=R%d(%d) rt=R%d(%d) rd=R%d imm=%d  RegDst=%d ULAOp=%d ULAFonte=%d beq=%d jump=%d EscMem=%d EscReg=%d MemParaReg=%d\n",opcode, rs, id_ex->A, rt, id_ex->B, rd, id_ex->immx,id_ex->RegDst, id_ex->ULAOp, id_ex->ULAFonte, id_ex->beq, id_ex->jump,id_ex->EscMem, id_ex->EscReg, id_ex->MemParaReg);
}





static void estagio_EX(RegID_EX *id_ex, RegEX_MEM *ex_mem)
{


    //flags da ula
    int overflow = 0,zero = 0;

    //ulamx
    int operandoB = ulamx(id_ex->ULAFonte,id_ex->A,id_ex->B,id_ex->immx);



//
//passando para o proximo pipe
//

    // saida da ula
    ex_mem->ULA_saida = ula(id_ex->ULAOp,id_ex->A,operandoB,&overflow,&zero);


    //regdst
    ex_mem->dest = Rdest(id_ex->RegDst,id_ex->rd,id_ex->rt);

    //endereço de salto

    if (id_ex->beq)
    {
        ex_mem->add_result = branch(id_ex->PC, (unsigned char)id_ex->immx);
    }

    if (id_ex->jump)
    {
        ex_mem->add_result = jump((unsigned char)id_ex->immx);
    }


    //registrador b
    ex_mem->B = id_ex->B;

    //sinais de controle
    ex_mem->EscReg= id_ex->EscReg;

    ex_mem->MemParaReg= id_ex->MemParaReg;

    ex_mem->EscMem= id_ex->EscMem;

    ex_mem->beq= id_ex->beq;

    ex_mem->jump= id_ex->jump;

    ex_mem->valid= 1;

    //flags

    ex_mem->zero= zero;








    printf("  [EX ] ULA=%d zero=%d EscReg=%d EscMem=%d beq=%d jump=%d\n",
           ex_mem->ULA_saida, ex_mem->zero,
           ex_mem->EscReg, ex_mem->EscMem, ex_mem->beq, ex_mem->jump);
}



static void estagio_MEM(instro *mem, unsigned char *PC, RegEX_MEM *ex_mem, RegMEM_WB *mem_wb)
{

    mem_wb->dest = ex_mem->dest;
    mem_wb->RegWrite = ex_mem->EscReg;
    mem_wb->MemToReg = ex_mem->MemParaReg;
    mem_wb->ULA_saida= ex_mem->ULA_saida;
    mem_wb->RDM= 0;
    mem_wb->valid= 1;


    //load
    if (ex_mem->MemParaReg)
    {
        mem_wb->RDM = (signed char)ler_unificada(mem, (unsigned char)ex_mem->ULA_saida);

        printf("  [MEM] LW: Mem[%d] = %d\n", (unsigned char)ex_mem->ULA_saida, mem_wb->RDM);
    }

    //store
    if (ex_mem->EscMem)
    {
        Store(mem, (signed char)ex_mem->ULA_saida, ex_mem->B);

        printf("  [MEM] SW: Mem[%d] <- %d\n", (unsigned char)ex_mem->ULA_saida, ex_mem->B);

        mem_wb->RegWrite = 0;

    }






    if (ex_mem->beq)
    {
        if (ex_mem->zero)
        {
            *PC = (unsigned char)ex_mem->add_result;
            printf("  [MEM] BEQ tomado: PC <- %d\n", *PC);
        }
        else
        {
            printf("  [MEM] BEQ nao tomado\n");
        }
        mem_wb->RegWrite = 0;
        incrementaInstr(conta, 0x8);
        (*n_instr)++;
    }

    if (ex_mem->jump)
    {
        *PC = (unsigned char)ex_mem->add_result;
        printf("  [MEM] JUMP: PC <- %d\n", *PC);
        mem_wb->RegWrite = 0;
        incrementaInstr(conta, 0x2);
        (*n_instr)++;
    }


}



static void estagio_WB(signed char *reg, RegMEM_WB *mem_wb)
{

    signed char valor = mem_wb->MemToReg ? mem_wb->RDM : mem_wb->ULA_saida;

    esc(reg, (int)mem_wb->dest, valor, 1);

    printf("  [WB ] R%d <- %d\n", mem_wb->dest, valor);


}







int main() {

    //operanado de controle do menu
    int op;

    //registrador
    signed char reg[8];

    iniat(reg);

    //instrução
    unsigned char instruction =0;

    // sao simalres entao nao ha necessidade de duas structs de 256 posições
	instro l = {0}, dat = {0};

    //primeiro registrador pipeline
    RegIF_ID  if_id  = {0};

    //segundo registrador pipeline
    RegID_EX  id_ex  = {0};

    //terceiro registrador pipeline
    RegEX_MEM ex_mem = {0};

    //quarto registrador pipeline
    RegMEM_WB mem_wb = {0};


    //RegEX_MEM ex_mem = {0};
    //RegMEM_WB mem_wb = {0};;

    //?????
    char caminho[100];

    int i;

   do{

        printf("\n 9 - executar \n 10 - para sair");
        scanf("%d", &op);

        switch (op)
        {

            case 1:

                caminho[0] = '\0';

                selecionar_arquivo(caminho);

                if (carregar(&l, caminho, &i) != 0)
                {

                    printf("\n erro ao carregar memoria de instruções");

                }

                else
                {

                    printf("\n memoria de instruções carregada com sucesso");

                }


            break;

            case 9:

            //decoficação

            //busca

            if(mem_wb.valid == 1)
            {
                estagio_WB(reg,&mem_wb);
            }


            if(ex_mem.valid == 1)
            {
                estagio_MEM(&l,&instruction,&ex_mem, &mem_wb);
            }

            if(id_ex.valid == 1)
            {
                estagio_EX(&id_ex, &ex_mem);
            }


            if(if_id.valid == 1)
            {
                estagio_ID(reg,&if_id, &id_ex);
            }


            estagio_IF(&l, &instruction, &if_id);



            break;


            default:

            printf("\n opção invalida\n");

            break;
}

   }while(op!=10);

    return 0;
}
