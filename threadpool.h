#pragma once
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <atomic>
//#include <windows.h>
#include <iostream>
#include <unistd.h>
#include "http_req.h"

#define T HTTP_R*
using namespace std;

class ThreadPool {

public:
	//ThreadPool();
	ThreadPool(int mtn, int mr);
	~ThreadPool();
	void stop();
	bool addTask(HTTP_R* a);
	void run(int id);
private:

	int m_thread_num; // max thread numbers
	vector<thread> threads;
	int m_request; // max requests numbers
	deque<HTTP_R*> tasks_que; // task deque
	std::mutex mu;
	condition_variable cond;
	atomic<bool>is_stop;
};
ThreadPool::ThreadPool(int mtn, int mr) {
	m_thread_num = mtn;
	m_request = mr;
	is_stop = false;

	for (int i = 0; i < m_thread_num; i++) {
		auto tmp = thread(&ThreadPool::run, this, i);
		tmp.detach(); //全部设置成分离线程
		threads.push_back(move(tmp));//因为线程是不能拷贝的，所以只能用移动语义

	}
}
ThreadPool::~ThreadPool() {is_stop = true;};


void ThreadPool::stop() {
	is_stop = true;
	usleep(10000);
	return;
}

bool ThreadPool::addTask(HTTP_R* a) {
	unique_lock<mutex> l(mu, try_to_lock);
	if (l) {
		int size = tasks_que.size();
		if (size >= m_request) {
			l.unlock();
			return false;
		}
		//a->process();
		tasks_que.push_back(a);
		l.unlock();
		return true;
	}
	return false;
}

void ThreadPool::run(int id) {
	cout << this_thread::get_id() << "   " << id << endl;
	while (!is_stop) {

		unique_lock<mutex> l(mu, try_to_lock);
		if (l) {
			if (!tasks_que.empty()) {
					auto req = tasks_que.front();
					tasks_que.pop_front();
					l.unlock();
					cout << "this is thread: " << id << endl;

					req->do_process();
			}
			else {
				l.unlock();
			}
		}
	}
	return;
}
