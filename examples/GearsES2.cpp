/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#include <gamp/Gamp.hpp>

#include <cstdio>
#include <cmath>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>
#include <jau/io/file_util.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/RenderContext.hpp>

#include "../demos/GearsES2.hpp"
#include "../demos/GLLauncher01.hpp"

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;

class Example : public GearsES2 {
  private:
    class MyKeyListener : public KeyListener {
      private:
        GearsES2& m_parent;

      public:
        MyKeyListener(GearsES2& p): m_parent(p) { }

        void keyPressed(KeyEvent& e, const KeyboardTracker&) override {
            const VKeyCode kc = e.keySym();
            if( e.keySym() == VKeyCode::VK_ESCAPE ) {
                WindowSRef win = e.source().lock();
                if( win ) {
                    win->dispose(e.when());
                }
            } else if( kc == VKeyCode::VK_PAUSE || kc == VKeyCode::VK_P ) {
                m_parent.setDoRotate(!m_parent.doRotate());
            } else if( kc == VKeyCode::VK_W ) {
                WindowSRef win = e.source().lock();
                jau::fprintf_td(e.when().to_ms(), stdout, "Source: %s\n", win ? win->toString().c_str() : "null");
            } else if( VKeyCode::VK_LEFT == kc ) {
                m_parent.rotEuler().y -= jau::adeg_to_rad(1.0f);
            } else if( VKeyCode::VK_RIGHT == kc ) {
                m_parent.rotEuler().y += jau::adeg_to_rad(1.0f);
            } else if( VKeyCode::VK_UP == kc ) {
                m_parent.rotEuler().x -= jau::adeg_to_rad(1.0f);
            } else if( VKeyCode::VK_DOWN == kc ) {
                m_parent.rotEuler().x += jau::adeg_to_rad(1.0f);
            }
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;

    class MyPointerListener : public PointerListener {
      private:
        GearsES2&        m_parent;
        jau::math::Vec2i preWinPos;
        jau::math::Vec3f startPos;
        GearsObjectES2* m_picked = nullptr;
        bool m_dragInit = true;

        bool mapWinToObj(const GearsObjectES2& shape, const jau::math::Vec2i& winPos, jau::math::Vec3f& viewPos) noexcept {
            // OK global: m_parent.m_pmvMatrix, m_parent.m_viewport
            const jau::math::Vec3f& ctr = shape.objBounds().center();
            if( m_parent.pmvMatrix().mapObjToWin(ctr, m_parent.viewport(), viewPos) ) {
                const float winZ = viewPos.z;
                return m_parent.pmvMatrix().mapWinToObj((float)winPos.x, (float)winPos.y, winZ, m_parent.viewport(), viewPos);
            }
            return false;
        }
        bool mapWinToObjRay(const jau::math::Vec2i& pos, jau::math::Ray3f& ray, const Mat4f& mPmvi) noexcept {
            // OK global: m_parent.m_pmvMatrix, m_parent.m_viewport
            constexpr float winZ0 = 0.0f;
            constexpr float winZ1 = 0.3f;
            return Mat4f::mapWinToAnyRay((float)pos.x, (float)pos.y, winZ0, winZ1, mPmvi, m_parent.viewport(), ray);
        }

        bool pick(const PointerEvent& e, const WindowSRef&, GearsObjectES2& shape) noexcept {
            // While being processed fast w/o matrix traversal of shapes,
            // we still need to use the cached PMvi matrix for win->obj ray for accuracy.
            // A win->view ray fails in certain angles in edge cases!
            jau::math::Vec3f objPos;
            jau::math::Ray3f objRay;
            const jau::math::Vec2i& winPos = e.position();
            if( !mapWinToObjRay(winPos, objRay, shape.matPMvi()) ) {
                return false;
            }
            const jau::math::geom::AABBox3f& objBox = shape.objBounds();

            if( !objBox.intersectsRay(objRay) ) {
                return false;
            }
            if( !objBox.getRayIntersection(objPos, objRay, std::numeric_limits<float>::epsilon(), /*assumeIntersection=*/true) ) {
                printf("obj  getRayIntersection failed\n");
                return false;
            }
            preWinPos = winPos;
            printf("XXX pick: mouse %s -> %s\n", winPos.toString().c_str(), objPos.toString().c_str());
            printf("XXX pick: %s\n", shape.toString().c_str());
            return true;
        }

        bool navigate(const PointerEvent& e, const WindowSRef& win, GearsObjectES2& shape) noexcept {
            jau::math::Vec3f objPos;
            const jau::math::Vec2i& winPos = e.position();
            if( !mapWinToObj(shape, winPos, objPos) ) {
                return false;
            }
            if( e.isControlDown() ) {
                if( m_dragInit ) {
                    m_dragInit = false;
                    startPos = objPos;
                } else {
                    jau::math::Vec3f flip = jau::math::util::getEulerAngleOrientation(m_parent.rotEuler());
                    jau::math::Vec3f diffPos = ( objPos - startPos ).mul(flip);
                    m_parent.pan() += diffPos;
                }
            } else {
                const jau::math::Vec2i& sdim    = win->surfaceSize();
                const float thetaY  = 360.0f * ((float)(winPos.x - preWinPos.x) / (float)sdim.x);
                const float thetaX  = 360.0f * ((float)(preWinPos.y - winPos.y) / (float)sdim.y);
                m_parent.rotEuler().x += jau::adeg_to_rad(thetaX);
                m_parent.rotEuler().y += jau::adeg_to_rad(thetaY);
                m_dragInit = true;
            }
            preWinPos = winPos;
            // printf("XXX navi: mouse %s -> %s\n", winPos.toString().c_str(), objPos.toString().c_str());
            return true;
        }

        PointerShapeAction pickAction, navigateAction;

      public:
        MyPointerListener(GearsES2& p): m_parent(p) {
            pickAction = jau::bind_member(this, &MyPointerListener::pick);
            navigateAction = jau::bind_member(this, &MyPointerListener::navigate);
        }
        void pointerPressed(PointerEvent& e) override {
            if( e.pointerCount() == 1 ) {
                WindowSRef win = e.source().lock();
                if( !win ) {
                    return;
                }
                GearsObjectES2* new_pick = m_parent.findPick(pickAction, e, win); // no matrix traversal
                if( m_picked ) {
                    m_picked->picked() = false;
                }
                m_picked = new_pick;
                if( m_picked ) {
                    m_picked->picked() = true;
                }
            }
        }
        void pointerDragged(PointerEvent& e) override {
            if( m_picked ) {
                WindowSRef win = e.source().lock();
                if( !win ) {
                    return;
                }
                if( !m_parent.dispatchForShape(*m_picked, navigateAction, e, win) ) { // matrix traversal
                    if( m_picked ) {
                        m_picked->picked() = false;
                    }
                    m_picked = nullptr;
                    printf("XXX shape: lost\n");
                }
            }
        }
        void pointerWheelMoved(PointerEvent& e) override {
            const jau::math::Vec3f& rot = e.rotation();
            if( e.isControlDown() ) {
                // alternative zoom
                float incr       = e.isShiftDown() ? rot.x : rot.y * 0.5f;
                m_parent.pan().z += incr;
            } else {
                // panning
                m_parent.pan().x -= rot.x;  // positive -> left
                m_parent.pan().y += rot.y;  // positive -> up
            }
        }
        void pointerReleased(PointerEvent&) override {
            if( m_picked ) {
                m_picked->picked() = false;
                printf("XXX shape: released\n");
            }
            m_picked = nullptr;
            m_dragInit = true;
        }
    };
    typedef std::shared_ptr<MyPointerListener> MyPointerListenerRef;

    MyKeyListenerRef           m_kl;
    MyPointerListenerRef       m_pl;

  public:
    Example()
    : GearsES2(),
      m_kl(std::make_shared<MyKeyListener>(*this)),
      m_pl(std::make_shared<MyPointerListener>(*this))
    { }

    bool init(const WindowSRef& win, const jau::fraction_timespec& when) override {
        if( !GearsES2::init(win, when) ) {
            return false;
        }
        win->addKeyListener(m_kl);
        win->addPointerListener(m_pl);
        return true;
    }
    void dispose(const WindowSRef& win, const jau::fraction_timespec& when) override {
        win->removeKeyListener(m_kl);
        win->removePointerListener(m_pl);
        GearsES2::dispose(win, when);
    }
};

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    return launch("GearsES2.hpp",
                  GLLaunchProps{.profile=GLProfile(GLProfile::GLES2),
                                .contextFlags=gamp::render::RenderContextFlags::verbose,
                                .requestedCaps=GLCapabilities()},
                  std::make_shared<Example>(), argc, argv);
}
