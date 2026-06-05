#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Head.h"
#include <locale.h>

#ifdef _WIN32
    #include <windows.h>
    #include <commdlg.h>
#endif

/* ============================================================
 *  MINI MIPS 8 BITS — SIMULADOR PIPELINE
 *  UNIPAMPA — Engenharia de Computacao — Projeto Integrador II.
 * ============================================================ */

static void linha(char c, int n)
{

    for (int i = 0; i < n; i++)
    {

        printf("%c", c);

    }

    printf("\n");

}

static void titulo(const char *txt)
{

    printf("\n");

    linha('=', 56);

    printf("  %s\n", txt);

    linha('=', 56);

}

static void secao(const char *txt)
{

    printf("\n  -- %s --\n", txt);

}

static void selecionar_arquivo(char *caminho)
{
#ifdef _WIN32
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.lpstrFile    = caminho;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile     = 260;
    ofn.lpstrFilter  = "Arquivos MEM\0*.mem\0Todos\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn))
        printf("  Arquivo selecionado: %s\n", caminho);
#else
    printf("\n  Digite o caminho do arquivo (.mem): ");
    scanf("%s", caminho);
    strcat(caminho, ".mem");
#endif
}

static void incrementaInstr(instrucoes *c, unsigned char opcode)
{
    switch (opcode)
    {

        case 0x0:

            c->tipoR++;

            break;

        case 0x4:

            c->addi++;

            break;

        case 0xB:

             c->lw++;

             break;

        case 0xF:

             c->sw++;

             break;

        case 0x8:

            c->beq++;

            break;

        case 0x2:

            c->jump++;

            break;


    }

}

static void printEstatisticas(instrucoes c)
{

    int total = c.tipoR + c.addi + c.lw + c.sw + c.beq + c.jump;

    printf("  %-10s %-10s\n", "INSTRUCAO", "QTD");

    printf("  %-10s %-10d\n", "Tipo R",   c.tipoR);

    printf("  %-10s %-10d\n", "Addi",     c.addi);

    printf("  %-10s %-10d\n", "LW",       c.lw);

    printf("  %-10s %-10d\n", "SW",       c.sw);

    printf("  %-10s %-10d\n", "BEQ",      c.beq);

    printf("  %-10s %-10d\n", "Jump",     c.jump);

    printf("  %-10s %-10d\n", "TOTAL",    total);

}

/* ============================================================
 *  Estagio 1 (IF) — Instruction Fetch/busca
 * ============================================================ */
static void estagio_IF(instro *mem, unsigned char *PC, RegIF_ID *if_id)
{

    if_id->RI    = ler_unificada(mem, *PC);

    if_id->PC    = busca(*PC);

    if_id->valid = 1;

    printf("  [IF ] PC=%-3d  RI=0x%04X  -> ", *PC, if_id->RI);

    print_asm(if_id->RI);

    *PC = if_id->PC;

}

/* ============================================================
 *  ESTAGIO ID — Instruction Decode / decoficação
 * ============================================================ */
static void estagio_ID(signed char *reg, RegIF_ID *if_id, RegID_EX *id_ex)
{
    //debug para nao lidar agora com hazards diretametne
    if (!if_id->valid)
    {

        memset(id_ex, 0, sizeof(*id_ex));

        return;
    }

    unsigned short instr = if_id->RI;

    id_ex->opcode = (instr >> 12) & 0xF;

    id_ex->rs     = (instr >> 9)  & 0x7;

    id_ex->rt     = (instr >> 6)  & 0x7;

    id_ex->rd     = (instr >> 3)  & 0x7;

    id_ex->funct  = instr & 0x7;

    id_ex->addr   = instr & 0xFF;

    id_ex->PC     = if_id->PC;

    id_ex->valid  = 1;

    unsigned char imm6 = instr & 0x3F;

    Estender(imm6, &id_ex->immx);

    read(reg, (signed char)id_ex->rs, (signed char)id_ex->rt,&id_ex->A, &id_ex->B);

    printf(" [ID ] op=0x%X rs=R%d(%d) rt=R%d(%d) rd=R%d imm=%d\n",id_ex->opcode,id_ex->rs, id_ex->A,id_ex->rt, id_ex->B,id_ex->rd,id_ex->immx);

}

/* ============================================================
 *  ESTAGIO EX — Execute/exewcução
 * ============================================================ */
