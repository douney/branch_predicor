#include "predictor.h"


/////////////// STORAGE BUDGET JUSTIFICATION ////////////////

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// Constructeur du prédicteur
PREDICTOR::PREDICTOR(char *prog, int argc, char *argv[])
{
   // La trace est tjs présente, et les arguments sont ceux que l'on désire
   if (argc != 2) {
      fprintf(stderr, "usage: %s <trace> pcbits countbits\n", prog);
      exit(-1);
   }

   uint32_t pcbits    = strtoul(argv[0], NULL, 0);
   uint32_t countbits = strtoul(argv[1], NULL, 0);

   nentries    = (1 << pcbits);        // nombre d'entrées dans la table
   pcmask      = (nentries - 1);       // masque pour n'accéder qu'aux bits significatifs de PC
   countmax    = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
   table       = new uint32_t[nentries]();
   historique  = 0;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction(UINT64 PC)
{
   uint32_t v = table[PC & pcmask];
   return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   UpdatePredictor2(PC, opType, resolveDir, predDir, branchTarget);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction0(UINT64 PC)
{
   return GetPrediction2(PC);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor0(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   uint32_t v = table[PC & pcmask];
   table[PC & pcmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction1(UINT64 PC)
{
   uint32_t v = table[historique & pcmask];
   return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor1(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   historique = (historique << 1) + ((resolveDir == TAKEN) ? 1 : 0);
   uint32_t v = table[historique & pcmask];
   if (v == 1 && resolveDir == NOT_TAKEN) {
      table[historique & pcmask] = 3;
   } else if (v == 2 && resolveDir == TAKEN) {
      table[historique & pcmask] = 0;
   } else {
      table[historique & pcmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
   }
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction2(UINT64 PC)
{
   uint32_t v = table[PC & pcmask];
   return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor2(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   uint32_t v = table[PC & pcmask];
   if (v == 1 && resolveDir == NOT_TAKEN) {
      table[PC & pcmask] = 3;
   } else if (v == 2 && resolveDir == TAKEN) {
      table[PC & pcmask] = 0;
   } else {
      table[PC & pcmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
   }
}

/***********************************************************/

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
   // This function is called for instructions which are not
   // conditional branches, just in case someone decides to design
   // a predictor that uses information from such instructions.
   // We expect most contestants to leave this function untouched.
}

