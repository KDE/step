/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   Step is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Step is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Step; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "unitscalc.h"

#ifdef STEP_WITH_QALCULATE
#include <libqalculate/qalculate.h>
#endif

class UnitsCalcHelper
{
public:
    UnitsCalcHelper(): q(0) {}
    ~UnitsCalcHelper() { delete q; }
    UnitsCalc* q;
};

Q_GLOBAL_STATIC(UnitsCalcHelper, s_unitsCalcHelper)

UnitsCalc* UnitsCalc::self()
{
    if(!s_unitsCalcHelper->q) {
        new UnitsCalc();
    }

    return s_unitsCalcHelper->q;
}

class UnitsCalcPrivate
{
public:
#ifdef STEP_WITH_QALCULATE
    EvaluationOptions eo;
#endif
};

UnitsCalc::UnitsCalc()
{
    Q_ASSERT(!s_unitsCalcHelper->q);
    s_unitsCalcHelper->q = this;

    d = new UnitsCalcPrivate;

#ifdef STEP_WITH_QALCULATE
    new Calculator();
    CALCULATOR->loadGlobalPrefixes();
    CALCULATOR->loadGlobalUnits();
    CALCULATOR->loadGlobalVariables();
    CALCULATOR->loadGlobalFunctions();

    ParseOptions po;
    po.unknowns_enabled = false;
    po.limit_implicit_multiplication = true;
    po.angle_unit = ANGLE_UNIT_RADIANS;

    d->eo.parse_options = po;
    d->eo.approximation = APPROXIMATION_APPROXIMATE;
    d->eo.allow_complex = false;
    d->eo.auto_post_conversion = POST_CONVERSION_BEST;
    d->eo.structuring = STRUCTURING_SIMPLIFY;
#endif
}

UnitsCalc::~UnitsCalc()
{
    delete d;

#ifdef STEP_WITH_QALCULATE
    delete CALCULATOR;
#endif
}

#ifdef STEP_WITH_QALCULATE
bool UnitsCalc::parseNumber(const QString& expression, const QString& units, double& result)
{
    std::string ulexpression = CALCULATOR->unlocalizeExpression(
                    expression.toUtf8().constData(), d->eo.parse_options);

    MathStructure expr;
    CALCULATOR->parse(&expr, ulexpression, d->eo.parse_options);
    expr.eval(d->eo);

    if(!units.isEmpty()) {
        CompositeUnit *cu = NULL;
        Unit *u = NULL;
        std::string strUnits(units.toUtf8().constData());
        u = CALCULATOR->getUnit(strUnits);
        if(!u) {
            cu = new CompositeUnit("", "temporary_composite_convert", "", strUnits);
            if(cu->get(1)) u = cu;
            else return false;
        }

        expr.convert(u, true);
        expr.divide(u, true);
        expr.eval(d->eo);

        delete cu;
    }

    if(!expr.isNumber() && expr.countChildren()) return false;
    result = expr.number().floatValue();
    return true;

}
#else
bool UnitsCalc::parseNumber(const QString&, const QString&, double&)
{
    return false;
}
#endif

