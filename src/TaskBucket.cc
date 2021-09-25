/*
 * Created by Costa Bushnaq
 *
 * 25-09-2021 @ 20:43:03
*/

#include "TaskBucket.hpp"

#include <iostream>

using namespace ykz;

TaskBucket::~TaskBucket()
{
	if (!m_is_stopped)
		stop();
}

TaskBucket::TaskBucket(std::size_t nb_threads)
{
	m_threads.reserve(nb_threads);

	for (std::size_t i = 0; i < nb_threads; ++i) {
		m_threads.push_back(
			std::thread(&TaskBucket::thread_routine, this)
		);
	}
}

void TaskBucket::thread_routine()
{
	for (; !m_is_stopped;) {
		// we could get locked here if cv is notified at that moment...
		std::unique_lock<std::mutex> lck(m_mut);
		m_cv.wait(lck, [this]{
			return has_task_nolock() || m_is_stopped;
		});

		if (m_is_stopped)
			return;

		auto task = get_task();
		task();

		lck.unlock();
	}
}

bool TaskBucket::has_task_nolock() const
{
	return m_bucket.size() > 0;
}

TaskBucket::Task TaskBucket::get_task()
{
	auto task = std::move(m_bucket.front());

	m_bucket.pop();

	return task;
}

void TaskBucket::stop()
{
	m_is_stopped = true;
	m_cv.notify_all();

	for (std::size_t i = 0; i < m_threads.size(); ++i) {
		m_threads[i].join();
	}
}
