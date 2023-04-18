//
// Tiny parallel 'for' loop thread pool implementation.
// Based on 'BS_thread_pool_light.hpp' library by Barak Shoshany - https://github.com/bshoshany/thread-pool/blob/master/BS_thread_pool_light.hpp
//

#pragma once

#include <cassert>
#include <memory>
#include <queue>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace san {

class parallel_for {
	int									m_num_threads;
	std::unique_ptr <std::thread[]>		m_threads;

	std::atomic <bool>					m_running		= false;
	std::atomic <bool>					m_waiting		= false;

	std::queue <std::function<void()>>	m_tasks;
	std::atomic <size_t>				m_tasks_total	= 0;

	mutable std::mutex					m_tasks_mutex;
	std::condition_variable				m_task_available_cv;
	std::condition_variable				m_task_done_cv;

	parallel_for( const parallel_for & ) = delete;
	parallel_for & operator = ( const parallel_for & ) = delete;

	void worker() {
		while ( m_running ) {

			std::unique_lock <std::mutex> tasks_lock( m_tasks_mutex );
			m_task_available_cv.wait( tasks_lock, [this]{ return !m_tasks.empty() || !m_running; } );

			if ( m_running ) {
				std::function <void()> task = std::move( m_tasks.front() );

				m_tasks.pop();

				tasks_lock.unlock();
				task();
				tasks_lock.lock();

				--m_tasks_total;
				if ( m_waiting ) {
					m_task_done_cv.notify_one();
				}
			}
		}
	}

public:
	parallel_for( uint32_t n_threads = 0 ) : m_num_threads( n_threads ) {
		if ( !m_num_threads ) m_num_threads = std::thread::hardware_concurrency();
		assert( m_num_threads > 0 );

#ifndef NDEBUG
		std::printf( "%s: %d threads created.\n", __FUNCTION__, m_num_threads );
#endif

		m_threads = std::make_unique<std::thread[]>( m_num_threads );
		m_running = true;
		for ( int i = 0; i < m_num_threads; i++ ) {
			m_threads[i] = std::thread( &parallel_for::worker, this );
		}
	}

	virtual ~parallel_for() {
		wait();
		m_running = false;
		m_task_available_cv.notify_all();
		for ( int i = 0; i < m_num_threads; ++i ) {
			m_threads[i].join();
		}
	}

	int num_threads() const { return m_num_threads; }

	void wait() {
		m_waiting = true;
		std::unique_lock <std::mutex> tasks_lock( m_tasks_mutex );
		m_task_done_cv.wait( tasks_lock, [this]{ return !m_tasks_total; } );
		m_waiting = false;
	}

	template <typename F>
	void run( int beg, int end, F && f, int override_num_threads = 0 ) {
		if ( beg >= end ) return;

		int n_threads = override_num_threads > 0 ? override_num_threads : m_num_threads;

		int total_size	= end - beg;
		int block_size	= total_size / n_threads;
		int rem			= total_size % n_threads;

		while ( n_threads-- > 0 ) {

			int curr_size = rem > 0 ? block_size + 1 : block_size;
			if ( !curr_size ) {
				assert( n_threads == 0 );
#ifndef NDEBUG
				fprintf( stderr, "%s: zero size block (n_threads = %d). Thread count > loop size?\n", __PRETTY_FUNCTION__, n_threads );
#endif
				break;
			}

			{ // Push task...
				const std::scoped_lock tasks_lock( m_tasks_mutex );
				m_tasks.push( std::bind( std::forward<F>( f ), beg, beg + curr_size ) );
			}

			beg += curr_size;
			rem--;

			++m_tasks_total;
			m_task_available_cv.notify_one();
		}
	}

	template <typename F>
	void run_and_wait( int beg, int end, F && f, int override_num_threads = 0 ) {
		run( beg, end, std::forward<F>( f ), override_num_threads );
		wait();
	}
}; // class parallel_for

} // namespace san
