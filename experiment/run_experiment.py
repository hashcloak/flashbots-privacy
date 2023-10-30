import json
import logging
from datetime import datetime
import subprocess
import random
import os.path

# Setup logging
logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s", level=logging.INFO
)

# Load configuration file
with open("experiment/config.json", "r") as config_file:
    config = json.load(config_file)


class Experiment:
    """Class that represents an experiment"""

    def __init__(
        self,
        algorithm: str,
        protocol: str,
        max_weight: int,
        max_value: int,
        tx_per_party: int,
        n_parties: int,
        net_controller=None,
    ) -> None:
        self.algorithm = algorithm
        self.protocol = protocol
        self.max_weight = max_weight
        self.max_value = max_value
        self.n_parties = n_parties
        self.tx_per_party = tx_per_party
        self.net_controller = net_controller
        self.has_finished = False
        self.result = None

    def run(self) -> str:
        """Runs the experiment with the specified parameters."""

        if self.net_controller is not None:
            self.net_controller.start()

        self.setup_ssl()
        self.create_mpc_input_files()
        self.compile_mpc_file()
        self.result = self.run_mpc_protocol()

        if self.net_controller is not None:
            self.net_controller.stop()

        self.has_finished = True
        self.save_result_file()
        
    def save_result_file(self) -> None:
        """Saves the result of the experiment in a file."""
        
        # Set the filename
        date = datetime.now()
        name = "result--{}--{}--{}--{}--{}".format(
            self.algorithm,
            self.protocol,
            self.max_weight,
            self.n_parties,
            str(date)
        ).split("/")[1]
        
        with open("experiment/" + name + ".txt", "w") as file_output:
            file_output.write(self.result)
        
        logging.info("Results saved in file {}".format(name))
        
    def run_mpc_protocol(self) -> str:
        """Runs the MPC protocol of the compiled algorithm using MP-SDPZ."""

        logging.info(
            "Running protocol {} for algorithm {}".format(self.protocol, self.algorithm)
        )
        path_protocol = os.path.join("./Scripts", self.protocol)

        mpc_file_name = self.algorithm.split("/")[-1].rstrip(
            ".mpc"
        ) + "-{}-{}-{}".format(self.n_parties, self.max_weight, self.tx_per_party)
        run_mpc_result = subprocess.run(
            ["env", "PLAYERS={}".format(self.n_parties), path_protocol, mpc_file_name],
            cwd=config["mp_spdz_root"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        run_mpc_result.check_returncode()
        return run_mpc_result.stdout.decode("utf-8")

    def create_mpc_input_files(self) -> None:
        """Creates the input files according to the experiment specifications."""

        for i in range(self.n_parties):
            rand_weights = random.sample(range(self.max_weight + 1), self.tx_per_party)
            rand_values = random.sample(range(self.max_value + 1), self.tx_per_party)
            input_file_name = "Input-P{}-0".format(i)
            path_file = os.path.join(config["mp_spdz_input_path"], input_file_name)
            with open(path_file, "w") as file_input:
                logging.info("Creating input file for party P{}".format(i))
                file_input.write(
                    "{}\n{}".format(" ".join(map(str, rand_weights)), " ".join(map(str, rand_values)))
                )

    def compile_mpc_file(self) -> None:
        """Compiles the .mpc file using MP-SDPZ."""

        logging.info("Compiling MPC file {}".format(self.algorithm))
        path_mpc_file = os.path.join("..", self.algorithm)
        compile_result = subprocess.run(
            [
                "./compile.py",
                "-F",
                "64",
                path_mpc_file,
                str(self.n_parties),
                str(self.max_weight),
                str(self.tx_per_party),
            ],
            cwd=config["mp_spdz_root"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
        compile_result.check_returncode()

    def setup_ssl(self) -> None:
        """Setups the SSL for the number of parties specified in the experiment."""

        logging.info("Setting up SSL for {} parties".format(self.n_parties))
        setup_result = subprocess.run(
            ["Scripts/setup-ssl.sh", str(self.n_parties)],
            cwd=config["mp_spdz_root"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
        setup_result.check_returncode()


class NetworkController:
    """
    Class that represents the network controller of the experiments. This 
    will be used in case that the experiment requires network limitations.
    """

    def __init__(self, bandwidth: str, latency: str) -> None:
        self.bandwidth = bandwidth
        self.latency = latency

    def start(self) -> None:
        """Starts the network limiter"""
        
        logging.info("Starting shaper.")
        start_shaper_result = subprocess.run(
            ["sh", config["shaper_path"], "start", self.bandwidth, self.latency],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
        start_shaper_result.check_returncode()
        logging.info("Shaper successfully started.")

    def stop(self) -> None:
        """Stops the network limiter"""
        
        logging.info("Stopping shaper.")
        stop_shaper_result = subprocess.run(
            ["sh", config["shaper_path"], "stop"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
        stop_shaper_result.check_returncode()
        logging.info("Shapper successfully stopped.")


if __name__ == "__main__":
    for exp in config["experiments"]:
        algorithm = exp["algorithm"]
        protocol = exp["protocol"]
        max_weight = exp["max_weight"]
        max_value = exp["max_value"]
        n_parties = exp["n_parties"]
        tx_per_party = exp["tx_per_party"]

        has_net_limit_response = exp["has_net_limit"]

        if has_net_limit_response == True:
            bandwidth = exp["net_limits"]["bandwidth"]
            latency = exp["net_limits"]["latency"]
            net_controller = NetworkController(bandwidth, latency)
        else:
            net_controller = None

        experiment = Experiment(
            algorithm,
            protocol,
            max_weight,
            max_value,
            tx_per_party,
            n_parties,
            net_controller,
        )
        experiment.run()

        print("=========== Results of the experiment ============")
        print(experiment.result)
        print("###############################################################")
