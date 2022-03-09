CC = gcc
CFLAGS = -O3 -Wall -Wextra
AS = $(CC) $(CFLAGS) -c

OBJS+= Keccak-simple.o
OBJS+= randombytes.o
OBJS+= cpucycles.o

OBJS+= network.o
OBJS+= ot_sender.o
OBJS+= ot_receiver.o

OBJS+= sc25519_random.o
OBJS+= sc25519_from32bytes.o
OBJS+= sc25519_window4.o

OBJS+= fe25519_add.o
OBJS+= fe25519_freeze.o
OBJS+= fe25519_getparity.o
OBJS+= fe25519_invert.o
OBJS+= fe25519_iseq_vartime.o
OBJS+= fe25519_mul.o
OBJS+= fe25519_neg.o
OBJS+= fe25519_nsquare.o
OBJS+= fe25519_pack.o
OBJS+= fe25519_pow2523.o
OBJS+= fe25519_setint.o
OBJS+= fe25519_square.o
OBJS+= fe25519_sub.o
OBJS+= fe25519_unpack.o

OBJS+= ge25519_pack.o
OBJS+= ge25519_unpack.o
OBJS+= ge25519_setneutral.o
OBJS+= ge25519_dbl_p1p1.o
OBJS+= ge25519_add_p1p1.o
OBJS+= ge25519_nielsadd2.o
OBJS+= ge25519_p1p1_to_p2.o
OBJS+= ge25519_p1p1_to_p3.o
OBJS+= ge25519_double.o
OBJS+= ge25519_add.o
OBJS+= ge25519_scalarmult_base.o
OBJS+= ge25519_scalarmult.o
OBJS+= ge25519_lookup.o
OBJS+= ge25519_lookup_niels.o

OBJS+= ge4x.o 
OBJS+= gfe4x.o 
OBJS+= gfe4x_add.o 
OBJS+= gfe4x_nsquare.o 
OBJS+= gfe4x_square.o 
OBJS+= gfe4x_getparity.o
OBJS+= gfe4x_iseq_vartime.o
OBJS+= gfe4x_mul.o 
OBJS+= gfe4x_pow2523.o
OBJS+= gfe4x_sub.o 

OBJS+= ge4x_double_p1p1.o
OBJS+= ge4x_add_p1p1.o
OBJS+= ge4x_niels_add_p1p1.o
OBJS+= ge4x_unpack_vartime.o
OBJS+= ge4x_pack.o
OBJS+= ge4x_lookup_niels.o
OBJS+= ge4x_lookup.o

OBJS+= consts.o
OBJS+= consts4x.o

######################################################

all: ot_sender_test ot_receiver_test libsimpleot

libsimpleot: $(OBJS)
	$(AR) -crs libsimpleot.a $(OBJS)

ot_sender_test: ot_sender_test.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ 

ot_receiver_test: ot_receiver_test.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ 

%.o: %.c 
	$(CC) $(CFLAGS) -c $<

%.o: %.S
	$(AS) $<

######################################################

.PHONY: clean

clean:
	-rm -f ot_sender_test 
	-rm -f ot_receiver_test
	-rm -f *.o
	-rm -f libsimpleot.a

