import random
import sys
sys.path.append("../mpc_knapsack_auction")
from knapsack_auction import knapsack_auction, Bidder
from operator import itemgetter


def generate_random_knapsack_auction():
    n_bidders = 10
    bidders = []
    for i in range(n_bidders):
        bidder = Bidder(i, round(random.uniform(0, 3000001)), round(random.randrange(21000, 3000001)))
        bidders.append(bidder)
    return bidders

def generate_data_files():
    bidders = generate_random_knapsack_auction()
    with open('client_bids.txt', 'w+') as f:
        for i in range(10):
            bidder = bidders[i]
            bid = bidder.bid
            alloc = bidder.allocation
            f.write( str(bidder.id) + " " + str(bid) + " " + str(alloc) + "\n")

def expected_knapsack_auction_allocation():
    bidders = []
    with open('client_bids.txt', 'r') as f:
        for i in range(10):
            bidder_id, bid, alloc = f.readline().split(" ")
            bid = int(bid)
            alloc = int(alloc)
            bidder = Bidder(bidder_id, bid, alloc)
            bidders.append(bidder)

    highest_bidder_index = max(enumerate(bidder.bid for bidder in bidders), key=itemgetter(1))[0]
    highest_bidder = bidders[highest_bidder_index]
    return knapsack_auction(bidders, 30000000, highest_bidder)

if __name__ == '__main__':
    generate_data_files()
    knapsack_auction_winner = expected_knapsack_auction_allocation()
    print(knapsack_auction_winner)

            
