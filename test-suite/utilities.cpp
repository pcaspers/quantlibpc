
/*
 Copyright (C) 2003, 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "utilities.hpp"
#include <ql/Instruments/payoffs.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <cppunit/TestCaller.h>

#define CHECK_DOWNCAST(Derived,Description) { \
    boost::shared_ptr<Derived> hd = boost::dynamic_pointer_cast<Derived>(h); \
    if (hd) \
        return Description; \
}

namespace QuantLib {

    std::string payoffTypeToString(const boost::shared_ptr<Payoff>& h) {

        CHECK_DOWNCAST(PlainVanillaPayoff, "plain-vanilla");
        CHECK_DOWNCAST(CashOrNothingPayoff, "cash-or-nothing");
        CHECK_DOWNCAST(AssetOrNothingPayoff, "asset-or-nothing");
        CHECK_DOWNCAST(SuperSharePayoff, "super-share");
        CHECK_DOWNCAST(GapPayoff, "gap");

        QL_FAIL("payoffTypeToString : unknown payoff type");
    }


    std::string exerciseTypeToString(const boost::shared_ptr<Exercise>& h) {

        CHECK_DOWNCAST(EuropeanExercise, "European");
        CHECK_DOWNCAST(AmericanExercise, "American");
        CHECK_DOWNCAST(BermudanExercise, "Bermudan");

        QL_FAIL("exerciseTypeToString : unknown exercise type");
    }


    boost::shared_ptr<TermStructure> 
    makeFlatCurve(const boost::shared_ptr<Quote>& forward,
                  DayCounter dc) {
        Date today = Date::todaysDate();
        return boost::shared_ptr<TermStructure>(
                       new FlatForward(today, today, 
                                       RelinkableHandle<Quote>(forward), dc));
    }

    boost::shared_ptr<BlackVolTermStructure> 
    makeFlatVolatility(const boost::shared_ptr<Quote>& vol,
                       DayCounter dc) {
        Date today = Date::todaysDate();
        return boost::shared_ptr<BlackVolTermStructure>(
                      new BlackConstantVol(today, 
                                           RelinkableHandle<Quote>(vol), dc));
    }

    double relativeError(double x1, double x2, double reference) {
        if (reference != 0.0)
            return QL_FABS(x1-x2)/reference;
        else
            // fall back to absolute error
            return QL_FABS(x1-x2);
    }



    void vanillaOptionTestFailed(
                           std::string greekName,
                           const boost::shared_ptr<StrikedTypePayoff>& payoff,
                           const boost::shared_ptr<Exercise>& exercise,
                           double s,
                           double q,
                           double r,
                           Date today,
                           DayCounter dc,
                           double v,
                           double expected,
                           double calculated,
                           double error,
                           double tolerance) {

        Time t = dc.yearFraction(today, exercise->lastDate());

        CPPUNIT_FAIL(exerciseTypeToString(exercise) + " "
            + OptionTypeFormatter::toString(payoff->optionType()) +
            " option with "
            + payoffTypeToString(payoff) + " payoff:\n"
            "    spot value: "
            + DoubleFormatter::toString(s) + "\n"
            "    strike:           "
            + DoubleFormatter::toString(payoff->strike()) +"\n"
            "    dividend yield:   "
            + DoubleFormatter::toString(q) + "\n"
            "    risk-free rate:   "
            + DoubleFormatter::toString(r) + "\n"
            "    reference date:   "
            + DateFormatter::toString(today) + "\n"
            "    maturity:         "
            + DateFormatter::toString(exercise->lastDate()) + "\n"
            "    time to expiry:   "
            + DoubleFormatter::toString(t) + "\n"
            "    volatility:       "
            + DoubleFormatter::toString(v) + "\n\n"
            "    expected   " + greekName + ": "
            + DoubleFormatter::toString(expected) + "\n"
            "    calculated " + greekName + ": "
            + DoubleFormatter::toString(calculated) + "\n"
            "    error:            "
            + DoubleFormatter::toString(error) + "\n"
            "    tolerance:        "
            + DoubleFormatter::toString(tolerance));
    }


}


