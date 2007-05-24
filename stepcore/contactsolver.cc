/* This file is part of StepCore library.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   StepCore library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepCore library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepCore; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "contactsolver.h"
#include "rigidbody.h"

#include <algorithm>

namespace StepCore {

STEPCORE_META_OBJECT(ContactSolver, "ContactSolver", MetaObject::ABSTRACT,
                        STEPCORE_SUPER_CLASS(Object),)
STEPCORE_META_OBJECT(DantzigLCPContactSolver, "DantzigLCPContactSolver", 0,
                        STEPCORE_SUPER_CLASS(ContactSolver),)

int DantzigLCPContactSolver::findClosestPoints(const Polygon* polygon0, const Polygon* polygon1)
{
    // Algorithm description can be found in 
    // "A Fast and Robust GJK Implementation for
    //    Collision Detection of Convex Objects"
    //    by Gino van den Bergen

    Polygon::VertexList vertexes[2];
    for(Polygon::VertexList::const_iterator it0 = polygon0->vertexes().begin();
                                        it0 != polygon0->vertexes().end(); ++it0) {
        vertexes[0].push_back(polygon0->pointLocalToWorld(*it0));
    }

    for(Polygon::VertexList::const_iterator it1 = polygon1->vertexes().begin();
                                        it1 != polygon1->vertexes().end(); ++it1) {
        vertexes[1].push_back(polygon1->pointLocalToWorld(*it1));
    }

    int wsize = 0;
    Vector2d w[3];  // Vertexes of current simplex
    Vector2d v;     // Closest point of current simplex
    Vector2d s;     // New support vertex in direction v

    Vector2d vv[2]; // Closest points on the first and second objects
    int wi[2][3];   // Indexes of vertexes corresponding to w
    int si[2];      // Indexes of vertexes corresponding to s

    // Start with arbitrary vertex (TODO: coherence)
    vv[0] = vertexes[0][0];
    vv[1] = vertexes[1][0];
    v = vv[1] - vv[0];

    bool intersects = false;
    unsigned int iteration = 0;
    for(;; ++iteration) {
        STEPCORE_ASSERT_NOABORT( iteration < vertexes[0].size()*vertexes[1].size() );

        double smin = v.norm2();

        // Check for penetration (part 1)
        // If we are closer to the origin then given tolerance
        // we should stop just now to avoid computational errors later
        if(smin < (1e-5)*(1e-5)) { // TODO XXX: tolerance
            intersects = true;
            break;
        }

        // Find support vertex in direction v
        // TODO: coherence optimization
        bool sfound = false;
        for(unsigned int i0=0; i0<vertexes[0].size(); ++i0) {
            for(unsigned int i1=0; i1<vertexes[1].size(); ++i1) {
                Vector2d sn = vertexes[1][i1] - vertexes[0][i0];
                double scurr = v.innerProduct(sn);
                if(smin - scurr > 1e-10) { // TODO XXX: tolerance
                    smin = scurr;
                    s = sn;
                    si[0] = i0;
                    si[1] = i1;
                    sfound = true;
                }
            }
        }

        // If no support vertex have been found than we are at minimum
        if(!sfound) {
            if(wsize == 0) { // we have guessed right point
                w[0] = v;
                wi[0][0] = 0;
                wi[1][0] = 0;
                wsize = 1;
            }
            break;
        }

        // Check for penetration (part 2)
        if(wsize == 2) {
            // objects are penetrating if origin lies inside the simplex
            // XXX: are there faster method to test it ?
            Vector2d w02 = w[0] - s;
            Vector2d w12 = w[1] - s;
            double det  =  w02[0]*w12[1] - w02[1]*w12[0];
            double det0 =   -s[0]*w12[1] +   s[1]*w12[0];
            double det1 = -w02[0]*  s[1] + w02[1]*  s[0];
            if(det0/det > 0 && det1/det > 0) { // XXX: tolerance
                w[wsize] = s;
                wi[0][wsize] = si[0];
                wi[1][wsize] = si[1];
                ++wsize;
                v.setZero();
                intersects = true;
                break;
            }
        }

        // Find v = dist(conv(w+s))
        double lambda;
        int ii = -1;
        for(int i=0; i<wsize; ++i) {
            double lambda0 = - s.innerProduct(w[i]-s) / (w[i]-s).norm2();
            if(lambda0 > 0) {
                Vector2d vn = s*(1-lambda0) + w[i]*lambda0;
                if(vn.norm2() < v.norm2()) {
                    v = vn; ii = i;
                    lambda = lambda0;
                }
            }
        }

        if(ii >= 0) { // Closest simplex is line
            vv[0] = vertexes[0][si[0]]*(1-lambda) + vertexes[0][wi[0][ii]]*lambda;
            vv[1] = vertexes[1][si[1]]*(1-lambda) + vertexes[1][wi[1][ii]]*lambda;
            if(wsize == 2) {
                w[1-ii] = s;
                wi[0][1-ii] = si[0];
                wi[1][1-ii] = si[1];
            } else {
                w[wsize] = s;
                wi[0][wsize] = si[0];
                wi[1][wsize] = si[1];
                ++wsize;
            }
        } else { // Closest simplex is vertex
            STEPCORE_ASSERT_NOABORT(iteration == 0 || s.norm2() < v.norm2());

            v = w[0] = s;
            vv[0] = vertexes[0][si[0]];
            vv[1] = vertexes[1][si[1]];
            wi[0][0] = si[0];
            wi[1][0] = si[1];
            wsize = 1;
        }
    }

    if(intersects) {
        qDebug("penetration detected");
        qDebug("iteration = %d", iteration);
        qDebug("simplexes:");
        qDebug("    1:   2:");
        for(int i=0; i<wsize; ++i) {
            qDebug("    %d    %d", wi[0][i], wi[1][i]);
        }
        return -1;
    }

    qDebug("distance = %f", v.norm());
    Vector2d v1 = v / v.norm();
    qDebug("normal = (%f,%f)", v1[0], v1[1]);
    qDebug("iteration = %d", iteration);
    qDebug("simplexes:");
    qDebug("    1:   2:");
    for(int i=0; i<wsize; ++i) {
        qDebug("    %d    %d", wi[0][i], wi[1][i]);
    }
    qDebug("contact points:");
    qDebug("    (%f,%f)    (%f,%f)", vv[0][0], vv[0][1], vv[1][0], vv[1][1]);

    // If the objects are close enough we need to find contact manifold
    double vnorm = v.norm();
    if(vnorm < 0.01) { // XXX: tolerance: made it configurable
        // We are going to find simplexes (lines) that are 'most parallel'
        // to contact plane and look for contact manifold among them. It
        // works for almost all cases when adjacent polygon edges are
        // not parallel
        Vector2d vunit = v / vnorm;
        Vector2d wm[0][2];

        bool m_is_point = false;
        for(int i=0; i<2; ++i) {
            wm[i][0] = vertexes[i][ wi[i][0] ];

            if(wsize < 2 || wi[i][0] == wi[i][1]) { // vertex contact
                // Check two adjacent edges
                int ai1 = wi[i][0] - 1; if(ai1 < 0) ai1 = vertexes[i].size()-1;
                Vector2d av1 = vertexes[i][ai1];
                Vector2d dv1 = wm[i][0] - av1;
                double dist1 = vunit.innerProduct( dv1 ) * (i==0 ? 1 : -1);
                double angle1 = dist1 / dv1.norm();

                int ai2 = wi[i][0] + 1; if(ai2 >= (int) vertexes[i].size()) ai2 = 0;
                Vector2d av2 = vertexes[i][ai2];
                Vector2d dv2 = wm[i][0] - av2;
                double dist2 = vunit.innerProduct( dv2 ) * (i==0 ? 1 : -1);
                double angle2 = dist2 / dv2.norm();

                if(angle1 <= angle2 && angle1 < 0.01 && dist1+vnorm < 0.01) { // XXX: tolerance
                    wm[i][1] = av1;
                } else if(angle2 <= angle1 && angle2 < 0.01 && dist2+vnorm < 0.01) { // XXX: tolerance
                    wm[i][1] = av2;
                } else {
                    wm[i][1] = wm[i][0]; m_is_point = true;
                    break;
                }
            } else { // edge contact
                wm[i][1] = vertexes[i][ wi[i][1] ];
            }
        }

        // Find intersection of two lines
        if(!m_is_point) {
            Vector2d vunit_o(-vunit[1], vunit[0]);
            double wm_o[2][2];

            for(int i=0; i<2; ++i) {
                wm_o[i][0] = vunit_o.innerProduct(wm[i][0]);
                wm_o[i][1] = vunit_o.innerProduct(wm[i][1]);

                if(wm_o[i][0] > wm_o[i][1]) {
                    std::swap(wm_o[i][0], wm_o[i][1]);
                    std::swap(wm[i][0], wm[i][1]);
                }
            }

            Vector2d m[2];
            if(wm_o[0][0] > wm_o[1][0]) m[0] = wm[0][0];
            else m[0] = wm[1][0];

            if(wm_o[0][1] < wm_o[1][1]) m[1] = wm[0][1];
            else m[1] = wm[1][1];

            if((m[1] - m[0]).norm() > 0.01) { // XXX: tolerance
                for(int i=0; i<2; ++i) {
                    qDebug("contact%d: (%f,%f)", i, m[i][0], m[i][1]);
                }
                return 0;
            }
        }

        qDebug("contact is one point: (%f %f) (%f %f)", vv[0][0], vv[0][1], vv[1][0], vv[1][1]);

        return 0;
    }

    return 1;
}


int DantzigLCPContactSolver::solveCollisions(double time, World::BodyList& bodies)
{
    // Detect and classify contacts
    findClosestPoints(dynamic_cast<Polygon*>(bodies.at(0)), dynamic_cast<Polygon*>(bodies.at(1)));

    // If there are penetrations abort the solver and try again with lower timestep

    // Solve colliding contact

    return 0;
}

int DantzigLCPContactSolver::solveConstraints(double time, World::BodyList& bodies)
{

    return 0;
}

} // namespace StepCore

