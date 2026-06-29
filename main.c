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
/* ===================================================================
 *  CHROME DA INTERFACE (cores, cabecalho, rodape, menu, diagrama)
 * 
 * ==================================================================== */
/* pares de cores */
#define CP_HEADER     1   /* barra de titulo                       */
#define CP_MENU_TITLE 2   /* titulos/secoes do menu                */
#define CP_MENU_KEY   3   /* teclas de atalho do menu              */
#define CP_FOOTER     4   /* barra de rodape                       */
#define CP_IF         5   /* estagio IF                            */
#define CP_ID         6   /* estagio ID                            */
#define CP_EX         7   /* estagio EX                            */
#define CP_MEM        8   /* estagio MEM                           */
#define CP_WB         9   /* estagio WB                            */
#define CP_HAZ        10  /* hazard / stall / flush                */
#define CP_SEP        11  /* separadores                           */
#define CP_BANNER     12  /* banner de ciclo                       */
#define CP_BORDER     13  /* bordas das janelas */                  

WINDOW *header_win = NULL;
WINDOW *footer_win = NULL;
int body_h = 0;// altura da regiao de menu/saida

//paineis do dashboard 
WINDOW *pipe_win = NULL;   //ocupacao do pipeline      
WINDOW *reg_win  = NULL;   // banco de registradores    
WINDOW *stat_win = NULL;   // estatisticas
WINDOW *dmem_win = NULL;   // memoria de dados
WINDOW *prog_win = NULL;   // programa / memoria de instrucoes
/* saida_win = painel de LOG (texto rolavel via saida_pad) */

// ponteiros para o estado vivo da simulacao

static signed char *g_reg = NULL;
static instro      *g_l = NULL, *g_dat = NULL;
static RegIF_ID    *g_if_id = NULL;
static RegID_EX    *g_id_ex = NULL;
static RegEX_MEM   *g_ex_mem = NULL;
static RegMEM_WB   *g_mem_wb = NULL;
static unsigned char *g_pc = NULL;
static int *g_ciclo=NULL, *g_instr_ok=NULL, *g_fwd=NULL, *g_stalls=NULL;
static int *g_flushes=NULL, *g_descart=NULL, *g_arit=NULL, *g_mem=NULL, *g_desv=NULL;

static void desenha_header(int ciclo, int pc, int instr_ok);
static void render_paineis_globais(void);

static void init_cores(void)
{
    if (!has_colors()) return;
    start_color();
    use_default_colors();
    init_pair(CP_HEADER,     COLOR_WHITE,   COLOR_BLUE);
    init_pair(CP_MENU_TITLE, COLOR_YELLOW,  -1);
    init_pair(CP_MENU_KEY,   COLOR_CYAN,    -1);
    init_pair(CP_FOOTER,     COLOR_BLACK,   COLOR_WHITE);
    init_pair(CP_IF,         COLOR_CYAN,    -1);
    init_pair(CP_ID,         COLOR_GREEN,   -1);
    init_pair(CP_EX,         COLOR_YELLOW,  -1);
    init_pair(CP_MEM,        COLOR_MAGENTA, -1);
    init_pair(CP_WB,         COLOR_BLUE,    -1);
    init_pair(CP_HAZ,        COLOR_RED,     -1);
    init_pair(CP_SEP,        COLOR_WHITE,   -1);
    init_pair(CP_BANNER,     COLOR_YELLOW,  -1);
    init_pair(CP_BORDER,     COLOR_CYAN,    -1);
}

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

static void desenha_footer(const char *txt)
{
    if (!footer_win) return;
    werase(footer_win);
    wbkgd(footer_win, COLOR_PAIR(CP_FOOTER));
    mvwprintw(footer_win, 0, 1, "%s", txt);
    wrefresh(footer_win);
}

/* ===================================================================
 *  PAINEIS DO DASHBOARD 
 *  Cada painel e uma janela com borda + titulo, redesenhada a partir
 *  do estado vivo da simulacao (ponteiros g_*).
 * =================================================================== */


