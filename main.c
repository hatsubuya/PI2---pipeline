#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "head.h"

#include <stdarg.h>


WINDOW *menu_win  = NULL;
WINDOW *saida_win = NULL;
WINDOW *saida_pad = NULL;

#define janela_saida saida_pad

#define PAD_ALTURA 2000

int pad_linha  = 0;

int pad_scroll = 0;

int saida_y0, saida_x0, saida_h, saida_w_sz;


void escrever_no_pad(const char *format, ...) {
    if (saida_pad) {
        va_list args;
        va_start(args, format);

        vw_printw(saida_pad, format, args);
        wprintw(saida_pad, "\n");
        pad_linha++;
        va_end(args);
    }
}


static void pad_reset(void)
{
    werase(saida_pad);
    pad_linha  = 0;
    pad_scroll = 0;
}


static void pad_show(void)
{

    prefresh(saida_pad,
             pad_scroll, 0,
             saida_y0, saida_x0,
             saida_y0 + saida_h - 1,
             saida_x0 + saida_w_sz - 1);
}


static void aguardar_scroll(void)
{
    int max_scroll = pad_linha - saida_h;

    if (max_scroll < 0) max_scroll = 0;


    mvprintw(saida_y0 + saida_h, saida_x0,
             "[ UP/DOWN: scroll | PgUp/PgDn | q/Enter: voltar ]");
    refresh();

    pad_show();

    keypad(stdscr, TRUE);

    int ch;

    while (1)
    {
        ch = getch();
        if (ch == 'q' || ch == '\n' || ch == ' ') break;

        if (ch == KEY_UP)        pad_scroll -= 1;
        else if (ch == KEY_DOWN) pad_scroll += 1;
        else if (ch == KEY_PPAGE) pad_scroll -= saida_h / 2;
        else if (ch == KEY_NPAGE) pad_scroll += saida_h / 2;
        else if (ch == KEY_HOME)  pad_scroll = 0;
        else if (ch == KEY_END)   pad_scroll = max_scroll;

        if (pad_scroll < 0)          pad_scroll = 0;
        if (pad_scroll > max_scroll) pad_scroll = max_scroll;

        pad_show();
    }


    move(saida_y0 + saida_h, saida_x0);
    clrtoeol();
    refresh();
}


void selecionar_arquivo(char *caminho)
{
    FILE *f = popen("zenity --file-selection --file-filter='*.mem' 2>/dev/null", "r");
    if (f && fgets(caminho, 100, f))
    {
        caminho[strcspn(caminho, "\n")] = 0;
        pclose(f);
    }
    else
    {
        if (f) pclose(f);
        echo();
        mvwprintw(saida_pad, pad_linha++, 0, "Digite o caminho (.mem): ");
        pad_show();
        char tmp[100];
        getnstr(tmp, 99);
        noecho();
        strcpy(caminho, tmp);
        strcat(caminho, ".mem");
    }
}

void selecionar_arquivo2(char *caminho)
{
    FILE *f = popen("zenity --file-selection --file-filter='*.dat' 2>/dev/null", "r");
    if (f && fgets(caminho, 260, f))
    {
        caminho[strcspn(caminho, "\n")] = 0;
        pclose(f);
    }
    else
    {
        if (f) pclose(f);
        echo();
        mvwprintw(saida_pad, pad_linha++, 0, "Digite o caminho (.dat): ");
        pad_show();
        char tmp[260];
        getnstr(tmp, 259);
        noecho();
        strcpy(caminho, tmp);
        strcat(caminho, ".dat");
    }
}



typedef struct {
    signed char    reg[8];

    unsigned short memoria[256];

    unsigned char  PC;
    int            ciclo;

    RegIF_ID       if_id;
    RegID_EX       id_ex;
    RegEX_MEM      ex_mem;
    RegMEM_WB      mem_wb;
    int f1, f2, f3, f4;
    int total_instrucoes_completadas;
    int total_forwarding_evitados;
    int total_stalls;
    int total_flushes;
    int total_instrucoes_descartadas;
    int qtd_aritmetica;
    int qtd_memoria;
    int qtd_desvio;
} Snapshot;


