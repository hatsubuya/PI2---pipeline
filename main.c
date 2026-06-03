#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Head.h"
#include <locale.h>

#ifdef _WIN32
    #include <windows.h>
    #include <commdlg.h>
#endif

static void linha(char c, int n)
{
    for (int i = 0; i < n; i++) printf("%c", c);
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

void incrementaInstr(instrucoes *contaInstrucoes, unsigned char opcode, unsigned char funct){
    switch (opcode)
    {
        case 0x0:                    // add, sub, and, or * 4
            contaInstrucoes->tipoR++;
            break;

        case 0x4:   // addi * 4
            contaInstrucoes->addi++;
            break;
        case 0xB:    // lw * 5
            contaInstrucoes->lw++;
            break;
        case 0xF:                    // sw * 4
            contaInstrucoes->sw++;
            break;

        case 0x8:                    // beq * 3
            contaInstrucoes->beq++;
            break;

        case 0x2:                    // jump * 3
            contaInstrucoes->jump++;
            break;

    }

}

void printEstatisticas(instrucoes contaInstrucoes){
    printf("%-15s %-15s %-20s %-10s\n",
                "INSTRUCAO",
                "QUANTIDADE",
                "QTD CICLOS",
                "TOTAL");

                printf("%-15s %-15d %-20d %-10d\n",
                "tipoR",
                contaInstrucoes.tipoR,
                4,
                contaInstrucoes.tipoR*4);

                printf("%-15s %-15d %-20d %-10d\n",
                "Addi",
                contaInstrucoes.addi,
                4,
                contaInstrucoes.addi*4);

                printf("%-15s %-15d %-20d %-10d\n",
                "LW",
                contaInstrucoes.lw,
                5,
                contaInstrucoes.lw*5);

                printf("%-15s %-15d %-20d %-10d\n",
                "SW",
                contaInstrucoes.sw,
                4,
                contaInstrucoes.sw*4);

                printf("%-15s %-15d %-20d %-10d\n",
                "BEQ",
                contaInstrucoes.beq,
                3,
                contaInstrucoes.beq*3);

                printf("%-15s %-15d %-20d %-10d\n",
                "jump",
                contaInstrucoes.jump,
                3,
                contaInstrucoes.jump*3);

                int somaTotal =
                contaInstrucoes.tipoR * 4 +
                contaInstrucoes.addi * 4 +
                contaInstrucoes.lw * 5 +
                contaInstrucoes.sw * 4 +
                contaInstrucoes.beq * 3 +
                contaInstrucoes.jump * 3;

            printf("\n%-15s %-15s %-20s %-10d\n",
                "TOTAL GERAL",
                "",
                "",
                somaTotal);

}

//seleçao do arquivo
void selecionar_arquivo(char *caminho)
{
#ifdef _WIN32
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile   = caminho;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile    = 260;
    ofn.lpstrFilter = "Arquivos MEM\0*.mem\0Todos\0*.*\0";
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

//IMpressao do estado final da FSM
static void print_estado(int estado)
{
    const char *nomes[] = {
        "BUSCA DA INSTRUCAO (fetch)",
        "DECODIFICACAO + LEITURA DOS REGISTRADORES",
        "CALCULO DO ENDERECO EFETIVO (lw / sw / addi)",
        "ACESSO A MEMORIA: leitura (lw)",
        "ESCRITA NO REGISTRADOR de destino (lw)",
        "ACESSO A MEMORIA: escrita (sw)",
        "ESCRITA NO REGISTRADOR de destino (addi)",
        "EXECUCAO DA ULA (tipo R)",
        "ESCRITA NO REGISTRADOR de destino (tipo R)",
        "COMPARACAO E DESVIO CONDICIONAL (beq)",
        "DESVIO INCONDICIONAL (jump)"
    };

    printf("\n");
    linha('-', 56);

    if (estado >= 0 && estado <= 10)
        printf("  CICLO  |  Estado %-2d : %s\n", estado, nomes[estado]);
    else
        printf("  CICLO  |  Estado invalido: %d\n", estado);

    linha('-', 56);
}


int executa_ciclo(instro *mem, signed char reg[8], int Sinais[16],
                  int *estado, unsigned char *PC, unsigned short *RI,
                  signed char *RDM, signed char *regA, signed char *regB,
                  signed char *ULASaida, int *ULAop, int *overflow,
                  int *zero, int *n_ciclo,instrucoes *contaInstrucoes)
{
    (*n_ciclo)++;

    unsigned char opcode = (*RI >> 12) & 0xF;
    unsigned char rs     = (*RI >> 9)  & 0x7;
    unsigned char rt     = (*RI >> 6)  & 0x7;
    unsigned char rd     = (*RI >> 3)  & 0x7;
    unsigned char imm6   = *RI & 0x3F;
    unsigned char funct  = *RI & 0x7;
    unsigned char addr   = *RI & 0xFF;
    signed char   immx   = 0;

    Estender(imm6, &immx);
    print_estado(*estado);
    Decodifica_estado(*estado, Sinais);

    int instrucao_concluida = 0;

    switch (*estado)
    {
        case 0:
            *RI = ler_unificada(mem, *PC);
            printf("  > Buscando instrucao na posicao %d da memoria\n", *PC);
            printf("    RI  <- Mem[%d] = ", *PC);
            print_bin(*RI);
            printf(" (0x%04X)\n", *RI);
            *PC = busca(*PC);
            printf("    PC  <- %d  (incrementado para proxima instrucao)\n", *PC);
            *estado = 1;
            break;

        case 1:
            opcode = (*RI >> 12) & 0xF;
            rs     = (*RI >> 9)  & 0x7;
            rt     = (*RI >> 6)  & 0x7;
            rd     = (*RI >> 3)  & 0x7;
            imm6   = *RI & 0x3F;
            funct  = *RI & 0x7;
            addr   = *RI & 0xFF;
            Estender(imm6, &immx);

            printf("  > Decodificando instrucao (opcode = 0x%X)\n", opcode);
            read(reg, rs, rt, regA, regB);
            printf("    A   <- R%d = %d\n", rs, *regA);
            printf("    B   <- R%d = %d\n", rt, *regB);
            *ULASaida = (signed char)(*PC) + immx;
            printf("    ULA_saida <- PC(%d) + imm(%d) = %d  [endereco de branch pre-calculado]\n",
                   *PC, immx, *ULASaida);
            *estado = proximo_estado(1, opcode);
            printf("  > Proximo estado: %d\n", *estado);
            break;

        case 2:
            *ULASaida = (signed char)ula(0, *regA, immx, overflow, zero);
            printf("  > Calculando endereco efetivo\n");
            printf("    ULA_saida <- R%d(%d) + imm(%d) = %d\n",
                   rs, *regA, immx, *ULASaida);
            *estado = proximo_estado(2, opcode);
            printf("  > Proximo estado: %d\n", *estado);
            break;

        case 3:
            *RDM = (signed char)ler_unificada(mem, (unsigned char)*ULASaida);
            printf("  > Lendo dado da memoria\n");
            printf("    RDM <- Mem[%d] = %d\n", (unsigned char)*ULASaida , *RDM);
            *estado = 4;
            break;

        case 4:
            esc(reg, rt, *RDM, 1);
            printf("  > Escrevendo dado no registrador\n");
            printf("    R%d  <- RDM = %d  [lw concluido]\n", rt, *RDM);
            *estado = 0;
            instrucao_concluida = 1;
            incrementaInstr(contaInstrucoes, opcode, funct);
            break;

        case 5:
            Store(mem, *ULASaida, *regB);
            printf("  > Escrevendo dado na memoria\n");
            printf("    Mem[%d] <- R%d = %d  [sw concluido]\n",
                   (unsigned char)*ULASaida , rt, *regB);
            *estado = 0;
            instrucao_concluida = 1;
            incrementaInstr(contaInstrucoes, opcode, funct);
            break;

        case 6:
            esc(reg, rt, *ULASaida, 1);
            printf("  > Escrevendo resultado no registrador\n");
            printf("    R%d  <- ULA_saida = %d  [addi concluido]\n", rt, *ULASaida);
            *estado = 0;
            instrucao_concluida = 1;
            incrementaInstr(contaInstrucoes, opcode, funct);
            break;

        case 7:
            *ULAop = funct;
            tipo(funct);
            *ULASaida = (signed char)ula(*ULAop, *regA, *regB, overflow, zero);
            printf("  > Executando operacao da ULA\n");
            printf("    ULA_saida <- R%d(%d) op R%d(%d) = %d  (funct=%d)\n",
                   rs, *regA, rt, *regB, *ULASaida, funct);
            *estado = 8;
            break;

        case 8:
            esc(reg, rd, *ULASaida, 1);
            printf("  > Escrevendo resultado no registrador\n");
            printf("    R%d  <- ULA_saida = %d  [tipo R concluido]\n", rd, *ULASaida);
            if (*overflow)
                printf("  ! ATENCAO: overflow detectado!\n");
            *estado = 0;
            instrucao_concluida = 1;
            incrementaInstr(contaInstrucoes, opcode, funct);
            break;

        case 9:
            ula(2, *regA, *regB, overflow, zero);
            printf("  > Verificando condicao do branch (R%d == R%d ?)\n", rs, rt);
            if (*zero)
            {
                *PC = (unsigned char)*ULASaida;
                printf("    Condicao VERDADEIRA -> branch tomado\n");
                printf("    PC  <- %d\n", *PC);
            }
            else
            {
                printf("    Condicao FALSA -> branch NAO tomado, PC permanece em %d\n", *PC);
            }
            *estado = 0;
            instrucao_concluida = 1;
            incrementaInstr(contaInstrucoes, opcode, funct);
            break;

        case 10:
            *PC = jump(addr);
            printf("  > Desvio incondicional\n");
            printf("    PC  <- %d  [jump concluido]\n", *PC);
            *estado = 0;
            instrucao_concluida = 1;
            incrementaInstr(contaInstrucoes, opcode, funct);
            break;

        default:
            printf("  ! Estado invalido %d — resetando para 0\n", *estado);
            *estado = 0;
            break;
    }

    return instrucao_concluida;
}


int main(void)
{

    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
    #endif
    setlocale(LC_ALL, ".UTF-8");

    instro        mem_unificada = {0};
    signed char   reg[8];
    instrucoes contaInstrucoes;
    iniat(reg);
    int           Sinais[16]  = {0};
    //unsigned short RI         = 0;
    //signed char    RDM        = 0;
    //signed char    regA       = 0;
    //signed char    regB       = 0;
    //signed char    ULASaida   = 0;

    //registradores de pipeline
    RegIF_ID  if_id  = {0};  
    RegID_EX  id_ex  = {0};  
    RegEX_MEM ex_mem = {0};  
    RegMEM_WB mem_wb = {0}; 
    int            estado     = 0;
    int            ULAop      = 0;
    int            overflow   = 0;
    int            zero       = 0;
    int            n_ciclo    = 0;
    int            n_instr    = 0;
    unsigned char  PC         = 0;
    Snapshot       pilha[500];
    int            sp         = -1;
    char           caminho[260];
    int            op         = 99;
    

    titulo("SIMULADOR MINI MIPS 8 BITS — MULTICICLO");
    printf("  UNIPAMPA — Engenharia de Computacao\n");
    printf("  Projeto Integrador II\n");

    do {
        titulo("MENU PRINCIPAL");
        printf("  1  — Carregar programa (.mem)\n");
        printf("  3  — Visualizar memoria (instrucoes e dados)\n");
        printf("  4  — Visualizar banco de registradores\n");
        printf("  5  — Imprimir simulador\n");
        printf("  6  — Salvar programa em assembly (.asm)\n");
        linha('-', 56);
        printf("  8  — Executar programa completo (run)\n");
        printf("  9  — Executar um ciclo de clock (step)\n");
        printf("  10 — Desfazer ultimo ciclo (back)\n");
        linha('-', 56);
        printf("  0  — Sair\n");
        printf("\n  Sua escolha: ");
        scanf("%d", &op);

        switch (op)
        {

            case 1:
                caminho[0] = '\0';
                selecionar_arquivo(caminho);
                if (carregar_unificado(&mem_unificada, caminho) != 0)
                {
                    printf("\n  ! Erro ao carregar o arquivo. Verifique o caminho.\n");
                }
                else
                {
                    PC      = 0;
                    estado  = 0;
                    n_ciclo = 0;
                    n_instr = 0;
                    sp      = -1;
                    iniat(reg);
                    printf("\n  Programa carregado com sucesso!\n");
                    printf("  Registradores zerados. PC = 0. Pronto para executar.\n");
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

                printf("\n");
                printf("====================================================\n");
                printf("              ESTADO DO SIMULADOR                   \n");
                printf("====================================================\n");

                printf(" PC                    │ %3d │ ", PC);
                print_bin8(PC);
                printf("\n");
                printf(" Estado FSM            │ %3d\n", estado);
                printf(" Clocks                │ %3d\n", n_ciclo);
                printf(" Instrucoes concluidas │ %3d\n", n_instr);

                printf("----------------------------------------------------\n");

                printf(" RI        │ ");
                print_bin(RI);
                printf(" │ 0x%04X\n", RI);
                printf(" RDM       │ %4d\n", RDM);
                printf(" RegA      │ %4d\n", regA);
                printf(" RegB      │ %4d\n", regB);
                printf(" ULASaida  │ %4d\n", ULASaida);

                printf("====================================================\n");

                printf("----------------------------------------------------\n");

                printEstatisticas(contaInstrucoes);

                printf("====================================================\n");

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
                PC      = 0;
                estado  = 0;
                n_ciclo = 0;
                n_instr = 0;
                sp      = -1;
                iniat(reg);
                printf("  PC = 0 | Registradores zerados\n");
                printf("  Iniciando execucao...\n");

                while (PC < 128)
                {
                    push_multi(pilha, &sp, reg, Sinais, ULAop, overflow,
                               &if_id, &id_ex, &ex_mem, &mem_wb, PC, estado, n_ciclo, n_instr, &mem_unificada, contaInstrucoes); 

                    int concluiu = executa_ciclo(&mem_unificada, reg, Sinais,
                                                 &estado, &PC, &RI, &RDM,
                                                 &regA, &regB, &ULASaida,
                                                 &ULAop, &overflow, &zero,
                                                 &n_ciclo,&contaInstrucoes);
                    if (concluiu)
                    {
                        n_instr++;
                        printf("\n");
                        linha('*', 56);
                        printf("  INSTRUCAO %d CONCLUIDA  |  ciclo total: %d\n",
                               n_instr, n_ciclo);
                        printf("  Registradores: ");
                        for (int i = 0; i < 8; i++)
                            printf("R%d=%d  ", i, reg[i]);
                        printf("\n");
                        linha('*', 56);
                    }

                    /*if (n_ciclo > 10000)
                    {
                        printf("\n  ! Limite de 10000 ciclos atingido. Execucao interrompida.\n");
                        break;
                    }

                    */
                }

                titulo("FIM DO PROGRAMA");
                printf("  Total de ciclos de clock : %d\n", n_ciclo);
                printf("  Total de instrucoes      : %d\n", n_instr);
                printf("  PC final                 : %d\n", PC);
                break;

            case 9:
                titulo("STEP — EXECUTAR UM CICLO");

                printf("  Ciclo: %d  |  PC: %d  |  Estado FSM: %d\n",n_ciclo + 1, PC, estado);

                push_multi(pilha, &sp, reg, Sinais, ULAop, overflow,
                            &if_id, &id_ex, &ex_mem, &mem_wb,
                            PC, estado, n_ciclo, n_instr, &mem_unificada, contaInstrucoes);

                {
                    int concluiu = executa_ciclo(&mem_unificada, reg, Sinais,
                                                 &estado, &PC, &RI, &RDM,
                                                 &regA, &regB, &ULASaida,
                                                 &ULAop, &overflow, &zero,
                                                 &n_ciclo,&contaInstrucoes);
                    if (concluiu)
                    {
                        n_instr++;
                        printf("\n");
                        linha('*', 56);
                        printf("  INSTRUCAO %d CONCLUIDA NESTE CICLO\n", n_instr);
                        linha('*', 56);
                    }
                    else
                    {
                        printf("\n  Instrucao ainda em execucao.\n");
                        printf("  Proximo estado: %d\n", estado);
                    }
                }

                secao("Registradores apos o ciclo");
                   printf(" RI        │ ");
                print_bin(RI);
                printf(" │ 0x%04X\n", RI);
                printf(" RDM       │ %4d\n", RDM);
                printf(" RegA      │ %4d\n", regA);
                printf(" RegB      │ %4d\n", regB);
                printf(" ULASaida  │ %4d\n", ULASaida);

                for (int i = 0; i < 8; i++)
                    printf("  R%d = %d\n", i, reg[i]);
                printf("\n  PC = %d  |  Estado FSM = %d\n", PC, estado);
                break;

            /* ------------------------------------------------------ */
            case 10:
                titulo("BACK — DESFAZER ULTIMO CICLO");
                pop_multi(pilha, &sp, reg, Sinais, &ULAop, &overflow,
                          &if_id, &id_ex, &ex_mem, &mem_wb,
                          &PC, &estado, &n_ciclo, &n_instr, &mem_unificada,&contaInstrucoes);
                printf("  Estado restaurado com sucesso.\n");
                printf("  PC = %d  |  Estado FSM = %d  |  Ciclo = %d\n",
                       PC, estado, n_ciclo);
                break;

            /* ------------------------------------------------------ */
            case 0:
                titulo("ENCERRANDO SIMULADOR");
                printf("  Ate logo!\n\n");
                break;

            /* ------------------------------------------------------ */
            default:
                printf("\n  ! Opcao invalida. Escolha um numero do menu.\n");
                break;
        }

    } while (op != 0);

    return 0;
}
