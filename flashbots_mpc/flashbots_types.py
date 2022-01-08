import sys

# Get MP-SPDZ types
sys.path.append("../MP-SPDZ/")
from Compiler.types import sfloat

class Tx:
    def __init__(self, gas, gas_price, fee_cap, priority_fee):
        self.gas = gas
        self.gas_price = gas_price
        self.fee_cap = fee_cap
        self.priority_fee = priority_fee

    def calculate_effective_fee_per_gas(self, basefee):
        return min(self.fee_cap, basefee + self.priority_fee)

    def calculate_miner_fee_per_gas(self, basefee):
        effecitve_fee = self.calculate_effective_fee_per_gas(basefee)
        return effecitve_fee - basefee

    def __str__(self):
        return "{} {} {} {}".format(self.gas, self.gas_price, self.fee_cap, self.priority_fee)

class Bundle:
    def __init__(self, txs=[]):
        self.txs = txs

    def __str__(self):
        return "\n".join([str(tx) for tx in self.txs])

    def score_bundle(self, mempool_txs, coinbase_difference, basefee):
        sum_over_txs_in_bundle = sum([tx.gas * tx.calculate_miner_fee_per_gas(basefee) for tx in self.txs])
        txs_in_bundle_and_mempool = list(set(mempool_txs).intersect(set(self.txs)))
        sum_over_txs_in_bundle_and_mempool = sum(tx.gas * tx.calculate_miner_fee_per_gas(basefee) for tx in txs_in_bundle_and_mempool)
        sum_of_gas_over_all_txs_in_bundle = sum(tx.gas for tx in self.txs)
        score = (coinbase_difference + sum_of_gas_over_all_txs_in_bundle - sum_over_txs_in_bundle_and_mempool) / sum_of_gas_over_all_txs_in_bundle
        return score

if __name__ == '__main__':
    pass