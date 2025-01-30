/*
 * Author: Sven Gothel <sgothel@jausoft.com> and Svenson Han Gothel
 * Copyright (c) 2022-2024 Gothel Software e.K.
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
#include "gamp/gamp.hpp"

#include <ctime>
#include <jau/environment.hpp>
#include <jau/file_util.hpp>

static std::string m_asset_dir;

//
//
//

std::string gamp::lookup_and_register_asset_dir(const char* exe_path, const char* asset_file, const char* asset_install_subdir) noexcept {
    m_asset_dir = jau::fs::lookup_asset_dir(exe_path, asset_file, asset_install_subdir);
    return m_asset_dir;
}
std::string gamp::asset_dir() noexcept { return m_asset_dir; }

std::string gamp::resolve_asset(const std::string &asset_file, bool lookup_direct) noexcept {
    if( lookup_direct && jau::fs::exists(asset_file) ) {
        return asset_file;
    }
    if( m_asset_dir.size() ) {
        std::string fname1 = m_asset_dir+"/"+asset_file;
        if( jau::fs::exists(fname1) ) {
            return fname1;
        }
    }
    return "";
}

