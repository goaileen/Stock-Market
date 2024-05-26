// IDENTIFIER  = 0E04A31E0D60C01986ACB20081C9D8722A1899B6
//  stocks.cpp
//  p2
//
//  Created by Aileen Gonzalez on 5/16/23.
//

#include "stocks.hpp"
#include "P2random.h"
#include <getopt.h>
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include "xcode_redirect.hpp"

using namespace std;

void StockMarket::getMode(int argc, char * argv[]) {
    // These are used with getopt_long()
    opterr = false;
    int choice;
    int index = 0;
    verbose_output = false;
    median_output = false;
    trader_info_output = false;
    time_traveler_output = false;
    // use getopt to find command line options
    option long_options[] = {{"verbose", no_argument, nullptr, 'v'},
        {"median", no_argument, nullptr, 'm'},
        {"trader_info", no_argument, nullptr, 'i'},
        {"time_travelers", no_argument, nullptr, 't'},
        { nullptr, 0, nullptr, '\0' }};
    /*
     Options with required_argument (print) need a colon after the
     char, options with no_argument do not (help).
     */
    while ((choice = getopt_long(argc, argv, "vmit", long_options, &index)) != -1) {
        switch (choice) {
            case 'v':
                verbose_output = true;
                break;
                
            case 'm':
                median_output = true;
                break;
                
            case 'i':
                trader_info_output = true;
                break;
                
            case 't':
                time_traveler_output = true;
                break;

            default:
                cerr << "Error: Unknown command line option" << endl;
                exit(1);
        } // switch
    } // while
} // get_options

void StockMarket::calculateMedian(Stocks &stock, int price) {
//    stock.smaller_price_PQ.push(price);
    if (!stock.smaller_price_PQ.empty() && (price > stock.smaller_price_PQ.top())) {
            stock.larger_price_PQ.push(price);
    }
    else {
        stock.smaller_price_PQ.push(price);
    }
    // rebalance
    // then calculate median
    while ((stock.smaller_price_PQ.size() != stock.larger_price_PQ.size() + 1) && (stock.smaller_price_PQ.size() + 1 != stock.larger_price_PQ.size()) && (stock.smaller_price_PQ.size() != stock.larger_price_PQ.size())) {
    
        // smallerPQ size is bigger
        if (stock.smaller_price_PQ.size() > stock.larger_price_PQ.size()) {
            stock.larger_price_PQ.push(stock.smaller_price_PQ.top());
            stock.smaller_price_PQ.pop();
        }
        // largerPQ size is bigger
        else if (stock.larger_price_PQ.size() > stock.smaller_price_PQ.size()) {
            stock.smaller_price_PQ.push(stock.larger_price_PQ.top());
            stock.larger_price_PQ.pop();
        }
    } // while
    
    if (stock.smaller_price_PQ.size() == stock.larger_price_PQ.size() + 1) {
        stock.median = stock.smaller_price_PQ.top();
    }
    // largerPQ size is bigger
    else if (stock.larger_price_PQ.size() == stock.smaller_price_PQ.size() + 1) {
        stock.median = stock.larger_price_PQ.top();
    }
    else if ((stock.smaller_price_PQ.size() == stock.larger_price_PQ.size())){
        stock.median = (stock.smaller_price_PQ.top() + stock.larger_price_PQ.top())/2;
    }
} // median

