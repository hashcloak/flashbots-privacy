import math
import numpy as np


def maybe_shift_by_k(enabled, values, K, i, j):
    assert i <= j
    assert j <= len(values)
    assert i < len(values)

    N = j - i

    K = K % N

    if K == 0:
        return

    for l in range(N - K):
        if enabled:
            # Exchange variables
            aux = values[i + ((l + K) % N)]
            values[i + ((l + K) % N)] = values[i + l]
            values[i + l] = aux

    K2 = (K - (N % K)) % K

    if K2 != 0:
        maybe_shift_by_k(enabled, values, K2, j - K, j)


def is_base_case(K, start, end, n_values):
    # Computations for the base case
    assert start <= end
    assert end <= n_values
    assert start < n_values
    N = end - start

    K = K % N

    return K == 0


def maybe_shift_by_k_iter(enabled, values, K, i, j):
    start = i
    end = j

    # Just assign K2 any non-zero value to do at least the first iteration and
    # the condition of the while loop only depends on the base case.
    K2 = 1

    while not is_base_case(K, start, end, len(values)) and K2 != 0:
        N = end - start
        K = K % N

        for l in range(N - K):
            if enabled:
                # Exchange variables
                aux = values[start + ((l + K) % N)]
                values[start + ((l + K) % N)] = values[start + l]
                values[start + l] = aux

        # Condition for next iteration
        K2 = (K - (N % K)) % K

        # Definition of subproblem
        start = end - K
        K = K2
        


def cyclic_shift(values, K, max_log_K, recursive=True):
    # Applies obivious ciclic shift on an array of vals by an ammount of
    # max_log_K
    for i in range(max_log_K):
        should_mov = (K & (1 << (max_log_K - i - 1))) != 0
        if recursive:
            maybe_shift_by_k(should_mov, values, 1 << (max_log_K - i - 1), 0,
                             len(values))
        else:
            maybe_shift_by_k_iter(should_mov, values, 1 << (max_log_K - i - 1),
                                  0, len(values))    
    
        sK = K ^ (1 << (max_log_K - i - 1))

        if should_mov:
            K = sK


def knapsack(weights, values, C, recursive=True):
    '''
    The recursive param is to choose if solving the problem using the iterative
    version or the recursive one.
    '''
    N = len(weights)
    lwC = math.ceil(math.log2(C + 1)) + 1
    ret = 0

    dp = np.zeros((2, C + 1))

    for i in range(N):
        for j in range(C + 1):
            dp[i % 2][j] = dp[(i + 1) % 2][j]

        cyclic_shift(dp[(i + 1) % 2], C + 1 - weights[i], lwC, recursive)

        for j in range(C + 1):
            opt2 = dp[(i + 1) % 2][j] + values[i]
            should_mov = (j >= weights[i]) * (opt2 > dp[i % 2][j])

            if should_mov:
                dp[i % 2][j] = opt2

        increased = dp[i % 2][C] > ret

        if increased:
            ret = dp[i % 2][C]
            
    return ret


if __name__ == "__main__":
    n = 5
    W = 10
    weights = [1, 2, 3, 4, 5]
    values = [5, 4, 3, 2, 1]
    
    max_val = knapsack(weights, values, W)
    print("Max value: ", max_val)

    max_val_iter = knapsack(weights, values, W, recursive=False)
    print("Max value iterative", max_val_iter)
