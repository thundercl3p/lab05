#include <gtest/gtest.h>
#include "Account.h"

namespace {

class AccountTest : public ::testing::Test {
protected:
    Account acc{1, 1000};
};

TEST_F(AccountTest, InitialBalanceIsCorrect) {
    EXPECT_EQ(acc.GetBalance(), 1000);
}

TEST_F(AccountTest, ModifyBalanceRequiresLock) {
    EXPECT_THROW(acc.ChangeBalance(100), std::runtime_error);
    acc.Lock();
    EXPECT_NO_THROW(acc.ChangeBalance(100));
    EXPECT_EQ(acc.GetBalance(), 1100);
}

TEST_F(AccountTest, DoubleLockThrowsException) {
    acc.Lock();
    EXPECT_THROW(acc.Lock(), std::runtime_error);
}

TEST_F(AccountTest, UnlockOnUnlockedAccountIsSafe) {
    EXPECT_NO_THROW(acc.Unlock());
}

TEST_F(AccountTest, UnauthorizedBalanceChangeThrows) {
    EXPECT_THROW(acc.ChangeBalance(50), std::runtime_error);
}

}
