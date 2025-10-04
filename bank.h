#ifndef BANK_H
#define BANK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LENGTH 100
#define MAX_TRANSACTIONS 20
#define DATA_DIR "/Library/WebServer/Documents/bank/data"
#define DATA_FILE DATA_DIR "/bank_data.dat"
#define DATA_MAGIC "BKSY1"

// Account structure for linked list
struct Account {
    long int account_number;
    char customer_name[MAX_NAME_LENGTH];
    double balance;
    struct Account* next;
};

// Transaction structure for stack
struct Transaction {
    long int account_number;
    char type[20];  // "Deposit" or "Withdrawal"
    double amount;
    char timestamp[30];
};

// Stack structure for transactions
struct TransactionStack {
    struct Transaction* transactions;
    int top;
    int capacity;
};

// Account management functions
struct Account* create_account(const char* name, double initial_balance);
struct Account* find_account(long int account_number);
int update_balance(long int account_number, double amount);
void save_data_to_disk();
void load_data_from_disk();
void free_accounts();

// Transaction stack functions
void init_transaction_stack();
void push_transaction(long int account_number, const char* type, double amount);
void free_transaction_stack();

// CGI handler functions
void handle_cgi_request();
void send_json_response(const char* json);
void send_error(const char* message);

// Global variables
extern struct Account* accounts_head;
extern struct TransactionStack transaction_stack;
extern long int next_account_number;

#endif // BANK_H
