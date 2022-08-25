/*
 ______________________________________________________________________
|            _                          _ _   _                  _     |
|           | |                        | | | (_)                | |    |
|   ___ _ __| |_   __   _ __ ___  _   _| | |_ _ _ __   __ _ _ __| |_   | Multipart parser library C++
|  / _ \ '__| __| |__| | '_ ` _ \| | | | | __| | '_ \ / _` | '__| __|  | Forked and modified from https://github.com/iafonov/multipart-parser-c
| |  __/ |  | |_       | | | | | | |_| | | |_| | |_) | (_| | |  | |_   | Version 1.0.z
|  \___|_|   \__|      |_| |_| |_|\__,_|_|\__|_| .__/ \__,_|_|   \__|  | https://github.com/testillano/multipart
|                                              | |                     |
|                                              |_|                     |
|______________________________________________________________________|

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Eduardo Ramos

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <string>
#include <stdlib.h>
#include <ctype.h>


namespace ert
{
namespace multipart
{

typedef struct multipart_parser multipart_parser;
typedef struct multipart_parser_settings multipart_parser_settings;
typedef struct multipart_parser_state multipart_parser_state;

typedef int (*multipart_data_cb) (multipart_parser*, const char *at, size_t length);
typedef int (*multipart_notify_cb) (multipart_parser*);

struct multipart_parser_settings {
    multipart_data_cb on_header_field;
    multipart_data_cb on_header_value;
    multipart_data_cb on_part_data;

    multipart_notify_cb on_part_data_begin;
    multipart_notify_cb on_headers_complete;
    multipart_notify_cb on_part_data_end;
    multipart_notify_cb on_body_end;
};

multipart_parser* multipart_parser_init
(const char *boundary, const multipart_parser_settings* settings);

void multipart_parser_free(multipart_parser* p);

size_t multipart_parser_execute(multipart_parser* p, const char *buf, size_t len);

void multipart_parser_set_data(multipart_parser* p, void* data);
void *multipart_parser_get_data(multipart_parser* p);

class Consumer {
    static int ReadHeaderName(multipart_parser* p, const char *at, size_t length);
    static int ReadHeaderValue(multipart_parser* p, const char *at, size_t length);
    static int ReadData(multipart_parser* p, const char *at, size_t length);

    multipart_parser* parser_;
    multipart_parser_settings callbacks_;

    static std::string last_header_name_;

public:

    /**
    * Default constructor
    *
    * @param boundary Multipart boundary string
    */
    Consumer(const std::string& boundary);
    ~Consumer();

    /**
    * Decode body multipart
    *
    * @param body Body content
    */
    void decode(const std::string& body);

    /**
    * Callback for new decoded header
    *
    * @param name header name (i.e. content-type)
    * @param value header value (i.e. application/json)
    */
    virtual void receiveHeader(const std::string &name, const std::string &value) = 0;

    /**
    * Callback for new decoded data part
    *
    * @param data Body data
    */
    virtual void receiveData(const std::string &data) = 0;
};

}
}

