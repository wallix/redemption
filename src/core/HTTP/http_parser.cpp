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

#include "http_parser.hpp"

#include <algorithm>
#include <map>

namespace http {

/** @brief internal state of the HTTP parser */
typedef enum {
	HTTP_WAITING_FIRST_LINE,
	HTTP_TREATING_HEADERS,
	HTTP_TREATING_BODY
} HttpParserState;

/** @brief internal data for a HTTP parser */
struct HttpParserImpl {
	std::string buffer;
	HttpParserState state;
	uint64_t bodyLength;
	uint64_t remainingBody;
	std::map<std::string, std::string> headers;

	HttpParserImpl()
		: state(HTTP_WAITING_FIRST_LINE)
		, bodyLength(0)
		, remainingBody(0)
	{
	}

	void reset() {
		this->state = HTTP_WAITING_FIRST_LINE;
		this->bodyLength = 0;
		this->remainingBody = 0;
		this->headers.clear();
	}
};

HttpParser::HttpParser() : impl(new HttpParserImpl()) {
}

HttpParser::~HttpParser() {
	delete impl;
}

bool HttpParser::onHeadersTreated() {
	return true;
}

bool HttpParser::onBodyContent(array_view_const_char /*content*/) {
	return true;
}

bool HttpParser::onParsingCompleted() {
	return true;
}

bool HttpParser::parse(array_view_const_char input) {
	impl->buffer.append(input.data(), input.size());

	while (true) {
		switch(impl->state) {
		case HTTP_WAITING_FIRST_LINE:
		case HTTP_TREATING_HEADERS: {
			// buffer looks like either the first line:
			// RDG_OUT_DATA /remoteDesktopGateway/ HTTP/1.1\r\n
			//                                        pos  ^
			// or a header line:
			// Pragma: no-cache\r\n
			//            pos  ^
			// or the last line:
			// \r\n
			// ^   pos
			size_t pos = impl->buffer.find("\r\n");
			if (pos == std::string::npos)
				return true;

			std::string line = impl->buffer.substr(0, pos);
			if (impl->state == HTTP_WAITING_FIRST_LINE) {
				if (!this->onFirstLine(line))
					return false;
				impl->state = HTTP_TREATING_HEADERS;
			} else if (pos == 0) {
				impl->remainingBody = impl->bodyLength;
				if (!this->onHeadersTreated())
					return false;

				if (impl->bodyLength == 0) {
					bool ret = this->onParsingCompleted();
					impl->reset();
					if (!ret)
						return ret;
					impl->state = HTTP_WAITING_FIRST_LINE;
				} else {
					impl->state = HTTP_TREATING_BODY;
				}

			} else {
				if (!this->treatHeaderLine(line))
					return false;
			}

			impl->buffer = impl->buffer.substr(pos + 2);
			break;
		}
		case HTTP_TREATING_BODY: {
			size_t toEat = (impl->remainingBody > impl->buffer.size()) ? impl->buffer.size() : impl->remainingBody;
			if (!toEat)
				return true;

			std::string bodyPiece = impl->buffer.substr(0, toEat);
			bool ret = this->onBodyContent(array_view_const_char{bodyPiece.data(), bodyPiece.size()});
			impl->remainingBody -= toEat;
			impl->buffer = impl->buffer.substr(toEat);
			if (!ret)
				return false;

			if (!impl->remainingBody) {
				bool ret = this->onParsingCompleted();
				impl->reset();

				if(!ret)
					return false;
			}
			break;
		}
		default:
			break;
		}
	}
}

bool HttpParser::treatHeaderLine(const std::string &line) {
	// line looks like:
	//       v  pos1
	// Pragma: no-cache
	//
	size_t pos1 = line.find(':');
	if (pos1 == std::string::npos)
		return false;

	std::string key = line.substr(0, pos1);

	size_t pos2 = line.find_first_not_of(" \t", pos1 + 1);
	if (pos2 == std::string::npos)
		return false;

	std::string value = line.substr(pos2, std::string::npos);
	std::string lowerKey;
	lowerKey.resize(key.size());
	std::transform(key.begin(), key.end(), lowerKey.begin(), ::tolower);
	if (lowerKey == "content-length") {
		try {
			impl->bodyLength = std::stoul(value);
		} catch(...) {
			return false;
		}
	}

	impl->headers.insert(std::pair<std::string,std::string>(key, value));
	return true;
}


bool HttpRequestParser::onFirstLine(const std::string &line) {
	//             v pos1
	// RDG_OUT_DATA /remoteDesktopGateway/ HTTP/1.1
	//                                    ^ pos2
	size_t pos1 = line.find(' ');
	if (pos1 == std::string::npos)
		return false;

	this->method = line.substr(0, pos1);
	size_t pos2 = line.rfind(' ');
	if ((pos2 == std::string::npos) || (pos2 == pos1))
		return false;

	this->httpVersion = line.substr(pos2 + 1, std::string::npos);
	if (this->httpVersion.find("HTTP/") != 0)
		return false;
	this->uri = line.substr(pos1 + 1, pos2 - pos1 -1);
	return true;
}


bool HttpResponseParser::onFirstLine(const std::string &line) {
	//         v  pos1
	// HTTP/1.1 401 Access Denied
	//             ^ pos2
	size_t pos1 = line.find(' ');
	if (pos1 == std::string::npos)
		return false;

	size_t pos2 = line.find(' ', pos1 + 1);
	if (pos1 == std::string::npos)
		return false;

	this->responseVersion = line.substr(0, pos1);
	try {
		std::string codeStr = line.substr(pos1, pos2-pos1);
		this->responseCode = std::stoul(codeStr);
	} catch(...) {
		return false;
	}
	this->responseString = line.substr(pos2 + 1);
	return true;
}

} // namespace http
