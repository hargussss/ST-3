// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

#include "TimedDoor.h"

using ::testing::Exactly;

namespace {

class MockTimerClient : public TimerClient {
 public:
	MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
	MOCK_METHOD(void, lock, (), (override));
	MOCK_METHOD(void, unlock, (), (override));
	MOCK_METHOD(bool, isDoorOpened, (), (override));
};

void OpenAndCloseDoor(Door* door) {
	door->unlock();
	door->lock();
}

class TimedDoorTest : public ::testing::Test {
 protected:
	TimedDoor* door {};
	DoorTimerAdapter* adapter {};

	void SetUp() override {
		door = new TimedDoor(0);
		adapter = new DoorTimerAdapter(*door);
	}

	void TearDown() override {
		delete adapter;
		delete door;
	}
};

TEST_F(TimedDoorTest, NewDoorIsClosed) {
	EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorIsOpenedAfterUnlockWhenTimeoutIsNegative) {
	TimedDoor noWaitDoor(-1);
	EXPECT_THROW(noWaitDoor.unlock(), std::runtime_error);
}

TEST_F(TimedDoorTest, DoorIsClosedAfterLock) {
	door->lock();
	EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, TimeoutValueIsStored) {
	TimedDoor customTimeoutDoor(7);
	EXPECT_EQ(customTimeoutDoor.getTimeOut(), 7);
}

TEST_F(TimedDoorTest, ThrowStateDoesNotThrowForClosedDoor) {
	door->lock();
	EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, ThrowStateThrowsForOpenedDoor) {
	TimedDoor openedDoor(-1);
	try {
		openedDoor.unlock();
	} catch (const std::runtime_error&) {
	}
	EXPECT_THROW(openedDoor.throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutThrowsWhenDoorIsOpened) {
	TimedDoor openedDoor(-1);
	DoorTimerAdapter openedAdapter(openedDoor);

	try {
		openedDoor.unlock();
	} catch (const std::runtime_error&) {
	}
	EXPECT_THROW(openedAdapter.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutDoesNotThrowWhenDoorIsClosed) {
	door->lock();
	EXPECT_NO_THROW(adapter->Timeout());
}

TEST(TimerTest, TimerCallsTimeoutExactlyOnce) {
	Timer timer;
	MockTimerClient client;

	EXPECT_CALL(client, Timeout()).Times(Exactly(1));
	EXPECT_NO_THROW(timer.tregister(0, &client));
}

TEST(TimerTest, TimerHandlesNullClient) {
	Timer timer;
	EXPECT_NO_THROW(timer.tregister(0, nullptr));
}

TEST(MockDoorInterfaceTest, MockMethodsAreCalledInsideOtherFunction) {
	MockDoor mockDoor;
	EXPECT_CALL(mockDoor, unlock()).Times(Exactly(1));
	EXPECT_CALL(mockDoor, lock()).Times(Exactly(1));

	OpenAndCloseDoor(&mockDoor);
}

TEST(MockDoorInterfaceTest, MockCanReturnDoorState) {
	MockDoor mockDoor;
	EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(::testing::Return(true));

	EXPECT_TRUE(mockDoor.isDoorOpened());
}

TEST(TimedDoorBehaviorTest, UnlockWithZeroTimeoutThrowsBecauseDoorStaysOpened) {
	TimedDoor shortDoor(0);
	EXPECT_THROW(shortDoor.unlock(), std::runtime_error);
}

}  // namespace