static int hazard_detection_unit(RegID_EX *id_ex, unsigned short proxima_instrucao)
{
    unsigned char rs_prox = (proxima_instrucao >> 9) & 0x7;
    unsigned char rt_prox = (proxima_instrucao >> 6) & 0x7;
    if (id_ex->valid && id_ex->MemParaReg &&
        (id_ex->rt == rs_prox || id_ex->rt == rt_prox))
        return 1;
    return 0;
}

static void forwarding_unit(unsigned char rs, unsigned char rt,RegEX_MEM *ex_mem, RegMEM_WB *mem_wb,int *fwdA, int *fwdB)
{
    *fwdA = 0; *fwdB = 0;

    if (ex_mem->valid && ex_mem->EscReg && ex_mem->dest != 0 && ex_mem->dest == rs) *fwdA = 1;

    if (ex_mem->valid && ex_mem->EscReg && ex_mem->dest != 0 && ex_mem->dest == rt) *fwdB = 1;

    if (mem_wb->valid && mem_wb->RegWrite && mem_wb->dest != 0 && mem_wb->dest == rs && *fwdA == 0) *fwdA = 2;

    if (mem_wb->valid && mem_wb->RegWrite && mem_wb->dest != 0 && mem_wb->dest == rt && *fwdB == 0) *fwdB = 2;
}

//fix pad
#define PLOG(...) do { \
    mvwprintw(saida_pad, pad_linha, 0, __VA_ARGS__); \
    pad_linha++; \
} while(0)


static void estagio_IF(instro *mem, unsigned char *PC, RegIF_ID *if_id)
{
    if_id->PC   = busca(*PC);
    if_id->RI   = ler(mem, *PC);
    if_id->valid = 1;
    *PC = if_id->PC;

    wmove(saida_pad, pad_linha, 0);
    wprintw(saida_pad, "[IF ] | PC=");
    print_bin(*PC);
    wprintw(saida_pad, "(%3d) | bin=", *PC);
    print_bin(if_id->RI);
    wprintw(saida_pad, " | asm: ");
    print_asm(if_id->RI);
    pad_linha++;
}

static void estagio_ID(signed char *reg, RegIF_ID *if_id, RegID_EX *id_ex)
{
    unsigned short instr  = if_id->RI;
    unsigned char  opcode = (instr >> 12) & 0xF;
    unsigned char  rs     = (instr >>  9) & 0x7;
    unsigned char  rt     = (instr >>  6) & 0x7;
    unsigned char  rd     = (instr >>  3) & 0x7;
    unsigned char  imm6   = instr & 0x3F;
    unsigned char  funct  = instr & 0x7;

    Decodifica_controle(opcode, &id_ex->RegDst, &id_ex->ULAOp, &id_ex->ULAFonte,
                        &id_ex->beq, &id_ex->jump, &id_ex->EscMem,
                        &id_ex->EscReg, &id_ex->MemParaReg, funct);
    id_ex->PC = if_id->PC;
    id_ex->rt = rt; id_ex->rd = rd; id_ex->rs = rs;
    ler_regs(reg, (signed char)rs, (signed char)rt, &id_ex->A, &id_ex->B);
    Estender(imm6, &id_ex->immx);
    id_ex->valid = 1;

    if (opcode == 0)

        PLOG("[ID ] Type R | opcode=%-2u | funct=%-2u | rs=R%d(%d) rt=R%d(%d) rd=R%d",
             opcode, funct, rs, id_ex->A, rt, id_ex->B, rd);
    else
        PLOG("[ID ] opcode=%-2u | rs=R%d(%d) rt=R%d(%d) | imm=%d",

             opcode, rs, id_ex->A, rt, id_ex->B, id_ex->immx);

    wmove(saida_pad, pad_linha, 0);

    wprintw(saida_pad, "      Sinais: ");

    print_sinais(id_ex->RegDst, id_ex->ULAOp, id_ex->ULAFonte,
                 id_ex->beq, id_ex->jump, id_ex->EscMem,
                 id_ex->EscReg, id_ex->MemParaReg);

    pad_linha++;

}

