# Generates fake inputs for the relayers to compute on

import flashbots_types
import random

def generate_random_tx():
    gas = random.randrange(21000, 3000001)
    gas_price = random.uniform(0, 3000001)
    fee_cap = random.uniform(0, 3000001)
    priority_fee = random.uniform(0, 3000001)
    return flashbots_types.Tx(gas, gas_price, fee_cap, priority_fee)

mempool_txs = []
mempool_txs.extend([generate_random_tx() for i in range(100)])

for i in range(10):
    with  open("Player-Data/Input-P{}-0".format(i), 'w+') as f:
        txs = []
        for j in range(6):
            tx = generate_random_tx()
            txs.append(tx)

        bundle = flashbots_types.Bundle(txs)
        mempool_txs.extend(random.sample(txs, 3))
        coinbase_difference = random.uniform(0, 3000001)
        basefee = random.uniform(0, 3000001)
        f.write(str(bundle) + "\n")
        f.write(str(coinbase_difference) + "\n")
        f.write(str(basefee) + "\n")
with open("../MP-SPDZ/Programs/Public-Input/bundle_scoring", "w+") as m:
    m.write("\n".join([str(tx) for tx in mempool_txs]))