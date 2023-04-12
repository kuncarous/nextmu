#ifndef __MU_THREADSMANAGER_H__
#define __MU_THREADSMANAGER_H__

#pragma once

#include <t_threading_helper.h>

class NThreadExecutorBase;

namespace MUThreadsManager
{
	typedef std::function<void(const mu_uint32 threadIndex)> RunFunction;

	const mu_boolean Initialize();
	void Destroy();

	const mu_uint32 GetThreadsCount();
	void Run(std::unique_ptr<NThreadExecutorBase> executor);
	void Worker(const mu_uint32 index);
}

class NThreadExecutorBase
{
private:
	friend void MUThreadsManager::Run(std::unique_ptr<NThreadExecutorBase> executor);
	friend void MUThreadsManager::Worker(const mu_uint32 index);
	virtual void Prepare(const mu_uint32 count) {}
	virtual void Execute(const mu_uint32 index, const mu_uint32 count) = 0;
};

class NThreadExecutor : public NThreadExecutorBase
{
public:
	typedef std::function<void(const mu_uint32)> Func;
	NThreadExecutor(Func func) : _Func(std::move(func)) {}

private:
	void Execute(const mu_uint32 index, const mu_uint32 count) override
	{
		_Func(index);
	}

private:
	Func _Func;
};

template<class Iter, class Func>
class NThreadExecutorIterator : public NThreadExecutorBase
{
	class NThreadRange
	{
	public:
		Iter begin;
		Iter end;
	};

public:
	NThreadExecutorIterator(Iter first, Iter last, Func func) : _First(first), _Last(last), _Func(std::move(func)) {}

private:
	virtual void Prepare(const mu_uint32 count) override
	{
		_Ranges.resize(count);
		const mu_uint32 elementsCount = static_cast<mu_uint32>(std::distance(_First, _Last));
		auto iter = _First, last = _Last;
		for (mu_uint32 n = 0; n < count; ++n)
		{
			mu_uint32 start, end;
			TThreading::GetIndexTasking(n, elementsCount, start, end, count);

			auto &range = _Ranges[n];
			range.begin = iter;
			for (; start < end; ++start, ++iter) {}
			range.end = iter;
		}
	}

	void Execute(const mu_uint32 index, const mu_uint32 count) override
	{
		auto &range = _Ranges[index];
		for (; range.begin != range.end; ++range.begin)
		{
			_Func(*range.begin);
		}
	}

private:
	std::vector<NThreadRange> _Ranges;
	Iter _First, _Last;
	Func _Func;
};

template<class Iter, class Func>
class NThreadExecutorRangeIterator : public NThreadExecutorBase
{
	class NThreadRange
	{
	public:
		Iter begin;
		Iter end;
	};

public:
	NThreadExecutorRangeIterator(Iter first, Iter last, Func func) : _First(first), _Last(last), _Func(std::move(func)) {}

private:
	virtual void Prepare(const mu_uint32 count) override
	{
		_Ranges.resize(count);
		const mu_uint32 elementsCount = static_cast<mu_uint32>(std::distance(_First, _Last));
		auto iter = _First, last = _Last;
		for (mu_uint32 n = 0; n < count; ++n)
		{
			mu_uint32 start, end;
			TThreading::GetIndexTasking(n, elementsCount, start, end, count);

			auto &range = _Ranges[n];
			range.begin = iter;
			for (; start < end; ++start, ++iter) {}
			range.end = iter;
		}
	}

	void Execute(const mu_uint32 index, const mu_uint32 count) override
	{
		auto &range = _Ranges[index];
		_Func(range.begin, range.end);
	}

private:
	std::vector<NThreadRange> _Ranges;
	Iter _First, _Last;
	Func _Func;
};

#endif