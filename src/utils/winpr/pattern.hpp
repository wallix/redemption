/**
 * WinPR: Windows Portable Runtime
 * File Functions
 *
 * Copyright 2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <cstdio>
#include <cstring>

enum {
    WILDCARD_STAR     = 0x00000001,
    WILDCARD_QM       = 0x00000002,
    WILDCARD_DOS      = 0x00000100,
    WILDCARD_DOS_STAR = 0x00000110,
    WILDCARD_DOS_QM   = 0x00000120,
    WILDCARD_DOS_DOT  = 0x00000140
};

/**
 * File System Behavior in the Microsoft Windows Environment:
 * http://download.microsoft.com/download/4/3/8/43889780-8d45-4b2e-9d3a-c696a890309f/File%20System%20Behavior%20Overview.pdf
 */

static const char * FilePatternFindNextWildcardA(const char * lpPattern, unsigned int* pFlags)
{
    *pFlags = 0;
    const char * lpWildcard = strpbrk(lpPattern, "*?~");

    if (lpWildcard)
    {
        if (lpWildcard[0] == '*')
        {
            *pFlags = WILDCARD_STAR;
            return lpWildcard;
        }
        if (lpWildcard[0] == '?')
        {
            *pFlags = WILDCARD_QM;
            return lpWildcard;
        }
        if (lpWildcard[0] == '~')
        {
            if (lpWildcard[1] == '*')
            {
                *pFlags = WILDCARD_DOS_STAR;
                return lpWildcard;
            }
            if (lpWildcard[1] == '?')
            {
                *pFlags = WILDCARD_DOS_QM;
                return lpWildcard;
            }
            if (lpWildcard[1] == '.')
            {
                *pFlags = WILDCARD_DOS_DOT;
                return lpWildcard;
            }
        }
    }
    return nullptr;
}

// TODO this function is incompatiable with utf8
static bool FilePatternMatchSubExpressionA(const char * lpFileName, size_t cchFileName,
        const char * lpX, size_t cchX, const char * lpY, size_t cchY, const char * lpWildcard, const char ** ppMatchEnd)
{
    // TODO "compiler warn that parameters passed as ppMatchEnd mays not be initialized";
    const char * lpMatch;

    if (*lpWildcard == '*')
    {
        /*
         *                            S
         *                         <-----<
         *                      X  |     |  e       Y
         * X * Y ==        (0)----->-(1)->-----(2)-----(3)
         */

        /*
         * State 0: match 'X'
         */

        if (strncasecmp(lpFileName, lpX, cchX) != 0)
        {
            return false;
        }

        /*
         * State 1: match 'S' or 'e'
         *
         * We use 'e' to transition to state 2
         */

        /**
         * State 2: match Y
         */

        if (cchY != 0)
        {
            /* TODO: case insensitive character search */
            lpMatch = strchr(&lpFileName[cchX], *lpY);

            if (!lpMatch)
            {
                return false;
            }

            if (strncasecmp(lpMatch, lpY, cchY) != 0)
            {
                return false;
            }
        }
        else
        {
            lpMatch = &lpFileName[cchFileName];
        }

        /**
         * State 3: final state
         */

        *ppMatchEnd = &lpMatch[cchY];

        return true;
    }
    if (*lpWildcard == '?')
    {
        /**
         *                     X     S     Y
         * X ? Y ==        (0)---(1)---(2)---(3)
         */

        /*
         * State 0: match 'X'
         */

        if (cchFileName < cchX)
        {
            return false;
        }

        if (strncasecmp(lpFileName, lpX, cchX) != 0)
        {
            return false;
        }

        /*
         * State 1: match 'S'
         */

        /**
         * State 2: match Y
         */

        if (cchY != 0)
        {
            /* TODO: case insensitive character search */
            lpMatch = strchr(&lpFileName[cchX + 1], *lpY);

            if (!lpMatch)
            {
                return false;
            }

            if (strncasecmp(lpMatch, lpY, cchY) != 0)
            {
                return false;
            }
        }
        else
        {
            if ((cchX + 1) > cchFileName)
            {
                return false;
            }

            lpMatch = &lpFileName[cchX + 1];
        }

        /**
         * State 3: final state
         */

        *ppMatchEnd = &lpMatch[cchY];

        return true;
    }
    if (*lpWildcard == '~')
    {
        fprintf(stderr, "warning: unimplemented '~' pattern match\n");

        return true;
    }

    return false;
}

