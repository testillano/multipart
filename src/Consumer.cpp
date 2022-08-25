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
Copyright (c) 2021 Eduardo Ramos

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

#include <stdio.h>
#include <string.h>

#include <ert/tracing/Logger.hpp>

#include <ert/multipart/Consumer.hpp>

#define NOTIFY_CB(FOR)                                                 \
do {                                                                   \
  if (p->settings->on_##FOR) {                                         \
    if (p->settings->on_##FOR(p) != 0) {                               \
      return i;                                                        \
    }                                                                  \
  }                                                                    \
} while (0)

#define EMIT_DATA_CB(FOR, ptr, len)                                    \
do {                                                                   \
  if (p->settings->on_##FOR) {                                         \
    if (p->settings->on_##FOR(p, ptr, len) != 0) {                     \
      return i;                                                        \
    }                                                                  \
  }                                                                    \
} while (0)


#define LF 10
#define CR 13


namespace ert
{
namespace multipart
{

struct multipart_parser {
    void * data;

    size_t index;
    size_t boundary_length;

    unsigned char state;

    const multipart_parser_settings* settings;

    char* lookbehind;
    char multipart_boundary[1];
};

enum state {
    s_uninitialized = 1,
    s_start,
    s_start_boundary,
    s_header_field_start,
    s_header_field,
    s_headers_almost_done,
    s_header_value_start,
    s_header_value,
    s_header_value_almost_done,
    s_part_data_start,
    s_part_data,
    s_part_data_almost_boundary,
    s_part_data_boundary,
    s_part_data_almost_end,
    s_part_data_end,
    s_part_data_final_hyphen,
    s_end
};

multipart_parser* multipart_parser_init
(const char *boundary, const multipart_parser_settings* settings) {

    size_t boundary_length = strlen(boundary) + 2; // boundary must be prefixed by "--"

    multipart_parser* p = (multipart_parser*)malloc(sizeof(multipart_parser) + 2*boundary_length + 9);

    strcpy(p->multipart_boundary, "--");
    strcpy(p->multipart_boundary + 2, boundary);
    p->boundary_length = boundary_length;

    p->lookbehind = (p->multipart_boundary + p->boundary_length + 1);

    p->index = 0;
    p->state = s_start;
    p->settings = settings;

    return p;
}

void multipart_parser_free(multipart_parser* p) {
    free(p);
}

void multipart_parser_set_data(multipart_parser *p, void *data) {
    p->data = data;
}

void *multipart_parser_get_data(multipart_parser *p) {
    return p->data;
}

size_t multipart_parser_execute(multipart_parser* p, const char *buf, size_t len) {
    size_t i = 0;
    size_t mark = 0;
    char c, cl;
    int is_last = 0;

    while(i < len) {
        c = buf[i];
        is_last = (i == (len - 1));
        switch (p->state) {
        case s_start:
            LOGDEBUG(ert::tracing::Logger::debug("s_start",  ERT_FILE_LOCATION));
            p->index = 0;
            p->state = s_start_boundary;

        // fallthrough
        case s_start_boundary:
            LOGDEBUG(ert::tracing::Logger::debug("s_start_boundary",  ERT_FILE_LOCATION));
            if (p->index == p->boundary_length) {
                if (c != CR) {
                    return i;
                }
                p->index++;
                break;
            } else if (p->index == (p->boundary_length + 1)) {
                if (c != LF) {
                    return i;
                }
                p->index = 0;
                NOTIFY_CB(part_data_begin);
                p->state = s_header_field_start;
                break;
            }
            if (c != p->multipart_boundary[p->index]) {
                return i;
            }
            p->index++;
            break;

        case s_header_field_start:
            LOGDEBUG(ert::tracing::Logger::debug("s_header_field_start",  ERT_FILE_LOCATION));
            mark = i;
            p->state = s_header_field;

        // fallthrough
        case s_header_field:
            LOGDEBUG(ert::tracing::Logger::debug("s_header_field",  ERT_FILE_LOCATION));
            if (c == CR) {
                p->state = s_headers_almost_done;
                break;
            }

            if (c == ':') {
                EMIT_DATA_CB(header_field, buf + mark, i - mark);
                p->state = s_header_value_start;
                break;
            }

            cl = tolower(c);
            if ((c != '-') && (cl < 'a' || cl > 'z')) {
                LOGDEBUG(ert::tracing::Logger::debug("invalid character in header name",  ERT_FILE_LOCATION));
                return i;
            }
            if (is_last)
                EMIT_DATA_CB(header_field, buf + mark, (i - mark) + 1);
            break;

        case s_headers_almost_done:
            LOGDEBUG(ert::tracing::Logger::debug("s_headers_almost_done",  ERT_FILE_LOCATION));
            if (c != LF) {
                return i;
            }

            p->state = s_part_data_start;
            break;

        case s_header_value_start:
            LOGDEBUG(ert::tracing::Logger::debug("s_header_value_start",  ERT_FILE_LOCATION));
            if (c == ' ') {
                break;
            }

            mark = i;
            p->state = s_header_value;

        // fallthrough
        case s_header_value:
            LOGDEBUG(ert::tracing::Logger::debug("s_header_value",  ERT_FILE_LOCATION));
            if (c == CR) {
                EMIT_DATA_CB(header_value, buf + mark, i - mark);
                p->state = s_header_value_almost_done;
                break;
            }
            if (is_last)
                EMIT_DATA_CB(header_value, buf + mark, (i - mark) + 1);
            break;

        case s_header_value_almost_done:
            LOGDEBUG(ert::tracing::Logger::debug("s_header_value_almost_done",  ERT_FILE_LOCATION));
            if (c != LF) {
                return i;
            }
            p->state = s_header_field_start;
            break;

        case s_part_data_start:
            LOGDEBUG(ert::tracing::Logger::debug("s_part_data_start",  ERT_FILE_LOCATION));
            NOTIFY_CB(headers_complete);
            mark = i;
            p->state = s_part_data;

        // fallthrough
        case s_part_data:
            LOGDEBUG(ert::tracing::Logger::debug("s_part_data",  ERT_FILE_LOCATION));
            if (c == CR) {
                EMIT_DATA_CB(part_data, buf + mark, i - mark);
                mark = i;
                p->state = s_part_data_almost_boundary;
                p->lookbehind[0] = CR;
                break;
            }
            if (is_last)
                EMIT_DATA_CB(part_data, buf + mark, (i - mark) + 1);
            break;

        case s_part_data_almost_boundary:
            LOGDEBUG(ert::tracing::Logger::debug("s_part_data_almost_boundary",  ERT_FILE_LOCATION));
            if (c == LF) {
                p->state = s_part_data_boundary;
                p->lookbehind[1] = LF;
                p->index = 0;
                break;
            }
            EMIT_DATA_CB(part_data, p->lookbehind, 1);
            p->state = s_part_data;
            mark = i --;
            break;

        case s_part_data_boundary:
            LOGDEBUG(ert::tracing::Logger::debug("s_part_data_boundary",  ERT_FILE_LOCATION));
            if (p->multipart_boundary[p->index] != c) {
                EMIT_DATA_CB(part_data, p->lookbehind, 2 + p->index);
                p->state = s_part_data;
                mark = i --;
                break;
            }
            p->lookbehind[2 + p->index] = c;
            if ((++ p->index) == p->boundary_length) {
                NOTIFY_CB(part_data_end);
                p->state = s_part_data_almost_end;
            }
            break;

        case s_part_data_almost_end:
            LOGDEBUG(ert::tracing::Logger::debug("s_part_data_almost_end",  ERT_FILE_LOCATION));
            if (c == '-') {
                p->state = s_part_data_final_hyphen;
                break;
            }
            if (c == CR) {
                p->state = s_part_data_end;
                break;
            }
            return i;

        case s_part_data_final_hyphen:
            LOGDEBUG(ert::tracing::Logger::debug("s_part_data_final_hyphen",  ERT_FILE_LOCATION));
            if (c == '-') {
                NOTIFY_CB(body_end);
                p->state = s_end;
                break;
            }
            return i;

        case s_part_data_end:
            LOGDEBUG(ert::tracing::Logger::debug("s_part_data_end",  ERT_FILE_LOCATION));
            if (c == LF) {
                p->state = s_header_field_start;
                NOTIFY_CB(part_data_begin);
                break;
            }
            return i;

        case s_end:
            LOGDEBUG(ert::tracing::Logger::debug(ert::tracing::Logger::asString("s_end: %02X", (int)c),  ERT_FILE_LOCATION));
            break;

        default:
            LOGDEBUG(ert::tracing::Logger::debug("Multipart parser unrecoverable error",  ERT_FILE_LOCATION));
            return 0;
        }
        ++ i;
    }

    return len;
}

// Consumer class:

int Consumer::ReadHeaderName(multipart_parser* p, const char *at, size_t length)
{
    Consumer* me = (Consumer*)multipart_parser_get_data(p);
    me->output_.append(at, length);
    me->output_.append("\n");

    return 0;
}


}
}

