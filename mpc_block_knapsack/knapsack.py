
def knapsack(n, W, values, weights):
    M = [[0 for i in range(W+1)] for i in range(n+1)]

    for i in range(n+1):
        for j in range(W+1):
            if i == 0 or j == 0:
                M[i][j] = 0
            elif weights[i-1] > j:
                M[i][j] = M[i-1][j]
            else:
                M[i][j] = max(M[i-1][j], M[i-1][j - weights[i-1]] + values[i-1])
                #print(M[i][j])
    return M[n][W]

if __name__ == '__main__':
    n = int(input("Enter n, number of distinct items: "))
    W = int(input("Enter W, knapsack capacity: "))
    values = list(map(int, input("Knapsack values: ").strip().split()))
    weights = list(map(int, input("Knapsack weights: ").strip().split()))

    max_val = knapsack(n, W, values, weights)
    print("Max value: ", max_val)
