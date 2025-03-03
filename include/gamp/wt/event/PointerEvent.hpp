/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2010-2025 Gothel Software e.K.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef GAMP_WTPOINTEREVENT_HPP_
#define GAMP_WTPOINTEREVENT_HPP_

#include <cstdint>
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/bitfield.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/enum_util.hpp>
#include <jau/fraction_type.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/io_util.hpp>
#include <jau/math/vec2i.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/string_util.hpp>

#include <gamp/GampTypes.hpp>
#include <gamp/wt/event/Event.hpp>

namespace gamp::wt::event {
    /** \addtogroup Gamp_WT
     *
     *  @{
     */

    using namespace jau::enums;

    /** Class of pointer types */
    enum class PointerClass : uint16_t {
        none      = 0,
        /** Desktop compatibility profile. */
        offscreen = 1U << 14,
        /** Desktop core profile. */
        onscreen  = 1U << 15
    };
    JAU_MAKE_ENUM_STRING(PointerClass, offscreen, onscreen);

    /** Type of pointer devices */
    enum class PointerType : uint16_t {
        none        = 0,
        /** Mouse */
        mouse       = number(PointerClass::offscreen) | 1U << 0,
        /** Touch pad, fingers off screen  */
        touchpad    = number(PointerClass::offscreen) | 1U << 1,
        /** Touch screen, fingers on screen. */
        touchscreen = number(PointerClass::onscreen)  | 1U << 2,
        /** Pen usually off-screen */
        pen         = number(PointerClass::offscreen) | 1U << 3
    };
    JAU_MAKE_ENUM_STRING(PointerType, mouse, touchpad, touchscreen, pen);

    constexpr PointerClass getPointerClass(PointerType pt) noexcept {
        const uint16_t m = 1U << 15 | 1U << 14;
        return static_cast<PointerClass>(number(pt) & m);
    }

    /**
     * Pointer event of type PointerType.
     *
     * See also [W3C pointerevent-interface](http://www.w3.org/Submission/pointer-events/#pointerevent-interface).
     *
     * <a name="coordUnit"><h5>Unit of Coordinates</h5></a>
     *
     * All pointer coordinates of this interface are represented in pixel units,
     * see {@link NativeSurface} and {@link NativeWindow}.
     *
     * <a name="multiPtrEvent"><h5>Multiple-Pointer Events</h5></a>
     *
     * In case an instance represents a multiple-pointer event, i.e. {@link #getPointerCount()} is &gt; 1,
     * the first data element of the multiple-pointer fields represents the pointer triggering this event.<br/>
     * For example {@link #getX(int) e.getX(0)} at {@link #EVENT_POINTER_PRESSED} returns the data of the pressed pointer, etc.
     *
     * Multiple-pointer event's {@link #getButton() button number} is mapped to the <i>first {@link #getPointerId(int) pointer ID}</i>
     * triggering the event and the {@link InputEvent#BUTTON1_MASK button mask bits} in the {@link #getModifiers() modifiers}
     * field represent the pressed pointer IDs.
     *
     * Users can query the pressed button and pointer count via {@link InputEvent#getButtonDownCount()}
     * or use the simple query {@link InputEvent#isAnyButtonDown()}.
     *
     * If representing a single-pointer {@link PointerType#Mouse} event, {@link #getPointerId(int) pointer-ID} is <code>0</code>
     * and a {@link #getButton() button value} of <code>0</code> denotes no button activity, i.e. {@link PointerType#Mouse} move.
     */
    class PointerEvent : public InputEvent {
      public:
        /** Returns the 3-axis XYZ rotation array by given rotation on Y axis or X axis (if InputModifier::shift is given in mods). */
        static jau::math::Vec3f swapRotation(float rotationXorY, InputModifier mods) {
            jau::math::Vec3f res;
            if( gamp::wt::event::is_set(mods, InputModifier::shift) ) {
                res.x = rotationXorY;
            } else {
                res.y = rotationXorY;
            }
            return res;
        }

