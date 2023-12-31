#ifndef __MU_RESIZABLEQUEUE_H__
#define __MU_RESIZABLEQUEUE_H__

#pragma once

template<typename Type, const mu_uint32 Incr = 1024>
class NResizableQueue
{
public:
	NResizableQueue() : Count(1), Group(0), Index(0)
	{
		Buffers.reserve(50);
		Buffers.push_back(reinterpret_cast<Type *>(mu_malloc(sizeof(Type) * Incr)));
	}

	~NResizableQueue()
	{
		for (auto buffer : Buffers)
		{
			mu_free(buffer);
		}
		Buffers.clear();
	}

	void Reset()
	{
		Group = 0;
		Index = 0;
	}

	Type *Allocate()
	{
		std::lock_guard lock(Mutex);
		if (Index >= Incr)
		{
			auto group = Group + 1;
			if (group >= Count)
			{
				auto buffer = reinterpret_cast<Type *>(mu_malloc(sizeof(Type) * Incr));
				if (buffer == nullptr) return nullptr;
				++Count;
				Buffers.push_back(buffer);
			}

			++Group;
			Index = 0;
		}

		return &Buffers[Group][Index++];
	}

private:
	std::mutex Mutex;
	mu_uint32 Count;
	mu_uint32 Group;
	mu_uint32 Index;
	std::vector<Type *> Buffers;
};

#endif