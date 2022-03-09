#include "ot_receiver.h"

#include <stdlib.h>

#include "ge25519.h"
#include "ge4x.h"
#include "to_4x.h"

void receiver_maketable(SIMPLEOT_RECEIVER * r)
{
	ge4x_maketable(r->table, &r->S, DIST);
}

void receiver_procS(SIMPLEOT_RECEIVER * r)
{
	bool success = receiver_procS_check(r);
	if (!success)
	{
		fprintf(stderr, "Error: point decompression failed\n");
		exit(-1);
	}
}

bool receiver_procS_check(SIMPLEOT_RECEIVER * r)
{
	int i;

	ge25519 S;

	if (ge25519_unpack_vartime(&S, r->S_pack) != 0)
	{ 
		return false;
	}

	for (i = 0; i < 3; i++) ge25519_double(&S, &S); // 8S

	ge25519_pack(r->S_pack, &S); // E_1(S)
	ge_to_4x(&r->S, &S);

	return true;
}

void receiver_rsgen(SIMPLEOT_RECEIVER * r, 
                     unsigned char * Rs_pack,
                     unsigned char * cs)
{
	int i;

	ge4x P;
	
	//

	for (i = 0; i < 4; i++) sc25519_random(&r->x[i], 1);
	ge4x_scalarsmults_base(&r->xB, r->x); // 8x^iB

	ge4x_sub(&P, &r->S, &r->xB); // 8S - 8x^iB
	ge4x_cmovs(&r->xB, &P, cs);

	ge4x_pack(Rs_pack, &r->xB); // E^1(R^i)

}

void receiver_keygen(SIMPLEOT_RECEIVER * r, 
                     unsigned char (*keys)[HASHBYTES])
{
	int i;

	unsigned char Rs_pack[ 4 * PACKBYTES ];
	ge4x P;
	
	//

	for (i = 0; i < 3; i++) ge4x_double(&r->xB, &r->xB);
	ge4x_pack(Rs_pack, &r->xB); // E_2(R^i)

	ge4x_scalarsmults_table(&P, r->table, r->x, DIST); // 64x^iS

	ge4x_hash(keys[0], r->S_pack, Rs_pack, &P); // E_2(x^iS)
}

