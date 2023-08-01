from operator import itemgetter

class Bidder:
    def __init__(self, id, bid, alloc):
        self.id = id
        self.bid = bid
        self.allocation = alloc

    def calculate_factor(self):
        return self.bid/self.allocation

    def __repr__(self):
        return "Bidder {} with bid {} with allocation {}".format(self.id, self.bid, self.allocation)

"""
Calculates winning bidders for a knapsack auction.
Assumes that bidders are sorted by bid/allocation in decreasing order
"""
def knapsack_auction(bidders, max_weight, highest_bidder):
    bidders_iterator = iter(bidders)
    allocation_capacity = 0
    winning_bidders = []

    while (current_bidder := next(bidders_iterator, None)) and (allocation_capacity + current_bidder.allocation <= max_weight):
        allocation_capacity += current_bidder.allocation
        winning_bidders.append(current_bidder)

    surplus_winning_bidders = sum(bidder.bid for bidder in winning_bidders)
    if surplus_winning_bidders > highest_bidder.bid:
        return winning_bidders
    else:
        return highest_bidder

if __name__ == '__main__':
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
