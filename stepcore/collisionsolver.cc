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

#include "collisionsolver.h"
#include "rigidbody.h"
#include "particle.h"

#include <algorithm>

namespace StepCore {

// XXX: units for toleranceAbs and localError
STEPCORE_META_OBJECT(CollisionSolver, "CollisionSolver", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),
    STEPCORE_PROPERTY_RW(double, toleranceAbs, STEPCORE_UNITS_1, "Allowed absolute tolerance", toleranceAbs, setToleranceAbs)
    STEPCORE_PROPERTY_R_D(double, localError, STEPCORE_UNITS_1, "Maximal local error during last step", localError))

STEPCORE_META_OBJECT(GJKCollisionSolver, "GJKCollisionSolver", 0,
                        STEPCORE_SUPER_CLASS(CollisionSolver),)

int GJKCollisionSolver::checkPolygonPolygon(Contact* contact)
{
    Polygon* polygon0 = static_cast<Polygon*>(contact->body0);
    Polygon* polygon1 = static_cast<Polygon*>(contact->body1);

    if(polygon0->vertexes().empty() || polygon1->vertexes().empty()) {
        return contact->state = Contact::Unknown;
    }

    // Algorithm description can be found in 
    // "A Fast and Robust GJK Implementation for
    //    Collision Detection of Convex Objects"
    //    by Gino van den Bergen

    Polygon::VertexList vertexes[2];
    vertexes[0].reserve(polygon0->vertexes().size());
    vertexes[1].reserve(polygon1->vertexes().size());

    const Polygon::VertexList::const_iterator p0_it_end = polygon0->vertexes().end();
    for(Polygon::VertexList::const_iterator it0 = polygon0->vertexes().begin();
                                        it0 != p0_it_end; ++it0) {
        vertexes[0].push_back(polygon0->pointLocalToWorld(*it0));
    }

    const Polygon::VertexList::const_iterator p1_it_end = polygon1->vertexes().end();
    for(Polygon::VertexList::const_iterator it1 = polygon1->vertexes().begin();
                                        it1 != p1_it_end; ++it1) {
        vertexes[1].push_back(polygon1->pointLocalToWorld(*it1));
    }

    int wsize;
    Vector2d w[3];  // Vertexes of current simplex
    Vector2d v;     // Closest point of current simplex
    Vector2d s;     // New support vertex in direction v

    Vector2d vv[2]; // Closest points on the first and second objects
    int wi[2][3];   // Indexes of vertexes corresponding to w
    int si[2];      // Indexes of vertexes corresponding to s

    wsize = 1;
    // Start with arbitrary vertex (TODO: cache the whole w simplex)
    if(contact->state >= Contact::Separating && contact->state < Contact::Intersected) {
        wi[0][0] = contact->_w1[0];
        wi[1][0] = contact->_w1[1];
    } else {
        wi[0][0] = 0;
        wi[1][0] = 0;
    }
    vv[0] = vertexes[0][wi[0][0]];
    vv[1] = vertexes[1][wi[1][0]];
    w[0] = v = vv[1] - vv[0];

    bool intersects = false;
    unsigned int iteration = 0;
    for(;; ++iteration) {
        //STEPCORE_ASSERT_NOABORT( iteration < vertexes[0].size()*vertexes[1].size() );

        double smin = v.norm2();

        // Check for penetration (part 1)
        // If we are closer to the origin then given tolerance
        // we should stop just now to avoid computational errors later
        if(smin < _toleranceAbs*_toleranceAbs*1e-4) { // XXX: separate tolerance for penetration ?
            intersects = true;
            break;
        }

        // Find support vertex in direction v
        // TODO: coherence optimization
        bool sfound = false;
        unsigned int vertex0_size = vertexes[0].size();
        unsigned int vertex1_size = vertexes[1].size();

        for(unsigned int i0=0; i0<vertex0_size; ++i0) {
            for(unsigned int i1=0; i1<vertex1_size; ++i1) {
                Vector2d sn = vertexes[1][i1] - vertexes[0][i0];
                double scurr = v.innerProduct(sn);
                if(smin - scurr > _toleranceAbs*_toleranceAbs*1e-4) { // XXX: separate tolerance ?
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
        double lambda = 0;
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
        /*
        qDebug("penetration detected");
        qDebug("iteration = %d", iteration);
        qDebug("simplexes:");
        qDebug("    1:   2:");
        for(int i=0; i<wsize; ++i) {
            qDebug("    %d    %d", wi[0][i], wi[1][i]);
        }
        */
        contact->distance = 0;
        contact->normal.setZero();
        contact->pointsCount = 0;
        return contact->state = Contact::Intersected;
    }

    /*
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
    */

    double vnorm = v.norm();
    contact->distance = vnorm;
    contact->normal = v/vnorm;
    contact->pointsCount = 0;
    contact->state = Contact::Separated;

    contact->_w1[0] = wi[0][0];
    contact->_w1[1] = wi[1][0];

    if(vnorm > _toleranceAbs) return contact->state;

    // If the objects are close enough we need to find contact manifold
    // We are going to find simplexes (lines) that are 'most parallel'
    // to contact plane and look for contact manifold among them. It
    // works for almost all cases when adjacent polygon edges are
    // not parallel
    Vector2d vunit = v / vnorm;
    Vector2d wm[2][2];

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

            if(angle1 <= angle2 && dist1 < (_toleranceAbs-vnorm)/2) {
                wm[i][1] = av1;
            } else if(angle2 <= angle1 && dist2 < (_toleranceAbs-vnorm)/2) {
                wm[i][1] = av2;
            } else {
                wm[i][1] = wm[i][0]; contact->pointsCount = 1;
                break;
            }
        } else { // edge contact
            wm[i][1] = vertexes[i][ wi[i][1] ];
        }
    }

    // Find intersection of two lines
    if(contact->pointsCount != 1) {
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

        if(wm_o[0][0] > wm_o[1][0]) contact->points[0] = wm[0][0];
        else contact->points[0] = wm[1][0];

        if(wm_o[0][1] < wm_o[1][1]) contact->points[1] = wm[0][1];
        else contact->points[1] = wm[1][1];

        // TODO: interpolate to midpoint
        if((contact->points[1] - contact->points[0]).norm() > _toleranceAbs) {
            /*
            for(int i=0; i<2; ++i) {
                qDebug("contact%d: (%f,%f)", i, contact->points[i][0], contact->points[i][1]);
            }
            */
            contact->pointsCount = 2;
        }
        /*
            contact->vrel[0] = contact->normal.innerProduct(
                                polygon1->velocityWorld(contact->points[0]) -
                                polygon0->velocityWorld(contact->points[0]));
            contact->vrel[1] = contact->normal.innerProduct(
                                polygon1->velocityWorld(contact->points[1]) -
                                polygon0->velocityWorld(contact->points[1]));
            if(contact->vrel[0] < 0 || contact->vrel[1] < 0)
                return contact->state = Contact::Colliding;
            else if(contact->vrel[0] < _toleranceAbs || contact->vrel[1] < _toleranceAbs) // XXX: tolerance
                return contact->state = Colliding::Contacted;
            return contact->state = Contact::Separating;
        }
        */
    }

    if(contact->pointsCount != 2) {
        contact->pointsCount = 1;
        contact->points[0] = vv[0]; // TODO: interpolate vv[0] and vv[1]
        //qDebug("contact is one point: (%f %f) (%f %f)", vv[0][0], vv[0][1], vv[1][0], vv[1][1]);
    }

    int pCount = contact->pointsCount;
    for(int i=0; i<pCount; ++i) {
        contact->vrel[i] = contact->normal.innerProduct(
                        polygon1->velocityWorld(contact->points[i]) -
                        polygon0->velocityWorld(contact->points[i]));

        if(contact->vrel[i] < 0)
            contact->pointsState[i] = Contact::Colliding;
        else if(contact->vrel[i] < _toleranceAbs) // XXX: tolerance
            contact->pointsState[i] = Contact::Contacted;
        else contact->pointsState[i] = Contact::Separating;

        if(contact->pointsState[i] > contact->state)
            contact->state = contact->pointsState[i];
    }

    return contact->state;
}

int GJKCollisionSolver::checkPolygonParticle(Contact* contact)
{
    Polygon* polygon0 = static_cast<Polygon*>(contact->body0);
    Particle* particle1 = static_cast<Particle*>(contact->body1);

    if(polygon0->vertexes().empty()) {
        return contact->state = Contact::Unknown;
    }

    // Simplier version of checkPolygonPolygon algorithm

    Polygon::VertexList vertexes;
    vertexes.reserve(polygon0->vertexes().size());

    const Polygon::VertexList::const_iterator p0_it_end = polygon0->vertexes().end();
    for(Polygon::VertexList::const_iterator it0 = polygon0->vertexes().begin();
                                        it0 != p0_it_end; ++it0) {
        vertexes.push_back(polygon0->pointLocalToWorld(*it0));
    }

    int wsize;
    Vector2d w[3];  // Vertexes of current simplex
    Vector2d v;     // Closest point of current simplex
    Vector2d s;     // New support vertex in direction v

    Vector2d vv; // Closest points on the polygon
    int wi[3];   // Indexes of vertexes corresponding to w
    int si;      // Indexes of vertexes corresponding to s

    // Start with arbitrary vertex (TODO: cache the whole w simplex)
    wsize = 1;
    if(contact->state >= Contact::Separating && contact->state < Contact::Intersected) {
        wi[0] = contact->_w1[0];
    } else {
        wi[0] = 0;
    }
    vv = vertexes[wi[0]];
    w[0] = v = particle1->position() - vv;

    bool intersects = false;
    unsigned int iteration = 0;
    for(;; ++iteration) {
        //STEPCORE_ASSERT_NOABORT( iteration < vertexes[0].size()*vertexes[1].size() );

        double smin = v.norm2();

        // Check for penetration (part 1)
        // If we are closer to the origin then given tolerance
        // we should stop just now to avoid computational errors later
        if(smin < _toleranceAbs*_toleranceAbs*1e-4) { // XXX: separate tolerance for penetration ?
            intersects = true;
            break;
        }

        // Find support vertex in direction v
        // TODO: coherence optimization
        bool sfound = false;
        unsigned int vertex_size = vertexes.size();

        for(unsigned int i0=0; i0<vertex_size; ++i0) {
            Vector2d sn = particle1->position() - vertexes[i0];
            double scurr = v.innerProduct(sn);
            if(smin - scurr > _toleranceAbs*_toleranceAbs*1e-4) { // XXX: separate tolerance ?
                smin = scurr;
                s = sn;
                si = i0;
                sfound = true;
            }
        }

        // If no support vertex have been found than we are at minimum
        if(!sfound) {
            if(wsize == 0) { // we have guessed right point
                w[0] = v;
                wi[0] = 0;
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
                wi[wsize] = si;
                ++wsize;
                v.setZero();
                intersects = true;
                break;
            }
        }

        // Find v = dist(conv(w+s))
        double lambda = 0;
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
            vv = vertexes[si]*(1-lambda) + vertexes[wi[ii]]*lambda;
            if(wsize == 2) {
                w[1-ii] = s;
                wi[1-ii] = si;
            } else {
                w[wsize] = s;
                wi[wsize] = si;
                ++wsize;
            }
        } else { // Closest simplex is vertex
            STEPCORE_ASSERT_NOABORT(iteration == 0 || s.norm2() < v.norm2());

            v = w[0] = s;
            vv = vertexes[si];
            wi[0] = si;
            wsize = 1;
        }
    }

    if(intersects) {
        /*
        qDebug("penetration detected");
        qDebug("iteration = %d", iteration);
        qDebug("simplexes:");
        qDebug("    1:   2:");
        for(int i=0; i<wsize; ++i) {
            qDebug("    %d    %d", wi[0][i], wi[1][i]);
        }
        */
        contact->distance = 0;
        contact->normal.setZero();
        contact->pointsCount = 0;
        return contact->state = Contact::Intersected;
    }

    /*
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
    */

    double vnorm = v.norm();
    contact->distance = vnorm;
    contact->normal = v/vnorm;

    contact->_w1[0] = wi[0];

    if(vnorm > _toleranceAbs) {
        contact->pointsCount = 0;
        contact->state = Contact::Separated;
        return contact->state;
    }

    contact->pointsCount = 1;
    contact->points[0] = particle1->position();
    contact->vrel[0] = contact->normal.innerProduct(
                        particle1->velocity() - 
                        polygon0->velocityWorld(contact->points[0]));

    if(contact->vrel[0] < 0)
        contact->pointsState[0] = Contact::Colliding;
    else if(contact->vrel[0] < _toleranceAbs) // XXX: tolerance
        contact->pointsState[0] = Contact::Contacted;
    else contact->pointsState[0] = Contact::Separating;

    contact->state = contact->pointsState[0];
    return contact->state;
}

