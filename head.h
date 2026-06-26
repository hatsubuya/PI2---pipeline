
#ifndef HEAD_H
#define HEAD_H
#include <ncurses.h>

extern WINDOW *saida_pad;
extern int pad_linha;
extern int saida_h; // Importante para o cálculo do scroll

#endif
//sinais
typedef enum
{

    REG_DST,
    JUMP,
    MEM_READ,
    MEM_WRITE,
    BRANCH,
    ALU_SRC,
    MEM_TO_REG,
    REG_WRITE
} SinalControle;

//struct de memoria
typedef struct {

    unsigned short instc[256];

    int n;

} instro;


//primeiro registrador pipeline
typedef struct {

    // instrução atual
    unsigned short RI;

    //proximo endereço de instrução
    unsigned char  PC;

    //flag de validação
    int valid;

} RegIF_ID;


//segundo registrador pipeline
typedef struct
{
    //sinais wb
    int EscReg;

    int MemParaReg;


    //sinal m
    int EscMem;


    //sinais ex
    int RegDst;

    int ULAOp;

    int ULAFonte;

    int beq;

    int jump;


    //proximo endereço de instrução
    unsigned char PC;

    signed char   A;  // Dado lido #1

    signed char   B;  // Dado lido #2


    //sinais

    signed char immx;

    unsigned char rt; // [8-6]

    unsigned char rd; // [5-3]

    int valid;

    //
    unsigned char rs;

} RegID_EX;









typedef struct {


    // enderço ao qual o salto ocorrera
    unsigned char add_result;

    // valor de saida da ula
    signed char  ULA_saida;

    //registrador B
    signed char  B;

    //registradro de destino
    unsigned char dest;


    //sinais de controle

    int EscReg;

    int MemParaReg;

    int EscMem;

    int beq;

    int jump;


    //flags
    int zero;

    int valid;





} RegEX_MEM;


typedef struct
{

    signed char   RDM;

    signed char   ULA_saida;

    unsigned char dest;

    int MemToReg;

    int RegWrite;

    int valid;




} RegMEM_WB;




//imprimir binario
void imprime_bits(unsigned char valor);

void print_bin(unsigned short x);

void escrever_no_pad(const char *format, ...);
//PC
unsigned char busca(unsigned char instruction);
unsigned char jump(unsigned char instruct);
unsigned char branch(unsigned char instruct,unsigned char imm);

//memoria de instruções
void print_mem_inst(instro *l, int tamanho);

int carregar(instro *l, const char *nome_arquivo,int *i);

unsigned short ler(instro *l, unsigned char instruct);

//banco de registradores
void print_regs(signed char reg[8]);

int iniat(signed char reg[8]);

int ler_regs(signed char reg[8], signed char A, signed char B,
             signed char *outA, signed char *outB);

int Rdest(int Sinal, signed char A, signed char B);

int esc(signed char reg[8], int dest, signed char A,int RegWrite);


//controle

void print_sinais(int RegDst,int ULAOp,int ULAFonte,int beq,int jump,int EscMem,int EscReg,int MemParaReg);

void Decodifica_controle(unsigned char opcode,int *RegDst,int *ULAOp,int *ULAFonte,int *beq,int *jump,int *EscMem,int *EscReg,int *MemParaReg, int funct);


//ULA

int ulamx(int Sinal, signed char A, signed char B, signed char immx);

int ula(int ULAop, signed char A, signed char B, int *overflow, int *zero);

void Estender(unsigned char imm, signed char *immx);

int wrmux(int Sinais[16], int result);

void tipo(int ULAop);

void Tipo2(unsigned char opcode, int *ULAop);


//memoria de dados

int Store(instro *dat, signed char B, signed char mem);
int temp(signed char A, signed char *mem);
void init_mem_incremental(instro *dat);
int carregar_dados(instro *dat, const char *nome_arquivo);
void print_mem(instro *dat);
void save_mem_dat(instro *dat, const char *nome_arquivo);
unsigned short load(instro *mem, unsigned char endereco);


//organizador de impressões asembly
void print_asm(unsigned short instr);

void print_program(instro *l, int tamanho);

void save_program_asm(instro *l, int tamanho, const char *nome_arquivo);
