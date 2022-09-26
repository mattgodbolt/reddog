#ifndef _SPARKS_H
#define _SPARKS_H

// Reset/reinitialise the sparks
void rResetSparks (void);

// Draw the sparks
void rDrawSparks (void);
void rUpdateSparks (void);

// Strat glue code
void MakeSpark (STRAT *strat, float x, float y, float z);

// Shield sparks
void MakeShieldSpark (void);
void MakeShieldSparkTable (Model *shieldModel);
extern Matrix ShieldSparkMatrix;

#endif