int GJKCollisionSolver::checkContact(Contact* contact)
{

    if(contact->type == Contact::UnknownType) {
        if(contact->body0->metaObject()->inherits<Polygon>()) {
            if(contact->body1->metaObject()->inherits<Polygon>()) contact->type = Contact::PolygonPolygonType;
            else if(contact->body1->metaObject()->inherits<Particle>()) contact->type = Contact::PolygonParticleType;
        } else if(contact->body0->metaObject()->inherits<Particle>()) {
            if(contact->body1->metaObject()->inherits<Polygon>()) {
                std::swap(contact->body0, contact->body1);
                contact->type = Contact::PolygonParticleType;
            }
        }
        contact->state = Contact::Unknown;
    }

    if(contact->type == Contact::PolygonPolygonType) return checkPolygonPolygon(contact);
    else if(contact->type == Contact::PolygonParticleType) return checkPolygonParticle(contact);
    return contact->state = Contact::Unknown;
}

int GJKCollisionSolver::checkContacts(BodyList& bodies)
{
    int state = Contact::Unknown;

    checkCache(bodies);

    // Detect and classify contacts
    const ContactList::const_iterator end = _contacts.end();
    for(ContactList::const_iterator it = _contacts.begin(); it != end; ++it) {
        Contact* contact = *it;

        if(contact->type == Contact::PolygonPolygonType) checkPolygonPolygon(contact);
        else if(contact->type == Contact::PolygonParticleType) checkPolygonParticle(contact);
        else contact->state = Contact::Unknown;

        if(contact->state > state) state = contact->state;
        if(contact->state == Contact::Intersected) return state;
    }

    return state;
}

