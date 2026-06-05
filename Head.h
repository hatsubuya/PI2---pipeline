

/* memoria unificada */
typedef struct
{
    unsigned short instc[256];
    int n;
} instro;

/* contagem de instrucoes */
typedef struct {

    int tipoR;

    int addi;

    int lw;

    int sw;

    int jump;

    int beq;

} instrucoes;

typedef struct {

    unsigned short RI;

    unsigned char  PC;

    int valid;

} RegIF_ID;

typedef struct
{
    unsigned char opcode;

    unsigned char rs;

    unsigned char rt;

    unsigned char rd;

    unsigned char funct;

    unsigned char addr;

    signed char   immx;

    signed char   A;

    signed char   B;

    unsigned char PC;

    int valid;

} RegID_EX;

typedef struct
{

    int ULA_saida;

    int zero;

    signed char   B;

    unsigned char rt;

    unsigned char rd;

    int RegWrite;

    int MemWrite;

    int MemRead;

    int MemToReg;

    int RegDst;

    int Branch;

    unsigned char PC_branch;

    int Jump;

    unsigned char addr;

    int valid;

} RegEX_MEM;

typedef struct
{

    signed char RDM;

    int ULA_saida;

    unsigned char rt;

    unsigned char rd;

    int RegWrite;

    int MemToReg;

    int RegDst;

    int valid;

} RegMEM_WB;

typedef struct
{

    signed char reg[8];

    unsigned char PC;

    int n_ciclo;

    int n_instr;

    RegIF_ID if_id;

    RegID_EX id_ex;

    RegEX_MEM ex_mem;

    RegMEM_WB mem_wb;

    unsigned short memoria[256];

    instrucoes contaInstrucoes;

} Snapshot;

typedef enum
{
    REG_DST      = 0,
    JUMP         = 1,
    MEM_READ     = 2,
    MEM_WRITE    = 3,
    BRANCH       = 4,
    ALU_SRC      = 5,
    MEM_TO_REG   = 6,
    REG_WRITE    = 7,
    IorD         = 8,
    IR_ESC       = 9,
    PC_ESC       = 10,
    PC_FONTE0    = 11,
    PC_FONTE1    = 12,
    ULA_FONTE_A  = 13,
    ULA_FONTE_B0 = 14,
    ULA_FONTE_B1 = 15

} SinalControle;

void print_regs(signed char *reg);
int iniat(signed char *reg);
int read(signed char *reg, signed char rs, signed char rt, signed char *outA, signed char *outB);
int Rdest(int *Sinais, signed char rd, signed char rt);
int esc(signed char *reg, int dest, signed char valor, int RegWrite);

int ulamx(int *Sinais, signed char A, signed char B, signed char immx);
int ula(int ULAop, signed char A, signed char B, int *overflow, int *zero);
void Estender(unsigned char imm, signed char *immx);
int  wrmux(int *Sinais, int result);
void tipo(int ULAop);
void Tipo2(unsigned char opcode, int *ULAop);

void print_sinais(int Sinais[16]);
void Decodifica_estado(int estado, int *Sinais);
int  proximo_estado(int estado_atual, unsigned char opcode);

int carregar_unificado(instro *mem, const char *nome_arquivo);
unsigned short ler_unificada(instro *mem, unsigned char endereco);
void print_mem_unificada(instro *mem);
int Store(instro *mem, signed char endereco, signed char valor);
void save_mem_dat(instro *mem, const char *nome_arquivo);

void print_asm(unsigned short instr);
void print_program(instro *mem, int tamanho);
void save_program_asm(instro *mem, int tamanho, const char *nome_arquivo);

unsigned char busca(unsigned char PC);
unsigned char jump(unsigned char addr);
unsigned char branch(unsigned char PC, unsigned char imm);

void push_pipeline(Snapshot *pilha, int *sp,signed char *reg, unsigned char PC,int n_ciclo, int n_instr,RegIF_ID *if_id, RegID_EX *id_ex,RegEX_MEM *ex_mem, RegMEM_WB *mem_wb,instro *mem, instrucoes contaInstrucoes);
void pop_pipeline(Snapshot *pilha, int *sp,signed char *reg, unsigned char *PC,int *n_ciclo, int *n_instr,RegIF_ID *if_id, RegID_EX *id_ex,RegEX_MEM *ex_mem, RegMEM_WB *mem_wb,instro *mem, instrucoes *contaInstrucoes);

void print_bin(unsigned short x);
void print_bin8(unsigned char x);


