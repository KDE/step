/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_UNITSCALC_H
#define STEP_UNITSCALC_H

#include <QString>

class UnitsCalcPrivate;
class UnitsCalc
{
public:
    static UnitsCalc *self();
    ~UnitsCalc();

    bool parseNumber(const QString& expression, const QString& units, double& result);

protected:
    UnitsCalc();

    UnitsCalcPrivate* d;
    friend class UnitsCalcHelper;
};

#endif

