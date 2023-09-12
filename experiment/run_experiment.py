import os.path
import json
import random
import subprocess
import logging

# Setup logging
logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s", level=logging.INFO
)

# Load configuration file
with open("experiment/config.json", "r") as config_file:
    config = json.load(config_file)


class Experiment:
    def __init__(
        self,
        algorithm: str,
        protocol: str,
        max_weight: int,
        max_value: int,
        n_parties: int,
        net_controller=None,
    ) -> None:
        self.algorithm = algorithm
        self.protocol = protocol
        self.max_weight = max_weight
        self.max_value = max_value
        self.n_parties = n_parties
        self.net_controller = net_controller
        self.has_finished = False
        self.result = None

    def run(self) -> str:
        if self.net_controller is not None:
            self.net_controller.start()

        self.create_mpc_input_files()
        self.compile_mpc_file()
        self.result = self.run_mpc_protocol()

        if self.net_controller is not None:
            self.net_controller.stop()

        self.has_finished = True

    def run_mpc_protocol(self) -> str:
        # TODO: Implement it with MPC arguments
        logging.info(
            "Running protocol {} for algorithm {}".format(self.protocol, self.algorithm)
        )
        path_protocol = os.path.join("Scripts", self.protocol)

        mpc_file_name = self.algorithm.split("/")[-1].rstrip(".mpc")
        run_mpc_result = subprocess.run(
            [path_protocol, mpc_file_name], cwd=config["mp_spdz_root"]
        )
        run_mpc_result.check_returncode()
        return run_mpc_result.stdout

    def create_mpc_input_files(self) -> None:
        for i in range(self.n_parties):
            rand_weight = random.randint(0, self.max_weight)
            rand_value = random.randint(0, self.max_value)
            input_file_name = "Input-P{}-0".format(i)
            path_file = os.path.join(config["mp_spdz_input_path"], input_file_name)
            with open(path_file, "w") as file_input:
                logging.info("Creating input file for party P{}".format(i))
                file_input.write("{} {}".format(rand_weight, rand_value))

    def compile_mpc_file(self) -> None:
        # TODO: implement it with MPC arguments
        logging.info("Compiling MPC file {}".format(self.algorithm))
        path_mpc_file = os.path.join("..", self.algorithm)
        compile_result = subprocess.run(
            ["./compile.py", "-F", "64", path_mpc_file], cwd=config["mp_spdz_root"]
        )
        compile_result.check_returncode()


class NetworkController:
    def __init__(self, bandwidth: str, latency: str) -> None:
        self.bandwidth = bandwidth
        self.latency = latency

    def start(self) -> None:
        logging.info("Starting shaper.")
        start_shaper_result = subprocess.run(
            ["sh", config["shaper_path"], "start", self.bandwidth, self.latency]
        )
        start_shaper_result.check_returncode()
        logging.info("Shaper successfully started.")

    def stop(self) -> None:
        logging.info("Stopping shaper.")
        stop_shaper_result = subprocess.run(["sh", config["shaper_path"], "stop"])
        stop_shaper_result.check_returncode()
        logging.info("Shapper successfully stopped.")


if __name__ == "__main__":
    algorithm = input("What algorithm do you want to test? ")
    protocol = input("With which protocol do you want to run the experiment? ")
    max_weight = int(input("How much is the max. weight? "))
    max_value = int(input("How much is the max. value? "))
    n_parties = int(input("How many parties do you want? "))

    has_net_limit_response = input("Do you want to limit the network? [y/n]: ")

    if has_net_limit_response == "y":
        bandwidth = input("\tInput the bandwidth: ")
        latency = input("\tInput the latency: ")
        net_controller = NetworkController(bandwidth, latency)
    else:
        net_controller = None

    experiment = Experiment(
        algorithm, protocol, max_weight, max_value, n_parties, net_controller
    )
    
    experiment.run()
