/*
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in
    compliance with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS"
    basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
    License for the specific language governing rights and limitations
    under the License.

    The Original Code is ______________________________________.

    The Initial Developer of the Original Code is ________________________.
    Portions created by ______________________ are Copyright (C) ______
    _______________________. All Rights Reserved.

    Contributor(s): ______________________________________.

    Alternatively, the contents of this file may be used under the terms
    of the _____ license (the  "[___] License"), in which case the
    provisions of [______] License are applicable instead of those
    above.  If you wish to allow use of your version of this file only
    under the terms of the [____] License and not to allow others to use
    your version of this file under the MPL, indicate your decision by
    deleting  the provisions above and replace  them with the notice and
    other provisions required by the [___] License.  If you do not delete
    the provisions above, a recipient may use your version of this file
    under either the MPL or the [___] License."

*/

#include "cube.h"
 #include "cube.h"


 #include <QtGui/QImage>
 #include <QtCore/QPropertyAnimation>

 static const qreal FACE_SIZE = 0.4;

 static const qreal speeds[] = { 1.8f, 2.4f, 3.6f };
 static const qreal amplitudes[] = { 2.0f, 2.5f, 3.0f };

 static inline void qSetColor(float colorVec[], QColor c)
 {
     colorVec[0] = c.redF();
     colorVec[1] = c.greenF();
     colorVec[2] = c.blueF();
     colorVec[3] = c.alphaF();
 }

 int Geometry::append(const QVector3D &a, const QVector3D &n, const QVector2D &t)
 {
     int v = vertices.count();
     vertices.append(a);
     normals.append(n);
     texCoords.append(t);
     faces.append(v);
     colors.append(QVector4D(0.6f, 0.6f, 0.6f, 1.0f));
     return v;
 }

 void Geometry::addQuad(const QVector3D &a, const QVector3D &b,
                            const QVector3D &c, const QVector3D &d,
                            const QVector<QVector2D> &tex)
 {
     QVector3D norm = QVector3D::normal(a, b, c);
     // append first triangle
     int aref = append(a, norm, tex[0]);
     append(b, norm, tex[1]);
     int cref = append(c, norm, tex[2]);
     // append second triangle
     faces.append(aref);
     faces.append(cref);
     append(d, norm, tex[3]);
 }

 void Geometry::loadArrays() const
 {
     glEnableClientState(GL_VERTEX_ARRAY);
     glEnableClientState(GL_NORMAL_ARRAY);
     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
     glEnableClientState(GL_COLOR_ARRAY);
     glVertexPointer(3, GL_FLOAT, 0, vertices.constData());
     glNormalPointer(GL_FLOAT, 0, normals.constData());
     glTexCoordPointer(2, GL_FLOAT, 0, texCoords.constData());
     glColorPointer(4, GL_FLOAT, 0, colors.constData());
 }

 void Geometry::setColors(int start, GLfloat colorArray[4][4])
 {
     int off = faces[start];
     for (int i = 0; i < 4; ++i)
         colors[i + off] = QVector4D(colorArray[i][0],
                                       colorArray[i][1],
                                       colorArray[i][2],
                                       colorArray[i][3]);
 }

 Tile::Tile(const QVector3D &loc)
     : location(loc)
     , start(0)
     , count(0)
     , useFlatColor(false)
     , geom(0)
 {
     qSetColor(faceColor, QColor(Qt::darkGray));
 }

 void Tile::setColors(GLfloat colorArray[4][4])
 {
     useFlatColor = true;
     geom->setColors(start, colorArray);
 }

 static inline void qMultMatrix(const QMatrix4x4 &mat)
 {
     if (sizeof(qreal) == sizeof(GLfloat))
         glMultMatrixf((GLfloat*)mat.constData());
 #ifndef QT_OPENGL_ES
     else if (sizeof(qreal) == sizeof(GLdouble))
         glMultMatrixd((GLdouble*)mat.constData());
 #endif
     else
     {
         GLfloat fmat[16];
         qreal const *r = mat.constData();
         for (int i = 0; i < 16; ++i)
             fmat[i] = r[i];
         glMultMatrixf(fmat);
     }
 }

 void Tile::draw() const
 {
     QMatrix4x4 mat;
     mat.translate(location);
     mat.rotate(orientation);
     glMatrixMode(GL_MODELVIEW);
     glPushMatrix();
     qMultMatrix(mat);
     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, faceColor);
     glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, geom->indices() + start);
     glPopMatrix();
 }

 TileBuilder::TileBuilder(Geometry *g, qreal depth, qreal size)
     : verts(4)
     , tex(4)
     , start(g->count())
     , count(0)
     , geom(g)
 {
     // front face - make a square with bottom-left at origin
     verts[br].setX(size);
     verts[tr].setX(size);
     verts[tr].setY(size);
     verts[tl].setY(size);

     // these vert numbers are good for the tex-coords
     for (int i = 0; i < 4; ++i)
         tex[i] = verts[i].toVector2D();

     // now move verts half cube width across so cube is centered on origin
     for (int i = 0; i < 4; ++i)
         verts[i] -= QVector3D(size / 2.0f, size / 2.0f, -depth);

     // add the front face
     g->addQuad(verts[bl], verts[br], verts[tr], verts[tl], tex);

     count = g->count() - start;
 }

 void TileBuilder::initialize(Tile *tile) const
 {
     tile->start = start;
     tile->count = count;
     tile->geom = geom;
     qSetColor(tile->faceColor, color);
 }

 Tile *TileBuilder::newTile(const QVector3D &loc) const
 {
     Tile *tile = new Tile(loc);
     initialize(tile);
     return tile;
 }

 Cube::Cube(const QVector3D &loc)
     : Tile(loc)
     , rot(0.0f)
     , r(0), a(0)
 {
 }

 Cube::~Cube()
 {
 }

 void Cube::setAltitude(qreal a)
 {
     if (location.y() != a)
     {
         location.setY(a);
         emit changed();
     }
 }

 void Cube::setRange(qreal r)
 {
     if (location.x() != r)
     {
         location.setX(r);
         emit changed();
     }
 }

 void Cube::setRotation(qreal r)
 {
     if (r != rot)
     {
         orientation = QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 1.0f, 1.0f), r);
         emit changed();
     }
 }

 void Cube::removeBounce()
 {
     delete a;
     a = 0;
     delete r;
     r = 0;
 }

 void Cube::startAnimation()
 {
     if (r)
     {
         r->start();
         r->setCurrentTime(startx);
     }
     if (a)
         a->start();
     if (rtn)
         rtn->start();
 }

 void Cube::setAnimationPaused(bool paused)
 {
     if (paused)
     {
         if (r)
             r->pause();
         if (a)
             a->pause();
         if (rtn)
             rtn->pause();
     }
     else
     {
         if (r)
             r->resume();
         if (a)
             a->resume();
         if (rtn)
             rtn->resume();
     }
 }

 CubeBuilder::CubeBuilder(Geometry *g, qreal depth, qreal size)
     : TileBuilder(g, depth)
     , ix(0)
 {
     for (int i = 0; i < 4; ++i)
         verts[i].setZ(size / 2.0f);
     // back face - "extrude" verts down
     QVector<QVector3D> back(verts);
     for (int i = 0; i < 4; ++i)
         back[i].setZ(-size / 2.0f);

     // add the back face
     g->addQuad(back[br], back[bl], back[tl], back[tr], tex);

     // add the sides
     g->addQuad(back[bl], back[br], verts[br], verts[bl], tex);
     g->addQuad(back[br], back[tr], verts[tr], verts[br], tex);
     g->addQuad(back[tr], back[tl], verts[tl], verts[tr], tex);
     g->addQuad(back[tl], back[bl], verts[bl], verts[tl], tex);

     count = g->count() - start;
 }

 Cube *CubeBuilder::newCube(const QVector3D &loc) const
 {
     Cube *c = new Cube(loc);
     initialize(c);
     qreal d = 4000.0f;
     qreal d3 = d / 3.0f;
     // Animate movement from left to right
     c->r = new QPropertyAnimation(c, "range");
     c->r->setStartValue(-1.3f);
     c->r->setEndValue(1.3f);
     c->startx = ix * d3 * 3.0f;
     c->r->setDuration(d * 4.0f);
     c->r->setLoopCount(-1);
     c->r->setEasingCurve(QEasingCurve(QEasingCurve::CosineCurve));
     // Animate movement from bottom to top
     c->a = new QPropertyAnimation(c, "altitude");
     c->a->setEndValue(loc.y());
     c->a->setStartValue(loc.y() + amplitudes[ix]);
     c->a->setDuration(d / speeds[ix]);
     c->a->setLoopCount(-1);
     c->a->setEasingCurve(QEasingCurve(QEasingCurve::CosineCurve));
     // Animate rotation
     c->rtn = new QPropertyAnimation(c, "rotation");
     c->rtn->setStartValue(c->rot);
     c->rtn->setEndValue(359.0f);
     c->rtn->setDuration(d * 2.0f);
     c->rtn->setLoopCount(-1);
     c->rtn->setDuration(d / 2);
     ix = (ix + 1) % 3;
     return c;
 }
