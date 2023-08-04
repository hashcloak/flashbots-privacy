
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

    while (allocation_capacity <= max_weight) and (current_bidder := next(bidders_iterator, None)):
        allocation_capacity += current_bidder.allocation
        winning_bidders.append(current_bidder)

    surplus_winning_bidders = sum(bidder.bid for bidder in winning_bidders)
    if surplus_winning_bidders > highest_bidder.bid:
        return winning_bidders
    else:
        return highest_bidder

