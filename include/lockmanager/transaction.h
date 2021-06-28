#pragma once

#include <mutex>
#include <set>

#include "lock.h"

/**
 * The internal representation of a transaction for the lock manager.
 * It keeps track of the lock budget, i.e. the maximum number of locks
 * the transaction is allowed to acquire, the set of acquired locks of
 * that transaction as well as its phase according to 2PL.
 */
class Transaction {
 public:
  /**
   * Assigns the transaction its lock budget when it is created.
   *
   * @param lockBudget the assigned lockBudget, as determined when registering
   *                   the transaction at the lock manager
   */
  Transaction(unsigned int lockBudget) : lockBudget_(lockBudget){};

  /**
   * According to 2PL, a transaction has two subsequent phases:
   * It starts with the growing phase, where it acquires all
   * the necessary locks at once. After the first lock is released,
   * it enters the shrinking phase. From thereon, the transaction is
   * not allowed to acquire new locks and only continues to release
   * the already existent locks.
   */
  enum Phase { kGrowing, kShrinking };

  /**
   * @returns the set of row IDs the transaction curently holds locks for
   */
  [[nodiscard]] auto getLockedRows() const -> std::set<unsigned int>;

  /**
   * @returns the phase the transaction is currently in, which is important for
   *          the lock manager to determine, if the transaction can acquire more
   *          locks (growing phase) or not (shrinking phase)
   */
  auto getPhase() -> Phase;

  /**
   * When the transaction acquires a new lock, the row ID that lock refers to is
   * added to the set of locked rows. Also decrements the lock budget by 1.
   *
   * @param rowId row ID of the newly acquired lock
   */
  void addLock(unsigned int rowId);

  /**
   * Checks if the transaction currently holds a lock on the given row ID.
   * If so, it enters the shrinking phase and removes the row ID from the set of
   * locked rows. Then it releases the lock.
   *
   * @param rowId row ID of the released lock
   * @param lock the lock to release
   */
  void releaseLock(unsigned int rowId, std::shared_ptr<Lock>& lock);

  /**
   * @returns maximum number of locks the transaction is allowed to acquire over
   *          its lifetime
   */
  [[nodiscard]] auto getLockBudget() const -> unsigned int;

  /**
   * Checks if the transaction has a lock on the specified row.
   *
   * @param rowId
   * @returns true if it has lock, else false
   */
  auto hasLock(unsigned int rowId) -> bool;

 private:
  std::set<unsigned int> lockedRows_;
  Phase phase_ = Phase::kGrowing;
  unsigned int lockBudget_;
  std::mutex mut_;
};