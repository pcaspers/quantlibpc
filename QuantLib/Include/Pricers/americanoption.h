
/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file americanoption.h
    \brief american option

    $Source$
    $Log$
    Revision 1.1  2001/03/02 08:36:44  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.


*/

#ifndef quantlib_pricers_american_option_h
#define quantlib_pricers_american_option_h

#include "qldefines.h"
#include "stepconditionoption.h"
#include "standardstepcondition.h"
#include "americancondition.h"

namespace QuantLib {
    namespace Pricers {
        class AmericanOption : public StepConditionOption {
        public:
            // constructor
            AmericanOption(Type type, double underlying, double strike, 
                           Rate dividendYield, Rate riskFreeRate, Time residualTime, 
                           double volatility, int timeSteps, int gridPoints)
                : StepConditionOption(type, underlying, strike, dividendYield, 
                                      riskFreeRate, residualTime,
                                      volatility, timeSteps, gridPoints)
                {
                    stepCondition_ = Handle<FiniteDifferences::StandardStepCondition>(
                        new AmericanCondition(initialPrices_));
                }
            // This method must be implemented to imply volatilities
            Handle<BSMOption> clone() const{    
                return Handle<BSMOption>(new AmericanOption(*this));
            }
        };
    }
}


#endif
