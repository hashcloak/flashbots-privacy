from knapsack_auction import knapsack_auction, Bidder
from operator import itemgetter

bidders = []
max_weight = int(input("Enter maximum weight: "))
input_data = input("Enter bidders as follows (id, bid, capacity. Press Enter to stop: ")
while input_data:
    id, bid, capacity = list(map(int, input_data.strip().split()))
    bidders.append(Bidder(id, bid, capacity))
    input_data = input("Enter bidders as follows (id, bid, capacity. Press Enter to stop: ")

highest_bidder_index = max(enumerate(bidder.bid for bidder in bidders), key=itemgetter(1))[0]
highest_bidder = bidders[highest_bidder_index]
print(bidders)
bidders.sort(key=lambda bidder: bidder.calculate_factor(), reverse=True)
print(bidders)

winners = knapsack_auction(bidders, max_weight, highest_bidder)
print("Winners: ", winners)