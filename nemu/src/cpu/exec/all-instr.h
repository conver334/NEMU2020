#include "prefix/prefix.h"

#include "data-mov/mov.h"
#include "data-mov/xchg.h"

#include "arith/dec.h"
#include "arith/inc.h"
#include "arith/neg.h"
#include "arith/imul.h"
#include "arith/mul.h"
#include "arith/idiv.h"
#include "arith/div.h"

#include "logic/and.h"
#include "logic/or.h"
#include "logic/not.h"
#include "logic/xor.h"
#include "logic/sar.h"
#include "logic/shl.h"
#include "logic/shr.h"
#include "logic/shrd.h"

#include "string/rep.h"

#include "misc/misc.h"

#include "special/special.h"

#include "con-trans/call.h"
#include "data-mov/push.h"
#include "logic/test.h"
#include "con-trans/je.h"
#include "arith/cmp.h"
#include "data-mov/pop.h"
#include "con-trans/ret.h"
#include "arith/sub.h"
#include "con-trans/jmp.h"
#include "con-trans/jbe.h"
#include "data-mov/leave.h"
#include "logic/setne.h"
#include "arith/add.h"
#include "arith/adc.h"
#include "data-mov/movext.h"
#include "con-trans/jne.h"
#include "con-trans/jle.h"
#include "con-trans/jg.h"
#include "con-trans/jl.h"
#include "con-trans/jge.h"
#include "arith/sbb.h"
#include "con-trans/ja.h"
#include "string/movs.h"
#include "string/lods.h"
#include "string/scas.h"
#include "string/stos.h"
#include "con-trans/jns.h"
#include "con-trans/js.h"
#include "data-mov/cltd.h"
#include "data-mov/lgdt.h"
#include "logic/cld.h"
#include "logic/std.h"


#include "data-mov/lidt.h"
#include "intr/intr.h"


#include "logic/sete.h"
#include "data-mov/cmovcc.h"
#include "data-mov/pusha.h"
#include "data-mov/popa.h"