        constexpr static uint16_t clickTimeout() { return 300; }

        /**
         * Constructor for traditional one-pointer event.
         *
         * @param eventType
         * @param source
         * @param when
         * @param modifiers
         * @param pos XY-axis
         * @param clickCount Mouse-button click-count
         * @param button button number, e.g. [{@link #BUTTON1}..{@link #BUTTON_COUNT}-1].
         *               A button value of <code>0</code> denotes no button activity, i.e. {@link PointerType#Mouse} move.
         * @param rotation Rotation of all axis
         * @param rotationScale Rotation scale
         */
        PointerEvent(uint16_t type, const jau::fraction_timespec& when, const WindowRef& source, InputModifier mods,
                     PointerType ptype, uint16_t id,
                     jau::math::Vec2i pos, uint16_t clickCount, InputButton button,
                     jau::math::Vec3f rotation, float rotationScale)
        : InputEvent(type, when, source, mods),
          m_clickCount(clickCount), m_button(button),
          m_rotation(rotation), m_rotationScale(rotationScale), m_maxPressure(1.0f)
        {
            m_pos.push_back(pos);
            switch( type ) {
                case EVENT_POINTER_CLICKED:
                case EVENT_POINTER_PRESSED:
                case EVENT_POINTER_DRAGGED:
                    m_pressure.push_back(constMousePressure1);
                    break;
                default:
                    m_pressure.push_back(constMousePressure0);
            }
            m_pointerID.push_back(id);
            m_pointerType.push_back(ptype);
        }

        /**
         * Constructor for a multiple-pointer event.
         * <p>
         * First element of multiple-pointer arrays represents the pointer which triggered the event!
         * </p>
         * <p>
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * </p>
         *
         * @param eventType
         * @param source
         * @param when
         * @param modifiers
         * @param pointerType PointerType for each pointer (multiple pointer)
         * @param pointerID Pointer ID for each pointer (multiple pointer). IDs start w/ 0 and are consecutive numbers.
         *                  A pointer-ID of -1 may also denote no pointer/button activity, i.e. {@link PointerType#Mouse} move.
         * @param pos XY-axis for each pointer (multiple pointer)
         * @param pressure Pressure for each pointer (multiple pointer)
         * @param maxPressure Maximum pointer pressure for all pointer
         * @param button Corresponding mouse-button
         * @param clickCount Mouse-button click-count
         * @param rotation Rotation of all axis
         * @param rotationScale Rotation scale
         */
        PointerEvent(uint16_t type, const jau::fraction_timespec& when, const WindowRef& source, InputModifier mods,
                     const std::vector<PointerType>& pointerType, const std::vector<uint16_t>& pointerID,
                     const std::vector<jau::math::Vec2i>& pos, const std::vector<float>& pressure, float maxPressure,
                     uint16_t clickCount, InputButton button,
                     const jau::math::Vec3f& rotation, float rotationScale)
        : InputEvent(type, when, source, mods) {
            m_pos                     = pos;
            const size_t pointerCount = pointerType.size();
            if( pointerCount != pointerID.size() ||
                pointerCount != m_pos.size() ||
                pointerCount != m_pressure.size() ) {
                throw jau::IllegalArgumentError("All multiple pointer arrays must be of same size", E_FILE_LINE);
            }
            if( 0.0f >= maxPressure ) {
                throw jau::IllegalArgumentError("maxPressure must be > 0.0f", E_FILE_LINE);
            }
            m_pressure      = pressure;
            m_maxPressure   = maxPressure;
            m_pointerID     = pointerID;
            m_clickCount    = clickCount;
            m_button        = button;
            m_rotation      = rotation;
            m_rotationScale = rotationScale;
            m_pointerType   = pointerType;
        }