int GJKCollisionSolver::solvePolygonPolygon(Contact* contact)
{
    RigidBody* body0 = static_cast<RigidBody*>(contact->body0);
    RigidBody* body1 = static_cast<RigidBody*>(contact->body1);

    if(contact->pointsCount == 2 &&
        contact->pointsState[0] == Contact::Colliding &&
        contact->pointsState[1] == Contact::Colliding) {
        qDebug("*********** Two-point collisions are still buggy!");
    }

    // calculate impulse
    double b = 1; // coefficient of bounceness

    int pointNum = (contact->pointsState[0] == Contact::Colliding ? 0 : 1);

    double vrel = contact->vrel[pointNum];
    STEPCORE_ASSERT_NOABORT( vrel < 0 );

    Vector2d r0 = contact->points[pointNum] - body0->position();
    Vector2d r1 = contact->points[pointNum] - body1->position();

    double r0n = r0[0]*contact->normal[1] - r0[1]*contact->normal[0];
    double r1n = r1[0]*contact->normal[1] - r1[1]*contact->normal[0];

    double term0 = contact->normal.innerProduct(
                Vector2d( -r0n*r0[1], r0n*r0[0] )) / body0->inertia();
    double term1 = contact->normal.innerProduct(
                Vector2d( -r1n*r1[1], r1n*r1[0] )) / body1->inertia();

    double term2 = 1/body0->mass() + 1/body1->mass();

    /*
    qDebug("vel0=(%f,%f) vel1=(%f,%f)", body0->velocity()[0], body0->velocity()[1],
                                        body1->velocity()[0], body1->velocity()[1]);
    qDebug("body0=%p, body1=%p", body0, body1);
    qDebug("vrel=%f", vrel);
    qDebug("normal=(%f,%f)", contact->normal[0], contact->normal[1]);
    */
    Vector2d j = contact->normal * ( -(1+b)*vrel / (term0 + term1 + term2) );
    //qDebug("mass0=%f mass1=%f j=(%f,%f)", body0->mass(), body1->mass(), j[0], j[1]);
    body0->setVelocity(body0->velocity() - j / body0->mass());
    body1->setVelocity(body1->velocity() + j / body1->mass());
    body0->setAngularVelocity(body0->angularVelocity() - j.norm() * r0n / body0->inertia());
    body1->setAngularVelocity(body1->angularVelocity() + j.norm() * r1n / body1->inertia());

    /*
    double vrel1 = contact->normal.innerProduct(
                    body1->velocityWorld(contact->points[pointNum]) -
                    body0->velocityWorld(contact->points[pointNum]));
    STEPCORE_ASSERT_NOABORT(vrel1 >= 0);
    qDebug("vrel1 = %f", vrel1);
    qDebug("vel0=(%f,%f) vel1=(%f,%f)", body0->velocity()[0], body0->velocity()[1],
                                        body1->velocity()[0], body1->velocity()[1]);
    qDebug(" ");
    */
    contact->pointsState[pointNum] = Contact::Separating;
    contact->state = Contact::Separating; // XXX
    return 2;//CollisionDetected;
}

