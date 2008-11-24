#include "kedbg.h"
#include "interface.h"


/* /\**************** Handler for vectors ****************\/ */
void            kedbg_resetstep_ia32(void)
{
  PROFILER_INQ();
  e2dbgworld.curthread->step = FALSE;
  PROFILER_OUTQ();
}


void            kedbg_setstep_ia32(void)
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);  
  e2dbgworld.curthread->step = TRUE;
  PROFILER_OUTQ();
}


eresi_Addr	*kedbg_getfp_ia32(void)
{
  gdbwrap_t     *loc = gdbwrap_current_get();

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  fprintf(stderr, "%s INTO \n", __PRETTY_FUNCTION__);
  fflush(stdout);

  /* First update all the reg. */
  gdbwrap_readgenreg(loc);

  printf("Returning: %#x\n", loc->reg32.ebp);
  PROFILER_ROUTQ((eresi_Addr *)loc->reg32.ebp);
}


void            *kedbg_bt_ia32(void *frame)
{
  PROFILER_INQ();
  NOT_USED(frame);
  fprintf(stderr, "%s NOT IMPLEMENTED (YET) \n", __PRETTY_FUNCTION__);
  fflush(stdout);

  PROFILER_ROUTQ(NULL);
}


void            *kedbg_getret_ia32(void *frame)
{
  gdbwrap_t     *loc = gdbwrap_current_get();

  PROFILER_INQ();
  fprintf(stderr, "%s INTO \n", __PRETTY_FUNCTION__);
  fflush(stdout);

  PROFILER_ROUTQ(gdbwrap_readmemory(*(la32*)frame, DWORD_IN_BYTE, loc));
}


/**
 * Set up a breakpoint. We have to change the memory on the server
 * side, thus we need to save the value we change.
 */
int             kedbg_setbp(elfshobj_t *f, elfshbp_t *bp)
{
  gdbwrap_t     *loc = gdbwrap_current_get();

  PROFILER_INQ();
  NOT_USED(f);
  gdbwrap_setbp(bp->addr, bp->savedinstr, loc);
  
  PROFILER_ROUTQ(0);
}


int             kedbg_delbp(elfshbp_t *bp)
{
  gdbwrap_t     *loc = gdbwrap_current_get();

  PROFILER_INQ();
  gdbwrap_delbp(bp->addr, bp->savedinstr, loc);

  PROFILER_ROUTQ(0);
}


void            kedbg_print_reg(void)
{
  gdbwrap_t     *loc = gdbwrap_current_get();
  gdbwrap_gdbreg32 *reg;

  PROFILER_INQ();
  reg = &loc->reg32;
  e2dbg_register_dump("EAX", reg->eax);
  e2dbg_register_dump("EBX", reg->ebx);
  e2dbg_register_dump("ECX", reg->ecx);
  e2dbg_register_dump("EDX", reg->edx);
  e2dbg_register_dump("ESI", reg->esi);
  e2dbg_register_dump("EDI", reg->edi);
  e2dbg_register_dump("ESP", reg->esp);
  e2dbg_register_dump("EBP", reg->ebp);
  e2dbg_register_dump("EIP", reg->eip);
  PROFILER_OUTQ();
}

void            *kedbg_readmema(elfshobj_t *file, eresi_Addr addr,
				void *buf, unsigned size)
{
  gdbwrap_t     *loc = gdbwrap_current_get();
  char          *ret;
  char          *charbuf = buf;
  unsigned      i;

  PROFILER_INQ();
  NOT_USED(file);
  ret = gdbwrap_readmemory(addr, size, loc);

  /* gdbserver sends a string, we need to convert it. Note that 2
     characters = 1 real Byte.*/
  for (i = 0; i < size; i++) 
    charbuf[i] = (u_char)gdbwrap_atoh(ret + 2 * i, 2);

  PROFILER_ROUTQ(charbuf);
}