        PointerEvent createVariant(uint16_t newEventType) {
            WindowRef s = source().lock();
            return PointerEvent(newEventType, when(), s, modifier(), allPointerTypes(), allPointerIDs(),
                                m_pos, m_pressure, m_maxPressure, m_clickCount, m_button, m_rotation, m_rotationScale);
        }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @return the count of pointers involved in this event
         */
        constexpr size_t pointerCount() const noexcept { return m_pointerType.size(); }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @return the {@link PointerType} for the data at index or null if index not available.
         */
        constexpr PointerType pointerType(size_t index) const noexcept {
            if( index >= m_pointerType.size() ) {
                return PointerType::none;
            }
            return m_pointerType[index];
        }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @return array of all {@link PointerType}s for all pointers
         */
        constexpr const std::vector<PointerType>& allPointerTypes() const noexcept { return m_pointerType; }

        /**
         * Return the pointer id for the given index or -1 if index not available.
         * <p>
         * IDs start w/ 0 and are consecutive numbers.
         * </p>
         * <p>
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * </p>
         */
        constexpr int pointerId(size_t index) const noexcept {
            if( index >= m_pointerID.size() ) {
                return -1;
            }
            return m_pointerID[index];
        }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @return the pointer index for the given pointer id or -1 if id not available.
         */
        int pointerIdx(uint16_t id) const {
            for( int i = gamp::castOrThrow<size_t, int>(m_pointerID.size() - 1); i >= 0; i-- ) {
                if( m_pointerID[i] == id ) {
                    return i;
                }
            }
            return -1;
        }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @return array of all pointer IDs for all pointers. IDs start w/ 0 and are consecutive numbers.
         */
        constexpr const std::vector<uint16_t>& allPointerIDs() const noexcept { return m_pointerID; }

        /**
         * Returns the button number, e.g. [{@link #BUTTON1}..{@link #BUTTON_COUNT}-1].
         * <p>
         * A button value of <code>0</code> denotes no button activity, i.e. {@link PointerType#Mouse} move.
         * </p>
         * <p>
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * </p>
         */
        constexpr InputButton button() const noexcept { return m_button; }

