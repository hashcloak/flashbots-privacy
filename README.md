# flashbots-privacy
Practical Experiments on how to add complete privacy to flashbots

## How to run

1. Git clone this repository
2. `cd MP-SDPZ`
3. Follow instructions on how to install MP-SPDZ or
```
make mpir
make shamir
```
for only compiling shamir-secret sharing based protocols
4. In order to compile the `bundle_scoring.mpc` file, still within the MP-SDPZ repo
```
./compile.py ../flashbots_mpc/bundle_scoring.mpc
```
5. Generate SSH keys for all the parties
`./Scripts/setup-ssl.sh 10`
6. Run the program using shamir secret sharing with 10 parties
`./Scripts/shamir.sh -IF ../flashbots_mpc/Player-Data -N 10 bundle_scoring`