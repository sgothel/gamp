/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2025 Gothel Software e.K.
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
