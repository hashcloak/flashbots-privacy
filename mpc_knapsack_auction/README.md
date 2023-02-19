# Overview
This is a simple implementation of a naive [knapsack auction](https://theory.stanford.edu/~gagan/papers/knapsack_SODA06.pdf) in MPC using the MP-SPDZ framework. 
Knapsack auctions are a kind of auction in which items are allocated to go into a knapsack by their weight and corresponding bid. 
It is a kind of variant of the original [knapsack problem](https://en.wikipedia.org/wiki/Knapsack_problem).

# How to Run

## Initial Setup
```
cd mpc_knapsack_auction
git submodule init
cd MP-SPDZ
./compile ./knapsack_auction.mpc
```
### Choice of which MPC protocol to run
For this experiment, we used a malicious shamir protocol as it is one of the most efficient protocol in the malicious, honest majority settings and
it's a reasonable model for the flashbots setting. You can compile via running `make mal-shamir` after running the [MP-SPDZ installation instructions](https://github.com/data61/MP-SPDZ#tldr-binary-distribution-on-linux-or-source-distribution-on-macos).
However, there are other potential reasonable MPC settings for this as well. Read the [MP-SPDZ README](https://github.com/data61/MP-SPDZ#preface) to learn more about decide which setting you'd like to try.

## Running the computation
```
python ../simulate_data.py
./Scripts/setup-ssl.sh 10
./Scripts/mal-shamir.sh knapsack_auction -N 10 -IF ../Player-Data/
```