        constexpr uint16_t clickCount() const noexcept { return m_clickCount; }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @param index pointer-index within [0 .. {@link #getPointerCount()}-1]
         * @return XYZ-Coord associated with the pointer-index in pixel units.
         * @see getPointerId(index)
         */
        constexpr const jau::math::Vec2i& position(size_t index = 0) const noexcept { return m_pos[index]; }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @return array of all X-Coords for all pointers in pixel units.
         */
        constexpr const std::vector<jau::math::Vec2i>& allPositions() const noexcept { return m_pos; }

        /**
         * @param normalized if true, method returns the normalized pressure, i.e. <code>pressure / maxPressure</code>
         * @return The pressure associated with the pointer-index 0.
         *         The value of zero is return if not available.
         * @see #getMaxPressure()
         */
        constexpr float pressure(bool normalized) const noexcept { return pressure(0, normalized); }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @param index pointer-index within [0 .. {@link #getPointerCount()}-1]
         * @param normalized if true, method returns the normalized pressure, i.e. <code>pressure / maxPressure</code>
         * @return The pressure associated with the pointer-index.
         *         The value of zero is return if not available.
         * @see #getMaxPressure()
         */
        constexpr float pressure(size_t index, bool normalized) const noexcept {
            return normalized ? m_pressure[index] / m_maxPressure : m_pressure[index];
        }

        /**
         * See details for <a href="#multiPtrEvent">multiple-pointer events</a>.
         * @return array of all raw, un-normalized pressures for all pointers
         */
        constexpr const std::vector<float>& allPressures() const noexcept { return m_pressure; }

        /**
         * Returns the maximum pressure known for the input device generating this event.
         * <p>
         * This value may be self calibrating on devices/OS, where no known maximum pressure is known.
         * Hence subsequent events may return a higher value.
         * </p>
         * <p>
         * Self calibrating maximum pressure is performed on:
         * <ul>
         *   <li>Android</li>
         * </ul>
         * </p>
         */
        constexpr float maxPressure() const noexcept { return m_maxPressure; }

        /**
         * Returns a 3-component float array filled with the values of the rotational axis
         * in the following order: horizontal-, vertical- and z-axis.
         * <p>
         * A vertical rotation of <b>&gt; 0.0f is up</b> and <b>&lt; 0.0f is down</b>.
         * </p>
         * <p>
         * A horizontal rotation of <b>&gt; 0.0f is left</b> and <b>&lt; 0.0f is right</b>.
         * </p>
         * <p>
         * A z-axis rotation of <b>&gt; 0.0f is back</b> and <b>&lt; 0.0f is front</b>.
         * </p>
         * <p>
         * <i>However</i>, on some OS this might be flipped due to the OS <i>default</i> behavior.
         * The latter is true for OS X 10.7 (Lion) for example.
         * </p>
         * <p>
         * On PointerClass {@link PointerClass#Onscreen onscreen} devices, i.e. {@link PointerType#TouchScreen touch screens},
         * rotation events are usually produced by a 2-finger movement, where horizontal and vertical rotation values are filled.
         * </p>
         * <p>
         * On PointerClass {@link PointerClass#Offscreen offscreen} devices, i.e. {@link PointerType#Mouse mouse},
         * either the horizontal or the vertical rotation value is filled.
         * </p>
         * <p>
         * The {@link InputEvent#SHIFT_MASK} modifier is set in case <b>|horizontal| &gt; |vertical|</b> value.<br/>
         * This can be utilized to implement only one 2d rotation direction, you may use {@link #isShiftDown()} to query it.
         * </p>
         * <p>
         * In case the pointer type is {@link PointerType#Mouse mouse},
         * events are usually send in steps of one, ie. <i>-1.0f</i> and <i>1.0f</i>.
         * Higher values may result due to fast scrolling.
         * Fractional values may result due to slow scrolling with high resolution devices.<br/>
         * Here the button number refers to the wheel number.
         * </p>
         * <p>
         * In case the pointer type is of class {@link PointerClass#Onscreen}, e.g. {@link PointerType#TouchScreen touch screen},
         * see {@link #getRotationScale()} for semantics.
         * </p>
         */
        constexpr const jau::math::Vec3f& rotation() const noexcept { return m_rotation; }

        /**
         * Returns the scale used to determine the {@link #getRotation() rotation value},
         * which semantics depends on the {@link #getPointerType() pointer type's} {@link PointerClass}.
         * <p>
         * For {@link PointerClass#Offscreen}, the scale is usually <code>1.0f</code> and denominates
         * an abstract value without association to a physical value.
         * </p>
         * <p>
         * For {@link PointerClass#Onscreen}, the scale varies and denominates
         * the divisor of the distance the finger[s] have moved on the screen.
         * Hence <code>scale * rotation</code> reproduces the screen distance in pixels the finger[s] have moved.
         * </p>
         */
        constexpr float rotationScale() const noexcept { return m_rotationScale; }

        std::string toString() const noexcept {
            std::string sb = "PointerEvent[";
            sb.append(getEventTypeString())
              .append(", pos[")
              .append(jau::to_string(m_pos))
              .append("], button ")
              .append(to_string(m_button))
              .append(", count ")
              .append(std::to_string(m_clickCount));
            if( type() == EVENT_POINTER_WHEEL ) {
              sb.append(", rotation [")
                .append(m_rotation.toString())
                .append("] * ")
                .append(std::to_string(m_rotationScale));
            }
            if( m_pointerID.size() > 0 ) {
                sb.append(", pointer<").append(std::to_string(m_pointerID.size())).append(">[");
                for( size_t i = 0; i < m_pointerID.size(); i++ ) {
                    if( i > 0 ) {
                        sb.append(", ");
                    }
                    sb.append(std::to_string(m_pointerID[i])).append("/").append(to_string(m_pointerType[i])).append(": ")
                      .append(m_pos[i].toString()).append(", ")
                      .append("p[").append(std::to_string(m_pressure[i])).append("/").append(std::to_string(m_maxPressure)).append("=")
                      .append(std::to_string(m_pressure[i] / m_maxPressure)).append("]");
                }
                sb.append("]");
            }
            sb.append(", ").append(InputEvent::toString()).append("]");
            return sb;
        }

      private:
        std::string getEventTypeString() const noexcept {
            switch( type() ) {
                case EVENT_POINTER_CLICKED:  return "CLICKED";
                case EVENT_POINTER_ENTERED:  return "ENTERED";
                case EVENT_POINTER_EXITED:   return "EXITED";
                case EVENT_POINTER_PRESSED:  return "PRESSED";
                case EVENT_POINTER_RELEASED: return "RELEASED";
                case EVENT_POINTER_MOVED:    return "MOVED";
                case EVENT_POINTER_DRAGGED:  return "DRAGGED";
                case EVENT_POINTER_WHEEL:    return "WHEEL";
                default:                     return "unknown (" + std::to_string(type()) + ")";
            }
        }

        /** PointerType for each pointer (multiple pointer) */
        std::vector<PointerType>      m_pointerType;
        /** Pointer-ID for each pointer (multiple pointer). IDs start w/ 0 and are consecutive numbers. */
        std::vector<uint16_t>         m_pointerID;
        /** XY-axis for each pointer (multiple pointer) */
        std::vector<jau::math::Vec2i> m_pos;
        /** Pressure for each pointer (multiple pointer) */
        std::vector<float>            m_pressure;
        // private final uint16_t tiltX[], tiltY[]; // TODO: A generic way for pointer axis information, see Android MotionEvent!
        uint16_t                      m_clickCount;
        /**
         * Returns the button number, e.g. [{@link #BUTTON1}..{@link #BUTTON_COUNT}-1].
         * <p>
         * A button value of <code>0</code> denotes no button activity, i.e. {@link PointerType#Mouse} move.
         * </p>
         */
        InputButton                   m_button;
        /** Rotation around the X, Y and X axis */
        jau::math::Vec3f              m_rotation;
        /** Rotation scale */
        float                         m_rotationScale;
        float                         m_maxPressure;

        constexpr static float       constMousePressure0    = 0.0f;
        constexpr static float       constMousePressure1    = 1.0f;
        constexpr static PointerType constMousePointerTypes = PointerType::mouse;
    };

    /**
     * Listener for PointerEvent
     *
     * @see PointerEvent
     */
    class PointerListener {
      public:
        virtual ~PointerListener() noexcept = default;

        virtual void pointerClicked(const PointerEvent&) { }
        /** Only generated for {@link PointerType#Mouse} */
        virtual void pointerEntered(const PointerEvent&) { }
        /** Only generated for {@link PointerType#Mouse} */
        virtual void pointerExited(const PointerEvent&) { }
        virtual void pointerPressed(const PointerEvent&) { }
        virtual void pointerReleased(const PointerEvent&) { }
        virtual void pointerMoved(const PointerEvent&) { }
        virtual void pointerDragged(const PointerEvent&) { }

        /**
         * Traditional event name originally produced by a PointerType::mouse pointer type.
         *
         * Triggered for any rotational pointer events, see
         * PointerEvent::rotation() and PointerEvent::rotationScale().
         */
        virtual void pointerWheelMoved(const PointerEvent&) { }
    };
    typedef std::shared_ptr<PointerListener> PointerListenerRef;

    /**@}*/

}  // namespace gamp::wt::event

#endif /* GAMP_WTPOINTEREVENT_HPP_ */
