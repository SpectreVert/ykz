/*
 * Created by Costa Bushnaq
 *
 * 07-05-2021 @ 02:16:39
 *
 * see LICENSE
*/

#ifndef _CONCURRENT_HPP
#define _CONCURRENT_HPP

#include "ConcurrentQueue.hpp"

#include <future>
#include <memory>
#include <thread>

#include <iostream>

namespace ykz {

template<class TValue>
class Concurrent {
	mutable TValue m_t;
	mutable ConcurrentQueue<std::function<void()>> m_q;
	bool m_is_done{false};
	std::thread m_worker;

	template<typename TProm, typename TFunc, typename TType>
	static void set_value(std::promise<TProm>& p, TFunc& fn, TType& t)
	{
		p.set_value(fn(t));
	}
	
	template<typename TFunc, typename TType>
	static void set_value(std::promise<void>& p, TFunc& fn, TType& t)
	{
		fn(t);
		p.set_value();
	}

public:
	using Value = TValue;

	Concurrent(Value t = Value{})
		: m_t{t}
		, m_worker{[=]{
			
			// pop should block until something is available
			while (!m_is_done)
				m_q.pop()();
		
		}} { /* body */ };

	~Concurrent()
	{
		m_q.push([=]{
			m_is_done = true;
		});
		m_worker.join();
	};

	template<typename TFunc>
	auto operator()(TFunc fn) const -> std::future<decltype(fn(m_t))> {
		auto p = std::make_shared<std::promise<decltype(fn(m_t))>>();
		auto ret = p->get_future();

		m_q.push([=]{
			try         { set_value(*p, fn, m_t); }
			catch (...) { p->set_exception(std::current_exception()); };
		});

		return ret;
	};

}; // class Concurrent

} // namespace ykz

#endif /* _CONCURRENT_HPP */
