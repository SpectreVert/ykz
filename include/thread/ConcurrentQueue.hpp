/*
 * Created by Costa Bushnaq
 *
 * 07-05-2021 @ 02:38:38
 *
 * see LICENSE
*/

#ifndef _CONCURRENTQUEUE_HPP
#define _CONCURRENTQUEUE_HPP

#include <condition_variable>
#include <functional>
#include <mutex>
#include <deque>

#include <iostream>

namespace ykz {

//! locking concurrent queue. Someday I might
//! update this to a lock-free queue.
//!
template<class TValue>
class ConcurrentQueue {
private:
	std::condition_variable m_cv;
	std::deque<TValue> m_q;
	mutable std::mutex m_q_mut;

	ConcurrentQueue(ConcurrentQueue const&) = delete;
	void operator=(ConcurrentQueue const&) = delete;

public:
	using Value = TValue;

	virtual ~ConcurrentQueue() = default;
	ConcurrentQueue() = default;

	bool can_pop() const
	{
		std::unique_lock<std::mutex> lck(m_q_mut);
		return !m_q.empty();
	};

	Value pop()
	{
		std::unique_lock<std::mutex> lck(m_q_mut);
		if (m_q.empty())
			m_cv.wait(lck, [&]{ return !m_q.empty(); });
		auto v = m_q.front();
		m_q.pop_front();

		return std::move(v);
	};

	void push(Value&& v)
	{
		std::unique_lock<std::mutex> lck(m_q_mut);
		m_q.push_back(std::move(v));
		m_cv.notify_one();
	};


}; // class ConcurrentQueue

} // namespace ykz

#endif /* _CONCURRENTQUEUE_HPP */
