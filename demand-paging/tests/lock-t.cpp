#include <gtest/gtest.h>

#include <thread>

#include "lock.h"

const unsigned int kTransactionIdA = 0;
const unsigned int kTransactionIdB = 1;

// Shared access works
TEST(LockTest, sharedAccess) {
  Lock lock = Lock();
  std::thread t1{&Lock::getSharedAccess, &lock, 1};
  std::thread t2{&Lock::getSharedAccess, &lock, 2};
  std::thread t3{&Lock::getSharedAccess, &lock, 3};
  std::thread t4{&Lock::getSharedAccess, &lock, 4};

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  EXPECT_EQ(lock.getMode(), Lock::LockMode::kShared);
  EXPECT_EQ(lock.getOwners().size(), 4);
};

// Exclusive access works
TEST(LockTest, exclusiveAccess) {
  Lock lock = Lock();
  lock.getExclusiveAccess(kTransactionIdA);
  EXPECT_EQ(lock.getMode(), Lock::LockMode::kExclusive);
  auto owners = lock.getOwners();
  EXPECT_EQ(owners.count(kTransactionIdA), 1);
  EXPECT_EQ(owners.size(), 1);
}

// Cannot acquire shared access on an exclusive lock
TEST(LockTest, noSharedOnExclusive) {
  Lock lock = Lock();
  EXPECT_TRUE(lock.getExclusiveAccess(kTransactionIdA));
  EXPECT_FALSE(lock.getSharedAccess(kTransactionIdB));
};

// Cannot get exclusive access on an exclusive lock
TEST(LockTest, noExclusiveOnExclusive) {
  Lock lock = Lock();
  EXPECT_TRUE(lock.getExclusiveAccess(kTransactionIdA));
  EXPECT_FALSE(lock.getExclusiveAccess(kTransactionIdB));
};

// Cannot acquire exclusive access on a shared lock
TEST(LockTest, noExclusiveOnShared) {
  Lock lock = Lock();
  EXPECT_TRUE(lock.getSharedAccess(kTransactionIdA));
  EXPECT_FALSE(lock.getExclusiveAccess(kTransactionIdB));
};

// Upgrade
TEST(LockTest, upgrade) {
  Lock lock = Lock();
  lock.getSharedAccess(kTransactionIdA);
  lock.upgrade(kTransactionIdA);
  auto owners = lock.getOwners();
  EXPECT_EQ(lock.getMode(), Lock::LockMode::kExclusive);
  EXPECT_EQ(owners.count(kTransactionIdA), 1);
  EXPECT_EQ(owners.size(), 1);
}

TEST(LockTest, releaseUnownedLock) {
  Lock lock = Lock();
  lock.getExclusiveAccess(kTransactionIdA);
  lock.release(kTransactionIdB);
  EXPECT_EQ(lock.getMode(), Lock::LockMode::kExclusive);
  EXPECT_EQ(lock.getOwners().count(kTransactionIdA), 1);
  EXPECT_EQ(lock.getOwners().size(), 1);
}