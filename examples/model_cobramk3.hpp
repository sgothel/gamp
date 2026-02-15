/*
 * ships.hpp
 *
 *  Created on: Jan 25, 2026
 *      Author: svenson
 */

#ifndef MODEL_COBRAMK3_HPP_
#define MODEL_COBRAMK3_HPP_

#include <cstdio>
#include <cmath>
#include <vector>

#include <gamp/graph/Graph.hpp>
#include <gamp/graph/Outline.hpp>
#include <gamp/graph/OutlineShape.hpp>

using namespace gamp::graph;

namespace models {
    inline void appendCobraMkIII(std::vector<OutlineShape> &oshapes, 
                                 const float height=1.0f, const float width=2.0f, const float deep=0.3f) {
        float twh = width/2.0f;
        float thh = height/2.0f;
        float tdh = deep/2.0f;

        float ctrX = 0, ctrY = 0, ctrZ = 0;

        float over   = ctrZ + tdh;
        float overh  = over - tdh/2.0f;
        float under  = ctrZ - tdh;
        float underh = under + tdh/2.0f;
        Point3f shoot_tl = Point3f(ctrX-width*1/100, ctrY+thh    , ctrZ);
        Point3f shoot_tr = Point3f(ctrX+width*1/100, ctrY+thh    , ctrZ);
        Point3f shoot_br = Point3f(ctrX+width*1/100, ctrY+thh*4/5, ctrZ);
        Point3f shoot_bl = Point3f(ctrX-width*1/100, ctrY+thh*4/5, ctrZ);

        Point3f ship_top_left  = Point3f(ctrX-width*1.5f/10, ctrY+thh*4/5, ctrZ);
        Point3f ship_top_right = Point3f(ctrX+width*1.5f/10, ctrY+thh*4/5, ctrZ);
        Point3f ship_bottom_right = Point3f(ctrX+width*2/06, ctrY-thh, overh);
        Point3f ship_bottom_left = Point3f(ctrX-width*2/06, ctrY-thh, overh);

        Point3f ship_center_top = Point3f(ctrX, ctrZ, over);

        Point3f ship_left = Point3f(ctrX-twh+(width/18), ctrY-thh/2, ctrZ);
        Point3f ship_right = Point3f(ctrX+twh-(width/18), ctrY-thh/2, ctrZ);
        // CCW
        { // Body over
            OutlineShape oshape;
            oshape.lineTo(shoot_bl.x, shoot_bl.y, shoot_bl.z);
            oshape.lineTo(ship_top_left.x, ship_top_left.y, ship_top_left.z);
            oshape.lineTo(ship_center_top.x, ship_center_top.y, ship_center_top.z);
            oshape.lineTo(ship_top_right.x, ship_top_right.y, ship_top_right.z);
            oshape.lineTo(shoot_br.x, shoot_br.y, shoot_br.z);
            oshape.lineTo(shoot_bl.x, shoot_bl.y, shoot_bl.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        
        { // Laser
            OutlineShape oshape;
            oshape.lineTo(shoot_tl.x, shoot_tl.y, shoot_tl.z);
            oshape.lineTo(shoot_bl.x, shoot_bl.y, shoot_bl.z);
            oshape.lineTo(shoot_br.x, shoot_br.y, shoot_br.z);
            oshape.lineTo(shoot_tr.x, shoot_tr.y, shoot_tr.z);
            oshape.lineTo(shoot_tl.x, shoot_tl.y, shoot_tl.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        
        { // right to over body (RTOB)
            OutlineShape oshape;
            oshape.lineTo(ship_center_top.x, ship_center_top.y, ship_center_top.z);
            oshape.lineTo(ship_bottom_right.x, ship_bottom_right.y, ship_bottom_right.z);
            oshape.lineTo(ship_top_right.x, ship_top_right.y, ship_top_right.z);
            oshape.lineTo(ship_center_top.x, ship_center_top.y, ship_center_top.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        { // left to over body (LTOB)
            OutlineShape oshape;
            oshape.lineTo(ship_center_top.x, ship_center_top.y, ship_center_top.z);
            oshape.lineTo(ship_top_left.x, ship_top_left.y, ship_top_left.z);
            oshape.lineTo(ship_bottom_left.x, ship_bottom_left.y, ship_bottom_left.z);
            oshape.lineTo(ship_center_top.x, ship_center_top.y, ship_center_top.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        { // under body
            OutlineShape oshape;
            oshape.lineTo(ship_bottom_left.x, ship_bottom_left.y, ship_bottom_left.z);
            oshape.lineTo(ship_bottom_right.x, ship_bottom_right.y, ship_bottom_right.z);
            oshape.lineTo(ship_center_top.x, ship_center_top.y, ship_center_top.z);
            oshape.lineTo(ship_bottom_left.x, ship_bottom_left.y, ship_bottom_left.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        { // left to LTOB
            OutlineShape oshape;
            oshape.lineTo(ship_top_left.x, ship_top_left.y, ship_top_left.z);
            oshape.lineTo(ship_left.x, ship_left.y, ship_left.z);
            oshape.lineTo(ship_bottom_left.x, ship_bottom_left.y, ship_bottom_left.z);
            oshape.lineTo(ship_top_left.x, ship_top_left.y, ship_top_left.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        { // right to RTOB
            OutlineShape oshape;
            oshape.lineTo(ship_top_right.x, ship_top_right.y, ship_top_right.z);
            oshape.lineTo(ship_bottom_right.x, ship_bottom_right.y, ship_bottom_right.z);
            oshape.lineTo(ship_right.x, ship_right.y, ship_right.z);
            oshape.lineTo(ship_top_right.x, ship_top_right.y, ship_top_right.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        { // left corner
            OutlineShape oshape;
            oshape.lineTo(ship_bottom_left.x, ship_bottom_left.y, ship_bottom_left.z);
            oshape.lineTo(ship_left.x, ship_left.y, ship_left.z);
            oshape.lineTo(ctrX-twh, ctrY-thh, ctrZ);
            oshape.lineTo(ship_bottom_left.x, ship_bottom_left.y, ship_bottom_left.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        { // right corner
            OutlineShape oshape;
            oshape.lineTo(ship_bottom_right.x, ship_bottom_right.y, ship_bottom_right.z);
            oshape.lineTo(ctrX+twh, ctrY-thh, ctrZ);
            oshape.lineTo(ship_right.x, ship_right.y, ship_right.z);
            oshape.lineTo(ship_bottom_right.x, ship_bottom_right.y, ship_bottom_right.z);
            oshape.closePath();
            oshapes.push_back(oshape);
        }
        std::vector<OutlineShape> oshapes_back = oshapes;
        for(OutlineShape& back : oshapes_back) {
            for(Outline& o : back.outlines()) {
                for(Vertex& v : o.vertices()) {
                    v.coord().z = ( (v.coord().z-ctrZ) * -1.0f + ctrZ - 0.001f);// + dz;
                }
            }
            back.normal() *= -1;
        }
        oshapes.reserve( oshapes.size() + oshapes_back.size() );
        for(OutlineShape& o : oshapes_back){
            oshapes.emplace_back(std::move(o));
        }
        { // eliminate the hole down with left and right motor
            OutlineShape oshape;
            oshape.lineTo(ctrX+twh, ctrY-thh, ctrZ);
            oshape.lineTo(ship_bottom_right.x, ship_bottom_right.y, overh);
            oshape.lineTo(ship_bottom_left.x, ship_bottom_left.y, overh);
            oshape.lineTo(ctrX-twh, ctrY-thh, ctrZ);
            oshape.lineTo(ctrX+width/ 7, ctrY-thh, ctrZ);
            oshape.lineTo(ctrX+width/ 7, ctrY-thh, ctrZ + tdh/4);
            oshape.lineTo(ctrX+width/26, ctrY-thh, ctrZ + tdh/4);
            oshape.lineTo(ctrX+width/26, ctrY-thh, ctrZ);
            oshape.lineTo(ctrX-width/26, ctrY-thh, ctrZ);
            oshape.lineTo(ctrX-width/26, ctrY-thh, ctrZ + tdh/4);
            oshape.lineTo(ctrX-width/ 7, ctrY-thh, ctrZ + tdh/4);
            oshape.lineTo(ctrX-width/ 7, ctrY-thh, ctrZ);
            oshape.closePath();
            oshape.normal() = Vec3f(0, -1, 0);
            oshapes.push_back(oshape);
        }
        { // eliminate the hole down with left and right motor
            OutlineShape oshape;
            oshape.lineTo(ctrX-twh, ctrY-thh, ctrZ);
            oshape.lineTo(ship_bottom_left.x, ship_bottom_left.y, underh);
            oshape.lineTo(ship_bottom_right.x, ship_bottom_right.y, underh);
            oshape.lineTo(ctrX+twh, ctrY-thh, ctrZ);
            oshape.lineTo(ctrX+width/26, ctrY-thh, ctrZ);
            oshape.lineTo(ctrX+width/26, ctrY-thh, ctrZ-tdh/4);
            oshape.lineTo(ctrX+width/ 7, ctrY-thh, ctrZ-tdh/8);
            oshape.lineTo(ctrX+width/ 7, ctrY-thh, ctrZ);
            oshape.lineTo(ctrX-width/ 7, ctrY-thh, ctrZ);
            oshape.lineTo(ctrX-width/ 7, ctrY-thh, ctrZ-tdh/8);
            oshape.lineTo(ctrX-width/26, ctrY-thh, ctrZ-tdh/4);
            oshape.lineTo(ctrX-width/26, ctrY-thh, ctrZ);
            oshape.closePath();
            oshape.normal() = Vec3f(0, -1, 0);
            oshapes.push_back(oshape);
        }
    }
};

#endif /* INCLUDE_SHIPS_HPP_ */
