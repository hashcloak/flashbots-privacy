#ifndef OT_RECEIVER_H
#define OT_RECEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>

#include "sc25519.h"
#include "ge4x.h"
#include "ot_config.h"

struct ot_receiver
{
	unsigned char S_pack[ PACKBYTES ];
	ge4x S;
	ge4x table[ 64/DIST ][8];

	// temporary

	ge4x xB;
	sc25519 x[4];
};

typedef struct ot_receiver SIMPLEOT_RECEIVER;

void receiver_maketable(SIMPLEOT_RECEIVER *);
void receiver_procS(SIMPLEOT_RECEIVER *);
bool receiver_procS_check(SIMPLEOT_RECEIVER *);
void receiver_rsgen(SIMPLEOT_RECEIVER *, unsigned char *, unsigned char *);
void receiver_keygen(SIMPLEOT_RECEIVER *, unsigned char (*)[HASHBYTES]);

#ifdef __cplusplus
}
#endif

#endif //ifndef OT_RECEIVER_H

