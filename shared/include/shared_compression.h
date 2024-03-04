#ifndef __SHARED_COMPRESSION_H__
#define __SHARED_COMPRESSION_H__

#pragma once

#include <zlib.h>

NEXTMU_INLINE const mu_boolean DecompressData(
	const mu_uint8 *input, mu_uint8 *output,
	const mu_size inputSize, const mu_size outputSize
)
{
	z_stream strm;

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;

	mu_int32 ret = inflateInit(&strm);
	if (ret != Z_OK)
	{
		return false;
	}

	strm.avail_in = static_cast<uInt>(inputSize);
	strm.next_in = const_cast<mu_uint8*>(input);
	strm.avail_out = static_cast<uInt>(outputSize);
	strm.next_out = output;

	ret = inflate(&strm, Z_FINISH);
	mu_assert(ret == Z_STREAM_END);

	inflateEnd(&strm);

	if (ret != Z_STREAM_END)
	{
		return false;
	}

	return true;
}

#endif