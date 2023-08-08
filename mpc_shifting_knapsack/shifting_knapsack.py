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
        maybe_shift_by_k(enabled, values, j - K, j)


def cyclic_shift(values, K, max_log_K):

    # Applies obivious ciclic shift on an array of vals by an ammount of
    # max_log_K
    for i in range(max_log_K):
        should_mov = (K & (1 << max_log_K)) != 0
        maybe_shift_by_k(should_mov, values, 1 << max_log_K)
        sK = K ^ (1 << max_log_K)

        if should_mov:
            K = sK


def knapsack(weights, values, C, W):
    N = len(weights)
    lwC = math.ceil(math.log2(C + 1)) + 1
    ret = 0

    dp = np.zeros((2, C + 1))

    for i in range(N):
        for j in range(C + 1):
            dp[i % 2][j] = dp[(i + 1) % 2][j]

        cyclic_shift(values[(i + 1) % 2], C + 1 - weights[i], lwC)

        for j in range(C + 1):
            opt2 = dp[(i + 1) % 2][j] + values[i]
            should_mov = (j >= weights[i]) * (opt2 > dp[i % 2][j])

            if should_mov:
                dp[i % 2][j] = opt2

        increased = dp[i % 2][C] > ret

        if increased:
            ret = dp[i % 2][C]

    return ret

