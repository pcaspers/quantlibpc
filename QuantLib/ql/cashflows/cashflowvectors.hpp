/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 StatPro Italia srl
 Copyright (C) 2006, 2007 Cristina Duminuco
 Copyright (C) 2006, 2007 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2015 Peter Caspers

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

/*! \file cashflowvectors.hpp
    \brief Cash flow vector builders
*/

#ifndef quantlib_cash_flow_vectors_hpp
#define quantlib_cash_flow_vectors_hpp

#include <ql/cashflows/fixedratecoupon.hpp>
//#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/replication.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/null.hpp>
#include <ql/utilities/vectors.hpp>
#include <ql/position.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

namespace detail {

template <class T>
T effectiveFixedRate(const std::vector<T> &spreads, const std::vector<T> &caps,
                     const std::vector<T> &floors, Size i);

template <class T>
bool noOption(const std::vector<T> &caps, const std::vector<T> &floors, Size i);
}

using std::max;
using std::min;

template <template <class> class InterestRateIndexType,
          template <class> class FloatingCouponType,
          template <class> class CappedFlooredCouponType, class T>
typename Leg_t<T>::Type FloatingLeg(
    const Schedule &schedule, const std::vector<T> &nominals,
    const boost::shared_ptr<InterestRateIndexType<T> > &index,
    const DayCounter &paymentDayCounter, BusinessDayConvention paymentAdj,
    const std::vector<Natural> &fixingDays, const std::vector<T> &gearings,
    const std::vector<T> &spreads, const std::vector<T> &caps,
    const std::vector<T> &floors, bool isInArrears, bool isZero) {

    Size n = schedule.size() - 1;
    QL_REQUIRE(!nominals.empty(), "no notional given");
    QL_REQUIRE(nominals.size() <= n, "too many nominals (" << nominals.size()
                                                           << "), only " << n
                                                           << " required");
    QL_REQUIRE(gearings.size() <= n, "too many gearings (" << gearings.size()
                                                           << "), only " << n
                                                           << " required");
    QL_REQUIRE(spreads.size() <= n, "too many spreads (" << spreads.size()
                                                         << "), only " << n
                                                         << " required");
    QL_REQUIRE(caps.size() <= n, "too many caps (" << caps.size() << "), only "
                                                   << n << " required");
    QL_REQUIRE(floors.size() <= n, "too many floors (" << floors.size()
                                                       << "), only " << n
                                                       << " required");
    QL_REQUIRE(!isZero || !isInArrears,
               "in-arrears and zero features are not compatible");

    typename Leg_t<T>::Type leg;
    leg.reserve(n);

    // the following is not always correct
    Calendar calendar = schedule.calendar();

    Date refStart, start, refEnd, end;
    Date lastPaymentDate = calendar.adjust(schedule.date(n), paymentAdj);

    for (Size i = 0; i < n; ++i) {
        refStart = start = schedule.date(i);
        refEnd = end = schedule.date(i + 1);
        Date paymentDate =
            isZero ? lastPaymentDate : calendar.adjust(end, paymentAdj);
        if (i == 0 && !schedule.isRegular(i + 1)) {
            BusinessDayConvention bdc = schedule.businessDayConvention();
            refStart = calendar.adjust(end - schedule.tenor(), bdc);
        }
        if (i == n - 1 && !schedule.isRegular(i + 1)) {
            BusinessDayConvention bdc = schedule.businessDayConvention();
            refEnd = calendar.adjust(start + schedule.tenor(), bdc);
        }
        if (detail::get(gearings, i, 1.0) == 0.0) { // fixed coupon
            leg.push_back(
                boost::shared_ptr<CashFlow_t<T> >(new FixedRateCoupon_t<T>(
                    paymentDate, detail::get(nominals, i, 1.0),
                    detail::effectiveFixedRate(spreads, caps, floors, i),
                    paymentDayCounter, start, end, refStart, refEnd)));
        } else { // floating coupon
            if (detail::noOption(caps, floors, i))
                leg.push_back(
                    boost::shared_ptr<CashFlow_t<T> >(new FloatingCouponType<T>(
                        paymentDate, detail::get(nominals, i, 1.0), start, end,
                        detail::get(fixingDays, i, index->fixingDays()), index,
                        detail::get(gearings, i, 1.0),
                        detail::get(spreads, i, 0.0), refStart, refEnd,
                        paymentDayCounter, isInArrears)));
            else {
                leg.push_back(boost::shared_ptr<CashFlow_t<T> >(
                    new CappedFlooredCouponType<T>(
                        paymentDate, detail::get(nominals, i, 1.0), start, end,
                        detail::get(fixingDays, i, index->fixingDays()), index,
                        detail::get(gearings, i, 1.0),
                        detail::get(spreads, i, 0.0),
                        detail::get(caps, i, Null<Rate>()),
                        detail::get(floors, i, Null<Rate>()), refStart, refEnd,
                        paymentDayCounter, isInArrears)));
            }
        }
    }
    return leg;
}

