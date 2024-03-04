#ifndef __SHARED_UUID_H__
#define __SHARED_UUID_H__

#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

class NObjectUUID
{
public:
	typedef mu_uint32 CompareType;
	static constexpr mu_size length = 16;
	static constexpr mu_size compare_length = length / sizeof(CompareType);

	NObjectUUID() { mu_zeromem(value, sizeof(value)); }
	NObjectUUID(const NObjectUUID &r)
	{
		mu_memcpy(value, r.value, sizeof(value));
	}
	NObjectUUID(const std::vector<mu_uint8> &r)
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
	NObjectUUID(const mu_uint8 *data, const mu_size length)
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
	NObjectUUID(const mu_uint8 data[NObjectUUID::length])
	{
		mu_memcpy(value, data, sizeof(value));
	}
	NObjectUUID(const boost::uuids::uuid &uuid)
	{
		mu_memcpy(value, uuid.data, sizeof(value));
	}
	NObjectUUID(const mu_utf8string id)
	{
		fromString(id);
	}

	void clear()
	{
		mu_zeromem(value, sizeof(value));
	}

	operator bool() const
	{
		static mu_uint8 empty_uuid[sizeof(value)] = {};
		return mu_memcmp(value, empty_uuid, sizeof(value)) != 0;
	}

	mu_boolean operator==(const NObjectUUID &r) const
	{
		return mu_memcmp(value, r.value, sizeof(value)) == 0;
	}

	mu_boolean operator!=(const NObjectUUID &r) const
	{
		return mu_memcmp(value, r.value, sizeof(value)) != 0;
	}

	mu_utf8string toString() const
	{
		mu_utf8string hex;
		mu_uint32 index = 0;
		for (; index < 4; ++index)
		{
			hex += fmt::format("{:02x}", value[index]);
		}
		hex += "-";
		for (; index < 6; ++index)
		{
			hex += fmt::format("{:02x}", value[index]);
		}
		hex += "-";
		for (; index < 8; ++index)
		{
			hex += fmt::format("{:02x}", value[index]);
		}
		hex += "-";
		for (; index < 10; ++index)
		{
			hex += fmt::format("{:02x}", value[index]);
		}
		hex += "-";
		for (; index < 16; ++index)
		{
			hex += fmt::format("{:02x}", value[index]);
		}
		return hex;
	}

	void fromString(mu_utf8string id)
	{
		id.erase(std::remove(id.begin(), id.end(), '-'), id.end());
		if (id.size() != length * 2) return;
		hex2bin(id, value, length);
	}

	std::vector<mu_uint8> toByteSeq() const
	{
		return std::vector<mu_uint8>(value, value + sizeof(value));
	}

public:
	mu_uint8 value[length];
};

typedef NObjectUUID mu_uuid;

NEXTMU_INLINE mu_uuid GenerateUUID()
{
	static thread_local boost::uuids::random_generator generator;
	return mu_uuid(generator());
}

#endif