# flashbots-privacy
Practical Experiments on how to add complete privacy to flashbots

## Overview
This repository contains snippets, ideas, etc on specific portions of the Flashbots stack that could potentially be improved via the use of privacy enhancing technologies (PETs).
So far, we have the following experiments:
- `mpc_bundle_scoring`: Computing Flashbots' bundle scoring algorithm using multiparty computation (MPC)
- `mpc_block_knapsack`: Dynamic Programming solver for block building implemented in MPC
- `mpc_knapsack_auction`: Greedy-based knapsack auction for block building implemented in MPC
- `mpc_shifting_knapsack`: An alternative to the dynamic programing solver without using ORAM.

## How to run

We have built a testing script to execute experiments using different combinations of parameters. To execute the script you first need to compile the virtual machines of the protocols that you want to test. To do this, we refer to the user to the [MP-SDPZ documentation](https://github.com/data61/MP-SPDZ/blob/master/README.md) where you can find a how to compile such virtual machines. As an example, you can compile the virtual machine for the MASCOT protocol by running the following commands:

```bash
cd MP-SDPZ
make -j8 mascot-party.x
```

Before executing the scripts you need to modify the contents of `experiment/config.json`, specifically, the `experiments` list. The script will execute all the experiments with the parameters specified in the JSON file. You can add as many experiments as you wish with different protocols, algorithms, parties, etc. An example of an experiment object that can be included in the list is presented next:

```json
{
    "algorithm": "mpc_shifting_knapsack/shifting_knapsack.mpc",
    "protocol": "mal-shamir.sh",
    "max_weight": 20,
    "max_value": 10,
    "n_parties": 4,
    "has_net_limit": false,
    "net_limits": {
        "bandwidth": "10gbps",
        "latency": "0.3ms"
    }
},
```

In the `algorithm` field you need to specify the path to the `.mpc` file relative to the algorithm that you want to test. The `protocol` field should have the name of the `.sh` file consistent with the available protocols in the MP-SDPZ framework. You can find all the `.sh` files supported [here](https://github.com/data61/MP-SPDZ/tree/master/Scripts). The field `has_net_limits` is a boolean that defines if the protocol will be executed with a specified bandwidth and latency. If this flag is set to `true`, you need to specify the desired bandwidth and latency in the `net_limits` JSON object. The bandwidth and latency must be specified according to the parameters section in the [`tc` command documentation](https://man7.org/linux/man-pages/man8/tc.8.html).

Once the JSON config file has all the desired experiments to be executed, you can run the experiments using the command

```bash
sudo python3 experiment/run_experiment.py
```

This testing tool works for the following algorithms:
- `mpc_block_knapsack/knapsack.mpc`
- `mpc_knapsack_auction/knapsack_auction.mpc`
- `mpc_shifting_knapsack/shifting_knapsack.mpc`