static void painel_titulo(WINDOW *w, const char *titulo)
{
    wattron(w, COLOR_PAIR(CP_BORDER));
    box(w, 0, 0);
    wattroff(w, COLOR_PAIR(CP_BORDER));
    wattron(w, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
    mvwprintw(w, 0, 2, " %s ", titulo);
    wattroff(w, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
}
// borda + titulo do painel de LOG (o texto fica no saida_pad por cima)

static void render_log_border(void)
{
    if (!saida_win) return;
    painel_titulo(saida_win, "LOG DO CICLO  (rolar: setas / PgUp / PgDn)");
    wrefresh(saida_win);
}

static void render_pipeline(void)
{
    WINDOW *w = pipe_win;
    if (!w) return;
    werase(w);
    painel_titulo(w, "PIPELINE   IF -> ID -> EX -> MEM -> WB");

    char buf[64];
    int linha = 1;

    struct { const char *tag; int cp; } st[5] = {
        {"IF ", CP_IF}, {"ID ", CP_ID}, {"EX ", CP_EX},
        {"MEM", CP_MEM}, {"WB ", CP_WB}
    };

    for (int i = 0; i < 5; i++) {
        wmove(w, linha, 2);
        wattron(w, COLOR_PAIR(st[i].cp) | A_BOLD);
        wprintw(w, "[%s]", st[i].tag);
        wattroff(w, COLOR_PAIR(st[i].cp) | A_BOLD);
        wprintw(w, " ");

        if (i == 0) { //if
            asm_str(g_l->instc[*g_pc], buf, sizeof buf);
            wprintw(w, "busca PC=%-3d : %s", *g_pc, buf);
        } else if (i == 1) {//id
            if (g_if_id->valid) { asm_str(g_if_id->RI, buf, sizeof buf); wprintw(w, "decodifica : %s", buf); }
            else wprintw(w, "-- bolha --");
        } else if (i == 2) { //ex
            if (g_id_ex->valid)
                wprintw(w, "executa -> destino R%d", Rdest(g_id_ex->RegDst, g_id_ex->rd, g_id_ex->rt));
            else wprintw(w, "-- bolha --");
        } else if (i == 3) { //mem
            if (g_ex_mem->valid) {
                if (g_ex_mem->EscMem)          wprintw(w, "escreve mem[%d]", (unsigned char)g_ex_mem->ULA_saida);
                else if (g_ex_mem->MemParaReg) wprintw(w, "le mem[%d]", (unsigned char)g_ex_mem->ULA_saida);
                else if (g_ex_mem->beq)        wprintw(w, "avalia desvio (beq)");
                else if (g_ex_mem->jump)       wprintw(w, "desvio (jump)");
                else                           wprintw(w, "sem acesso a memoria");
            } else wprintw(w, "-- bolha --");
        } else { //wb
            if (g_mem_wb->valid) {
                if (g_mem_wb->RegWrite) wprintw(w, "escreve R%d <- %s", g_mem_wb->dest, g_mem_wb->MemToReg ? "memoria" : "ULA");
                else wprintw(w, "sem escrita em registrador");
            } else wprintw(w, "-- bolha --");
        }
        linha++;
    }
    wrefresh(w);
}

static void render_registradores(void)
{
    WINDOW *w = reg_win;
    if (!w) return;
    werase(w);
    painel_titulo(w, "REGISTRADORES");

    int h, wd; getmaxyx(w, h, wd);
    int dest = (g_mem_wb && g_mem_wb->valid && g_mem_wb->RegWrite) ? g_mem_wb->dest : -1;

    // duas colunas se couber
    int col2 = (wd >= 24);
    for (int i = 0; i < 8; i++) {
        int linha = col2 ? (1 + (i % 4)) : (1 + i);
        int x     = col2 ? (2 + (i / 4) * ((wd - 4) / 2)) : 2;
        if (linha >= h - 1) break;
        if (i == dest) wattron(w, COLOR_PAIR(CP_WB) | A_BOLD);
        mvwprintw(w, linha, x, "R%d = %-5d", i, (int)g_reg[i]);
        if (i == dest) wattroff(w, COLOR_PAIR(CP_WB) | A_BOLD);
    }
    wrefresh(w);
}

// Painel PROGRAMA ASM / MEMORIA DE INSTRUCOES:
 
static void render_estatisticas(void)
{
    WINDOW *w = stat_win;
    if (!w) return;
    werase(w);
    painel_titulo(w, "ESTATISTICAS");

    float cpi = (*g_instr_ok > 0) ? ((float)(*g_ciclo) / *g_instr_ok) : 0.0f;
    int linha = 1;
    mvwprintw(w, linha++, 2, "Ciclos: %-4d  Instr.OK: %-4d", *g_ciclo, *g_instr_ok);
    mvwprintw(w, linha++, 2, "CPI: %.2f", cpi);
    wattron(w, COLOR_PAIR(CP_HAZ) | A_BOLD);
    mvwprintw(w, linha++, 2, "Fwd:%d  Stall:%d  Flush:%d", *g_fwd, *g_stalls, *g_flushes);
    mvwprintw(w, linha++, 2, "Descartadas: %d", *g_descart);
    wattroff(w, COLOR_PAIR(CP_HAZ) | A_BOLD);
    mvwprintw(w, linha++, 2, "Aritm:%d  Mem:%d  Desvio:%d", *g_arit, *g_mem, *g_desv);
    wrefresh(w);
}

static void bin16_str(unsigned short x, char *buf)
{
    for (int b = 15; b >= 0; b--) buf[15 - b] = ((x >> b) & 1) ? '1' : '0';
    buf[16] = '\0';
}

static void render_memoria_dados(void)
{
    WINDOW *w = dmem_win;
    if (!w) return;
    werase(w);
    painel_titulo(w, "MEMORIA DE DADOS");

    int h, wd; getmaxyx(w, h, wd);
    int rows = h - 2;
    if (rows < 1) rows = 1;
    int cell = 11;
    int cols = (wd - 2) / cell; if (cols < 1) cols = 1;
    int cap  = rows * cols; if (cap > 256) cap = 256;

    for (int k = 0; k < cap; k++) {
        int linha = 1 + (k % rows);
        int x     = 2 + (k / rows) * cell;
        wattron(w, COLOR_PAIR(CP_MEM));
        mvwprintw(w, linha, x, "M[%3d]=%-3d", k, (int)(signed char)g_dat->instc[k]);
        wattroff(w, COLOR_PAIR(CP_MEM));
    }
    if (cap < 256)
        mvwprintw(w, h - 1, 2, "... (opcao 3 = ver tudo)");
    wrefresh(w);
}

static void render_programa(void)
{
    WINDOW *w = prog_win;
    if (!w) return;
    werase(w);
    painel_titulo(w, "PROGRAMA / MEM. INSTRUCOES (bin + asm)");

    int h, wd; getmaxyx(w, h, wd);
    int rows = h - 2;
    if (rows < 1) rows = 1;

    int pc = (int)*g_pc;

    int first = pc - 2;
    if (first < 0) first = 0;
    if (first > 256 - rows) first = 256 - rows;
    if (first < 0) first = 0;

    char bin[20], asm_buf[40];
    for (int r = 0; r < rows; r++) {
        int idx = first + r;
        if (idx >= 256) break;

        bin16_str(g_l->instc[idx], bin);
        asm_str(g_l->instc[idx], asm_buf, sizeof asm_buf);

        int destaque = (idx == pc);
        if (destaque) wattron(w, COLOR_PAIR(CP_IF) | A_BOLD);

        mvwprintw(w, 1 + r, 1, "%c%3d %s %-18.18s",
                  destaque ? '>' : ' ', idx, bin, asm_buf);

        if (destaque) wattroff(w, COLOR_PAIR(CP_IF) | A_BOLD);
    }
    wrefresh(w);
}

static void render_paineis_globais(void)
{
    if (!g_reg) return;
    desenha_header(*g_ciclo, *g_pc, *g_instr_ok);
    render_pipeline();
    render_registradores();
    render_memoria_dados();
    render_estatisticas();
    render_programa();
    render_log_border();
}

static const char *nome_estagio(const char *tag)
{
    if (tag[0] == 'I' && tag[1] == 'F') return "BUSCA";
    if (tag[0] == 'I' && tag[1] == 'D') return "DECODIFICACAO";
    if (tag[0] == 'E')                  return "EXECUCAO";
    if (tag[0] == 'M')                  return "ACESSO A MEMORIA";
    return "WRITE-BACK";
}

//cabaçalho do ciclo
static void log_banner_ciclo(int ciclo)
{
    pad_linha++;
    wattron(saida_pad, COLOR_PAIR(CP_BANNER) | A_BOLD);
    mvwprintw(saida_pad, pad_linha++, 0,
              "======================  CICLO %d  ======================", ciclo);
    wattroff(saida_pad, COLOR_PAIR(CP_BANNER) | A_BOLD);
}

//linah do tempo do pipeline
static void log_timeline(RegIF_ID *if_id, RegID_EX *id_ex,
                         RegEX_MEM *ex_mem, RegMEM_WB *mem_wb)
{
    struct { const char *t; int cp; int on; } st[5] = {
        {"IF",  CP_IF,  1},
        {"ID",  CP_ID,  if_id->valid},
        {"EX",  CP_EX,  id_ex->valid},
        {"MEM", CP_MEM, ex_mem->valid},
        {"WB",  CP_WB,  mem_wb->valid},
    };
    wmove(saida_pad, pad_linha, 0);
    wattron(saida_pad, COLOR_PAIR(CP_SEP));
    wprintw(saida_pad, "  fluxo: ");
    wattroff(saida_pad, COLOR_PAIR(CP_SEP));
    for (int i = 0; i < 5; i++) {
        if (i) {
            wattron(saida_pad, COLOR_PAIR(CP_SEP));
            wprintw(saida_pad, " -> ");
            wattroff(saida_pad, COLOR_PAIR(CP_SEP));
        }
        if (st[i].on) {
            wattron(saida_pad, COLOR_PAIR(st[i].cp) | A_BOLD);
            wprintw(saida_pad, "[%s]", st[i].t);
            wattroff(saida_pad, COLOR_PAIR(st[i].cp) | A_BOLD);
        } else {
            wattron(saida_pad, COLOR_PAIR(CP_SEP));
            wprintw(saida_pad, " %s ", st[i].t);
            wattroff(saida_pad, COLOR_PAIR(CP_SEP));
        }
    }
    pad_linha += 2;
}

static void log_card_estagio(const char *tag, int cp, int PC, int RI)
{
    unsigned short instr = (unsigned short)RI;
    unsigned char opcode = (instr >> 12) & 0xF;
    unsigned char rs    = (instr >>  9) & 0x7;
    unsigned char rt    = (instr >>  6) & 0x7;
    unsigned char rd    = (instr >>  3) & 0x7;
    unsigned char funct =  instr        & 0x7;
    unsigned char imm6  =  instr        & 0x3F;
    signed char   immx  = (imm6 & 0x20) ? (signed char)(imm6 | 0xC0) : (signed char)imm6;
    unsigned char addr  =  instr        & 0xFF;

    char bin[20]; bin16_str(instr, bin);
    char asmb[40]; asm_str(instr, asmb, sizeof asmb);

    char grp[40];
    if (opcode == 0x0)
        snprintf(grp, sizeof grp, "%.4s %.3s %.3s %.3s %.3s", bin, bin+4, bin+7, bin+10, bin+13);
    else if (opcode == 0x2)
        snprintf(grp, sizeof grp, "%.4s %.4s %.8s", bin, bin+4, bin+8);
    else
        snprintf(grp, sizeof grp, "%.4s %.3s %.3s %.6s", bin, bin+4, bin+7, bin+10);

    char campos[80];
    if (opcode == 0x0)
        snprintf(campos, sizeof campos, "opcode=%-2u rs=R%u rt=R%u rd=R%u funct=%u",
                 opcode, rs, rt, rd, funct);
    else if (opcode == 0x2)
        snprintf(campos, sizeof campos, "opcode=%-2u addr=%u", opcode, addr);
    else
        snprintf(campos, sizeof campos, "opcode=%-2u rs=R%u rt=R%u imm=%d",
                 opcode, rs, rt, immx);

    pad_linha++;

    wmove(saida_pad, pad_linha, 0);
    wattron(saida_pad, COLOR_PAIR(cp) | A_BOLD);
    wprintw(saida_pad, "  >> [%s] %-16s  PC=%-3d", tag, nome_estagio(tag), PC);
    wattroff(saida_pad, COLOR_PAIR(cp) | A_BOLD);
    pad_linha++;

    wmove(saida_pad, pad_linha, 0);
    wattron(saida_pad, COLOR_PAIR(cp));
    wprintw(saida_pad, "     bin: %-22s asm: %s", grp, asmb);
    wattroff(saida_pad, COLOR_PAIR(cp));
    pad_linha++;

    wmove(saida_pad, pad_linha, 0);
    wattron(saida_pad, COLOR_PAIR(CP_SEP));
    wprintw(saida_pad, "     campos: %s", campos);
    wattroff(saida_pad, COLOR_PAIR(CP_SEP));
    pad_linha++;
}

static int aguardar_scroll(void)
{
    int max_scroll = pad_linha - saida_h;

    if (max_scroll < 0) max_scroll = 0;

    render_paineis_globais();

    desenha_footer(" Pressione 'q' para retornar ao menu   |   ENTER: continuar   |   setas/PgUp/PgDn: rolar o log ");

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

    desenha_footer(" Use o menu a esquerda  |  digite o numero da opcao e tecle Enter ");
    return ch;
}

static void redraw_chrome(void)
{
    if (header_win) { touchwin(header_win); wrefresh(header_win); }
    if (menu_win)   { touchwin(menu_win);   wrefresh(menu_win);   }
    if (saida_win)  { touchwin(saida_win);  wrefresh(saida_win);  }
    if (footer_win) { touchwin(footer_win); wrefresh(footer_win); }
    pad_show();
}

static void ler_caminho(const char *titulo, const char *ext,
                        char *out, int outsz)
{
    int h = 7, w = 64;
    if (w > COLS - 2)  w = COLS - 2;
    int y = (LINES - h) / 2;
    int x = (COLS  - w) / 2;
    if (y < 0) y = 0;
    if (x < 0) x = 0;

    WINDOW *dlg = newwin(h, w, y, x);
    keypad(dlg, TRUE);

    wattron(dlg, COLOR_PAIR(CP_BORDER));
    box(dlg, 0, 0);
    wattroff(dlg, COLOR_PAIR(CP_BORDER));

    wattron(dlg, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
    mvwprintw(dlg, 0, 2, " %s ", titulo);
    wattroff(dlg, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);

    mvwprintw(dlg, 2, 2, "Digite o nome/caminho do arquivo (%s) e tecle Enter:", ext);
    mvwprintw(dlg, 4, 2, "> ");
    wrefresh(dlg);

    char tmp[256] = {0};
    echo();
    curs_set(1);
    mvwgetnstr(dlg, 4, 4, tmp, (int)sizeof(tmp) - 1);
    curs_set(0);
    noecho();

    delwin(dlg);

    tmp[strcspn(tmp, "\r\n")] = 0;

    out[0] = '\0';
    strncpy(out, tmp, outsz - 1);
    out[outsz - 1] = '\0';

    size_t le = strlen(ext), lo = strlen(out);
    if (lo < le || strcmp(out + lo - le, ext) != 0)
        strncat(out, ext, outsz - lo - 1);

    redraw_chrome();
}

void selecionar_arquivo(char *caminho)
{
    ler_caminho("Carregar memoria de instrucoes", ".mem", caminho, 256);
}

void selecionar_arquivo2(char *caminho)
{
    ler_caminho("Carregar memoria de dados", ".dat", caminho, 256);
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

    log_card_estagio("IF ", CP_IF, *PC, if_id->RI);
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

    wattron(saida_pad, COLOR_PAIR(CP_ID) | A_BOLD);

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

    wattroff(saida_pad, COLOR_PAIR(CP_ID) | A_BOLD);

    pad_linha++;

}

static void estagio_EX(RegID_EX *id_ex, RegEX_MEM *ex_mem,
                       RegEX_MEM *ex_mem_old, RegMEM_WB *mem_wb_old,
                       int *cont_fwd)
{
    int fwdA, fwdB;
    forwarding_unit(id_ex->rs, id_ex->rt, ex_mem_old, mem_wb_old, &fwdA, &fwdB);
    signed char valorA = id_ex->A, valorB = id_ex->B;

    wattron(saida_pad, COLOR_PAIR(CP_EX) | A_BOLD);

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
    wattroff(saida_pad, COLOR_PAIR(CP_EX) | A_BOLD);
}

static void estagio_MEM(instro *mem, unsigned char *PC,
                        RegEX_MEM *ex_mem, RegMEM_WB *mem_wb, int *flush)
{
    *flush = 0;
    mem_wb->dest = ex_mem->dest; mem_wb->RegWrite = ex_mem->EscReg;
    mem_wb->MemToReg = ex_mem->MemParaReg; mem_wb->ULA_saida = ex_mem->ULA_saida;
    mem_wb->RDM = 0; mem_wb->valid = 1;

    wattron(saida_pad, COLOR_PAIR(CP_MEM) | A_BOLD);

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

    wattroff(saida_pad, COLOR_PAIR(CP_MEM) | A_BOLD);
}

static void estagio_WB(signed char *reg, RegMEM_WB *mem_wb)
{
    if (!mem_wb->RegWrite) {
        wattron(saida_pad, COLOR_PAIR(CP_WB) | A_BOLD);
        PLOG("[WB ] Nenhum registrador escrito nesse estagio");
        wattroff(saida_pad, COLOR_PAIR(CP_WB) | A_BOLD);
        return;
    }
    signed char valor = mem_wb->MemToReg ? mem_wb->RDM : mem_wb->ULA_saida;
    esc(reg, (int)mem_wb->dest, valor, 1);
    wattron(saida_pad, COLOR_PAIR(CP_WB) | A_BOLD);
    PLOG("[WB ] R%d <- %d (%s)", mem_wb->dest, valor, mem_wb->MemToReg ? "Memoria" : "ULA");
    wattroff(saida_pad, COLOR_PAIR(CP_WB) | A_BOLD);
}

//Cabecalho com status ao vivo 
static void desenha_header(int ciclo, int pc, int instr_ok)
{
    if (!header_win) return;
    werase(header_win);
    wbkgd(header_win, COLOR_PAIR(CP_HEADER));
    box(header_win, 0, 0);
    wattron(header_win, COLOR_PAIR(CP_HEADER) | A_BOLD);
    mvwprintw(header_win, 1, 2,
              "SIMULADOR MIPS 8 bits - PIPELINE");
    char status[120];
    snprintf(status, sizeof status,
             "Ciclo: %-5d  PC: %-3d  Instr. concluidas: %-4d ",
             ciclo, pc, instr_ok);
    int w = getmaxx(header_win);
    int x = w - (int)strlen(status) - 2;
    if (x < 2) x = 2;
    mvwprintw(header_win, 1, x, "%s", status);
    wattroff(header_win, COLOR_PAIR(CP_HEADER) | A_BOLD);
    wrefresh(header_win);
}
//componentes do menu
static void menu_secao(WINDOW *w, int *r, const char *titulo)
{
    wattron(w, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
    mvwprintw(w, (*r)++, 2, "%s", titulo);
    wattroff(w, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
}

static void menu_item(WINDOW *w, int *r, const char *tecla, const char *desc)
{
    wattron(w, COLOR_PAIR(CP_MENU_KEY) | A_BOLD);
    mvwprintw(w, *r, 3, "%-2s", tecla);
    wattroff(w, COLOR_PAIR(CP_MENU_KEY) | A_BOLD);
    wprintw(w, " %s", desc);
    (*r)++;
}

static void log_stage_header(const char *tag, int cp, int PC, int RI)
{
    log_card_estagio(tag, cp, PC, RI);
}

int main(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_cores();

    int altura, largura;
    getmaxyx(stdscr, altura, largura);

   
    int header_h = 3; //barra de titulo no topo 
    int footer_h = 1; // barra fixa no rodape     
    int menu_w   = 31; // coluna do menu (esq.)    
    int right_w  = 46; // coluna de paineis (dir.) 

    body_h = altura - header_h - footer_h;
//largura das colunas
    int center_w = largura - menu_w - right_w;
    if (center_w < 30) {
        right_w  = largura - menu_w - 30;
        center_w = 30;
        if (right_w < 22) { right_w = 22; menu_w = largura - center_w - right_w; }
        if (menu_w  < 18) { menu_w  = 18; center_w = largura - menu_w - right_w; }
    }
    int center_x = menu_w;
    int right_x  = menu_w + center_w;
//alturas
    int pipe_h = 8;
    if (pipe_h > body_h - 4) pipe_h = body_h - 4;
    if (pipe_h < 4) pipe_h = 4;
    int log_h = body_h - pipe_h;
// coluna direita = registradores + estatisticas + programa(mem.instr) 
    int reg_h  = 6;
    int stat_h = 7;
    int rest   = body_h - reg_h - stat_h;
    int dmem_h = rest / 3 - 5;
    if (dmem_h < 3) dmem_h = 3;
    int prog_h = rest - dmem_h;
    if (prog_h < 6) { prog_h = 6; dmem_h = rest - prog_h; if (dmem_h < 3) dmem_h = 3; }
//Log (dentro do painel saida_win)
    saida_y0   = header_h + pipe_h + 1;
    saida_x0   = center_x + 1;
    saida_h    = log_h - 2;
    saida_w_sz = center_w - 2;
    if (saida_h < 1) saida_h = 1;
    if (saida_w_sz < 1) saida_w_sz = 1;

    header_win = newwin(header_h, largura, 0, 0);
    footer_win = newwin(footer_h, largura, altura - footer_h, 0);
    menu_win   = newwin(body_h, menu_w, header_h, 0);

    pipe_win   = newwin(pipe_h, center_w, header_h, center_x);
    saida_win  = newwin(log_h,  center_w, header_h + pipe_h, center_x);

    reg_win    = newwin(reg_h,  right_w, header_h, right_x);
    stat_win   = newwin(stat_h, right_w, header_h + reg_h, right_x);
    prog_win   = newwin(prog_h, right_w, header_h + reg_h + stat_h, right_x);
    dmem_win   = newwin(dmem_h, right_w, header_h + reg_h + stat_h + prog_h, right_x);

    saida_pad  = newpad(PAD_ALTURA, saida_w_sz);

    if (!menu_win || !saida_win || !saida_pad || !header_win || !footer_win
        || !pipe_win || !reg_win || !dmem_win || !stat_win || !prog_win)
    {
        endwin();
        fprintf(stderr, "Erro: terminal muito pequeno (use pelo menos ~100x28).\n");
        return 1;
    }

    scrollok(saida_pad, TRUE);

    desenha_header(0, 0, 0);
    desenha_footer(" Pressione 'q' para retornar ao menu ");

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

    // liga os ponteiros do dashboard ao estado vivo da simulacao 
    g_reg = reg; g_l = &l; g_dat = &dat;
    g_if_id = &if_id; g_id_ex = &id_ex; g_ex_mem = &ex_mem; g_mem_wb = &mem_wb;
    g_pc = &instruction; g_ciclo = &ciclo;
    g_instr_ok = &total_instrucoes_completadas;
    g_fwd = &total_forwarding_evitados; g_stalls = &total_stalls;
    g_flushes = &total_flushes; g_descart = &total_instrucoes_descartadas;
    g_arit = &qtd_aritmetica; g_mem = &qtd_memoria; g_desv = &qtd_desvio;

    render_paineis_globais(); // dashboard

    do
    {
        // atualiza cabecalho + todos os paineis com o estado atual 
        render_paineis_globais();

        werase(menu_win);
        wattron(menu_win, COLOR_PAIR(CP_BORDER));
        box(menu_win, 0, 0);
        wattroff(menu_win, COLOR_PAIR(CP_BORDER));

        int r = 1;
        menu_secao(menu_win, &r, "==== MENU PRINCIPAL ====");
        r++;

        menu_secao(menu_win, &r, "-- ARQUIVOS --");
        menu_item(menu_win, &r, "1", "Carregar instrucoes (.mem)");
        menu_item(menu_win, &r, "2", "Carregar dados (.dat)");
        menu_item(menu_win, &r, "3", "Salvar programa ASM");
        menu_item(menu_win, &r, "4", "Salvar dados DAT");
        r++;

        menu_secao(menu_win, &r, "-- VISUALIZAR --");
        menu_item(menu_win, &r, "5", "Imprimir memorias");
        r++;

        menu_secao(menu_win, &r, "-- EXECUCAO --");
        menu_item(menu_win, &r, "6", "Run (executa ate o fim)");
        menu_item(menu_win, &r, "7", "Step (1 ciclo)");
        menu_item(menu_win, &r, "8", "Back (volta 1 ciclo)");
        menu_item(menu_win, &r, "0", "Sair");
        r++;

        wattron(menu_win, COLOR_PAIR(CP_SEP));
        mvwprintw(menu_win, r++, 2, "Registradores, memorias,");
        mvwprintw(menu_win, r++, 2, "estatisticas e pipeline estao");
        mvwprintw(menu_win, r++, 2, "sempre nos paineis a direita ->");
        wattroff(menu_win, COLOR_PAIR(CP_SEP));
        r++;

        wattron(menu_win, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
        mvwprintw(menu_win, r, 2, "Opcao: ");
        wattroff(menu_win, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
        wrefresh(menu_win);

        echo();
        curs_set(1);
        char buf[16] = {0};
        mvwgetnstr(menu_win, r, 10, buf, sizeof(buf)-1);
        curs_set(0);
        noecho();

        //se o enter tiver vazio nao faz nada (evita sair sem querer com '0'/Enter solto)
        if (buf[0] == '\0') { op = -1; continue; }
        op = atoi(buf);

        switch (op)
        {
        case 1:
            caminho[0] = '\0';
            selecionar_arquivo(caminho);
            pad_reset();
            if (carregar(&l, caminho, &i) != 0)
                PLOG("Erro ao carregar memoria de instrucoes (verifique o caminho: %s).", caminho);
            else {
                PLOG("Memoria de instrucoes carregada: %d instrucoes.", i);
                PLOG(" ");
                PLOG("Agora use:  6=Run   7=Step   5=Imprimir memorias");
            }
            aguardar_scroll();
            break;

        case 2:
            caminho[0] = '\0';
            selecionar_arquivo2(caminho);
            pad_reset();
            if (carregar_dados(&dat, caminho) != 0)
                PLOG("Erro ao carregar memoria de dados (verifique o caminho: %s).", caminho);
            else
                PLOG("Memoria de dados carregada com sucesso.");
            aguardar_scroll();
            break;

        case 5:
            pad_reset();
            {
                char b[40];
                wattron(saida_pad, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
                PLOG("===== MEMORIAS  (cada memoria em uma coluna) =====");
                wattroff(saida_pad, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);

                wattron(saida_pad, COLOR_PAIR(CP_SEP));
                PLOG(" pos | %-8s | %-16s | %s", "DADOS", "INSTRUCAO (bin)", "assembly");
                PLOG("-----+----------+------------------+--------------------------");
                wattroff(saida_pad, COLOR_PAIR(CP_SEP));

                for (int k = 0; k < 256; k++) {
                    asm_str(l.instc[k], b, sizeof b);
                    wmove(saida_pad, pad_linha, 0);
                    wprintw(saida_pad, "%4d | ", k);
                    wattron(saida_pad, COLOR_PAIR(CP_MEM));
                    wprintw(saida_pad, "%-8d", (int)(signed char)dat.instc[k]);
                    wattroff(saida_pad, COLOR_PAIR(CP_MEM));
                    wprintw(saida_pad, " | ");
                    wattron(saida_pad, COLOR_PAIR(CP_IF));
                    print_bin(l.instc[k]);
                    wattroff(saida_pad, COLOR_PAIR(CP_IF));
                    wprintw(saida_pad, " | %s", b);
                    pad_linha++;
                }
            }
            aguardar_scroll();
            break;

        case 3:
            pad_reset();
            save_program_asm(&l, 256, "programa.asm");
            {
                int fim = 0;
                for (int k = 0; k < 256; k++) if (l.instc[k] != 0) fim = k + 1;
                if (fim < 8) fim = 8;

                wattron(saida_pad, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
                PLOG("===== PROGRAMA EM ASSEMBLY (Memoria de Instrucoes) =====");
                wattroff(saida_pad, COLOR_PAIR(CP_MENU_TITLE) | A_BOLD);
                PLOG("%4s | %-16s | %s", "idx", "binario", "assembly");
                PLOG("-----+------------------+--------------------------");

                for (int k = 0; k < fim; k++) {
                    char b[40];
                    asm_str(l.instc[k], b, sizeof b);
                    wmove(saida_pad, pad_linha, 0);
                    wprintw(saida_pad, "%4d | ", k);
                    print_bin(l.instc[k]);
                    wprintw(saida_pad, " | %s", b);
                    pad_linha++;
                }
                PLOG(" ");
                PLOG("Arquivo 'programa.asm' salvo (256 instrucoes).");
            }
            aguardar_scroll();
            break;

        case 4:
            pad_reset();
            save_mem_dat(&dat, "dados_saida.dat");
            PLOG("Arquivo dados_saida.dat salvo.");
            aguardar_scroll();
            break;

        case 6:

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
            pad_reset();
            log_banner_ciclo(ciclo);
            log_timeline(&if_id, &id_ex, &ex_mem, &mem_wb);

            RegEX_MEM ex_mem_old = ex_mem;
            RegMEM_WB mem_wb_old = mem_wb;

            if (mem_wb.valid == 1)
            {
                if (f4 != 0) { PC=0; RI=0; f4--; }
                else if (sp>=3) { PC=pilha[sp-3].if_id.PC-1; RI=pilha[sp-3].if_id.RI; }
                else { PC=0; RI=0; }
                log_stage_header("WB ", CP_WB, PC, RI);
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
                log_stage_header("MEM", CP_MEM, PC, RI);
                if (ex_mem.EscMem) { total_instrucoes_completadas++; qtd_memoria++; }
                else if (ex_mem.beq || ex_mem.jump) { total_instrucoes_completadas++; qtd_desvio++; }
                estagio_MEM(&dat, &instruction, &ex_mem, &mem_wb, &flush);
            }

            if (id_ex.valid == 1)
            {
                if (f2 != 0) { PC=0; RI=0; f2--; }
                else if (sp>=1) { PC=pilha[sp-1].if_id.PC-1; RI=pilha[sp-1].if_id.RI; }
                else { PC=0; RI=0; }
                log_stage_header("EX ", CP_EX, PC, RI);
                estagio_EX(&id_ex, &ex_mem, &ex_mem_old, &mem_wb_old, &total_forwarding_evitados);
            }

            int stall = hazard_detection_unit(&id_ex, if_id.RI);
            if (stall)
            {
                total_stalls++;
                wattron(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
                PLOG("[HAZARD] load-use detectado -> stall 1 ciclo");
                wattroff(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
                estagio_ID(reg, &if_id, &id_ex);
                memset(&id_ex, 0, sizeof(id_ex));
                wattron(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
                PLOG("[ID ] (stall - aguardando lw)");
                PLOG("[IF ] (stall - PC mantido)");
                wattroff(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
            }
            else
            {

                if (if_id.valid == 1)
                {
                    if (f1 != 0) { PC=0; RI=0; f1--; }
                    else { PC=if_id.PC-1; RI=if_id.RI; }
                    log_stage_header("ID ", CP_ID, PC, RI);
                    estagio_ID(reg, &if_id, &id_ex);
                }

                estagio_IF(&l, &instruction, &if_id);
                }

                if (flush)
                {
                    wattron(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD); PLOG("[FLUSH] desvio tomado -> invalidando id_ex, if_id e ex_mem"); wattroff(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
                    memset(&id_ex,0,sizeof(id_ex)); memset(&if_id,0,sizeof(if_id)); memset(&ex_mem,0,sizeof(ex_mem));
                    id_ex.valid=1; if_id.valid=1; ex_mem.valid=1;
                    total_flushes++; total_instrucoes_descartadas += 2;
                    f1=1; f2=2; f3=3; f4=4;
                }

                if (aguardar_scroll() == 'q') break;

            }while(PC!=255);

            break;

        case 7:
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
            pad_reset();
            log_banner_ciclo(ciclo);
            log_timeline(&if_id, &id_ex, &ex_mem, &mem_wb);

            RegEX_MEM ex_mem_old = ex_mem;
            RegMEM_WB mem_wb_old = mem_wb;

            if (mem_wb.valid == 1)
            {
                if (f4 != 0) { PC=0; RI=0; f4--; }
                else if (sp>=3) { PC=pilha[sp-3].if_id.PC-1; RI=pilha[sp-3].if_id.RI; }
                else { PC=0; RI=0; }
                log_stage_header("WB ", CP_WB, PC, RI);
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
                log_stage_header("MEM", CP_MEM, PC, RI);
                if (ex_mem.EscMem) { total_instrucoes_completadas++; qtd_memoria++; }
                else if (ex_mem.beq || ex_mem.jump) { total_instrucoes_completadas++; qtd_desvio++; }
                estagio_MEM(&dat, &instruction, &ex_mem, &mem_wb, &flush);
            }

            if (id_ex.valid == 1)
            {
                if (f2 != 0) { PC=0; RI=0; f2--; }
                else if (sp>=1) { PC=pilha[sp-1].if_id.PC-1; RI=pilha[sp-1].if_id.RI; }
                else { PC=0; RI=0; }
                log_stage_header("EX ", CP_EX, PC, RI);
                estagio_EX(&id_ex, &ex_mem, &ex_mem_old, &mem_wb_old, &total_forwarding_evitados);
            }

            int stall = hazard_detection_unit(&id_ex, if_id.RI);
            if (stall)
            {
                total_stalls++;
                wattron(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
                PLOG("[HAZARD] load-use detectado -> stall 1 ciclo");
                wattroff(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
                estagio_ID(reg, &if_id, &id_ex);
                memset(&id_ex, 0, sizeof(id_ex));
                wattron(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
                PLOG("[ID ] (stall - aguardando lw)");
                PLOG("[IF ] (stall - PC mantido)");
                wattroff(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
            }
            else
            {

                if (if_id.valid == 1)
                {
                    if (f1 != 0) { PC=0; RI=0; f1--; }
                    else { PC=if_id.PC-1; RI=if_id.RI; }
                    log_stage_header("ID ", CP_ID, PC, RI);
                    estagio_ID(reg, &if_id, &id_ex);
                }

                estagio_IF(&l, &instruction, &if_id);
            }

            if (flush)
            {
                wattron(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD); PLOG("[FLUSH] desvio tomado -> invalidando id_ex, if_id e ex_mem"); wattroff(saida_pad, COLOR_PAIR(CP_HAZ) | A_BOLD);
                memset(&id_ex,0,sizeof(id_ex)); memset(&if_id,0,sizeof(if_id)); memset(&ex_mem,0,sizeof(ex_mem));
                id_ex.valid=1; if_id.valid=1; ex_mem.valid=1;
                total_flushes++; total_instrucoes_descartadas += 2;
                f1=1; f2=2; f3=3; f4=4;
            }

            aguardar_scroll();
            break;
        }

        case 8:
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
            /* opcao 11 (estatisticas) removida: painel ESTATISTICAS sempre visivel. */
        case 0:
            break;

        default:
            pad_reset();
            PLOG("Opcao invalida.");
            aguardar_scroll();
            break;
        }

    } while (op != 0);

    delwin(saida_pad);
    delwin(saida_win);
    delwin(menu_win);
    delwin(pipe_win);
    delwin(reg_win);
    delwin(dmem_win);
    delwin(stat_win);
    delwin(prog_win);
    delwin(header_win);
    delwin(footer_win);
    endwin();
    return 0;
}
