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
#include <jau/file_util.hpp>
#include <jau/float_types.hpp>
#include <jau/fraction_type.hpp>

#include <gamp/Gamp.hpp>
#include <gamp/render/RenderContext.hpp>

#include "../demos/RedSquareES2.hpp"
#include "../demos/GLLauncher01.hpp"

using namespace jau::math;
using namespace jau::math::util;

using namespace gamp::wt;
using namespace gamp::wt::event;

class Example : public RedSquareES2 {
  private:
    class MyKeyListener : public KeyListener {
      private:
        RedSquareES2& m_parent;
      public:
        MyKeyListener(RedSquareES2& p) : m_parent(p) {}

        void keyPressed(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyPressed: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().bitCount());
            if( e.keySym() == VKeyCode::VK_ESCAPE ) {
                WindowRef win = e.source().lock();
                if( win ) {
                    win->dispose(e.when());
                }
            } else if( e.keySym() == VKeyCode::VK_PAUSE || e.keySym() == VKeyCode::VK_P ) {
                m_parent.animating() = !m_parent.animating();
            } else if( e.keySym() == VKeyCode::VK_W ) {
                WindowRef win = e.source().lock();
                jau::fprintf_td(e.when().to_ms(), stdout, "Source: %s\n", win ? win->toString().c_str() : "null");
            }
        }
        void keyReleased(KeyEvent& e, const KeyboardTracker& kt) override {
            jau::fprintf_td(e.when().to_ms(), stdout, "KeyRelease: %s; keys %zu\n", e.toString().c_str(), kt.pressedKeyCodes().bitCount());
        }
    };
    typedef std::shared_ptr<MyKeyListener> MyKeyListenerRef;
    MyKeyListenerRef m_kl;

  public:
    Example()
    : RedSquareES2(),
      m_kl(std::make_shared<MyKeyListener>(*this)) {  }

    bool init(const WindowRef& win, const jau::fraction_timespec& when) override {
        if( !RedSquareES2::init(win, when) ) {
            return false;
        }
        win->addKeyListener(m_kl);
        return true;
    }
    void dispose(const WindowRef& win, const jau::fraction_timespec& when) override {
        win->removeKeyListener(m_kl);
        RedSquareES2::dispose(win, when);
    }
};

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    return launch("RedSquareES2.hpp",
                  GLLaunchProps{GLProfile(GLProfile::GLES2), gamp::render::RenderContextFlags::verbose},
                  std::make_shared<Example>(), argc, argv);
}
