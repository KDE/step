/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_ARROW_H
#define STEP_ARROW_H

#include <QLineF>

class QPainter;

class Arrow : public QLineF
{
public:
    Arrow(const QLineF& line, int width);
    Arrow(const QPointF& p1, const QPointF& p2, int width);
    Arrow(qreal x1, qreal y1, qreal x2, qreal y2, int width);
    
    void draw(QPainter* painter) const;
    
private:
    int _width;
};

#endif