// TODO this function is incompatiable with utf8
static bool FilePatternMatchA(const char * lpFileName, const char * lpPattern)
{
    size_t cchPattern;
    size_t cchFileName;
    unsigned int dwFlags;
    unsigned int dwNextFlags;
    const char * lpWildcard;

    /**
     * Wild Card Matching
     *
     * '*'  matches 0 or more characters
     * '?'  matches exactly one character
     *
     * '~*' DOS_STAR - matches 0 or more characters until encountering and matching final '.'
     *
     * '~?' DOS_QM - matches any single character, or upon encountering a period or end of name
     *               string, advances the expresssion to the end of the set of contiguous DOS_QMs.
     *
     * '~.' DOS_DOT - matches either a '.' or zero characters beyond name string.
     */

    if (!lpPattern)
    {
        return false;
    }

    if (!lpFileName)
    {
        return false;
    }

    cchPattern = strlen(lpPattern);
    cchFileName = strlen(lpFileName);

    /**
     * First and foremost the file system starts off name matching with the expression “*”.
     * If the expression contains a single wild card character ‘*’ all matches are satisfied
     * immediately. This is the most common wild card character used in Windows and expression
     * evaluation is optimized by looking for this character first.
     */

    if ((lpPattern[0] == '*') && (cchPattern == 1))
    {
        return true;
    }

    /**
     * Subsequently evaluation of the “*X” expression is performed. This is a case where
     * the expression starts off with a wild card character and contains some non-wild card
     * characters towards the tail end of the name. This is evaluated by making sure the
     * expression starts off with the character ‘*’ and does not contain any wildcards in
     * the latter part of the expression. The tail part of the expression beyond the first
     * character ‘*’ is matched against the file name at the end uppercasing each character
     * if necessary during the comparison.
     */

    if (lpPattern[0] == '*')
    {
        const char * const lpTail = &lpPattern[1];
        size_t const cchTail = strlen(lpTail);

        if (!FilePatternFindNextWildcardA(lpTail, &dwFlags))
        {
            /* tail contains no wildcards */

            if (cchFileName < cchTail)
            {
                return false;
            }

            return strcasecmp(&lpFileName[cchFileName - cchTail], lpTail) == 0;
        }
    }

    /*
     * The remaining expressions are evaluated in a non deterministic
     * finite order as listed below, where:
     *
     * 'S' is any single character
     * 'S-.' is any single character except the final '.'
     * 'e' is a null character transition
     * 'EOF' is the end of the name string
     *
     *                            S
     *                         <-----<
     *                      X  |     |  e       Y
     * X * Y ==        (0)----->-(1)->-----(2)-----(3)
     *
     *
     *                           S-.
     *                         <-----<
     *                      X  |     |  e       Y
     * X ~* Y ==       (0)----->-(1)->-----(2)-----(3)
     *
     *
     *                     X     S     S     Y
     * X ?? Y ==       (0)---(1)---(2)---(3)---(4)
     *
     *
     *                     X     S-.     S-.     Y
     * X ~?~? ==      (0)---(1)-----(2)-----(3)---(4)
     *                        |       |_______|
     *                        |            ^  |
     *                        |_______________|
     *                            ^EOF of .^
     *
     */

    lpWildcard = FilePatternFindNextWildcardA(lpPattern, &dwFlags);

    if (lpWildcard)
    {
        const char * lpMatchEnd = nullptr;

        size_t cchSubPattern = cchPattern;
        const char * lpSubPattern = lpPattern;
        size_t cchSubFileName = cchFileName;
        const char * lpSubFileName = lpFileName;
        size_t cchWildcard = ((dwFlags & WILDCARD_DOS) ? 2 : 1);
        const char * lpNextWildcard = FilePatternFindNextWildcardA(&lpWildcard[cchWildcard], &dwNextFlags);

        if (!lpNextWildcard)
        {
            const char * lpX = lpSubPattern;
            size_t cchX = (lpWildcard - lpSubPattern);

            const char * lpY = &lpSubPattern[cchX + cchWildcard];
            size_t cchY = (cchSubPattern - (lpY - lpSubPattern));

            bool match = FilePatternMatchSubExpressionA(lpSubFileName, cchSubFileName,
                    lpX, cchX, lpY, cchY, lpWildcard, &lpMatchEnd);

            return match;
        }

        while (lpNextWildcard)
        {
            cchSubFileName = cchFileName - (lpSubFileName - lpFileName);
            size_t cchNextWildcard = ((dwNextFlags & WILDCARD_DOS) ? 2 : 1);

            const char * lpX = lpSubPattern;
            size_t cchX = (lpWildcard - lpSubPattern);

            const char * lpY = &lpSubPattern[cchX + cchWildcard];
            size_t cchY = (lpNextWildcard - lpWildcard) - cchWildcard;

            bool match = FilePatternMatchSubExpressionA(lpSubFileName, cchSubFileName,
                    lpX, cchX, lpY, cchY, lpWildcard, &lpMatchEnd);

            if (!match)
            {
                return false;
            }

            lpSubFileName = lpMatchEnd;

            cchWildcard = cchNextWildcard;
            lpWildcard = lpNextWildcard;
            dwFlags = dwNextFlags;

            lpNextWildcard = FilePatternFindNextWildcardA(&lpWildcard[cchWildcard], &dwNextFlags);
        }

        return true;
    }

    /* no wildcard characters */
    return (strcasecmp(lpFileName, lpPattern) == 0);
}