template <template <class> class InterestRateIndexType,
          template <class> class FloatingCouponType,
          template <class> class DigitalCouponType, class T>
typename Leg_t<T>::Leg FloatingDigitalLeg(
    const Schedule &schedule, const std::vector<T> &nominals,
    const boost::shared_ptr<InterestRateIndexType<T> > &index,
    const DayCounter &paymentDayCounter, BusinessDayConvention paymentAdj,
    const std::vector<Natural> &fixingDays, const std::vector<T> &gearings,
    const std::vector<T> &spreads, bool isInArrears,
    const std::vector<T> &callStrikes, Position::Type callPosition,
    bool isCallATMIncluded, const std::vector<T> &callDigitalPayoffs,
    const std::vector<T> &putStrikes, Position::Type putPosition,
    bool isPutATMIncluded, const std::vector<T> &putDigitalPayoffs,
    const boost::shared_ptr<DigitalReplication> &replication) {
    Size n = schedule.size() - 1;
    QL_REQUIRE(!nominals.empty(), "no notional given");
    QL_REQUIRE(nominals.size() <= n, "too many nominals (" << nominals.size()
                                                           << "), only " << n
                                                           << " required");
    QL_REQUIRE(gearings.size() <= n, "too many gearings (" << gearings.size()
                                                           << "), only " << n
                                                           << " required");
    QL_REQUIRE(spreads.size() <= n, "too many spreads (" << spreads.size()
                                                         << "), only " << n
                                                         << " required");
    QL_REQUIRE(callStrikes.size() <= n, "too many call rates ("
                                            << callStrikes.size() << "), only "
                                            << n << " required");
    QL_REQUIRE(putStrikes.size() <= n, "too many put rates ("
                                           << putStrikes.size() << "), only "
                                           << n << " required");

    Leg_t<T> leg;
    leg.reserve(n);

    // the following is not always correct
    Calendar calendar = schedule.calendar();

    Date refStart, start, refEnd, end;
    Date paymentDate;

    for (Size i = 0; i < n; ++i) {
        refStart = start = schedule.date(i);
        refEnd = end = schedule.date(i + 1);
        paymentDate = calendar.adjust(end, paymentAdj);
        if (i == 0 && !schedule.isRegular(i + 1)) {
            BusinessDayConvention bdc = schedule.businessDayConvention();
            refStart = calendar.adjust(end - schedule.tenor(), bdc);
        }
        if (i == n - 1 && !schedule.isRegular(i + 1)) {
            BusinessDayConvention bdc = schedule.businessDayConvention();
            refEnd = calendar.adjust(start + schedule.tenor(), bdc);
        }
        if (detail::get(gearings, i, 1.0) == 0.0) { // fixed coupon
            leg.push_back(boost::shared_ptr<CashFlow>(new FixedRateCoupon_t<T>(
                paymentDate, detail::get(nominals, i, 1.0),
                detail::get(spreads, i, 1.0), paymentDayCounter, start, end,
                refStart, refEnd)));
        } else { // floating digital coupon
            boost::shared_ptr<FloatingCouponType<T> > underlying(
                new FloatingCouponType<T>(
                    paymentDate, detail::get(nominals, i, 1.0), start, end,
                    detail::get(fixingDays, i, index->fixingDays()), index,
                    detail::get(gearings, i, 1.0), detail::get(spreads, i, 0.0),
                    refStart, refEnd, paymentDayCounter, isInArrears));
            leg.push_back(boost::shared_ptr<CashFlow>(new DigitalCouponType<T>(
                underlying, detail::get(callStrikes, i, Null<Real>()),
                callPosition, isCallATMIncluded,
                detail::get(callDigitalPayoffs, i, Null<Real>()),
                detail::get(putStrikes, i, Null<Real>()), putPosition,
                isPutATMIncluded,
                detail::get(putDigitalPayoffs, i, Null<Real>()), replication)));
        }
    }
    return leg;
}

// implementation

namespace detail {

template <class T>
T effectiveFixedRate(const std::vector<T> &spreads, const std::vector<T> &caps,
                     const std::vector<T> &floors, Size i) {
    T result = get(spreads, i, 0.0);
    T floor = get(floors, i, Null<Rate>());
    if (floor != Null<Rate>())
        result = max(floor, result);
    T cap = get(caps, i, Null<Rate>());
    if (cap != Null<Rate>())
        result = min(cap, result);
    return result;
}

template <class T>
bool noOption(const std::vector<T> &caps, const std::vector<T> &floors,
              Size i) {
    return (get(caps, i, Null<Rate>()) == Null<Rate>()) &&
           (get(floors, i, Null<Rate>()) == Null<Rate>());
}
}

} // namespace QuantLib

#endif
