/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "arrow.h"

#include <cmath>

#include <QPainter>

const int ARROW_STROKE = 6;

Arrow::Arrow(const QLineF& line, int width) : QLineF(line), _width(width)
{
}

Arrow::Arrow(const QPointF& p1, const QPointF& p2, int width) :
    QLineF(p1, p2), _width(width)
{
}

Arrow::Arrow(qreal x1, qreal y1, qreal x2, qreal y2, int width) :
    QLineF(x1, y1, x2, y2), _width(width)
{
}

void Arrow::draw(QPainter* painter) const
{
    const QTransform& transform = painter->worldTransform();
    QLineF transformed = transform.map(*this);
    QLineF real_transformed(floor(transformed.x1()) + 0.5,
                            floor(transformed.y1()) + 0.5,
                            floor(transformed.x2()) + 0.5,
                            floor(transformed.y2()) + 0.5);
    QLineF line_unit = real_transformed.unitVector();
    line_unit.translate(-line_unit.p1());
    QLineF line_normal = line_unit.normalVector();
    
    QPointF arrow_head_base =
        real_transformed.p2() - line_unit.p2() * 0.866 * _width;
    QLineF arrow_line(transformed);
    arrow_line.setLength(arrow_line.length() - 0.5 * _width);
    
    QPolygonF arrow_head(3);
    arrow_head[0] = real_transformed.p2();
    arrow_head[1] = arrow_head_base + line_normal.p2() * 0.5 * _width;
    arrow_head[2] = arrow_head_base - line_normal.p2() * 0.5 * _width;
    
    painter->save();
    painter->setWorldMatrixEnabled(false);
    QPen pen = painter->pen();
    pen.setWidth(0);
    painter->setPen(pen);
    painter->drawLine(arrow_line);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(pen.color());
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(arrow_head);
    painter->restore();
}
