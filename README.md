# Домашнее задание
## Студента группы ИУ8-22 Kiselyov Artyom
1. Создайте `CMakeList.txt` для библиотеки *banking*.
```
$ cd lab05
$ cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.14)
project(lab05 LANGUAGES CXX)

add_subdirectory(banking)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
EOF
```
```
$ cd banking
$ cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.14)
project(banking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(banking STATIC
    Account.cpp
    Transaction.cpp
)
target_include_directories(banking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
EOF
```
```
$ mkdir -p tests
$ cd tests
$ cat > CMakeLists.txt << 'EOF'
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(tests
    AccountTest.cpp
    TransactionTest.cpp
)

target_link_libraries(tests PRIVATE
    banking
    GTest::gtest_main
    GTest::gmock 
)

include(GoogleTest)
gtest_discover_tests(tests)


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(banking PRIVATE --coverage)
    target_link_libraries(banking PRIVATE --coverage)
    
    # Для тестов
    target_compile_options(tests PRIVATE --coverage)
    target_link_libraries(tests PRIVATE --coverage)
endif()
EOF
```

2. Создайте модульные тесты на классы `Transaction` и `Account`.
    * Используйте mock-объекты.
    * Покрытие кода должно составлять 100%.
```
$ nano AccountTest.cpp
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
```
```
$ nano TransactionTest.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"

using namespace testing;

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int diff), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};

TEST(TransactionTest, TransferBetweenDifferentAccounts) {
    MockAccount from(1, 2000);
    MockAccount to(2, 500);
    Transaction tr;

    testing::InSequence seq;

    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);

    EXPECT_CALL(from, GetBalance()).WillOnce(Return(2000));
    EXPECT_CALL(from, ChangeBalance(-301)).Times(1); // 300 + fee=1

    EXPECT_CALL(to, ChangeBalance(300)).Times(1);

    EXPECT_CALL(to, Unlock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);

    EXPECT_CALL(from, GetBalance()).WillOnce(Return(1699));
    EXPECT_CALL(to, GetBalance()).WillOnce(Return(800));   

    ASSERT_TRUE(tr.Make(from, to, 300));
}



TEST(TransactionTest, TransferToSameAccountThrows) {
    MockAccount acc(1, 100);
    Transaction tr;
    ASSERT_THROW(tr.Make(acc, acc, 200), std::logic_error);
}

TEST(TransactionTest, NegativeSumThrows) {
    MockAccount from(1, 200);
    MockAccount to(2, 100);
    Transaction tr;
    ASSERT_THROW(tr.Make(from, to, -50), std::invalid_argument);
}

TEST(TransactionTest, SmallSumThrows) {
    MockAccount from(1, 200);
    MockAccount to(2, 100);
    Transaction tr;
    ASSERT_THROW(tr.Make(from, to, 99), std::logic_error);
}

TEST(TransactionTest, FeeExceedsHalfSumReturnsFalse) {
    MockAccount from(1, 300);
    MockAccount to(2, 0);
    Transaction tr;

    EXPECT_CALL(from, Lock()).Times(0);
    EXPECT_CALL(to, Lock()).Times(0);
    ASSERT_FALSE(tr.Make(from, to, 1)); 
}

TEST(TransactionTest, DebitFailsDueToInsufficientBalance) {
    MockAccount from(1, 100);
    MockAccount to(2, 0);
    Transaction tr;

   
    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);

    EXPECT_CALL(from, GetBalance()).WillOnce(Return(100));
    ASSERT_FALSE(tr.Make(from, to, 150)); 
}

TEST(TransactionTest, UnlockOrderIsCorrectOnFailure) {
    MockAccount from(1, 50);
    MockAccount to(2, 0);
    Transaction tr;

    testing::InSequence seq;
    EXPECT_CALL(from, Lock());
    EXPECT_CALL(to, Lock());
    EXPECT_CALL(from, GetBalance()).WillOnce(Return(50));
    EXPECT_CALL(to, Unlock());
    EXPECT_CALL(from, Unlock());

    ASSERT_FALSE(tr.Make(from, to, 100));
}
```
```
$ cd ~/lab05
$ mkdir -p .github/workflows
$ touch .github/workflows/ci.yml
$ nano .github/workflows/ci.yml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake g++ git lcov
        
    - name: Build
      run: |
        mkdir build && cd build
        cmake .. -DBUILD_TESTS=ON
        cmake --build .
        
    - name: Run tests
      run: |
        cd build
        ./tests/tests
        
    - name: Generate coverage
      run: |
        cd build
        lcov --ignore-errors mismatch --capture --directory . --output-file coverage.info
        lcov --remove coverage.info \
             '*/tests/*' \
             '/usr/*' \
             '*/include/*' \
             --output-file coverage.info
        lcov --list coverage.info
        
    - name: Upload to Coveralls
      uses: coverallsapp/github-action@v2
      with:
        github-token: ${{ secrets.GITHUB_TOKEN }}
        path-to-lcov: build/coverage.info
```

3. Настройте сборочную процедуру на **Action**.  
[Настроили](https://github.com/thundercl3p/lab05/tree/main)

4. Настройте [Coveralls.io](https://coveralls.io/).  
  
    [![Coverage Status](https://coveralls.io/repos/github/thundercl3p/lab05/badge.svg?branch=main)](https://coveralls.io/github/thundercl3p/lab05?branch=main)
