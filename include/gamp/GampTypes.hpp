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

#ifndef JAU_GAMP_TYPES_HPP_
#define JAU_GAMP_TYPES_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <optional>

#include <jau/cpp_lang_util.hpp>
#include <jau/environment.hpp>
#include <jau/os/os_support.hpp>

#include <jau/float_math.hpp>
#include <jau/math/vec2f.hpp>
#include <jau/math/vec2i.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/recti.hpp>
#include <jau/math/util/pmvmat4.hpp>
#include <jau/math/util/float_util.hpp>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

namespace gamp {
    /** \addtogroup Gamp
     *
     *  @{
     */

    /** A native handle type, big enough to store a pointer. */
    typedef uintptr_t handle_t;

    class GampException : public jau::RuntimeException {
      protected:
        GampException(std::string &&type, std::string const& m, const char* file, int line) noexcept
        : RuntimeException(std::move(type), m, file, line) {}

      public:
        GampException(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("GampException", m, file, line) {}
    };

    template<typename T, typename U>
    inline bool checkOverflow(T has) {
        return has <= std::numeric_limits<U>::max();
    }
    template<typename T, typename U>
    inline void throwOnOverflow(T has) {
        if( has > std::numeric_limits<U>::max() ) {
            throw GampException("Value "+std::to_string(has)+" > "+std::to_string(std::numeric_limits<U>::max()), E_FILE_LINE);
        }
    }
    template<typename T, typename U>
    inline U castOrThrow(T has) {
        if( has > std::numeric_limits<U>::max() ) {
            throw GampException("Value "+std::to_string(has)+" > "+std::to_string(std::numeric_limits<U>::max()), E_FILE_LINE);
        }
        return static_cast<U>(has);
    }

    /** An attachable object */
    class Attachable {
      public:
        virtual ~Attachable() noexcept = default;
        virtual const jau::type_info& signature() const noexcept { return jau::static_ctti<Attachable>(); }
    };
    typedef std::shared_ptr<Attachable> AttachableRef;

    template<typename K, typename V>
    using StringlikeHashMap = std::unordered_map<K, V, jau::string_hash, std::equal_to<>>;

    template<typename Value_type, typename Novalue_type, Novalue_type no_value>
    class StringHashMapWrap {
      private:
        jau::StringHashMap<Value_type> m_map;

      public:
        jau::StringHashMap<Value_type>& map() noexcept { return m_map; }
        const jau::StringHashMap<Value_type>& map() const noexcept { return m_map; }

        /** Returns the mapped value for the given name or `no_value` */
        Value_type get(std::string_view key) const {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                return it->second;
            }
            return no_value;
        }

        /** Returns true if the given name maps to a value or `no_value`. */
        bool containsKey(std::string_view key) const {
            return m_map.contains(key);
        }

        /** Returns the string_view key of the first value, otherwise std::nullopt. Note: O(n) operation, slow. */
        std::optional<std::string_view> containsValue(const Value_type& value) const {
            for (const std::pair<const std::string, Value_type>& n : m_map) {
                if( n.second == value ) {
                    return std::optional<std::string_view>{n.first};
                }
            }
            return std::nullopt;
        }

        /** Clears the hash map. */
        void clear() { m_map.clear(); }

        /**
         * Maps the value for the given name, overwrites old mapping if exists.
         * @return previously mapped value or `no_value`.
         */
        Value_type put(std::string_view key, const Value_type& obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                Value_type old = it->second;
                it->second        = obj;
                return old;
            }
            m_map.insert({std::string(key), obj });
            // m_attachedMap[key] = obj;
            return no_value;
        }

        /** Removes value if mapped and returns it, otherwise returns `no_value`. */
        Value_type remove(std::string_view key) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                Value_type old = it->second;
                m_map.erase(it);
                return old;
            }
            return no_value;
        }
    };
    using StringAttachables = StringHashMapWrap<AttachableRef, std::nullptr_t, nullptr>;

    /**@}*/
}  // namespace gamp

#endif /*  JAU_GAMP_TYPES_HPP_ */