static void estagio_EX(RegID_EX *id_ex, RegEX_MEM *ex_mem)
{

    if (!id_ex->valid)
    {

        memset(ex_mem, 0, sizeof(*ex_mem));

        return;

    }

    unsigned char opcode = id_ex->opcode;

    int overflow = 0, zero = 0;

    memset(ex_mem, 0, sizeof(*ex_mem));

    ex_mem->rt    = id_ex->rt;

    ex_mem->rd    = id_ex->rd;

    ex_mem->B     = id_ex->B;

    ex_mem->addr  = id_ex->addr;

    ex_mem->valid = 1;

    int ula_result = 0;

    switch (opcode)
    {
        case 0x0: //Tipo R: add / sub / and / or

            tipo(id_ex->funct);

            ula_result= ula(id_ex->funct, id_ex->A, id_ex->B,&overflow, &zero);

            ex_mem->RegWrite = 1;

            ex_mem->RegDst   = 1;

            break;

        case 0x4: // addi: rt = rs + imm

            ula_result= ula(0, id_ex->A, id_ex->immx,&overflow, &zero);

            ex_mem->RegWrite = 1;

            ex_mem->RegDst   = 0;

            break;

        case 0xB: // lw: endereco = rs + imm

            ula_result       = ula(0, id_ex->A, id_ex->immx,&overflow, &zero);

            ex_mem->MemRead  = 1;

            ex_mem->MemToReg = 1;

            ex_mem->RegWrite = 1;

            ex_mem->RegDst   = 0;

            break;

        case 0xF: // sw: endereco = rs + imm

            ula_result= ula(0, id_ex->A, id_ex->immx,&overflow, &zero);

            ex_mem->MemWrite = 1;

            break;

        case 0x8: // beq

            ula_result= ula(2, id_ex->A, id_ex->B,&overflow, &zero);

            ex_mem->Branch= 1;

            ex_mem->PC_branch = branch(id_ex->PC, (unsigned char)id_ex->immx);

            break;

        case 0x2: // jump

            ex_mem->Jump = 1;

            break;

        default:

            printf("  [EX ] opcode desconhecido: 0x%X\n", opcode);

            break;

    }

    ex_mem->ULA_saida = ula_result;

    ex_mem->zero= zero;


    printf("  [EX ] op=0x%X ULA=%d zero=%d  RW=%d MR=%d MW=%d Branch=%d Jump=%d\n",opcode, ex_mem->ULA_saida, ex_mem->zero,ex_mem->RegWrite, ex_mem->MemRead, ex_mem->MemWrite,ex_mem->Branch, ex_mem->Jump);

}

/* ============================================================
 *  ESTAGIO MEM — Memory Access/ acesso a memoria
 * ============================================================ */