void StockMarket::processOrders(istream &inputStream) {
    int net_transfer = 0;
    int shares_sold = 0;
    int shares_bought = 0;
    int timestamp, trader_ID, stock_ID, price, quantity;
    char intent;
    uint32_t unique_ID = 0;
    
    while (inputStream >> timestamp >> intent >> junk >> junk1 >> trader_ID >> junk1 >> stock_ID >> junk1 >> price >> junk1 >> quantity) {
 
        // TL Input Errors:
        if (input_mode == "TL") {
            if (timestamp < 0) {
                cerr << "Error: Negative timestamp";
                exit(1);
            }
            
            if (timestamp < current_timestamp) {
                cerr << "Error: Decreasing timestamp";
                exit(1);
            }
        
            if (trader_ID < 0 || trader_ID >= num_traders) {
                cerr << "Error: Invalid trader ID";
                exit(1);
            }
            if (stock_ID < 0 || stock_ID >= num_stocks) {
                cerr << "Error: Invalid stock ID";
                exit(1);
            }
            if (price < 0) {
                cerr << "Error: Invalid price";
                exit(1);
            }
            if (quantity < 0) {
                cerr << "Error: Invalid quantity";
                exit(1);
            }
        } // if TL mode
        
        // You print median the moment timestamp changes. You print the median for all existed battles before the next timestamp. If there is no battle before the next timestamp at all, then you don’t print
        
        // && trades_completed > current_trade_amt
        if (median_output && (timestamp != current_timestamp)) {
            for (int i = 0; i < num_stocks; i++) {
                if (stock_info[i].median != 0) {
                    cout << "Median match price of Stock " << i << " at time " << current_timestamp << " is $" << stock_info[i].median << endl;
                }
            }
        }
        
    // once we read our stock in, we create our order object, put it in correct PQ, ex would go into seller PQ for stock 0
        
        Order newOrder = {unique_ID, trader_ID, price, quantity};
        
        if (intent == 'S') {
            stock_info[stock_ID].sellerPQ.push(newOrder);
            // sell order might cause them to transition to new state where they can buy
            // ex: like p1, so went north = came from the south, here its i've seen a sell so i can buy
            if (time_traveler_output) {
                if (stock_info[stock_ID].sellOrder == TimeStatus::NoTrades) {
                    stock_info[stock_ID].best_sell_price = price;
                    stock_info[stock_ID].best_sell_price_TS = timestamp;
                    stock_info[stock_ID].sellOrder = TimeStatus::CanBuy;
                }
                // seller order might cause transition to new state
                // if theres a buy order you can sell to
                
                // seeing sell order means we can buy!
                // dont care about the price, just know if someone is selling it, can buy it!
                
                // if see another sell order, stay in this state
                // if new sell price is lower, first selling for 50, but second selling for 40, stay in this state but change price and time to new values
                
                // if selling to us price is lower
                if (newOrder.price < stock_info[stock_ID].best_sell_price) {
                    if (stock_info[stock_ID].sellOrder == TimeStatus::Completed) {
                        stock_info[stock_ID].sellOrder = TimeStatus::Potential;
                        stock_info[stock_ID].potential_TS_sell = timestamp;
                        stock_info[stock_ID].potential_price_sell = newOrder.price;
                    }
                    else if (stock_info[stock_ID].sellOrder == TimeStatus::CanBuy){
                        stock_info[stock_ID].sellOrder = TimeStatus::CanBuy;
                        stock_info[stock_ID].best_sell_price = newOrder.price;
                        stock_info[stock_ID].best_sell_price_TS = timestamp;
                    }
                    // lhs better than rhs
                }
                // transition to potential if get better sell price
            }
            
            while (!stock_info[stock_ID].buyerPQ.empty() && !stock_info[stock_ID].sellerPQ.empty() && (stock_info[stock_ID].buyerPQ.top().price >= stock_info[stock_ID].sellerPQ.top().price)) {
                    // trade!
                  
                    if (stock_info[stock_ID].sellerPQ.top().quantity >= stock_info[stock_ID].buyerPQ.top().quantity) {
                        shares_sold = stock_info[stock_ID].buyerPQ.top().quantity;
                    }
                    else {
                        shares_sold = stock_info[stock_ID].sellerPQ.top().quantity;
                    }
                    
                    stock_info[stock_ID].buyerPQ.top().quantity -= shares_sold;
                    stock_info[stock_ID].sellerPQ.top().quantity -= shares_sold;
                    
                    if (verbose_output) {
                        cout << "Trader " << stock_info[stock_ID].buyerPQ.top().trader_ID << " purchased " << shares_sold << " shares of Stock " << stock_ID << " from Trader " << stock_info[stock_ID].sellerPQ.top().trader_ID << " for " << "$" << stock_info[stock_ID].buyerPQ.top().price << "/share" << endl;
                    }
                    // selling = gain money
                    net_transfer = stock_info[stock_ID].buyerPQ.top().price * shares_sold;
                    
                    // seller ++ profit
                    trader_info[stock_info[stock_ID].sellerPQ.top().trader_ID].net_transfer += net_transfer;
                    
                    // buyer -- profit
                    trader_info[stock_info[stock_ID].buyerPQ.top().trader_ID].net_transfer -= net_transfer;
                    
                //seler
                    trader_info[stock_info[stock_ID].sellerPQ.top().trader_ID].number_sold += shares_sold;
                    
                //buyer
                    trader_info[stock_info[stock_ID].buyerPQ.top().trader_ID].number_bought += shares_sold;
                    
                trades_completed++;
                
                if (median_output) {
                    calculateMedian(stock_info[stock_ID], stock_info[stock_ID].buyerPQ.top().price);
                }
                
                    if (stock_info[stock_ID].buyerPQ.top().quantity == 0) {
                        stock_info[stock_ID].buyerPQ.pop();
                    }
                    if (stock_info[stock_ID].sellerPQ.top().quantity == 0) {
                        stock_info[stock_ID].sellerPQ.pop();
                    }
                
                } // while
//            } // if not empty &...
        } // if seller
        else if (intent == 'B'){
            stock_info[stock_ID].buyerPQ.push(newOrder);
            
            if (time_traveler_output) {
                if (stock_info[stock_ID].buyOrder == TimeStatus::NoTrades && stock_info[stock_ID].sellOrder != TimeStatus::NoTrades) {
                    stock_info[stock_ID].best_buy_price = price;
                    stock_info[stock_ID].best_buy_price_TS = timestamp;
                }
                // if someone shows me a buy order
                // might change to a state where we have a complete trade
                // is any old buy gna do?
                // if someone said before theyll sell to me for fifty, recorded that, someone else says theyll sell to me for forty, even better, record that, but then someone says ill buy it for 20,
                // erm no cuz is not a profit so not gonna cause a complete trade
                if (stock_info[stock_ID].sellOrder == TimeStatus::CanBuy) {
                    // if best_sell_price - neworderPrice > 0, transition
                    // else we no do trade
                    if (stock_info[stock_ID].best_buy_price  - stock_info[stock_ID].best_sell_price > 0) {
                        // means we have a profit
                        // we like price to complete trade if we like profit
                        stock_info[stock_ID].sellOrder = TimeStatus::Completed;
                        stock_info[stock_ID].buyOrder = TimeStatus::Completed;
                        // if we in the state and see new buyer with better price, we record what time and price we can sell to this buyer
                        
                    }
                }
                if (stock_info[stock_ID].buyOrder == TimeStatus::Completed) {
                    if (newOrder.price > stock_info[stock_ID].best_buy_price) {
                        stock_info[stock_ID].buyOrder = TimeStatus::Potential;
                        stock_info[stock_ID].potential_TS_buy = timestamp;
                        stock_info[stock_ID].potential_price_buy = newOrder.price;
                    }
                }
                else if (stock_info[stock_ID].sellOrder == TimeStatus::Potential) {
                    // if willing to buy and can make more profit
                    // replace old trade w new one
                    // go back to completed trade state
                    if (newOrder.price > stock_info[stock_ID].best_buy_price) {
                        stock_info[stock_ID].best_buy_price = newOrder.price;
                        stock_info[stock_ID].best_buy_price_TS = timestamp;
                        stock_info[stock_ID].sellOrder = TimeStatus::Completed;

                    }
//                    if (stock_info[stock_ID].potential_price < stock_info[stock_ID].best_sell_price) {
//                        stock_info[stock_ID].best_sell_price = stock_info[stock_ID].potential_price;
//                        stock_info[stock_ID].best_sell_price_TS = stock_info[stock_ID].potential_TS;
//                        stock_info[stock_ID].sellOrder = TimeStatus::Completed;
//
//                    }
                }
            }
            
            while (!stock_info[stock_ID].buyerPQ.empty() && !stock_info[stock_ID].sellerPQ.empty() && (stock_info[stock_ID].sellerPQ.top().price <= stock_info[stock_ID].buyerPQ.top().price)) {
                    // trade!
                    if (stock_info[stock_ID].buyerPQ.top().quantity >= stock_info[stock_ID].sellerPQ.top().quantity) {
                        shares_bought = stock_info[stock_ID].sellerPQ.top().quantity;
                    }
                    else {
                        shares_bought = stock_info[stock_ID].buyerPQ.top().quantity;
                    }
                    
                    stock_info[stock_ID].sellerPQ.top().quantity -= shares_bought;
                    stock_info[stock_ID].buyerPQ.top().quantity -= shares_bought;
                    
                    if (verbose_output) {
                        cout << "Trader " << stock_info[stock_ID].buyerPQ.top().trader_ID << " purchased " << shares_bought << " shares of Stock " << stock_ID << " from Trader " << stock_info[stock_ID].sellerPQ.top().trader_ID << " for " << "$" << stock_info[stock_ID].sellerPQ.top().price << "/share" << endl;
                    }
                    // buying = lose money
                    net_transfer = stock_info[stock_ID].sellerPQ.top().price * shares_bought;
                    // buyer -- profit
                    trader_info[stock_info[stock_ID].buyerPQ.top().trader_ID].net_transfer -= net_transfer;
                    // seller ++ profit
                    trader_info[stock_info[stock_ID].sellerPQ.top().trader_ID].net_transfer += net_transfer;
                    
                    trader_info[stock_info[stock_ID].buyerPQ.top().trader_ID].number_bought += shares_bought;
                    
                    trader_info[stock_info[stock_ID].sellerPQ.top().trader_ID].number_sold += shares_bought;
                    
                    trades_completed++;
                
                if (median_output) {
                    calculateMedian(stock_info[stock_ID], stock_info[stock_ID].sellerPQ.top().price);
                }
                
                    if (stock_info[stock_ID].buyerPQ.top().quantity == 0) {
                        stock_info[stock_ID].buyerPQ.pop();
                    }
                    if (stock_info[stock_ID].sellerPQ.top().quantity == 0) {
                        stock_info[stock_ID].sellerPQ.pop();
                    }

                } // while
//            } // if not empty &...
        } // if buyer
        //netvalue
        unique_ID++;
        
        current_timestamp = timestamp;
//        current_trade_amt = trades_completed;
    }  // while ..inputStream
}  // processOrders()

