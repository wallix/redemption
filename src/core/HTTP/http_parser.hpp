/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): David Fort
*/

#pragma once

#include "utils/sugar/array_view.hpp"
#include <string>


namespace http {

/** @brief a generic HTTP parser (for request or response) */
class HttpParser {
public:
    /** default ctor */
    HttpParser();

    virtual ~HttpParser();

    /** callbacks that should be implemented by inherited classes to implement the
     * expected behavior
     * @{ */

    /** called when the first line is read
     * @param line the content of the line
     * @return if the parsing completed successfully
     */
    virtual bool onFirstLine(const std::string &line) = 0;

    /** called when the headers have been treated and we're treating the body
     * @return if the operation was successful
     */
    virtual bool onHeadersTreated();

    /** called when we have some body content
     * @return if the processing was successful
     */
    virtual bool onBodyContent(array_view_const_char input);

    /** called when the parsing is finished
     * @return if the processing was successful
     */
    virtual bool onParsingCompleted();

    /** @} */

    /** inject the buffer in the automata doing all appropriate treatments
     * @param content input data for the parser
     * @return if the processing was successful
     */
    bool parse(array_view_const_char content);

    /** returns if the corresponding header was parsed in the headers
     * @param header the name of the header
     * @return if the corresponding header was parsed in the headers
     */
    [[nodiscard]] bool haveHeader(const std::string &header) const;

    /** returns the value of the given HTTP header
     * @param header the name of the header
     * @return the value of the given HTTP header
     */
    [[nodiscard]] std::string getHeader(const std::string &header) const;

protected:
    bool treatHeaderLine(const std::string &line);

protected:
    class HttpParserImpl;
    HttpParserImpl *impl;
};

/** @brief a parser for HTTP requests */
class HttpRequestParser : public HttpParser {
public:
    bool onFirstLine(const std::string &line) override;
public:
    std::string method;
    std::string uri;
    std::string httpVersion;
};


/** @brief a parser for HTTP responses */
class HttpResponseParser : public HttpParser {
public:
    bool onFirstLine(const std::string &line) override;
public:
    std::string responseVersion;
    int responseCode;
    std::string responseString;
};

} // namespace http