static void estagio_MEM(instro *mem, unsigned char *PC,RegEX_MEM *ex_mem, RegMEM_WB *mem_wb,instrucoes *conta, int *n_instr)
{

    if (!ex_mem->valid)
    {

        memset(mem_wb, 0, sizeof(*mem_wb));

        return;

    }

    mem_wb->rt= ex_mem->rt;

    mem_wb->rd= ex_mem->rd;

    mem_wb->RegWrite  = ex_mem->RegWrite;

    mem_wb->MemToReg  = ex_mem->MemToReg;

    mem_wb->RegDst= ex_mem->RegDst;

    mem_wb->ULA_saida = ex_mem->ULA_saida;

    mem_wb->RDM= 0;

    mem_wb->valid= 1;


    // LW
    if (ex_mem->MemRead)
    {

        mem_wb->RDM = (signed char)ler_unificada(mem,(unsigned char)ex_mem->ULA_saida);

        printf("  [MEM] LW: Mem[%d] = %d\n",(unsigned char)ex_mem->ULA_saida, mem_wb->RDM);

    }

    // SW
    if (ex_mem->MemWrite)
    {

        Store(mem, (signed char)ex_mem->ULA_saida, ex_mem->B);

        printf("  [MEM] SW: Mem[%d] <- %d\n",(unsigned char)ex_mem->ULA_saida, ex_mem->B);

        mem_wb->RegWrite = 0;

        incrementaInstr(conta, 0xF);

        (*n_instr)++;

    }

    // BEQ
    if (ex_mem->Branch)
    {

        if (ex_mem->zero)
        {

            *PC = ex_mem->PC_branch;

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

    // JUMP
    if (ex_mem->Jump)
    {
        *PC = jump(ex_mem->addr);

        printf("  [MEM] JUMP: PC <- %d\n", *PC);

        mem_wb->RegWrite = 0;

        incrementaInstr(conta, 0x2);

        (*n_instr)++;

    }
}

/* ============================================================
 *  ESTAGIO WB — Write Back/escrita
 * ============================================================ */
static void estagio_WB(signed char *reg, RegMEM_WB *mem_wb,instrucoes *conta, int *n_instr)
{

    if (!mem_wb->valid || !mem_wb->RegWrite)
    {
        return;
    }


    int dest = mem_wb->RegDst ? (int)mem_wb->rd : (int)mem_wb->rt;

    signed char valor = mem_wb->MemToReg ? mem_wb->RDM: (signed char)mem_wb->ULA_saida;

    esc(reg, dest, valor, 1);

    printf("  [WB ] R%d <- %d\n", dest, valor);

    if (mem_wb->MemToReg)
    {

        incrementaInstr(conta, 0xB);   // lw
    }

    else if (mem_wb->RegDst)
    {

        incrementaInstr(conta, 0x0);   // tipo R

    }

    else
    {

        incrementaInstr(conta, 0x4);   // addi

    }

    (*n_instr)++;

}

/* ============================================================
 *  executa_ciclo — avanca todos os 5 estagios em paralelo
 * ============================================================ */
static void executa_ciclo(instro *mem, signed char *reg,unsigned char *PC,RegIF_ID  *if_id,  RegID_EX  *id_ex,RegEX_MEM *ex_mem, RegMEM_WB *mem_wb,int *n_ciclo, int *n_instr,instrucoes *conta)
{
    (*n_ciclo)++;

    printf("\n");

    linha('-', 56);

    printf("  CICLO %-3d  PC=%d\n", *n_ciclo, *PC);

    linha('-', 56);


    RegIF_ID  if_id_in  = *if_id;
    RegID_EX  id_ex_in  = *id_ex;
    RegEX_MEM ex_mem_in = *ex_mem;
    RegMEM_WB mem_wb_in = *mem_wb;

    estagio_WB (reg,  &mem_wb_in,          conta, n_instr);
    estagio_MEM(mem, PC, &ex_mem_in, mem_wb, conta, n_instr);
    estagio_EX (&id_ex_in,  ex_mem);
    estagio_ID (reg,  &if_id_in,  id_ex);
    estagio_IF (mem, PC, if_id);

}

/* ============================================================
 *  Impressao do estado do pipeline
 * ============================================================ */
static void print_pipeline_state(unsigned char PC,RegIF_ID *if_id, RegID_EX *id_ex,RegEX_MEM *ex_mem, RegMEM_WB *mem_wb,int n_ciclo, int n_instr)
{

    printf("\n====================================================\n");
    printf("           ESTADO DO PIPELINE\n");
    printf("====================================================\n");
    printf(" Clocks                | %d\n", n_ciclo);
    printf(" Instrucoes concluidas | %d\n", n_instr);
    printf(" PC atual              | %d\n", PC);
    printf("----------------------------------------------------\n");
    printf(" IF/ID  [%s]: RI=0x%04X  PC=%d\n",if_id->valid  ? "OK" : "--", if_id->RI,  if_id->PC);

    printf(" ID/EX  [%s]: op=0x%X rs=R%d rt=R%d rd=R%d imm=%d A=%d B=%d\n",id_ex->valid  ? "OK" : "--",id_ex->opcode, id_ex->rs, id_ex->rt, id_ex->rd,id_ex->immx, id_ex->A, id_ex->B);

    printf(" EX/MEM [%s]: ULA=%d zero=%d RW=%d MR=%d MW=%d Br=%d Jmp=%d\n",ex_mem->valid ? "OK" : "--",ex_mem->ULA_saida, ex_mem->zero,ex_mem->RegWrite, ex_mem->MemRead, ex_mem->MemWrite,ex_mem->Branch, ex_mem->Jump);

    printf(" MEM/WB [%s]: RDM=%d ULA=%d RW=%d MtR=%d RDst=%d\n",mem_wb->valid ? "OK" : "--",mem_wb->RDM, mem_wb->ULA_saida,mem_wb->RegWrite, mem_wb->MemToReg, mem_wb->RegDst);

    printf("====================================================\n");

}

/* ============================================================
 *  MAIN
 * ============================================================ */
int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
    setlocale(LC_ALL, ".UTF-8");

    instro      mem_unificada   = {0};
    signed char reg[8];
    instrucoes  contaInstrucoes = {0};
    iniat(reg);

    RegIF_ID  if_id  = {0};
    RegID_EX  id_ex  = {0};
    RegEX_MEM ex_mem = {0};
    RegMEM_WB mem_wb = {0};

    unsigned char PC      = 0;
    int           n_ciclo = 0;
    int           n_instr = 0;

    Snapshot pilha[500];
    int      sp = -1;

    char caminho[260];
    int  op = 99;

    titulo("SIMULADOR MINI MIPS 8 BITS -- PIPELINE");

    printf("  UNIPAMPA -- Engenharia de Computacao\n");

    printf("  Projeto Integrador II\n");

    do {

        titulo("MENU PRINCIPAL");

        printf("  1  -- Carregar programa (.mem)\n");

        printf("  3  -- Visualizar memoria\n");

        printf("  4  -- Visualizar banco de registradores\n");

        printf("  5  -- Imprimir estado do pipeline\n");

        printf("  6  -- Salvar programa em assembly (.asm)\n");

        linha('-', 56);

        printf("  8  -- Executar programa completo (run)\n");

        printf("  9  -- Executar um ciclo de clock (step)\n");

        printf("  10 -- Desfazer ultimo ciclo (back)\n");

        linha('-', 56);

        printf("  0  -- Sair\n");

        printf("\n  Sua escolha: ");

        scanf("%d", &op);

        switch (op)
        {

            case 1:

                caminho[0] = '\0';

                selecionar_arquivo(caminho);

                if (carregar_unificado(&mem_unificada, caminho) != 0)
                {

                    printf("\n  ! Erro ao carregar o arquivo.\n");

                }

                else
                {

                    PC= 0;

                    n_ciclo= 0;

                    n_instr= 0;

                    sp= -1;


                    contaInstrucoes = (instrucoes){0};

                    memset(&if_id,  0, sizeof(if_id));

                    memset(&id_ex,  0, sizeof(id_ex));

                    memset(&ex_mem, 0, sizeof(ex_mem));

                    memset(&mem_wb, 0, sizeof(mem_wb));

                    iniat(reg);

                    printf("\n  Programa carregado. PC=0. Pronto para executar.\n");

                }

                break;


            case 3:

                titulo("MEMORIA DO SIMULADOR");

                print_mem_unificada(&mem_unificada);

                break;


            case 4:

                titulo("BANCO DE REGISTRADORES");

                print_regs(reg);

                break;


            case 5:

                print_pipeline_state(PC, &if_id, &id_ex, &ex_mem, &mem_wb,n_ciclo, n_instr);

                printEstatisticas(contaInstrucoes);

                print_regs(reg);

                print_mem_unificada(&mem_unificada);

                break;


            case 6:

                titulo("SALVAR ASSEMBLY");

                print_program(&mem_unificada, 128);

                save_program_asm(&mem_unificada, 128, "programa.asm");

                printf("\n  Arquivo salvo: programa.asm\n");

                break;


            case 8:

                titulo("EXECUTANDO PROGRAMA COMPLETO");

                PC= 0;

                n_ciclo= 0;

                n_instr= 0;

                sp= -1;

                contaInstrucoes = (instrucoes){0};

                memset(&if_id,  0, sizeof(if_id));

                memset(&id_ex,  0, sizeof(id_ex));

                memset(&ex_mem, 0, sizeof(ex_mem));

                memset(&mem_wb, 0, sizeof(mem_wb));

                iniat(reg);


                    int restante = 0;

                    while (PC < (unsigned char)mem_unificada.n || restante < 4)
                    {

                        if (PC >= (unsigned char)mem_unificada.n)
                        {

                            restante++;

                        }

                        push_pipeline(pilha, &sp, reg, PC, n_ciclo, n_instr,&if_id, &id_ex, &ex_mem, &mem_wb,&mem_unificada, contaInstrucoes);

                        executa_ciclo(&mem_unificada, reg, &PC,&if_id, &id_ex, &ex_mem, &mem_wb,&n_ciclo, &n_instr, &contaInstrucoes);

                        printf("  Regs: ");

                        for (int i = 0; i < 8; i++)
                        {

                            printf("R%d=%d  ", i, reg[i]);

                        }


                        printf("\n");

                    }



                titulo("FIM DO PROGRAMA");

                printf("  Total de ciclos de clock : %d\n", n_ciclo);

                printf("  Total de instrucoes: %d\n", n_instr);

                printf("  PC final: %d\n", PC);

                printEstatisticas(contaInstrucoes);

                break;

            case 9:

                titulo("STEP -- EXECUTAR UM CICLO");

                push_pipeline(pilha, &sp, reg, PC, n_ciclo, n_instr,&if_id, &id_ex, &ex_mem, &mem_wb,&mem_unificada, contaInstrucoes);

                executa_ciclo(&mem_unificada, reg, &PC,&if_id, &id_ex, &ex_mem, &mem_wb,&n_ciclo, &n_instr, &contaInstrucoes);

                secao("Registradores apos o ciclo");

                for (int i = 0; i < 8; i++)
                {

                    printf("  R%d = %d\n", i, reg[i]);

                }


                printf("\n  PC = %d  |  Ciclo = %d\n", PC, n_ciclo);

                break;


            case 10:

                titulo("BACK -- DESFAZER ULTIMO CICLO");

                pop_pipeline(pilha, &sp, reg, &PC, &n_ciclo, &n_instr,&if_id, &id_ex, &ex_mem, &mem_wb,&mem_unificada, &contaInstrucoes);

                printf("  Estado restaurado. PC=%d | Ciclo=%d\n", PC, n_ciclo);

                break;


            case 0:

                titulo("ENCERRANDO SIMULADOR");

                printf("  Ate logo!\n\n");

                break;

            default:

                printf("\n  ! Opcao invalida.\n");

                break;
        }

    } while (op != 0);

    return 0;
}