void StockMarket::read_data() {
    cout << "Processing orders..." << endl;
    // getline and throw comment away
    getline(cin, junk);
    // First read Input File Header (mode, num_traders, num_stocks)
    cin >> junk >> input_mode >> junk >> num_traders >> junk >> num_stocks;
    
    // resize vectors here for num_traders and num_stocks
    stock_info.resize(num_stocks);
    trader_info.resize(num_traders);
    
    // Create a stringstream object in case the PROG is used
      stringstream ss;
    
    if (input_mode == "PR") {
        cin >> junk >> seed >> junk >> num_orders >> junk >> rate;

        // Initialize the PROG and populate ss with orders
        P2random::PR_init(ss, seed, num_traders, num_stocks, num_orders, rate);
    }  // if PR mode

      // Call the function with either the stringstream produced by PR_init()
      // or cin
    if (input_mode == "PR") {
        processOrders(ss);
    } // if PR mode
    else {
        processOrders(cin);
    } // if TL mode
    
} // read_data()

void StockMarket::print_output() {
    // printing
    if (median_output) {
        if (median_output && (timestamp != current_timestamp)) {
            for (int i = 0; i < num_stocks; i++) {
                if (stock_info[i].median != 0) {
                    cout << "Median match price of Stock " << i << " at time " << current_timestamp << " is $" << stock_info[i].median << endl;
                }
            }
        }
    }
    
    cout << "---End of Day---\nTrades Completed: " << trades_completed << endl;
    
    if (trader_info_output) {
        cout << "---Trader Info---" << endl;
        for (int i = 0; i < num_traders; i++) {
            cout << "Trader " << i << " bought " << trader_info[i].number_bought << " and sold " << trader_info[i].number_sold << " for a net transfer of $" << trader_info[i].net_transfer << endl;
        }
    }
    if (time_traveler_output) {
        cout << "---Time Travelers---" << endl;;
        for (int i = 0; i < num_stocks; i++) {
            if (stock_info[i].sellOrder == TimeStatus::Completed || stock_info[i].sellOrder == TimeStatus::Potential) {
                cout << "A time traveler would buy Stock " << i << " at time " <<  stock_info[i].best_sell_price_TS << " for $" << stock_info[i].best_sell_price << " and sell it at time " << stock_info[i].best_buy_price_TS << " for $" << stock_info[i].best_buy_price << endl;
            }
            else {
                cout << "A time traveler could not make a profit on Stock " <<  i << endl;
            }
        }
    }
} // run_trading

int main(int argc, char *argv[]) {
    ios_base::sync_with_stdio(false);
    xcode_redirect(argc, argv);

    StockMarket order;
    
    order.getMode(argc, argv);
    order.read_data();
    order.print_output();
    
    return 0;
}
