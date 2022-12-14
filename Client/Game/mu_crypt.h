#ifndef __MU_CRYPT_H__
#define __MU_CRYPT_H__

#pragma once

NEXTMU_INLINE void XorDecrypt(mu_uint8 *dest, const mu_uint8 *src, mu_uint32 size)
{
	static const std::array<mu_uint8, 16> XorKey = { { 0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27, 0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2 } };
	mu_uint16 key = 0x5E;
	for (mu_uint32 index = 0; index < size; ++index, ++src, ++dest)
	{
		const mu_uint8 s = *src;
		*dest = (s ^ XorKey[index % XorKey.size()]) - static_cast<mu_uint8>(key);
		key = static_cast<mu_uint8>(static_cast<mu_uint32>(s) + 0x3D);
	}
}

NEXTMU_INLINE void BuxConvert(mu_uint8 *buffer, mu_uint32 size)
{
	static const std::array<mu_uint8, 3> BuxCode = { 0xfc,0xcf,0xab };
	for (mu_uint32 index = 0; index < size; index++) {
		buffer[index] ^= BuxCode[index % static_cast<mu_uint32>(BuxCode.size())];
	}
}

#endif