static void estagio_EX(RegID_EX *id_ex, RegEX_MEM *ex_mem,
                       RegEX_MEM *ex_mem_old, RegMEM_WB *mem_wb_old,
                       int *cont_fwd)
{
    int fwdA, fwdB;
    forwarding_unit(id_ex->rs, id_ex->rt, ex_mem_old, mem_wb_old, &fwdA, &fwdB);
    signed char valorA = id_ex->A, valorB = id_ex->B;

    if (fwdA == 1) { valorA = ex_mem_old->ULA_saida; PLOG("[FWD] A <- EX/MEM (R%d)", id_ex->rs); (*cont_fwd)++; }
    else if (fwdA == 2) { valorA = mem_wb_old->MemToReg ? mem_wb_old->RDM : mem_wb_old->ULA_saida; PLOG("[FWD] A <- MEM/WB (R%d)", id_ex->rs); (*cont_fwd)++; }
    if (fwdB == 1) { valorB = ex_mem_old->ULA_saida; PLOG("[FWD] B <- EX/MEM (R%d)", id_ex->rt); (*cont_fwd)++; }
    else if (fwdB == 2) { valorB = mem_wb_old->MemToReg ? mem_wb_old->RDM : mem_wb_old->ULA_saida; PLOG("[FWD] B <- MEM/WB (R%d)", id_ex->rt); (*cont_fwd)++; }

    int overflow = 0, zero = 0;
    int operandoB = ulamx(id_ex->ULAFonte, valorA, valorB, id_ex->immx);
    ex_mem->ULA_saida  = ula(id_ex->ULAOp, valorA, operandoB, &overflow, &zero);
    ex_mem->dest       = Rdest(id_ex->RegDst, id_ex->rd, id_ex->rt);
    if (id_ex->beq)  ex_mem->add_result = branch(id_ex->PC, (unsigned char)id_ex->immx);
    if (id_ex->jump) ex_mem->add_result = jump((unsigned char)id_ex->immx);
    ex_mem->B = valorB; ex_mem->EscReg = id_ex->EscReg;
    ex_mem->MemParaReg = id_ex->MemParaReg; ex_mem->EscMem = id_ex->EscMem;
    ex_mem->beq = id_ex->beq; ex_mem->jump = id_ex->jump;
    ex_mem->valid = 1; ex_mem->zero = zero;

    wmove(saida_pad, pad_linha, 0);
    wprintw(saida_pad, "      Sinais EX: ");
    print_sinais(id_ex->RegDst, id_ex->ULAOp, id_ex->ULAFonte,
                 id_ex->beq, id_ex->jump, id_ex->EscMem,
                 id_ex->EscReg, id_ex->MemParaReg);
    pad_linha++;
    PLOG("[EX ] A=%d  ULAFonte_B=%d  => ULA_saida=%d | zero=%d | dest=R%d",
         valorA, operandoB, ex_mem->ULA_saida, ex_mem->zero, ex_mem->dest);
}

static void estagio_MEM(instro *mem, unsigned char *PC,
                        RegEX_MEM *ex_mem, RegMEM_WB *mem_wb, int *flush)
{
    *flush = 0;
    mem_wb->dest = ex_mem->dest; mem_wb->RegWrite = ex_mem->EscReg;
    mem_wb->MemToReg = ex_mem->MemParaReg; mem_wb->ULA_saida = ex_mem->ULA_saida;
    mem_wb->RDM = 0; mem_wb->valid = 1;

    if (ex_mem->MemParaReg) { mem_wb->RDM = (signed char)load(mem, (unsigned char)ex_mem->ULA_saida); PLOG("[MEM] LW: Mem[%d] = %d", (unsigned char)ex_mem->ULA_saida, mem_wb->RDM); }
    if (ex_mem->EscMem)     { Store(mem, (signed char)ex_mem->ULA_saida, ex_mem->B); PLOG("[MEM] SW: Mem[%d] <- %d", (unsigned char)ex_mem->ULA_saida, ex_mem->B); mem_wb->RegWrite = 0; }
    if (!ex_mem->MemParaReg && !ex_mem->EscMem && !ex_mem->beq && !ex_mem->jump)
        PLOG("[MEM] Sem acesso a memoria");

    if (ex_mem->beq) {
        if (ex_mem->zero) { *PC = (unsigned char)ex_mem->add_result; PLOG("[MEM] BEQ tomado: PC <- %d (FLUSH)", *PC); *flush = 1; }
        else PLOG("[MEM] BEQ nao tomado");
        mem_wb->RegWrite = 0;
    }
    if (ex_mem->jump) { *PC = (unsigned char)ex_mem->add_result; PLOG("[MEM] JUMP: PC <- %d (FLUSH)", *PC); *flush = 1; mem_wb->RegWrite = 0; }
}