int GJKCollisionSolver::solvePolygonParticle(Contact* contact)
{
    RigidBody* body0 = static_cast<RigidBody*>(contact->body0);
    Particle*  body1 = static_cast<Particle*>(contact->body1);

    STEPCORE_ASSERT_NOABORT( contact->pointsCount == 1 );

    // calculate impulse
    double b = 1; // coefficient of bounceness

    double vrel = contact->vrel[0];
    STEPCORE_ASSERT_NOABORT( vrel < 0 );
    
    Vector2d r0 = contact->points[0] - body0->position();
    double r0n = r0[0]*contact->normal[1] - r0[1]*contact->normal[0];
    double term0 = contact->normal.innerProduct(
                Vector2d( -r0n*r0[1], r0n*r0[0] )) / body0->inertia();

    double term2 = 1/body0->mass() + 1/body1->mass();

    /*
    qDebug("vel0=(%f,%f) vel1=(%f,%f)", body0->velocity()[0], body0->velocity()[1],
                                        body1->velocity()[0], body1->velocity()[1]);
    qDebug("body0=%p, body1=%p", body0, body1);
    qDebug("vrel=%f", vrel);
    qDebug("normal=(%f,%f)", contact->normal[0], contact->normal[1]);
    */
    Vector2d j = contact->normal * ( -(1+b)*vrel / (term0 + term2) );
    //qDebug("mass0=%f mass1=%f j=(%f,%f)", body0->mass(), body1->mass(), j[0], j[1]);
    body0->setVelocity(body0->velocity() - j / body0->mass());
    body1->setVelocity(body1->velocity() + j / body1->mass());
    body0->setAngularVelocity(body0->angularVelocity() - j.norm() * r0n / body0->inertia());

    /*
    double vrel1 = contact->normal.innerProduct(
                    body1->velocity() -
                    body0->velocityWorld(contact->points[0]));
    STEPCORE_ASSERT_NOABORT(vrel1 >= 0);
    qDebug("vrel1 = %f", vrel1);
    qDebug("vel0=(%f,%f) vel1=(%f,%f)", body0->velocity()[0], body0->velocity()[1],
                                        body1->velocity()[0], body1->velocity()[1]);
    qDebug(" ");
    */
    contact->pointsState[0] = Contact::Separating;
    contact->state = Contact::Separating; // XXX
    return 2;//CollisionDetected;
}

