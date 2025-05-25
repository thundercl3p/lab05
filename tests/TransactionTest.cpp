#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};

class TransactionTest : public ::testing::Test {
protected:
    MockAccount from{1, 2000};
    MockAccount to{2, 500};
    Transaction tr;
};

TEST_F(TransactionTest, SuccessfulTransferBetweenAccounts) {
    {
        InSequence seq;
        EXPECT_CALL(from, Lock());
        EXPECT_CALL(to, Lock());
        EXPECT_CALL(from, GetBalance()).WillOnce(Return(2000));
        EXPECT_CALL(from, ChangeBalance(-301));
        EXPECT_CALL(to, ChangeBalance(300));
        EXPECT_CALL(to, Unlock());
        EXPECT_CALL(from, Unlock());
    }

    EXPECT_TRUE(tr.Make(from, to, 300));
}

TEST_F(TransactionTest, SelfTransferThrowsLogicError) {
    EXPECT_THROW(tr.Make(from, from, 200), std::logic_error);
}

TEST_F(TransactionTest, NegativeAmountThrowsInvalidArgument) {
    EXPECT_THROW(tr.Make(from, to, -50), std::invalid_argument);
}

TEST_F(TransactionTest, SmallAmountThrowsLogicError) {
    EXPECT_THROW(tr.Make(from, to, 99), std::logic_error);
}

TEST_F(TransactionTest, SmallTransferWithHighFeeFails) {
    EXPECT_FALSE(tr.Make(from, to, 1));
}

TEST_F(TransactionTest, InsufficientFundsCancelsTransfer) {
    EXPECT_CALL(from, GetBalance()).WillOnce(Return(100));
    
    InSequence seq;
    EXPECT_CALL(from, Lock());
    EXPECT_CALL(to, Lock());
    EXPECT_CALL(to, Unlock());
    EXPECT_CALL(from, Unlock());

    EXPECT_FALSE(tr.Make(from, to, 150));
}

TEST_F(TransactionTest, UnlockOrderPreservedOnFailure) {
    InSequence seq;
    EXPECT_CALL(from, Lock());
    EXPECT_CALL(to, Lock());
    EXPECT_CALL(from, GetBalance()).WillOnce(Return(50));
    EXPECT_CALL(to, Unlock());
    EXPECT_CALL(from, Unlock());

    EXPECT_FALSE(tr.Make(from, to, 100));
}
