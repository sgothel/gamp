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

#ifndef GAMP_GLMISC_HPP_
#define GAMP_GLMISC_HPP_

#include <optional>
#include <string_view>

#include <jau/basic_types.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>
#include <jau/util/VersionNumber.hpp>

#include <gamp/gamp_types.hpp>
#include <gamp/renderer/gl/glheader.hpp>
#include <gamp/renderer/gl/glliterals.hpp>
#include <gamp/renderer/gl/glversionnum.hpp>

namespace gamp::render::gl {

    /** \addtogroup Gamp_GL
     *
     *  @{
     */

    class GLException : public jau::RuntimeException {
      public:
        GLException(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("GLException", m, file, line) {}
    };

    template<typename T, typename U>
    inline void throwOnOverflow(T has) {
        if( has > std::numeric_limits<U>::max() ) {
            throw GLException("Value "+std::to_string(has)+" > "+std::to_string(std::numeric_limits<U>::max()), E_FILE_LINE);
        }
    }
    template<typename T, typename U>
    inline U castOrThrow(T has) {
        if( has > std::numeric_limits<U>::max() ) {
            throw GLException("Value "+std::to_string(has)+" > "+std::to_string(std::numeric_limits<U>::max()), E_FILE_LINE);
        }
        return static_cast<U>(has);
    }

    /** An attachable object */
    struct Attachable {};
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

} // namespace gamp::render::gl


#endif /* GAMP_GLMISC_HPP_ */
