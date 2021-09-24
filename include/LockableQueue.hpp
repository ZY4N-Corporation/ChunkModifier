#pragma once

#include <queue>
#include <mutex>

template <typename T>
class LockableQueue : public std::queue<T> {
private:
	std::mutex mtx;

public:
	void push(T&& value) {
		mtx.lock();
		std::queue<T>::push(std::move(value));
		mtx.unlock();
	}

	void push(const T& value) {
		mtx.lock();
		std::queue<T>::push(value);
		mtx.unlock();
	}

	T* pop() {
		T* ret = nullptr;
		mtx.lock();
		if (!std::queue<T>::empty()) {
			ret = new T(std::queue<T>::front());
			std::queue<T>::pop();
		}
		mtx.unlock();
		return ret;
	}
};