static void estagio_WB(signed char *reg, RegMEM_WB *mem_wb)
{
    if (!mem_wb->RegWrite) { PLOG("[WB ] Nenhum registrador escrito nesse estagio"); return; }
    signed char valor = mem_wb->MemToReg ? mem_wb->RDM : mem_wb->ULA_saida;
    esc(reg, (int)mem_wb->dest, valor, 1);
    PLOG("[WB ] R%d <- %d (%s)", mem_wb->dest, valor, mem_wb->MemToReg ? "Memoria" : "ULA");
}


int main(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int altura, largura;
    getmaxyx(stdscr, altura, largura);

    int menu_w = 35;
    saida_y0   = 0;
    saida_x0   = menu_w;
    saida_h    = altura - 1;   /* -1 para linha de rodapé */
    saida_w_sz = largura - menu_w;

    menu_win  = newwin(altura, menu_w, 0, 0);
    saida_win = newwin(altura, saida_w_sz, 0, saida_x0); /* janela de borda */
    saida_pad = newpad(PAD_ALTURA, saida_w_sz);

    if (!menu_win || !saida_win || !saida_pad)
    {
        endwin();
        fprintf(stderr, "Erro: terminal muito pequeno.\n");
        return 1;
    }

    scrollok(saida_pad, TRUE);


    int op = 24;
    signed char reg[8]; iniat(reg);
    unsigned char instruction = 0;
    instro l = {0}, dat = {0};
    RegIF_ID if_id = {0}; RegID_EX id_ex = {0};
    RegEX_MEM ex_mem = {0}; RegMEM_WB mem_wb = {0};
    int ciclo = 0;
    char caminho[260];
    int i;
    Snapshot pilha[500]; int sp = -1;
    int total_instrucoes_completadas = 0, total_forwarding_evitados = 0;
    int total_stalls = 0, total_flushes = 0, total_instrucoes_descartadas = 0;
    int qtd_aritmetica = 0, qtd_memoria = 0, qtd_desvio = 0;
    int f1=0, f2=0, f3=0, f4=0, RI=0, PC=0;

    do
    {
        werase(menu_win);
        box(menu_win, 0, 0);
        mvwprintw(menu_win,  1, 2, "SIMULADOR MIPS");
        mvwprintw(menu_win,  3, 2, "1  Carregar mem instrucoes");
        mvwprintw(menu_win,  4, 2, "2  Carregar mem dados");
        mvwprintw(menu_win,  5, 2, "3  Imprimir memorias");
        mvwprintw(menu_win,  6, 2, "4  Banco registradores");
        mvwprintw(menu_win,  7, 2, "5  Estado simulador");
        mvwprintw(menu_win,  8, 2, "6  Salvar ASM");
        mvwprintw(menu_win,  9, 2, "7  Salvar DAT");
        mvwprintw(menu_win, 10, 2, "8  run");
        mvwprintw(menu_win, 11, 2, "9  Step");
        mvwprintw(menu_win, 12, 2, "10 Back");
        mvwprintw(menu_win, 13, 2, "11 Estatisticas");
        mvwprintw(menu_win, 14, 2, "0  Sair");
        mvwprintw(menu_win, 16, 2, "Opcao: ");
        wrefresh(menu_win);

        echo();
        curs_set(1);
        char buf[16] = {0};
        mvwgetnstr(menu_win, 15, 9, buf, sizeof(buf)-1);
        curs_set(0);
        noecho();
        op = atoi(buf);

        //pad_reset();

        switch (op)
        {
        case 1:
            caminho[0] = '\0';
            selecionar_arquivo(caminho);
            if (carregar(&l, caminho, &i) != 0)
                PLOG("Erro ao carregar memoria de instrucoes.");
            else
                PLOG("Memoria de instrucoes carregada: %d instrucoes.", i);
            aguardar_scroll();
            break;

        case 2:
            caminho[0] = '\0';
            selecionar_arquivo2(caminho);
            if (carregar_dados(&dat, caminho) != 0)
                PLOG("Erro ao carregar memoria de dados.");
            else
                PLOG("Memoria de dados carregada com sucesso.");
            aguardar_scroll();
            break;

        case 3:
            pad_reset();
            print_mem(&dat);
            print_mem_inst(&l, 256);
            aguardar_scroll();
            break;

        case 4:
            pad_reset();
            print_regs(reg);
            aguardar_scroll();
            break;

        case 5:

            pad_reset();


            escrever_no_pad("PC(dec): %d", instruction);



            wmove(saida_pad, 2, 0);
            print_mem(&dat);


            wmove(saida_pad, 259, 0);
            print_mem_inst(&l, 256);


            wmove(saida_pad, 516, 0);
            print_regs(reg);


            pad_linha = 516 + 10;

            aguardar_scroll();
            break;

        case 6:
            print_program(&l, 256);
            save_program_asm(&l, 256, "programa.asm");
            PLOG("Arquivo programa.asm salvo.");
            aguardar_scroll();
            break;

        case 7:
            save_mem_dat(&dat, "dados_saida.dat");
            PLOG("Arquivo dados_saida.dat salvo.");
            aguardar_scroll();
            break;

        case 8:

            do
            {

                if (sp >= 499) { for (int k=0;k<499;k++) pilha[k]=pilha[k+1]; }
            else sp++;
            Snapshot *s = &pilha[sp];
            for (int k=0;k<8;k++) s->reg[k]=reg[k];
            for (int k=0;k<256;k++) s->memoria[k]=dat.instc[k];
            s->PC=instruction; s->ciclo=ciclo;
            s->if_id=if_id; s->id_ex=id_ex; s->ex_mem=ex_mem; s->mem_wb=mem_wb;
            s->f1=f1; s->f2=f2; s->f3=f3; s->f4=f4;
            s->total_instrucoes_completadas=total_instrucoes_completadas;
            s->total_forwarding_evitados=total_forwarding_evitados;
            s->total_stalls=total_stalls; s->total_flushes=total_flushes;
            s->total_instrucoes_descartadas=total_instrucoes_descartadas;
            s->qtd_aritmetica=qtd_aritmetica; s->qtd_memoria=qtd_memoria; s->qtd_desvio=qtd_desvio;

            ciclo++;
            PLOG("\n \n \n \n \n \n \n");
            PLOG("================ CICLO %d ================", ciclo);

            RegEX_MEM ex_mem_old = ex_mem;
            RegMEM_WB mem_wb_old = mem_wb;


            if (mem_wb.valid == 1)
            {
                if (f4 != 0) { PC=0; RI=0; f4--; }
                else if (sp>=3) { PC=pilha[sp-3].if_id.PC-1; RI=pilha[sp-3].if_id.RI; }
                else { PC=0; RI=0; }
                PLOG("--------------------------------------------------------------");
                wmove(saida_pad, pad_linha, 0);
                wprintw(saida_pad, "[WB ] | PC="); print_bin(PC);
                wprintw(saida_pad, "(%3d) | bin=", PC); print_bin(RI);
                wprintw(saida_pad, " | asm: "); print_asm(RI);
                pad_linha++;
                if (mem_wb.MemToReg) { qtd_memoria++; total_instrucoes_completadas++; }
                else if (mem_wb.RegWrite) { qtd_aritmetica++; total_instrucoes_completadas++; }
                estagio_WB(reg, &mem_wb);
            }


            int flush = 0;

            if (ex_mem.valid == 1)
            {
                if (f3 != 0) { PC=0; RI=0; f3--; }
                else if (sp>=2) { PC=pilha[sp-2].if_id.PC-1; RI=pilha[sp-2].if_id.RI; }
                else { PC=0; RI=0; }
                PLOG("--------------------------------------------------------------");
                wmove(saida_pad, pad_linha, 0);
                wprintw(saida_pad, "[MEM] | PC="); print_bin(PC);
                wprintw(saida_pad, "(%3d) | bin=", PC); print_bin(RI);
                wprintw(saida_pad, " | asm: "); print_asm(RI);
                pad_linha++;
                if (ex_mem.EscMem) { total_instrucoes_completadas++; qtd_memoria++; }
                else if (ex_mem.beq || ex_mem.jump) { total_instrucoes_completadas++; qtd_desvio++; }
                estagio_MEM(&dat, &instruction, &ex_mem, &mem_wb, &flush);
            }


            if (id_ex.valid == 1)
            {
                if (f2 != 0) { PC=0; RI=0; f2--; }
                else if (sp>=1) { PC=pilha[sp-1].if_id.PC-1; RI=pilha[sp-1].if_id.RI; }
                else { PC=0; RI=0; }
                PLOG("--------------------------------------------------------------");
                wmove(saida_pad, pad_linha, 0);
                wprintw(saida_pad, "[EX ] | PC="); print_bin(PC);
                wprintw(saida_pad, "(%3d) | bin=", PC); print_bin(RI);
                wprintw(saida_pad, " | asm: "); print_asm(RI);
                pad_linha++;
                estagio_EX(&id_ex, &ex_mem, &ex_mem_old, &mem_wb_old, &total_forwarding_evitados);
            }


            int stall = hazard_detection_unit(&id_ex, if_id.RI);
            if (stall)
            {
                total_stalls++;
                PLOG("[HAZARD] load-use detectado -> stall 1 ciclo");
                estagio_ID(reg, &if_id, &id_ex);
                memset(&id_ex, 0, sizeof(id_ex));
                PLOG("[ID ] (stall - aguardando lw)");
                PLOG("[IF ] (stall - PC mantido)");
            }
            else
            {

                if (if_id.valid == 1)
                {
                    if (f1 != 0) { PC=0; RI=0; f1--; }
                    else { PC=if_id.PC-1; RI=if_id.RI; }
                    PLOG("--------------------------------------------------------------");
                    wmove(saida_pad, pad_linha, 0);
                    wprintw(saida_pad, "[ID ] | PC="); print_bin(PC);
                    wprintw(saida_pad, "(%3d) | bin=", PC); print_bin(RI);
                    wprintw(saida_pad, " | asm: "); print_asm(RI);
                    pad_linha++;
                    estagio_ID(reg, &if_id, &id_ex);
                }

                PLOG("--------------------------------------------------------------");
                estagio_IF(&l, &instruction, &if_id);
                }


                if (flush)
                {
                    PLOG("[FLUSH] desvio tomado -> invalidando id_ex, if_id e ex_mem");
                    memset(&id_ex,0,sizeof(id_ex)); memset(&if_id,0,sizeof(if_id)); memset(&ex_mem,0,sizeof(ex_mem));
                    id_ex.valid=1; if_id.valid=1; ex_mem.valid=1;
                    total_flushes++; total_instrucoes_descartadas += 2;
                    f1=1; f2=2; f3=3; f4=4;
                }

                aguardar_scroll();



            }while(PC!=255);

            break;

        case 9:
        {
            if (sp >= 499) { for (int k=0;k<499;k++) pilha[k]=pilha[k+1]; }
            else sp++;
            Snapshot *s = &pilha[sp];
            for (int k=0;k<8;k++) s->reg[k]=reg[k];
            for (int k=0;k<256;k++) s->memoria[k]=dat.instc[k];
            s->PC=instruction; s->ciclo=ciclo;
            s->if_id=if_id; s->id_ex=id_ex; s->ex_mem=ex_mem; s->mem_wb=mem_wb;
            s->f1=f1; s->f2=f2; s->f3=f3; s->f4=f4;
            s->total_instrucoes_completadas=total_instrucoes_completadas;
            s->total_forwarding_evitados=total_forwarding_evitados;
            s->total_stalls=total_stalls; s->total_flushes=total_flushes;
            s->total_instrucoes_descartadas=total_instrucoes_descartadas;
            s->qtd_aritmetica=qtd_aritmetica; s->qtd_memoria=qtd_memoria; s->qtd_desvio=qtd_desvio;

            ciclo++;
            PLOG("\n \n \n \n \n \n \n");
            PLOG("================ CICLO %d ================", ciclo);

            RegEX_MEM ex_mem_old = ex_mem;
            RegMEM_WB mem_wb_old = mem_wb;


            if (mem_wb.valid == 1)
            {
                if (f4 != 0) { PC=0; RI=0; f4--; }
                else if (sp>=3) { PC=pilha[sp-3].if_id.PC-1; RI=pilha[sp-3].if_id.RI; }
                else { PC=0; RI=0; }
                PLOG("--------------------------------------------------------------");
                wmove(saida_pad, pad_linha, 0);
                wprintw(saida_pad, "[WB ] | PC="); print_bin(PC);
                wprintw(saida_pad, "(%3d) | bin=", PC); print_bin(RI);
                wprintw(saida_pad, " | asm: "); print_asm(RI);
                pad_linha++;
                if (mem_wb.MemToReg) { qtd_memoria++; total_instrucoes_completadas++; }
                else if (mem_wb.RegWrite) { qtd_aritmetica++; total_instrucoes_completadas++; }
                estagio_WB(reg, &mem_wb);
            }


            int flush = 0;

            if (ex_mem.valid == 1)
            {
                if (f3 != 0) { PC=0; RI=0; f3--; }
                else if (sp>=2) { PC=pilha[sp-2].if_id.PC-1; RI=pilha[sp-2].if_id.RI; }
                else { PC=0; RI=0; }
                PLOG("--------------------------------------------------------------");
                wmove(saida_pad, pad_linha, 0);
                wprintw(saida_pad, "[MEM] | PC="); print_bin(PC);
                wprintw(saida_pad, "(%3d) | bin=", PC); print_bin(RI);
                wprintw(saida_pad, " | asm: "); print_asm(RI);
                pad_linha++;
                if (ex_mem.EscMem) { total_instrucoes_completadas++; qtd_memoria++; }
                else if (ex_mem.beq || ex_mem.jump) { total_instrucoes_completadas++; qtd_desvio++; }
                estagio_MEM(&dat, &instruction, &ex_mem, &mem_wb, &flush);
            }


            if (id_ex.valid == 1)
            {
                if (f2 != 0) { PC=0; RI=0; f2--; }
                else if (sp>=1) { PC=pilha[sp-1].if_id.PC-1; RI=pilha[sp-1].if_id.RI; }
                else { PC=0; RI=0; }
                PLOG("--------------------------------------------------------------");
                wmove(saida_pad, pad_linha, 0);
                wprintw(saida_pad, "[EX ] | PC="); print_bin(PC);
                wprintw(saida_pad, "(%3d) | bin=", PC); print_bin(RI);
                wprintw(saida_pad, " | asm: "); print_asm(RI);
                pad_linha++;
                estagio_EX(&id_ex, &ex_mem, &ex_mem_old, &mem_wb_old, &total_forwarding_evitados);
            }


            int stall = hazard_detection_unit(&id_ex, if_id.RI);
            if (stall)
            {
                total_stalls++;
                PLOG("[HAZARD] load-use detectado -> stall 1 ciclo");
                estagio_ID(reg, &if_id, &id_ex);
                memset(&id_ex, 0, sizeof(id_ex));
                PLOG("[ID ] (stall - aguardando lw)");
                PLOG("[IF ] (stall - PC mantido)");
            }
            else
            {

                if (if_id.valid == 1)
                {
                    if (f1 != 0) { PC=0; RI=0; f1--; }
                    else { PC=if_id.PC-1; RI=if_id.RI; }
                    PLOG("--------------------------------------------------------------");
                    wmove(saida_pad, pad_linha, 0);
                    wprintw(saida_pad, "[ID ] | PC="); print_bin(PC);
                    wprintw(saida_pad, "(%3d) | bin=", PC); print_bin(RI);
                    wprintw(saida_pad, " | asm: "); print_asm(RI);
                    pad_linha++;
                    estagio_ID(reg, &if_id, &id_ex);
                }

                PLOG("--------------------------------------------------------------");
                estagio_IF(&l, &instruction, &if_id);
            }


            if (flush)
            {
                PLOG("[FLUSH] desvio tomado -> invalidando id_ex, if_id e ex_mem");
                memset(&id_ex,0,sizeof(id_ex)); memset(&if_id,0,sizeof(if_id)); memset(&ex_mem,0,sizeof(ex_mem));
                id_ex.valid=1; if_id.valid=1; ex_mem.valid=1;
                total_flushes++; total_instrucoes_descartadas += 2;
                f1=1; f2=2; f3=3; f4=4;
            }

            aguardar_scroll();
            break;
        }

        case 10:
        {


            if (sp < 0) {

                pad_reset();

                PLOG("[BACK] Pilha vazia.");

                aguardar_scroll();

                break;
            }

            Snapshot *s = &pilha[sp];
            sp--;


            for (int k = 0; k < 8; k++) reg[k] = s->reg[k];
            for (int k = 0; k < 256; k++) dat.instc[k] = s->memoria[k];

            instruction = s->PC;
            ciclo = s->ciclo;
            if_id = s->if_id;
            id_ex = s->id_ex;
            ex_mem = s->ex_mem;
            mem_wb = s->mem_wb;
            f1 = s->f1; f2 = s->f2; f3 = s->f3; f4 = s->f4;

            total_instrucoes_completadas = s->total_instrucoes_completadas;
            total_forwarding_evitados = s->total_forwarding_evitados;
            total_stalls = s->total_stalls;
            total_flushes = s->total_flushes;
            total_instrucoes_descartadas = s->total_instrucoes_descartadas;
            qtd_aritmetica = s->qtd_aritmetica;
            qtd_memoria = s->qtd_memoria;
            qtd_desvio = s->qtd_desvio;


            pad_reset();
            PLOG("[BACK] Voltou para ciclo %d | PC=%d", ciclo, instruction);

            aguardar_scroll();
            break;
}

        case 11:
        {
            float cpi = (total_instrucoes_completadas > 0) ? ((float)ciclo / total_instrucoes_completadas) : 0.0f;


            pad_reset();

            PLOG("==================================================================");
            PLOG("            ESTATISTICAS DO PROGRAMA");
            PLOG("==================================================================");
            PLOG("[DESEMPENHO GERAL]");
            PLOG("- Ciclos: %d", ciclo);
            PLOG("- Instrucoes completas: %d", total_instrucoes_completadas);
            PLOG("- CPI: %.2f", cpi);
            PLOG("[HAZARDS]");
            PLOG("- Forwardings: %d", total_forwarding_evitados);
            PLOG("- Stalls: %d", total_stalls);
            PLOG("- Flushes: %d", total_flushes);
            PLOG("- Instrucoes descartadas: %d", total_instrucoes_descartadas);
            PLOG("[INSTRUCOES]");
            PLOG("- Aritmeticas/Logicas: %d", qtd_aritmetica);
            PLOG("- Memoria: %d", qtd_memoria);
            PLOG("- Desvios: %d", qtd_desvio);
            PLOG("==================================================================");


            aguardar_scroll();
            break;
        }
        case 0:
            break;

        default:
            PLOG("Opcao invalida.");
            aguardar_scroll();
            break;
        }

    } while (op != 0);

    delwin(saida_pad);
    delwin(saida_win);
    delwin(menu_win);
    endwin();
    return 0;
}
