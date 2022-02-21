# Generates fake inputs for the relayers to compute on

from email.mime import base
import flashbots_types
import random

def generate_random_tx():
    gas = random.randrange(21000, 3000001)
    gas_price = random.uniform(0, 3000001)
    fee_cap = random.uniform(0, 3000001)
    priority_fee = random.uniform(0, 3000001)
    return flashbots_types.Tx(gas, gas_price, fee_cap, priority_fee)

def generate_data_file(): 
    mempool_txs = []
    mempool_txs.extend([generate_random_tx().to_int_rep() for i in range(100)])

    for i in range(10):
        with  open("Player-Data/Input-P{}-0".format(i), 'w+') as f:
            txs = []
            for j in range(6):
                tx = generate_random_tx()
                txs.append(tx.to_int_rep())

            bundle = flashbots_types.Bundle(txs)
            mempool_txs.extend(random.sample(txs, 3))
            coinbase_difference = random.uniform(0, 3000001)
            basefee = random.uniform(0, 3000001)
            f.write(str(bundle) + "\n")
            f.write(str(coinbase_difference) + "\n")
            f.write(str(basefee) + "\n")
    with open("../MP-SPDZ/Programs/Public-Input/bundle_scoring", "w+") as m:
        m.write("\n".join([str(tx) for tx in mempool_txs]))

def expected_scores():
    scores = []
    with open("../MP-SPDZ/Programs/Public-Input/bundle_scoring", "r") as public_data:
        bundles = []
        mempool = []
        for i in range(10):
            with open("./Player-Data/Input-P{}-0".format(i), 'r') as private_data:
                txs_bundle = []
                for j in range(6):
                    line = private_data.readline().strip()
                    txs_bundle.append(flashbots_types.Tx(int(line[0]), int(line[1]), int(line[2]), int(line[3])))
                coinbase_difference = float(private_data.readline().strip())
                basefee = float(private_data.readline().strip())

                bundle = flashbots_types.Bundle(txs_bundle)
                bundles.append([bundle, coinbase_difference, basefee])
        for line in public_data.readlines():
            line = line.strip()
            tx = flashbots_types.Tx(int(line[0]), int(line[1]), int(line[2]), int(line[3])).from_int_rep()
            mempool.append(tx)
        for bundle in bundles:
            scores.append(bundle[0].score_bundle(mempool, bundle[1], bundle[2]))

    return scores

if __name__ == '__main__':
    scores = expected_scores()
    print(scores)