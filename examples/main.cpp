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

// C
#include <libgen.h> // basename
#include <signal.h>

// Standard
#include <iostream>
#include <string>
#include <unistd.h>
#include <thread>
#include <vector>

#include <ert/tracing/Logger.hpp>

#include <ert/multipart/Consumer.hpp>

const char* progname;

bool fromHexString(const std::string &input, std::string &output) {

    bool result = true; // supposed successful by default

    bool has0x = (input.rfind("0x", 0) == 0);

    if((input.length() % 2) != 0) {
        std::cerr << "Invalid hexadecimal string due to odd length" << '\n';
        return false;
    }

    output = "";
    const char* src = input.data(); // fastest that accessing input[ii]
    unsigned char hex;
    int aux;

    for(int ii = 1 + (has0x ? 2:0), maxii = input.length(); ii < maxii; ii += 2) {
        if(isxdigit(aux = src[ii-1]) == 0) {
            std::cerr << "Invalid hexadecimal string" << '\n';
            return false;
        }

        hex = ((aux >= '0' && aux <= '9') ? (aux - '0') : ((aux - 'a') + 0x0a)) << 4;

        if(isxdigit(aux = src[ii]) == 0) {
            std::cerr << "Invalid hexadecimal string" << '\n';
            return false;
        }

        hex |= (aux >= '0' && aux <= '9') ? (aux - '0') : ((aux - 'a') + 0x0a);
        output += hex;
    }

    return result;
}

void sighndl(int signal)
{
    LOGWARNING(ert::tracing::Logger::warning(ert::tracing::Logger::asString("Signal received: %d", signal), ERT_FILE_LOCATION));
    switch (signal) {
    case SIGTERM:
    case SIGINT:
        exit(1);
        break;
    }
}

int main(int argc, char* argv[]) {

    progname = basename(argv[0]);
    ert::tracing::Logger::initialize(progname);
    ert::tracing::Logger::verbose();
    //ert::tracing::Logger::setLevel(ert::tracing::Logger::Debug);

    // Capture TERM/INT signals for graceful exit:
    signal(SIGTERM, sighndl);
    signal(SIGINT, sighndl);

    ert::multipart::Consumer consumer("abcdef12345");

    // Octetstream part: 0x26800126 (0x26 is '&')
    std::string bodyAsHex = "0x2d2d61626364656631323334350d0a436f6e74656e742d547970653a206170706c69636174696f6e2f6a736f6e0d0a0d0a7b226e314e6f74696679537562736372697074696f6e4964223a226e6f74696679537562736372697074696f6e49443030303031222c226e314d657373616765436f6e7461696e6572223a7b226e314d657373616765436c617373223a2255504450222c226e314d657373616765436f6e74656e74223a7b22636f6e74656e744964223a227565506f6c227d7d7d0d0a2d2d61626364656631323334350d0a436f6e74656e742d547970653a206170706c69636174696f6e2f6f637465742d73747265616d0d0a0d0a268001260d0a2d2d61626364656631323334352d2d";
    std::string body;
    bool ok = fromHexString(bodyAsHex, body);

    /*
    This is the buffer:

    --abcdef12345
    Content-Type: application/json

    {"n1NotifySubscriptionId":"notifySubscriptionID00001","n1MessageContainer":{"n1MessageClass":"UPDP","n1MessageContent":{"contentId":"uePol"}}}
    --abcdef12345
    Content-Type: application/octet-stream

    &&
    --abcdef12345--
    */

    std::cout << std::endl << "=== Body as string ===" << std::endl << body << std::endl;
    std::cout << std::endl << "=== Body multipart decoded ===" << std::endl << consumer.decode(body) << std::endl;

    return 0;
}
