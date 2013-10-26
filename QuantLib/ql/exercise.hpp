/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file exercise.hpp
    \brief Option exercise classes and payoff function
*/

#ifndef quantlib_exercise_type_h
#define quantlib_exercise_type_h

#include <ql/time/date.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/errors.hpp>
#include <vector>

namespace QuantLib {

    //! Base exercise class
    class Exercise {
      public:
        enum Type {
            American, Bermudan, European
        };
        // constructor
        explicit Exercise(Type type) : type_(type) {}
        virtual ~Exercise() {}
        // inspectors
        Type type() const { return type_; }
        Date date(Size index) const;
        Real rebate(Size index) const;
        Date rebatePaymentDate(Size index) const;
        //! Returns all exercise dates
        const std::vector<Date>& dates() const { return dates_; }
        //! Returns all rebates
        const std::vector<Real>& rebates() const { return rebates_; }
        Date lastDate() const { return dates_.back(); }
      protected:
        std::vector<Date> dates_;
		std::vector<Real> rebates_;
		Natural rebateSettlementDays_;
		Calendar rebatePaymentCalendar_;
		BusinessDayConvention rebatePaymentConvention_;
        Type type_;
    };

    inline Date Exercise::date(Size index) const {
        QL_REQUIRE(index < dates_.size(),
                   "date with index " << index << " does not exist (0..."
                                      << dates_.size() << ")");
        return dates_[index];
    }

    inline Real Exercise::rebate(Size index) const {
        QL_REQUIRE(index < rebates_.size(),
                   "rebate with index " << index << " does not exist (0..."
                                        << rebates_.size() << ")");
        return rebates_[index];
    }

    inline Date Exercise::rebatePaymentDate(Size index) const {
        return rebatePaymentCalendar_.advance(dates_[index],
                                              rebateSettlementDays_, Days,
                                              rebatePaymentConvention_);
    }

    //! Early-exercise base class
    /*! The payoff can be at exercise (the default) or at expiry */
    class EarlyExercise : public Exercise {
      public:
        EarlyExercise(Type type,
                      bool payoffAtExpiry = false)
        : Exercise(type), payoffAtExpiry_(payoffAtExpiry) {}
        bool payoffAtExpiry() const { return payoffAtExpiry_; }
      private:
        bool payoffAtExpiry_;
    };

    //! American exercise
    /*! An American option can be exercised at any time between two
        predefined dates; the first date might be omitted, in which
        case the option can be exercised at any time before the expiry.

        \todo check that everywhere the American condition is applied
              from earliestDate and not earlier
    */
    class AmericanExercise : public EarlyExercise {
      public:
        AmericanExercise(const Date& earliestDate,
                         const Date& latestDate,
                         bool payoffAtExpiry = false);
        AmericanExercise(const Date& latestDate,
                         bool payoffAtExpiry = false);
    };

    //! Bermudan exercise
    /*! A Bermudan option can only be exercised at a set of fixed dates.
    */
    class BermudanExercise : public EarlyExercise {
      public:
        BermudanExercise(const std::vector<Date>& dates,
                         bool payoffAtExpiry = false,
						 const std::vector<Real>& rebates = std::vector<Real>(),     
                         // in case of exercise the holder receives the rebate (if positive) or 
                         // pays it (if negative) on the rebate settlement date
						 const Natural rebateSettlementDays = 0,
						 const Calendar rebatePaymentCalendar = NullCalendar(),
						 const BusinessDayConvention rebatePaymentConvention = Following);
    };

    //! European exercise
    /*! A European option can only be exercised at one (expiry) date.
    */
    class EuropeanExercise : public Exercise {
      public:
        EuropeanExercise(const Date& date, const Real rebate = 0.0, 
                         // in case of exercise the holder receives the rebate (if positive) or 
                         // pays it (if negative) on the rebate settlement date
                         const Natural rebateSettlementDays = 0, 
                         const Calendar rebatePaymentCalendar = NullCalendar(),
                         const BusinessDayConvention rebatePaymentConvention = Following);
    };

}


#endif
