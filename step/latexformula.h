/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_LATEXFORMULA_H
#define STEP_LATEXFORMULA_H

#include <QString>
#include <QByteArray>

class LatexFormula
{
public:
    static bool isLatexInstalled();
    static bool compileFormula(const QString& formula, QByteArray* result, QString* error);

protected:
    LatexFormula() {}
};

#endif

