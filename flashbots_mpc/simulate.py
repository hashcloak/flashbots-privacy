# Generates fake inputs for the relayers to compute on

import flashbots_types
import random

def generate_random_tx():
    gas = random.randrange(21000, 3000001)
    gas_price = random.uniform(0, 3000001)
    fee_cap = random.uniform(0, 3000001)
    priority_fee = random.uniform(0, 3000001)
    return flashbots_types.Tx(gas, gas_price, fee_cap, priority_fee)

for i in range(10):
    with  open("Player-Data/Input-P{}-0".format(i), 'w+') as f:
        txs = []
        mempool_txs = []
        for j in range(3):
            tx = generate_random_tx()
            txs.append(tx)

        bundle = flashbots_types.Bundle(txs)
        mempool_txs.extend(random.sample(txs + [generate_random_tx() for i in range(10)], 6))
        coinbase_difference = random.uniform(0, 3000001)
        basefee = random.uniform(0, 3000001)
        f.write("bundle: \n" + str(bundle) + "\n")
        f.write("mempool_txs: \n" + "\n".join([str(tx) for tx in mempool_txs]) + "\n")
        f.write("coinbase_difference: " + str(coinbase_difference) + "\n")
        f.write("basefee: " + str(basefee) + "\n")
        
    