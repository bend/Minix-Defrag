#include <machine/asm.h>

IMPORT(_defrag)
ENTRY(defrag)
	jmp	_C_LABEL(_defrag)

