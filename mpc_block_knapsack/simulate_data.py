import knapsack
import random

def generate_random_knapsack():
    W = 30000000
    n = random.randint(1,301)
    values = [random.uniform(0, 3000001) for i in range(n)]
    weights = [random.randrange(21000, 3000001) for i in range(n)]
    return (W, n , values, weights)

def chunks(lst, n):
    for i in range(0, n):
        yield lst[i::n]

def generate_data_files():
    W, n, values, weights = generate_random_knapsack()
    values_per_builder = list(chunks(values, 10))
    weights_per_builder = list(chunks(weights, 10))
    for i in range(10):
        with open('Player-Data/Input-P{}-0'.format(i), 'w+') as f:
            values_i = values_per_builder[i]
            weights_i = weights_per_builder[i]
            f.write(" ".join(str(value) for value in values_i) + "\n")
            f.write(" ".join(str(weight) for weight in weights_i) + "\n")

    with open("./MP-SPDZ/Programs/Public-Input/knapsack", "w+") as f2:
        f2.write(str(W) + " " + str(n))

def expected_knapsack_max():
    W = None
    n = None
    with open("./MP-SPDZ/Programs/Public-Input/knapsack", "r") as pd:
        W, n = list(map(int, pd.readline().split(" ")))

    values = []
    weights = []
    for i in range(10):
        with open("./Player-Data/Input-P{}-0".format(i), 'r') as f:
            values_i, weights_i = f.readlines()
            values_i = list(map(float, values_i.split(" ")))
            weights_i = list(map(float, weights_i.split(" ")))
            values.extend(values_i)
            weights.extend(weights_i)

    return knapsack.knapsack(n, W, values, weights)

if __name__ == '__main__':
    #generate_data_files()
    knapsack_max = expected_knapsack_max()
    print(knapsack_max)

            
