simulador: main.c Back.c Controle.c ULA.c Banco_reg.c Pc.c bin.c mem_dat.c mem_ins.c asembly.c
	gcc main.c Back.c Controle.c ULA.c Banco_reg.c Pc.c bin.c mem_dat.c mem_ins.c asembly.c -o simulador -lncurses

clean:
	rm -f simulador