void            *kedbg_readmem(elfshsect_t *sect)
{
  void *ptr;

  PROFILER_INQ();
  if(!elfsh_get_section_writableflag(sect->shdr))
    ptr = sect->data;
  else
    {
      ptr = malloc(sect->shdr->sh_size);
      kedbg_readmema(sect->parent, sect->shdr->sh_addr, ptr, sect->shdr->sh_size);
    }

  PROFILER_ROUTQ(ptr);
}


int             kedbg_writemem(elfshobj_t *file, eresi_Addr addr, void *data,
			       unsigned size)
{
  gdbwrap_t     *loc = gdbwrap_current_get();

  PROFILER_INQ();
  NOT_USED(file);
  gdbwrap_writememory(addr, data, size, loc);

  PROFILER_ROUTQ(0);
}


/* Get %IP */
eresi_Addr      *kedbg_getpc_ia32(void)
{
  gdbwrap_t     *loc = gdbwrap_current_get();

  PROFILER_INQ();
  /* First update all the reg. */
  gdbwrap_readgenreg(loc);
  PROFILER_ROUTQ((eresi_Addr *)&loc->reg32.eip);
}


/**
 * We first sync the registers with the server, then we write the new
 * set ones. They'll be sent to the server when we'll do a "cont".
 */
void            kedbg_set_regvars_ia32(void)
{
  gdbwrap_t     *loc = gdbwrap_current_get();
  gdbwrap_gdbreg32   *reg;

  PROFILER_INQ();
  reg =  gdbwrap_readgenreg(loc);

  E2DBG_SETREG(E2DBG_EAX_VAR, reg->eax);
  E2DBG_SETREG(E2DBG_EBX_VAR, reg->ebx);
  E2DBG_SETREG(E2DBG_ECX_VAR, reg->ecx);
  E2DBG_SETREG(E2DBG_EDX_VAR, reg->edx);
  E2DBG_SETREG(E2DBG_ESI_VAR, reg->esi);
  E2DBG_SETREG(E2DBG_EDI_VAR, reg->edi);
  E2DBG_SETREG(E2DBG_SP_VAR,  reg->esp);
  E2DBG_SETREG(E2DBG_FP_VAR,  reg->ebp);
  E2DBG_SETREG(E2DBG_PC_VAR,  reg->eip);
  PROFILER_OUTQ();
}


void            kedbg_get_regvars_ia32(void)
{
  gdbwrap_t     *loc = gdbwrap_current_get();

  PROFILER_INQ();
  gdbwrap_readgenreg(loc);
  

  E2DBG_GETREG(E2DBG_EAX_VAR, loc->reg32.eax);
  E2DBG_GETREG(E2DBG_EBX_VAR, loc->reg32.ebx);
  E2DBG_GETREG(E2DBG_ECX_VAR, loc->reg32.ecx);
  E2DBG_GETREG(E2DBG_EDX_VAR, loc->reg32.edx);
  E2DBG_GETREG(E2DBG_ESI_VAR, loc->reg32.esi);
  E2DBG_GETREG(E2DBG_EDI_VAR, loc->reg32.edi);
  E2DBG_GETREG(E2DBG_SP_VAR,  loc->reg32.esp);
  //E2DBG_GETREG(E2DBG_SSP_VAR, loc->reg32.eax);
  E2DBG_GETREG(E2DBG_FP_VAR,  loc->reg32.ebp);
  E2DBG_GETREG(E2DBG_PC_VAR,  loc->reg32.eip);
  PROFILER_OUTQ();
}

/* Todo:
   - display
   - set $eip 0x12345678 -> cont -> eip doit etre a 0x12... On l'appelera depuis le cont.
   - voir avec bochs
   - mettre commande not display + commande.
   
(kedbg-0.82-a4-dev@local) # ok good - donc je te propose ce programme:
OK (kedbg-0.82-a4-dev@local) # - rajouter lappel a setregs
(kedbg-0.82-a4-dev@local) # - rajouter lappel a display
(kedbg-0.82-a4-dev@local) # - cleaner loutput pour que ce soit la meme chose que e2dbg  
(kedbg-0.82-a4-dev@local) # - cleaner les messages derrors pour que ca utilise PROFILER_ERR() correctement et quon ait pas des msg incomprehensible de gdbwrap ou des assert

*/