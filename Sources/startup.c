/* Código de Startup ANSI-C para HCS12, small memory model. Versión minimalista. */

/* El vector de reset apunta a la función _Startup (más abajo). Ese código hace lo siguiente:
    1) inicializa stack pointer:
            - Para eso, el linker define el símbolo __SEG_END_SSTACK, que indica el valor a cargar en SP
       
    2) pone en cero las áreas de RAM donde se encuentran las variables  
            - Cuando hago "static char var;", 'var' debe valer cero al inicio del programa.
               Como 'var' está reservado en RAM, y la RAM puede tener cualquier valor cuando se resetea el micro,
               entonces alguien debe ponerla en cero antes de que se llame a 'main'. Este trabajo lo hace
               el código de _Startup.
    
    3) copia los valores de las variables inicializadas (de ROM a RAM)
            - Cuando hago "static char buff[] = {11, 22};", 'buff' debe valer {11,22} al inicio del programa.
               Como 'buff' está reservado en RAM, alguien debe cargar los valores {11,22} en 'buff' antes de
               que se llame a 'main'. El código de _Startup copia los valores {11,22} de ROM a RAM.
    
    4) jump a main                                 
            - Como, en teoría, 'main' no retorna nunca, no vamos a gastar stack con un JSR. Directamente hacemos
               un JMP.
*/


/* attention: the linker scans the debug information for these structures */
/* to obtain the available fields and their sizes. */
/* So do not change the names in this file. */
typedef struct _Range {
  unsigned char * beg; 
  int size;
} _Range;

typedef struct _Copy {
    int size; 
    unsigned char * dest;
} _Copy;

    
/* _startupData está reservada en ROM, e inicializada por el linker */
#pragma DATA_SEG STARTUP_DATA 
struct {
     unsigned int   nofZeroOuts;    /* number of zero out ranges */
     _Range         *pZeroOut;      /* vector of ranges with nofZeroOuts elements */
     _Copy          *toCopyDownBeg; /* rom-address where copydown-data begins */
} _startupData;
#pragma DATA_SEG DEFAULT


/* símbolos definidos externos a este archivo */
extern char __SEG_END_SSTACK[];     /* este símbolo lo define el linker */
void main(void);                    /* prototipo de main */

/* ¡Hay que decirle al Linker que cargue el vector de reset con la dirección de _Startup! (ver ivt.c) */
#pragma MESSAGE DISABLE C12053 /* Stack-pointer change not in debugging-information */
#pragma NO_ENTRY    /* deshabilita la generación de código "extra" de entrada. */
#pragma NO_EXIT     /* deshabilita la generación de código "extra" de salida. */
#pragma NO_RETURN   /* deshabilita la generación del RTS */
void _Startup(void) 
{
    __asm {
    
/* estos NOPs no son necesarios, pero el simulador tiene un BUG
    que se soluciona de esta manera: */
                NOP
                NOP
                NOP
                NOP
                
/* inicializar stack pointer */
                LDS #__SEG_END_SSTACK;
   
/* zero out */
ZeroOut:
                LDX   _startupData.pZeroOut    ; X=*pZeroOut
                LDY   _startupData.nofZeroOuts ; Y=nofZeroOuts
                BEQ   CopyDown                 ; if nothing to zero out

NextZeroOut:    PSHY                           ; save nofZeroOuts
                LDY   2,X+                     ; Y=start address, X=X+2
                LDD   2,X+                     ; D=byte count, X=X+2

NextWord:       CLR   1,Y+                     ; clear memory byte
                DBNE  D, NextWord              ; dec byte count

                PULY                           ; restore nofZeroOuts
                DEY                            ; dec nofZeroOuts
                BNE  NextZeroOut


/* copy down */
CopyDown:
                LDX   _startupData.toCopyDownBeg ; load address of copy down desc.

NextBlock:      LDD   2,X+                     ; D=size of init-data, X=X+2
                BEQ   Done                     ; end of copy down desc.
                LDY   2,X+                     ; Y=destination address, X=X+2

Copy:           MOVB  1,X+,1,Y+                ; move a byte from ROM to the data area
                DBNE  D,Copy                   ; copy-byte loop

                BRA   NextBlock

/* jump a main(). ¡Asegurarse de no ejecutar NUNCA un RTS en main()! */
Done:
                JMP main;

    }
}