int GJKCollisionSolver::solveCollisions(BodyList& bodies)
{
    int ret = 0;

    // Detect and classify contacts
    ret = checkContacts(bodies);
    STEPCORE_ASSERT_NOABORT(ret != Contact::Intersected);

    // Solve collisions

    const ContactList::const_iterator end = _contacts.end();
    for(ContactList::const_iterator it = _contacts.begin(); it != end; ++it) {
        Contact* contact = *it;

        if(contact->state != Contact::Colliding) continue;

        if(contact->type == Contact::PolygonPolygonType) ret = solvePolygonPolygon(contact);
        else if(contact->type == Contact::PolygonParticleType) ret = solvePolygonParticle(contact);
        else STEPCORE_ASSERT_NOABORT(0);

    }

    return 0;
}

#if 0
int GJKCollisionSolver::solveConstraints(BodyList& /*bodies*/)
{

    return 0;
}
#endif

void GJKCollisionSolver::checkCache(BodyList& bodies)
{
    if(!_contactsIsValid) {
        unsigned int ccount = 0;
        
        BodyList::const_iterator end = bodies.end();
        for(BodyList::const_iterator i0 = bodies.begin(); i0 != end; ++i0) {
            for(BodyList::const_iterator i1 = i0+1; i1 != end; ++i1) {
                Body* body0 = *i0;
                Body* body1 = *i1;
                int type = Contact::UnknownType;

                if(body0->metaObject()->inherits<Polygon>()) {
                    if(body1->metaObject()->inherits<Polygon>()) type = Contact::PolygonPolygonType;
                    else if(body1->metaObject()->inherits<Particle>()) type = Contact::PolygonParticleType;
                } else if(body0->metaObject()->inherits<Particle>()) {
                    if(body1->metaObject()->inherits<Polygon>()) {
                        std::swap(body0, body1);
                        type = Contact::PolygonParticleType;
                    }
                }

                if(type == Contact::UnknownType) continue;

                Contact* contact;
                if(ccount < _contacts.size()) {
                    contact = _contacts[ccount];
                } else {
                    contact = new Contact;
                    _contacts.push_back(contact);
                }

                contact->type = type;
                contact->body0 = body0;
                contact->body1 = body1;
                contact->state = Contact::Unknown;
                ++ccount;
            }
        }

        if(ccount < _contacts.size()) {
            for(unsigned int i = ccount; i<_contacts.size(); ++i) delete _contacts[i];
            _contacts.resize(ccount);
        }
        _contactsIsValid = true;
    }
}

