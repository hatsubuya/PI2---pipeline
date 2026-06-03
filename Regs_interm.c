#include <stdio.h>
#include "Head.h"
void init_regs_interm(RegsInterm *rt)
{
    rt->RI        = 0;
    rt->RDM       = 0;
    rt->A         = 0;
    rt->B         = 0;
    rt->ULA_saida = 0;
}
void print_regs_interm(RegsInterm *rt)
{
    printf("\n--- REGISTRADORES INTERMEDIARIOS ---\n");
    printf("RI        = %u\n",  rt->RI);
    printf("A         = %d\n",  rt->A);
    printf("B         = %d\n",  rt->B);
    printf("ULA_saida = %d\n",  rt->ULA_saida);
    printf("RDM       = %d\n",  rt->RDM);
    printf("---------------------------------\n");
}
