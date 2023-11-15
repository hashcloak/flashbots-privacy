import json
import logging
from datetime import datetime
import subprocess
import random
import os.path
import os
import multiprocessing

# Setup logging
logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s", level=logging.INFO
)

# Load configuration file
with open("experiment/config.json", "r") as config_file:
    config = json.load(config_file)

ring_protocols = [
    "semi2k.sh",
    "spdz2k.sh",
    "rep4-ring.sh",
    "sy-rep-ring.sh",
]

field_protocols = [
    "shamir.sh"
    "mascot.sh"
    "mal-shamir.sh",
    "semi.sh",
    "sy-shamir.sh"
]

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
        repetitions: int,
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
        self.repetitions = repetitions
        self.result = None

    def parallel_run(self):
        """Runs the experiment with the specified parameters."""
        num_processes = multiprocessing.cpu_count() // 2 
        pool = multiprocessing.Pool(processes=num_processes)
        
        self.times = pool.map(self.run_repetition, range(repetitions))
        
        pool.close()
        pool.join()
        
        self.average_time = sum(self.times) / len(self.times)
        self.save_general_results_file()
    
    def regular_run(self) -> None:
        self.times = []
        for i in range(repetitions):
            self.times.append(self.run_repetition(i))
            
        self.average_time = sum(self.times) / len(self.times)
        self.save_general_results_file()
        
    def run_repetition(self, repetition):
        logging.info("Executing repetition {}".format(repetition))
        
        if self.net_controller is not None:
            self.net_controller.start()

        self.setup_ssl()
        self.create_mpc_input_files()
        self.compile_mpc_file()
        result = self.run_mpc_protocol()

        if self.net_controller is not None:
            self.net_controller.stop()

        self.has_finished = True
        self.save_individual_result_file(repetition, result)
        
        running_time = self.extract_time(result)
        return running_time
        
    def save_general_results_file(self) -> None:
        if not os.path.exists("experiment/results"):
            os.makedirs("experiment/results")
            
        date = datetime.now()
        file_name = "{}--{}--{}--{}--{}".format(
            self.algorithm,
            self.protocol,
            self.max_weight,
            self.n_parties,
            str(date)
        ).split("/")[1] + ".txt"
        
        file_name = "general_result--" + file_name
        
        header = [
            "Algorithm: {}".format(self.algorithm),
            "Protocol: {}".format(self.protocol),
            "Max. weight: {}".format(self.max_weight),
            "# Parties: {}".format(self.n_parties),
            "# Tx per party: {}".format(self.tx_per_party),
            "Date: {}".format(str(date)),
        ]
        
        repetition_results = []
        for i in range(self.repetitions):
            repetition_results.append(
                "Time repetition {}: {}".format(str(i), self.times[i])
            )
            
        statistics = [
            "Avg. running time: {}".format(self.average_time)
        ]
        
        contents = header + [""] + repetition_results + [""] + statistics
        contents_str = "\n".join(contents)
        
        with open("experiment/results/" + file_name, "w") as file:
            file.write(contents_str)
           
    def extract_time(self, result: str) -> float:
        result_array = result.split("\n");
        for line in result_array:
            if line.startswith("Time ="):
                time_str = line.lstrip("Time =").split("seconds")[0].strip()
                return float(time_str)
        
    def save_individual_result_file(self, repetition: int, result: str) -> None:
        """Saves the result of the experiment in a file."""
        
        if not os.path.exists("experiment/results"):
            os.makedirs("experiment/results")
            
        # Set the filename
        date = datetime.now()
        name = "{}--{}--{}--{}--{}--{}".format(
            self.algorithm,
            self.protocol,
            self.max_weight,
            self.n_parties,
            str(date),
            str(repetition)
        ).split("/")[1] + ".txt"
        
        name = "result--" + name
        
        with open("experiment/results/" + name, "w") as file_output:
            file_output.write(result)
        
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
            rand_weights = random.sample(range(1, self.max_weight + 1), self.tx_per_party)
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
        
        # Set the compilation flag for rings and fields
        domain = "-F"
        if self.protocol in ring_protocols:
            domain = "-R"
        elif self.protocol in field_protocols:
            domain = "-F"
        
        # Set the ring size for the greedy aproach in a different way given the
        # use of fixed-point arithmetic.    
        ring_size = "64"
        if self.algorithm == "mpc_knapsack_auction/knapsack_auction.mpc":
            ring_size = "80"
        
        compile_result = subprocess.run(
            [
                "./compile.py",
                domain,
                ring_size,
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
    
    time = datetime.now()
    for exp in config["experiments"]:
        algorithm = exp["algorithm"]
        protocol = exp["protocol"]
        max_weight = exp["max_weight"]
        max_value = exp["max_value"]
        n_parties = exp["n_parties"]
        tx_per_party = exp["tx_per_party"]
        repetitions = exp["repetitions"]

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
            repetitions,
            net_controller,
        )
        
        if exp["in_parallel"]:
            experiment.parallel_run()
        else:
            experiment.regular_run()
    
    time = datetime.now() - time
    print("Running time = {}".format(time))
