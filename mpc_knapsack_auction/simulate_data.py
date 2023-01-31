import knapsack_auction
import random
import os
from operator import itemgetter


def generate_random_knapsack_auction(f=16):
    n_bidders = 10
    bidders = []
    for i in range(n_bidders):
        bidder = knapsack_auction.Bidder(i, round(random.uniform(0, 3000001) * (2**f)), round(random.randrange(21000, 3000001))* (2**f))
        bidders.append(bidder)
    return bidders

def generate_data_files():
    bidders = generate_random_knapsack_auction()
    os.mkdir("Player-Data")
    for i in range(10):
        with open('Player-Data/Input-P{}-0'.format(i), 'w+') as f:
            bidder = bidders[i]
            bid = bidder.bid
            alloc = bidder.allocation
            f.write( str(bid) + " " + str(alloc) + "\n")

def expected_knapsack_auction_allocation():
    bidders = []
    for i in range(10):
        with open("./Player-Data/Input-P{}-0".format(i), 'r') as f:
            bid, alloc = f.readline().split(" ")
            bid = int(bid)
            alloc = int(alloc)
            bidder = knapsack_auction.Bidder(i, bid, alloc)
            bidders.append(bidder)

    highest_bidder_index = max(enumerate(bidder.bid for bidder in bidders), key=itemgetter(1))[0]
    highest_bidder = bidders[highest_bidder_index]
    return knapsack_auction.knapsack_auction(bidders, 30000000, highest_bidder)

if __name__ == '__main__':
    generate_data_files()
    knapsack_auction_winner = expected_knapsack_auction_allocation()
    print(knapsack_auction_winner)

            
