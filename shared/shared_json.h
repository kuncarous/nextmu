#ifndef __SHARED_JSON_H__
#define __SHARED_JSON_H__

#pragma once

#include <nlohmann/json.hpp>

NEXTMU_INLINE mu_utf8string JsonStripComments(const mu_char* inputBuffer, const mu_uint32 bufferLength)
{
	enum
	{
		COMMENT_TYPE_NONE,
		COMMENT_TYPE_SINGLE,
		COMMENT_TYPE_MULTI,
	};
	mu_utf8string buffer(inputBuffer, bufferLength);
	mu_utf8string ret;
	ret.reserve(bufferLength);

	mu_char currentChar, nextChar;
	mu_boolean insideString = false;
	mu_int32 commentType = COMMENT_TYPE_NONE;

	mu_size offset = 0;
	for (mu_size i = 0; i < bufferLength; i++)
	{
		currentChar = buffer[i];

		if (i < bufferLength - 1)
		{
			nextChar = buffer[i + 1];
		}
		else
		{
			nextChar = '\0';
		}

		// If we're not in a comment, check for a quote.
		if (commentType == COMMENT_TYPE_NONE && currentChar == '"')
		{
			mu_boolean escaped = false;

			// If the previous character was a single slash, and the one before
			// that was not (i.e. the previous character is escaping this quote
			// and is not itself escaped), then the quote is escaped.
			if (i >= 2 && buffer[i - 1] == '\\' && buffer[i - 2] != '\\')
			{
				escaped = true;
			}

			if (!escaped)
			{
				insideString = !insideString;
			}
		}

		if (insideString)
		{
			continue;
		}

		if (commentType == COMMENT_TYPE_NONE && currentChar == '/' && nextChar == '/')
		{
			ret.append(buffer, offset, i - offset);
			offset = i;
			commentType = COMMENT_TYPE_SINGLE;

			// Skip second '/'
			i++;
		}
		else if (commentType == COMMENT_TYPE_SINGLE && currentChar == '\r' && nextChar == '\n')
		{
			// Skip '\r'
			i++;

			commentType = COMMENT_TYPE_NONE;
			offset = i;

			continue;
		}
		else if (commentType == COMMENT_TYPE_SINGLE && currentChar == '\n')
		{
			commentType = COMMENT_TYPE_NONE;
			offset = i;
		}
		else if (commentType == COMMENT_TYPE_NONE && currentChar == '/' && nextChar == '*')
		{
			ret.append(buffer, offset, i - offset);
			offset = i;
			commentType = COMMENT_TYPE_MULTI;

			// Skip the '*'
			i++;
			continue;
		}
		else if (commentType == COMMENT_TYPE_MULTI && currentChar == '*' && nextChar == '/')
		{
			// Skip '*'
			i++;

			commentType = COMMENT_TYPE_NONE;
			offset = i + 1;
			continue;
		}
	}

	ret.append(buffer, offset, bufferLength - offset);
	return ret;
}

NEXTMU_INLINE mu_utf8string JsonStripComments(const mu_utf8string& str)
{
	enum
	{
		COMMENT_TYPE_NONE,
		COMMENT_TYPE_SINGLE,
		COMMENT_TYPE_MULTI,
	};
	mu_utf8string ret;
	ret.reserve(str.length());

	mu_char currentChar, nextChar;
	mu_boolean insideString = false;
	mu_int32 commentType = COMMENT_TYPE_NONE;

	mu_size offset = 0;
	for (mu_size i = 0; i < str.length(); i++)
	{
		currentChar = str[i];

		if (i < str.length() - 1)
		{
			nextChar = str[i + 1];
		}
		else
		{
			nextChar = '\0';
		}

		// If we're not in a comment, check for a quote.
		if (commentType == COMMENT_TYPE_NONE && currentChar == '"')
		{
			mu_boolean escaped = false;

			// If the previous character was a single slash, and the one before
			// that was not (i.e. the previous character is escaping this quote
			// and is not itself escaped), then the quote is escaped.
			if (i >= 2 && str[i - 1] == '\\' && str[i - 2] != '\\')
			{
				escaped = true;
			}

			if (!escaped)
			{
				insideString = !insideString;
			}
		}

		if (insideString)
		{
			continue;
		}

		if (commentType == COMMENT_TYPE_NONE && currentChar == '/' && nextChar == '/')
		{
			ret.append(str, offset, i - offset);
			offset = i;
			commentType = COMMENT_TYPE_SINGLE;

			// Skip second '/'
			i++;
		}
		else if (commentType == COMMENT_TYPE_SINGLE && currentChar == '\r' && nextChar == '\n')
		{
			// Skip '\r'
			i++;

			commentType = COMMENT_TYPE_NONE;
			offset = i;

			continue;
		}
		else if (commentType == COMMENT_TYPE_SINGLE && currentChar == '\n')
		{
			commentType = COMMENT_TYPE_NONE;
			offset = i;
		}
		else if (commentType == COMMENT_TYPE_NONE && currentChar == '/' && nextChar == '*')
		{
			ret.append(str, offset, i - offset);
			offset = i;
			commentType = COMMENT_TYPE_MULTI;

			// Skip the '*'
			i++;
			continue;
		}
		else if (commentType == COMMENT_TYPE_MULTI && currentChar == '*' && nextChar == '/')
		{
			// Skip '*'
			i++;

			commentType = COMMENT_TYPE_NONE;
			offset = i + 1;
			continue;
		}
	}

	ret.append(str, offset, str.length() - offset);
	ret.shrink_to_fit();
	return ret;
}

#endif