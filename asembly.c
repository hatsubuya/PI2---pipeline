#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "head.h"

extern WINDOW *saida_pad;
#define saida_pad saida_pad

/* decodifica uma instrução para string assembly */
static void decode_asm(unsigned short instr, char *buf, int bufsz)
{
    unsigned char opcode = (instr >> 12) & 0xF;
    unsigned char rs     = (instr >>  9) & 0x7;
    unsigned char rt     = (instr >>  6) & 0x7;
    unsigned char rd     = (instr >>  3) & 0x7;
    unsigned char imm    = instr & 0x3F;
    unsigned char funct  = instr & 0x7;
    unsigned char addr   = instr & 0xFF;
    signed char   immx   = (imm & 0x20) ? (imm | 0xC0) : imm;

    if (opcode == 0x0)
    {
        switch (funct)
        {
            case 0: snprintf(buf, bufsz, "add $r%d, $r%d, $r%d", rd, rs, rt); break;
            case 2: snprintf(buf, bufsz, "sub $r%d, $r%d, $r%d", rd, rs, rt); break;
            case 4: snprintf(buf, bufsz, "and $r%d, $r%d, $r%d", rd, rs, rt); break;
            case 5: snprintf(buf, bufsz, "or  $r%d, $r%d, $r%d", rd, rs, rt); break;
            default:snprintf(buf, bufsz, "tipo_r%d $r%d,$r%d,$r%d", funct, rd, rs, rt); break;
        }
    }
    else if (opcode == 0x4) snprintf(buf, bufsz, "addi $r%d, $r%d, %d",  rt, rs, immx);
    else if (opcode == 0xB) snprintf(buf, bufsz, "lw   $r%d, %d($r%d)",  rt, immx, rs);
    else if (opcode == 0xF) snprintf(buf, bufsz, "sw   $r%d, %d($r%d)",  rt, immx, rs);
    else if (opcode == 0x8) snprintf(buf, bufsz, "beq  $r%d, $r%d, %d",  rt, rs, immx + 1);
    else if (opcode == 0x2) snprintf(buf, bufsz, "j    %d",               addr);
    else                    snprintf(buf, bufsz, "op_0x%X $r%d,$r%d,%d", opcode, rs, rt, immx);
}

void print_asm(unsigned short instr)
{
    char buf[64];
    decode_asm(instr, buf, sizeof(buf));

    if (saida_pad)
        wprintw(saida_pad, "%s", buf);
    else
        printf("%s", buf);
}

void print_program(instro *l, int tamanho)
{
    if (saida_pad)
    {
        wprintw(saida_pad, "\n--- PROGRAMA EM ASSEMBLY ---\n");
        for (int i = 0; i < tamanho; i++)
        {
            char buf[64];
            decode_asm(l->instc[i], buf, sizeof(buf));
            wprintw(saida_pad, "%3d: %s\n", i, buf);
        }
        wprintw(saida_pad, "----------------------------\n");
        wrefresh(saida_pad);
    }
    else
    {
        printf("\n--- PROGRAMA EM ASSEMBLY ---\n");
        for (int i = 0; i < tamanho; i++)
        {
            char buf[64];
            decode_asm(l->instc[i], buf, sizeof(buf));
            printf("%3d: %s\n", i, buf);
        }
        printf("----------------------------\n");
    }
}

void save_program_asm(instro *l, int tamanho, const char *nome_arquivo)
{
    FILE *f = fopen(nome_arquivo, "w");
    if (!f)
    {
        if (saida_pad) wprintw(saida_pad, "\nErro ao criar arquivo ASM\n");
        else           printf("\nErro ao criar arquivo ASM\n");
        return;
    }
    for (int i = 0; i < tamanho; i++)
    {
        char buf[64];
        decode_asm(l->instc[i], buf, sizeof(buf));
        fprintf(f, "%s\n", buf);
    }
    fclose(f);
}
