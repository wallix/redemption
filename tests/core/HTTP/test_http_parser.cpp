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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test for http parsers
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/HTTP/http_parser.hpp"
#include <vector>

using namespace http;

class CustomHttpReqParser : public HttpRequestParser
{
public:
	bool onHeadersTreated() override {
		headersParsed = true;
		return true;
	}
	bool onBodyContent(chars_view content) override {
		bodySizes.push_back(content.size());
		return true;
	}

	bool onParsingCompleted() override {
		parsingCompleted++;
		return true;
	}

	std::vector<size_t> bodySizes;
	int parsingCompleted = 0;
	bool headersParsed = false;
};

RED_AUTO_TEST_CASE(TestRequestParser)
{
	CustomHttpReqParser reqParser;

	// =========================================================================
	// First test: a simple request in 1 chunk
	std::string_view reqStr =
		"RDG_OUT_DATA /remoteDesktopGateway/ HTTP/1.1\r\n"
		"Cache-Control: no-cache\r\n"
		"Connection: Upgrade\r\n"
		"Pragma: no-cache\r\n"
		"\r\n";

	RED_CHECK_EQ(reqParser.parse(reqStr), true);
	RED_CHECK_EQ(reqParser.parsingCompleted, 1);
	RED_CHECK_EQ(reqParser.bodySizes.size(), 0);

	// =========================================================================
	// second test: a simple request in but splitted in multiple chunks
	reqParser.parsingCompleted = 0;
	reqParser.headersParsed = false;
	std::string_view splittedRequest[] = {
		"RDG_OUT_DATA /remoteDesktopGateway/ HTTP/1.1\r\nCache-Control:",
		"no-cache\r\nConnection: Upg",
		"rade\r\nPragma: no-cache\r\n\r\n"
	};

	RED_CHECK_EQ(reqParser.parse(splittedRequest[0]), true);
	RED_CHECK_EQ(reqParser.parsingCompleted, 0);
	RED_CHECK_EQ(reqParser.headersParsed, false);

	RED_CHECK_EQ(reqParser.parse(splittedRequest[1]), true);
	RED_CHECK_EQ(reqParser.parsingCompleted, 0);
	RED_CHECK_EQ(reqParser.headersParsed, false);

	RED_CHECK_EQ(reqParser.parse(splittedRequest[2]), true);
	RED_CHECK_EQ(reqParser.parsingCompleted, 1);
	RED_CHECK_EQ(reqParser.headersParsed, true);
	RED_CHECK_EQ(reqParser.bodySizes.size(), 0);

	// =========================================================================
	// third test: a simple request with a body
	reqParser.parsingCompleted = 0;
	std::string_view reqWithBody =
		"POST /path/script.cgi HTTP/1.0\r\n"
		"Content-Length: 32\r\n"
		"\r\n"
		"home=Cosby&favorite+flavor=flies";
	RED_CHECK_EQ(reqParser.parse(reqWithBody), true);
	RED_CHECK_EQ(reqParser.parsingCompleted, 1);
	RED_CHECK_EQ(reqParser.bodySizes.size(), 1);
	RED_CHECK_EQ(reqParser.bodySizes.at(0), 32);

	// =========================================================================
	// fourth test: a request with a chunked body
	reqParser.parsingCompleted = 0;
	reqParser.bodySizes.clear();
	std::string_view reqBodyChunked[] = {
		"POST /path/script.cgi HTTP/1.0\r\n",
		"Content-Length: 32\r\n",
		"\r\n",
		"home=Cosby",
		"&favorite+fla",
		"vor=flies"
	};

	for (std::string_view reqPart : reqBodyChunked) {
		RED_CHECK_EQ(reqParser.parse(reqPart), true);
	}

	RED_CHECK_EQ(reqParser.parsingCompleted, 1);
	RED_CHECK_EQ(reqParser.uri, "/path/script.cgi");
	RED_CHECK_EQ(reqParser.method, "POST");
	RED_CHECK_EQ(reqParser.httpVersion, "HTTP/1.0");
	RED_CHECK_EQ(reqParser.bodySizes.size(), 3);
	RED_CHECK_EQ(reqParser.bodySizes.at(0), 10);
	RED_CHECK_EQ(reqParser.bodySizes.at(1), 13);
	RED_CHECK_EQ(reqParser.bodySizes.at(2), 9);
}

class CustomHttpResponseParser : public HttpResponseParser {
public:
	bool onHeadersTreated() override {
		headersParsed = true;
		return true;
	}
	bool onBodyContent(chars_view content) override {
		bodySizes.push_back(content.size());
		return true;
	}

	bool onParsingCompleted() override {
		parsingCompleted++;
		return true;
	}

	std::vector<size_t> bodySizes;
	int parsingCompleted = 0;
	bool headersParsed = false;
};


RED_AUTO_TEST_CASE(TestResponseParser)
{
	CustomHttpResponseParser respParser;

	// =========================================================================
	// First test: a response with an empty body
	std::string_view respStr =
		"HTTP/1.1 401 Access Denied\r\n"
		"Server: Microsoft-HTTPAPI/2.0\r\n"
		"RDG-Auth-Scheme: SMARTCARD\r\n"
		"WWW-Authenticate: Negotiate\r\n"
		"Content-Length: 0\r\n"
		"\r\n";

	RED_CHECK_EQ(respParser.parse(respStr), true);
	RED_CHECK_EQ(respParser.responseCode, 401);
	RED_CHECK_EQ(respParser.responseVersion, "HTTP/1.1");
	RED_CHECK_EQ(respParser.responseString, "Access Denied");
	RED_CHECK_EQ(respParser.bodySizes.size(), 0);

	// =========================================================================
	// Second test: a response with chunked body
	respParser.headersParsed = false;
	std::string_view chunkedRespStr[] = {
		"HTTP/1.1 200 Ok\r\n"
		"Server: Microsoft-HTTPAPI/2.0\r\n"
		"RDG-Auth-Scheme: SMARTCARD\r\n"
		"WWW-Authenticate: Negotiate\r\n",

		"Content-Length: 6\r\n"
		"\r\n",

		"cou",

		"cou"
	};

	RED_CHECK_EQ(respParser.parse(chunkedRespStr[0]), true);
	RED_CHECK_EQ(respParser.headersParsed, false);
	RED_CHECK_EQ(respParser.bodySizes.size(), 0);
	RED_CHECK_EQ(respParser.parsingCompleted, 1);

	RED_CHECK_EQ(respParser.parse(chunkedRespStr[1]), true);
	RED_CHECK_EQ(respParser.responseCode, 200);
	RED_CHECK_EQ(respParser.responseVersion, "HTTP/1.1");
	RED_CHECK_EQ(respParser.responseString, "Ok");
	RED_CHECK_EQ(respParser.parsingCompleted, 1);
	RED_CHECK_EQ(respParser.headersParsed, true);

	RED_CHECK_EQ(respParser.parse(chunkedRespStr[2]), true);
	RED_CHECK_EQ(respParser.parsingCompleted, 1);

	RED_CHECK_EQ(respParser.parse(chunkedRespStr[3]), true);
	RED_CHECK_EQ(respParser.parsingCompleted, 2);
	RED_CHECK_EQ(respParser.bodySizes.size(), 2);
	RED_CHECK_EQ(respParser.bodySizes.at(0) + respParser.bodySizes.at(1), 6);
}
