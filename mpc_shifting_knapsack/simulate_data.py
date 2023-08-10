import random
import os
import shifting_knapsack

MAX_WEIGHT = 3000
MIN_WEIGHT = 2100


class Object:
    def __init__(self, id, weight, value) -> None:
        self.id = id
        self.value = value
        self.weight = weight
        

def save_data(objects):
    try:
        os.makedirs("MP-SPDZ/Player-Data", exist_ok=True)
    except OSError as error:
        print("Directory can not be created:", error)
    for obj in objects:
        with open("MP-SPDZ/Player-Data/Input-P{}-0".format(obj.id),
                  "w+") as file:
            file.write("{} {}".format(obj.value, obj.weight))
            

def generate_knapsack_problem(n_objects):
    objects = []
    for i in range(n_objects):
        value = random.randint(0, MAX_WEIGHT)
        weight = random.randint(MIN_WEIGHT, MAX_WEIGHT)
        object = Object(i, weight, value)
        objects.append(object)
    return objects


def extract_knapsack_info(objects):
    values, weights = [], []
    for obj in objects:
        values.append(obj.value)
        weights.append(obj.weight)
    return values, weights

        
if __name__ == "__main__":
    n_objects = int(input("Input the number of different objects: "))
    objects = generate_knapsack_problem(n_objects)
    save_data(objects)
    
    # Execute knapsack for this problem
    values, weights = extract_knapsack_info(objects)
    result = shifting_knapsack.knapsack(weights, values, MAX_WEIGHT,
                                        recursive=False)
    
    print("Result: ", result)
    
    