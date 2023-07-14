/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "unitscalc.h"

#ifdef STEP_WITH_QALCULATE
#include <libqalculate/qalculate.h>
#endif

class UnitsCalcHelper
{
public:
    UnitsCalcHelper(): q(nullptr) {}
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
        CompositeUnit *cu = nullptr;
        Unit *u = nullptr;
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

