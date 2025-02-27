/// \file   momentum_investor.cpp
///
/// \brief
///
/// \authors    maarten
/// \date       2020-02-13
/// \copyright  Copyright 2017-2020 The Institute for New Economic Thinking,
/// Oxford Martin School, University of Oxford
///
///             Licensed under the Apache License, Version 2.0 (the "License");
///             you may not use this file except in compliance with the License.
///             You may obtain a copy of the License at
///
///                 http://www.apache.org/licenses/LICENSE-2.0
///
///             Unless required by applicable law or agreed to in writing,
///             software distributed under the License is distributed on an "AS
///             IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
///             express or implied. See the License for the specific language
///             governing permissions and limitations under the License.
///
///             You may obtain instructions to fulfill the attribution
///             requirements in CITATION.cff
///

#include "TrendTest.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////
using std::make_tuple;
using std::shared_ptr;
using std::seed_seq;
using std::array;
using std::vector;
using std::tuple;
using std::endl;
using std::get;
using std::map;


using esl::economics::markets::walras::quote_message;
using namespace esl::economics::finance;
using esl::simulation::time_interval;
using namespace esl::simulation;
using namespace esl::economics;
using esl::law::property;
using esl::identity;
using namespace esl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Excess Demand Function

map<identity<law::property>, esl::variable>
TTest::excess_demand(
        const std::map<identity<law::property>,
                tuple<markets::quote, esl::variable>> &quotes)const{


    //print statement to test if clearer is reaching this point
    std::cout<< "trading 0" <<std::endl;
        map<esl::identity<property>, esl::variable> result_;
            auto scale_ = double(net_asset_value) / quotes.size();


    for(auto &[k, v] : quotes){
        const auto &[quote_, variable_] = v;
            const auto quoted_price_ = double(get<price>(quote_.type));
                std::cout<< quoted_price_ <<std::endl;
                    auto i = valuations.find(k);
                        //print statement to test if clearer is reaching this point
                            std::cout<< "trading 1" <<std::endl;


        if(valuations.end() != i){
            auto value_ = double(i->second);
                auto j = this->supply.find(k);
                    if(supply.end() == j){
                        std::cout<< "trading 2" <<std::endl;
                            result_.emplace(k,  100*quoted_price_ * variable_);
                    }           else{//print statement to test if clearer is reaching this point
                                    std::cout<< "trading 3" <<std::endl;
                                        result_.emplace(k, 100*quoted_price_ * variable_);
                                }
        }
    }
    return result_;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Trend Agent Constructor

TrendAgent::TrendAgent(const identity<fund> &i, const jurisdiction &j, size_t window)
: agent(i)
, owner<cash>(i)
, owner<stock>(i)
, fund(i, j)
, window(window)
{

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Invest Function - Similar to act() function/

time_point
TrendAgent::invest(shared_ptr<quote_message> message, time_interval interval, seed_seq &seed)
{
    vector<tuple<identity<property>, double, size_t, double>> trends_;
        auto nav_ = net_asset_value(interval);
            time_duration window_ = window;
                size_t index_ = 0;


    if(message->received < interval.lower){return interval.lower;}
    if(nav_.value <= 0){/*for(auto &[p,q]: inventory) {
                    auto cast_ = std::dynamic_pointer_cast<cash>(p);
                    if(cast_) {
                        q.amount += (-nav_.value) + 1000000;
                    }
                }*/return interval.upper;}
    if(this->target_net_asset_value.has_value() && double(target_net_asset_value.value()) <= 1.){
        return interval.upper;}


    //LOG(trace) << describe() << " " << identifier << " inventory " <<  inventory << std::endl;
    for(auto [stock_, quote_] : message->proposed){
        auto i = historic_prices.find(stock_);
            if(i == historic_prices.end()) {
                i = historic_prices.emplace(stock_, std::map<time_point, price>()).first;
            }


                if(interval.lower > window_ + 1){
                    auto t_j = i->second.begin();
                        while(t_j->first < interval.lower - window_ - 1){
                            i->second.erase(t_j);
                                t_j = i->second.begin();
                        }
                }


        i->second[interval.lower] = std::get<price>(quote_.type);
            std::map<time_point, price> &prices_ = i->second;
                double trend_ = 0.;

        if(!prices_.empty() && prices_.rbegin()->first > window_){
            if(prices_.size() > this->window){
                trend_ =  double(prices_[interval.lower - window+1]) / std::max(0.0001, double(prices_[interval.lower - window])) - 1;
                    trend_*=10;
            }
        }

                trends_.emplace_back(stock_->identifier, trend_, index_, trend_);
                    output_signal->put(interval.lower, trend_);
                        ++index_;
    }

    map<identity<property>, double> valuations_;
        for(const auto &t : trends_) {
            valuations_.emplace(get<0>(t),  get<3>(t));
        }

        std::cout << "sending TTest" << std::endl;
    auto message_ = this->template create_message<TTest>(
            message->sender, interval.lower, (*this), message->sender,
                interval.lower, interval.lower, nav_, valuations_);



    message_->agression = this->aggression;
        message_->leverage = this->maximum_leverage;



    for(auto [p,q]: inventory){
        auto cast_ = std::dynamic_pointer_cast<stock>(p);
            if(cast_){
                if(0 == q.amount) {continue;}
                    message_->supply.emplace(p->identifier, std::make_tuple(q, quantity(0)));
            }else{

                auto cast2_ = std::dynamic_pointer_cast<securities_lending_contract>(p);
                    if(!cast2_ || 0 == q.amount){continue;}
                        if(message_->supply.end() != message_->supply.find(cast2_->security)){
                            std::get<1>( message_->supply.find(cast2_->security)->second ) = q;
                    }else{
                           message_->supply.emplace(cast2_->security, std::make_tuple(quantity(0), q));
                    }
            }
    }


    return interval.lower;
}


