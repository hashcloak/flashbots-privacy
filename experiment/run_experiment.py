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
        n_parties: int,
        net_controller=None,
    ) -> None:
        self.algorithm = algorithm
        self.protocol = protocol
        self.max_weight = max_weight
        self.n_parties = n_parties
        self.net_controller = net_controller
        self.has_finished = False
        self.result = None

    def execute_experiment(self) -> str:
        pass

    def execute_mpc_command(self) -> str:
        pass

    def create_mpc_input_files(self) -> None:
        pass

    def compile_mpc_file(self) -> None:
        pass

    def get_info(self) -> str:
        pass


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
    controller = NetworkController("10mbps", "0.2ms")
    controller.start()
    controller.stop()
