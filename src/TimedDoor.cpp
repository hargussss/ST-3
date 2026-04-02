// Copyright 2021 GHA Test Team
#include "TimedDoor.h"

#include <chrono>
#include <stdexcept>
#include <thread>

namespace {

class DoorStateClient : public TimerClient {
 public:
	explicit DoorStateClient(TimedDoor& doorRef) : door(doorRef) {}

	void Timeout() override {
		door.throwState();
	}

 private:
	TimedDoor& door;
};

}  // namespace

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& timedDoor) : door(timedDoor) {}

void DoorTimerAdapter::Timeout() {
	Timer timer;
	DoorStateClient client(door);
	timer.tregister(door.getTimeOut(), &client);
}

TimedDoor::TimedDoor(int timeout)
		: adapter(new DoorTimerAdapter(*this)), iTimeout(timeout), isOpened(false) {}

bool TimedDoor::isDoorOpened() {
	return isOpened;
}

void TimedDoor::unlock() {
	isOpened = true;
	adapter->Timeout();
}

void TimedDoor::lock() {
	isOpened = false;
}

int TimedDoor::getTimeOut() const {
	return iTimeout;
}

void TimedDoor::throwState() {
	if (isOpened) {
		throw std::runtime_error("Door timeout expired while opened");
	}
}

void Timer::sleep(int timeoutSeconds) {
	if (timeoutSeconds <= 0) {
		return;
	}

	std::this_thread::sleep_for(std::chrono::seconds(timeoutSeconds));
}

void Timer::tregister(int timeoutSeconds, TimerClient* timerClient) {
	client = timerClient;
	sleep(timeoutSeconds);
	if (client != nullptr) {
		client->Timeout();
	}
}
