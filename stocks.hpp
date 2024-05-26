// IDENTIFIER  = 0E04A31E0D60C01986ACB20081C9D8722A1899B6
//  stocks.hpp
//  p2
//
//  Created by Aileen Gonzalez on 5/16/23.
//

#ifndef stocks_hpp
#define stocks_hpp

#include <stdio.h>
#include <getopt.h>
#include <vector>
#include <queue>
#include <deque>
#include "xcode_redirect.hpp"

#endif /* stocks_hpp */

using namespace std;

// so for
// TimeStatus status = TimeStatus::NoTrades

// later
// if (status == TimeStatus::NoTrades) {
        // .. do stuff
// }

class StockMarket {
private:
    enum class TimeStatus:char { NoTrades, CanBuy, Completed, Potential };
    // Your program market will receive a series of “orders,” or intentions to buy or sell shares of a certain stock. An order consists of the following information:
    struct Order {
        // be efficient abt this
//        char intent; // whether the trader wants to buy or sell shares
        uint32_t unique_ID;
        int trader_ID;  // the trader who is issuing the order
//        int stock_ID; //  the stock that the trader is interested in
        int price; // the max/min amount the trader is willing to pay/receive per share
        mutable int quantity; // the number of shares the trader is interested in
        
    };
    
    // When trying to match orders, use priority queues to identify the lowest-price seller and the highest-price buyer.
    //minPQ
    struct compareSeller {
        bool operator()(const Order &order1, const Order &order2) const{
            if (order1.price == order2.price) {
                if (order1.unique_ID > order2.unique_ID) {
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                return order1.price > order2.price;
            }
        } // bool operator()
    }; // compareSeller
    //maxPQ
    struct compareBuyer {
        bool operator()(const Order &order1, const Order &order2) const{
            if (order1.price == order2.price) {
                if (order1.unique_ID > order2.unique_ID) {
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                return order1.price < order2.price;
            }
        } // bool operator()
    }; // compareBuyer
    
    struct Traders {
        int number_bought;
        int number_sold;
        int net_transfer;
    };
    
    struct Stocks {
        int median;
        // Object with higher price has higher priority in the buy queue and the opposite in the sell queue. It's worth noticing that not only the price but also the id @295 contributes to the priority.
        priority_queue<Order, vector<Order>, compareBuyer> buyerPQ;
        priority_queue<Order, vector<Order>, compareSeller> sellerPQ;
        // max pq
        priority_queue<int> smaller_price_PQ;
        // min queue
        priority_queue<int,vector<int>,greater<int>> larger_price_PQ;
        
        // for timeTraveler
        TimeStatus buyOrder;
        TimeStatus sellOrder;
        // initialize to no trade
        
        // int profit; can be local to fxn
        // time traveler doesnt care abt quantity, just wants to know the best profit that can be made off one share of tht stock
        //
        // initialize to 0
        // how much we can make someone buy for this stock, we want it to be as high as possible
        int best_buy_price;
        int best_buy_price_TS;
        // how much they are selling to us for, we want it to be as low as possible
        int best_sell_price;
        int best_sell_price_TS;
        int potential_price_sell;
        int potential_TS_sell;
        int potential_price_buy;
        int potential_TS_buy;
    };
    
    bool verbose_output;
    bool median_output;
    bool trader_info_output;
    bool time_traveler_output;

    char junk1;
    int timestamp = 0; //  the timestamp that this order comes in
    int current_timestamp = 0;
    int trades_completed = 0;
    int num_traders = 0;
    int num_stocks = 0;
    
    uint32_t seed = 0;
    uint32_t num_orders = 0;
    uint32_t rate = 0;
    
    string junk;
    string input_mode;
    
    // need seller and buyer PQ
    // need one for every stock
    
    // need vector of stocks
    // useful for median
    vector<Stocks> stock_info;
    
    // need vector of traders
    // push back to this for trader info ouput
    vector<Traders> trader_info;
    
    // have stock class
    // would have at first: buyer PQ, seller PQ
    // diff bc need to decide whos most important in diff ways so need diff functors
    //  if u make a stock a class, later on need more data for each class/stock ,, add to the class
    // in PQ, put in an order (buy/sell), so need a data struct for an order
    // have to have structs or classes to be the functors, one for buyer PQ and one for seller PQ
    // break ties using unique identifier number for each one (not timestamp)
    
    // if trader info isnt on , the vector will still exist but we will remember the num of traders but wont resize vector or put anything in
    
    // median mode -- YT channel vid
    // need a median per stock,
    // every stock will have a buyer & seller PQ, and a median mode data structure
    // median mode data structure will exist but will never fill it if median mode off
    

public:
    void getMode(int argc, char * argv[]);
    void processOrders(istream &inputStream);
    void calculateMedian(Stocks &stock, int price);
    void read_data();
    void print_output();
};
