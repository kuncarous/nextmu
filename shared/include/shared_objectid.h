#ifndef __SHARED_OBJECTID_H__
#define __SHARED_OBJECTID_H__

#pragma once

class NObjectID
{
public:
	static constexpr mu_size length = 12;

	NObjectID() { mu_zeromem(value, sizeof(value)); }
	NObjectID(const NObjectID &r)
	{
		mu_memcpy(value, r.value, sizeof(value));
	}
	NObjectID(const std::vector<mu_uint8> &r)
	{
		if (r.size() == sizeof(value))
		{
			mu_memcpy(value, r.data(), sizeof(value));
		}
		else
		{
			mu_zeromem(value, sizeof(value));
		}
	}
	NObjectID(const mu_uint8* data, const mu_size length)
	{
		if (length == sizeof(value))
		{
			mu_memcpy(value, data, sizeof(value));
		}
		else
		{
			mu_zeromem(value, sizeof(value));
		}
	}
	NObjectID(const mu_utf8string id)
	{
		fromString(id);
	}

	void clear()
	{
		mu_zeromem(value, sizeof(value));
	}

	operator bool() const
	{
		static mu_uint8 empty_oid[sizeof(value)] = {};
		return mu_memcmp(value, empty_oid, sizeof(value)) != 0;
	}

	mu_boolean operator==(const NObjectID &r) const
	{
		return mu_memcmp(value, r.value, sizeof(value)) == 0;
	}

	mu_boolean operator!=(const NObjectID &r) const
	{
		return mu_memcmp(value, r.value, sizeof(value)) != 0;
	}

	mu_utf8string toString() const
	{
		mu_utf8string hex;
		for (mu_uint32 index = 0; index < sizeof(value); ++index)
		{
			hex += fmt::format("{:02x}", value[index]);
		}
		return hex;
	}

	std::vector<mu_uint8> toByteSeq() const
	{
		return std::vector<mu_uint8>(value, value + sizeof(value));
	}

	void fromString(mu_utf8string id)
	{
		if (id.size() != length * 2) return;
		hex2bin(id, value, length);
	}

public:
	mu_uint8 value[length];
};

typedef NObjectID mu_oid;

#endif