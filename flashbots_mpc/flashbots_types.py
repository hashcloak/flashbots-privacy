import sys

# Get MP-SPDZ types
sys.path.append("../MP-SPDZ/")
from Compiler.types import cfix

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

    def to_int_rep(self, f=16):
        gas = round(self.gas * (2**f))
        gas_price = round(self.gas_price * (2**f))
        fee_cap = round(self.fee_cap * (2**f))
        priority_fee = round(self.priority_fee * (2**f))
        return Tx(gas, gas_price, fee_cap, priority_fee)

    def from_int_rep(self, f=16):
        gas = self.gas / (2**f)
        gas_price = self.gas_price / (2**f)
        fee_cap = self.fee_cap / (2**f)
        priority_fee = self.priority_fee / (2**f)
        return Tx(gas, gas_price, fee_cap, priority_fee)

    def __str__(self):
        return "{} {} {} {}".format(self.gas, self.gas_price, self.fee_cap, self.priority_fee)

class Bundle:
    def __init__(self, coinbase_difference, basefee, txs=[]):
        self.txs = txs
        self.coinbase_difference = coinbase_difference
        self.basefee = basefee

    def __str__(self):
        return "\n".join([str(tx) for tx in self.txs])

    def score_bundle(self, mempool_txs):
        sum_over_txs_in_bundle = sum([tx.gas * tx.calculate_miner_fee_per_gas(self.basefee) for tx in self.txs])
        txs_in_bundle_and_mempool = list(set(mempool_txs).intersection(set(self.txs)))
        sum_over_txs_in_bundle_and_mempool = sum(tx.gas * tx.calculate_miner_fee_per_gas(self.basefee) for tx in txs_in_bundle_and_mempool)
        sum_of_gas_over_all_txs_in_bundle = sum(tx.gas for tx in self.txs)
        score = (self.coinbase_difference + sum_over_txs_in_bundle - sum_over_txs_in_bundle_and_mempool) / sum_of_gas_over_all_txs_in_bundle
        return score

if __name__ == '__main__':
    pass