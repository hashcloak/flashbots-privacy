#include <cmath>
#include <cstdint>
#include <vector>
#include <iostream>


void CMOV(bool enabled, uint64_t& dst, uint64_t& src) {
    if (enabled) {
        dst = src;
    }
}

template<typename T>
void CXCHG(bool enabled, T& src, T& dst) {
    if (enabled) {
        T aux = src;
        src = dst;
        dst = aux;
    }
}

template<typename T>
void maybeShiftByK(
    bool enabled, // data oblivious
    std::vector<T>& vals, // data oblivious, public length 
    uint64_t K, // public
    uint64_t i, // first index, public
    uint64_t j // last index + 1, public
)
{
    std::cout << vals.size() << " " << i << " " << j << std::endl;

    uint64_t N = j-i;

    K=K%N;
    if (K == 0) return;

    for(uint64_t l=0; l<(N-K); l++) {
        CXCHG<T>(enabled, vals[i + ((l+K)%N)], vals[i+l]);
    }

    // Last K elements are shifted inside by (-K)%N:
    //
    uint64_t K2 = (K - (N % K)) % K;

    if (K2 != 0) {
        maybeShiftByK(enabled, vals, K2, j-K, j);
    }
}

template<typename T>
void maybeShiftByK(
    bool enabled, // data oblivious
    std::vector<T>& vals, // data oblivious, public length 
    uint64_t K // public
)
{
    maybeShiftByK(enabled, vals, K, 0, vals.size());
}

template<typename T>
void cyclicShift(
    std::vector<T>& vals, // data oblivious, public length 
    uint64_t K,           // oblivious 
    uint64_t maxLogK      // public 
)
{
    // Applies oblivious cyclic shift on an array of vals by ammount K, leaking maxlogK
    //
    while (maxLogK != 0) {
        maxLogK--;
        bool shouldMov = (K & (1ULL<<maxLogK)) != 0;
        maybeShiftByK(shouldMov, vals, 1<<maxLogK);
        uint64_t sK = K^(1ULL<<maxLogK);
        CMOV(shouldMov, K, sK);
    }
}

template <typename S>
std::ostream& operator<<(std::ostream& os,
                    const std::vector<S>& vector)
{
    // Printing all the elements
    // using <<
    for (auto element : vector) {
        os << element << " ";
    }
    return os;
}

template<bool Oblivious>
uint64_t knapsack_val(
    std::vector<uint64_t>& weights, // data oblivious, public length
    std::vector<uint64_t>& values,  // data oblivious, public length
    uint64_t C,                     // public
    uint64_t W                      // public
)
{
    uint64_t N = weights.size();
    if constexpr (Oblivious) {
        // UNDONE(): Prove we can remove the outer +1:
        //
        uint64_t lwC =std::ceil(std::log2(C+1))+1;
        uint64_t ret = 0;
        std::vector<std::vector<uint64_t> > dp;
        dp.push_back(std::vector<uint64_t>());
        dp.push_back(std::vector<uint64_t>());
        for (uint64_t i=0; i<=C; i++) {
            dp[0].push_back(0);
            dp[1].push_back(0);
        }

        for (uint64_t i=0; i<N; i++) {
             for (uint64_t j=0; j<=C; j++) {
                dp[i%2][j] = dp[(i+1)%2][j];
            }
            cyclicShift(dp[(i+1)%2], C+1-weights[i], lwC);
            for (uint64_t j=0; j<=C; j++) {
                uint64_t opt2 = dp[(i+1)%2][j] + values[i];
                bool shouldMov = (j >= weights[i]) * (opt2 > dp[i%2][j]);
                CMOV(shouldMov, dp[i%2][j], opt2);
            }
            bool increased = dp[i%2][C] > ret;
            CMOV(increased, ret, dp[i%2][C]);

            std::cout << dp << std::endl;
            // We need this if we want to recover the knapsack items:
            // cyclicShift(dp[(i+1)%2], weights[i], lwC);
            //
        }
        return ret;
    } else {
        std::vector<uint64_t> dp;
        for (uint64_t i=0; i<=C; i++) {
            dp.push_back(0);
        }
        for (uint64_t i=0; i<N; i++) {
            for (uint64_t j=C; j>=weights[i]; j--) {
                dp[j] = std::max(dp[j], dp[j-weights[i]] + values[i]);
                if (j == 0) break;
            }
        }
        return dp[C];
    }
}


int main(int argc, char const *argv[])
{
    std::vector<uint64_t> weights {2415, 2829, 2633, 2982, 2351};
    std::vector<uint64_t> values {2470, 2895, 1718, 321, 2595};
    uint64_t W = 10;
    uint64_t C = 3000;

    auto result = knapsack_val<true>(weights, values, C, W);

    std::cout << "Result: " << result << std::endl;
    return 0;
}
