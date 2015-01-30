
#include "pin.H"
#include "Triton.h"


/*
 * TODO :
 *
 * reg <- done
 * imm <- done
 *
 * mem <- todo
 *
 * */

static VOID alignStack(std::string insDis, ADDRINT insAddr, CONTEXT *ctx, UINT64 mem)
{
  std::stringstream expr;

  /* Sub RSP */
  if (symbolicEngine->symbolicReg[ID_RSP] != (UINT64)-1)
    expr << "(- #" << std::dec << symbolicEngine->symbolicReg[ID_RSP] << " " << smt2lib_bv(8, REG_Size(REG_RSP)) << ")";
  else
    expr << "(- " << smt2lib_bv(PIN_GetContextReg(ctx, REG_RSP), REG_Size(REG_RSP)) << " " << smt2lib_bv(8, REG_Size(REG_RSP)) << ")";

  SymbolicElement *elem = symbolicEngine->newSymbolicElement(expr);
  symbolicEngine->symbolicReg[ID_RSP] = elem->getID();

  /* Memory reference */
  symbolicEngine->addMemoryReference(mem, elem->getID());

  displayTrace(insAddr, insDis, elem);

  return;
}


static VOID setMemReg(std::string insDis, ADDRINT insAddr, CONTEXT *ctx, REG reg1, UINT64 mem, UINT32 writeSize)
{
  /* Push in memory */
  UINT64 reg1_ID = translatePinRegToID(reg1);

  std::stringstream expr;

  if (symbolicEngine->symbolicReg[reg1_ID] != (UINT64)-1)
    expr << "#" << std::dec << symbolicEngine->symbolicReg[reg1_ID];
  else
    expr << smt2lib_bv(PIN_GetContextReg(ctx, getHighReg(reg1)), writeSize);

  SymbolicElement *elem = symbolicEngine->newSymbolicElement(expr);

  /* We remove the taint by default */
  unsigned int offset = 0;
  for (; offset < writeSize ; offset++){
    taintEngine->untaintAddress(mem+offset);
  }

  /* Then, we taint if the reg is tainted */
  if (taintEngine->isRegTainted(reg1_ID)){
    for (offset = 0; offset < writeSize ; offset++){
      taintEngine->taintAddress(mem+offset);
    }
  }

  /* Memory reference */
  symbolicEngine->addMemoryReference(mem, elem->getID());

  displayTrace(0, "", elem);
}


static VOID setMemImm(std::string insDis, ADDRINT insAddr, CONTEXT *ctx, UINT64 imm, UINT64 mem, UINT32 writeSize)
{
  std::stringstream expr;

  expr << smt2lib_bv(imm, writeSize);

  SymbolicElement *elem = symbolicEngine->newSymbolicElement(expr);

  /* We remove the taint by default */
  unsigned int offset = 0;
  for (; offset < writeSize ; offset++){
    taintEngine->untaintAddress(mem+offset);
  }

  /* Memory reference */
  symbolicEngine->addMemoryReference(mem, elem->getID());

  displayTrace(insAddr, insDis, elem);
}


VOID pushReg(std::string insDis, ADDRINT insAddr, CONTEXT *ctx, REG reg1, UINT64 mem, UINT32 writeSize)
{
  if (_analysisStatus == LOCKED || insAddr > LIB_MAPING_MEMORY)
    return;

  alignStack(insDis, insAddr, ctx, mem);
  setMemReg(insDis, insAddr, ctx, reg1, mem, writeSize);

  return;
}


VOID pushImm(std::string insDis, ADDRINT insAddr, CONTEXT *ctx, UINT64 imm, UINT64 mem, UINT32 writeSize)
{
  if (_analysisStatus == LOCKED || insAddr > LIB_MAPING_MEMORY)
    return;
 
  alignStack(insDis, insAddr, ctx, mem);
  setMemImm(insDis, insAddr, ctx, imm, mem, writeSize);

  return;
}

