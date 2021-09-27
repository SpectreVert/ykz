/*
 * Created by Costa Bushnaq
 *
 * 25-09-2021 @ 17:58:12
 *
 * see LICENSE
*/

#ifndef _TASKBUCKET_HPP
#define _TASKBUCKET_HPP

#include <functional>
#include <future> /* includes thread, condition_variable and mutex */
#include <memory>
#include <mutex>
#include <queue>

#include "Message.hpp"

namespace ykz {

class TaskBucket {
	using Task = std::function<void()>;

	std::queue<Task> m_bucket;
	std::vector<std::thread> m_threads;
	std::condition_variable m_cv;
	std::mutex mutable m_mut;

	std::atomic<bool> m_is_stopped{false};

	void thread_routine();

	// need to acquire lock before going in there
	bool has_task_nolock() const;
	Task get_task();

public:
	virtual ~TaskBucket();
	TaskBucket(std::size_t nb_threads);

	void stop();

	template<typename T>
	std::future<T> schedule_task(std::function<T()> task)
	{
		auto promise = std::make_shared<std::promise<T>>();
		auto future  = promise->get_future();
		auto closure = [npromise=std::move(promise), ntask=std::move(task)]() {
				npromise->set_value(ntask());
				// maybe add `notify_server` or something
		};

		{
			std::lock_guard<std::mutex> lck(m_mut);
			m_bucket.push(std::move(closure));
		}
		m_cv.notify_one();

		return future;
	}

}; // TaskBucket

} // namespace ykz

#endif /* _TASKBUCKET_HPP */
