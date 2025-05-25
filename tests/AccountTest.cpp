#include <gtest/gtest.h>
#include "Account.h"

TEST(AccountTest, GetBalanceReturnsInitialValue) 
{
    Account acc(1, 1000);
    ASSERT_EQ(acc.GetBalance(), 1000);
}

TEST(AccountTest, ChangeBalanceRequiresLock) 
{
    Account acc(1, 1000);
    ASSERT_THROW(acc.ChangeBalance(100), std::runtime_error);
    acc.Lock();
    ASSERT_NO_THROW(acc.ChangeBalance(100));
    ASSERT_EQ(acc.GetBalance(), 1100);
}

TEST(AccountTest, LockThrowsWhenAlreadyLocked) 
{
    Account acc(1, 100);
    acc.Lock();
    ASSERT_THROW(acc.Lock(), std::runtime_error);
}

TEST(AccountTest, UnlockDoesNotThrowWhenUnlocked) 
{
    Account acc(1, 100);
    ASSERT_NO_THROW(acc.Unlock());
}

TEST(AccountTest, ChangeBalanceWithoutLockThrows) 
{
    Account acc(1, 100);
    ASSERT_THROW(acc.ChangeBalance(50), std::runtime_error);
}
