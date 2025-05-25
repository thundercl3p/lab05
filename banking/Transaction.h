#pragma once

class Account;

class Transaction {
 public:
  Transaction();
  ~Transaction();

  bool Make(Account& from, Account& to, int sum);
  int fee() const { return fee_; }
  void set_fee(int fee) { fee_ = fee; }

 private:
  void Credit(Account& account, int sum);  
    bool Debit(Account& account, int sum);   
    void SaveToDataBase(Account& from, Account& to, int sum);
  int fee_;
};