void GJKCollisionSolver::bodyAdded(BodyList& bodies, Body* body)
{
    if(!_contactsIsValid) return;

    BodyList::const_iterator end = bodies.end();
    for(BodyList::const_iterator i1 = bodies.begin(); i1 != end; ++i1) {
        Body* body0 = body;
        Body* body1 = *i1;
        int type = Contact::UnknownType;

        if(body0->metaObject()->inherits<Polygon>()) {
            if(body1->metaObject()->inherits<Polygon>()) type = Contact::PolygonPolygonType;
            else if(body1->metaObject()->inherits<Particle>()) type = Contact::PolygonParticleType;
        } else if(body0->metaObject()->inherits<Particle>()) {
            if(body1->metaObject()->inherits<Polygon>()) {
                std::swap(body0, body1);
                type = Contact::PolygonParticleType;
            }
        }

        if(type != Contact::UnknownType) {
            Contact* contact = new Contact;
            contact->type = type;
            contact->body0 = body0;
            contact->body1 = body1;
            contact->state = Contact::Unknown;
            _contacts.push_back(contact);
        }
    }
}

void GJKCollisionSolver::bodyRemoved(BodyList&, Body* body)
{
    if(!_contactsIsValid) return;

    const ContactList::iterator end = _contacts.end();
    ContactList::iterator it = _contacts.begin();
    for(; it != end; ++it)
        if((*it)->body0 == body || (*it)->body1 == body) break;
    if(it != end) _contacts.erase(it);
}

void GJKCollisionSolver::resetCaches()
{
    _contactsIsValid = false;
}

} // namespace StepCore

