#include "bank.h"
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <json-c/json.h>

// Global variables
struct Account* accounts_head = NULL;
struct TransactionStack transaction_stack;
long int next_account_number = 1000;

// Initialize the transaction stack
void init_transaction_stack() {
    transaction_stack.capacity = MAX_TRANSACTIONS;
    transaction_stack.top = -1;
    transaction_stack.transactions = (struct Transaction*)malloc(MAX_TRANSACTIONS * sizeof(struct Transaction));
    if (!transaction_stack.transactions) {
        perror("Failed to allocate memory for transaction stack");
        exit(EXIT_FAILURE);
    }
}

// Create a new account and add to the linked list
struct Account* create_account(const char* name, double initial_balance) {
    struct Account* new_account = (struct Account*)malloc(sizeof(struct Account));
    if (!new_account) {
        perror("Failed to allocate memory for new account");
        return NULL;
    }

    // Generate a unique account number (simple implementation using timestamp)
    new_account->account_number = next_account_number++;
    
    strncpy(new_account->customer_name, name, MAX_NAME_LENGTH - 1);
    new_account->customer_name[MAX_NAME_LENGTH - 1] = '\0';
    new_account->balance = initial_balance;
    new_account->next = accounts_head;
    accounts_head = new_account;

    // Record the account creation as a transaction
    push_transaction(new_account->account_number, "Account Created", initial_balance);
    
    return new_account;
}

// Find an account by account number
struct Account* find_account(long int account_number) {
    struct Account* current = accounts_head;
    while (current != NULL) {
        if (current->account_number == account_number) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Update account balance
int update_balance(long int account_number, double amount) {
    struct Account* account = find_account(account_number);
    if (!account) {
        return 0; // Account not found
    }
    
    // Check for sufficient funds for withdrawal
    if (amount < 0 && account->balance < -amount) {
        return 0; // Insufficient funds
    }
    
    account->balance += amount;
    return 1; // Success
}

// Push a new transaction to the stack
void push_transaction(long int account_number, const char* type, double amount) {
    // If stack is full, remove the oldest transaction (bottom of stack)
    if (transaction_stack.top == transaction_stack.capacity - 1) {
        // Shift all elements down by one
        for (int i = 0; i < transaction_stack.capacity - 1; i++) {
            transaction_stack.transactions[i] = transaction_stack.transactions[i + 1];
        }
        transaction_stack.top--;
    }
    
    // Add new transaction to the top of the stack
    transaction_stack.top++;
    struct Transaction* t = &transaction_stack.transactions[transaction_stack.top];
    t->account_number = account_number;
    strncpy(t->type, type, sizeof(t->type) - 1);
    t->type[sizeof(t->type) - 1] = '\0';
    t->amount = amount;
    
    // Set current timestamp
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(t->timestamp, sizeof(t->timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
}

// Save all data to disk
void save_data_to_disk() {
    // Ensure data directory exists
    if (access(DATA_DIR, F_OK) != 0) {
        if (mkdir(DATA_DIR, 0755) != 0 && errno != EEXIST) {
            perror("Failed to create data directory");
            return;
        }
    }

    FILE* file = fopen(DATA_FILE, "wb");
    if (!file) {
        perror("Failed to open data file for writing");
        return;
    }
    
    // Write header
    fwrite(DATA_MAGIC, sizeof(char), strlen(DATA_MAGIC), file);
    fwrite(&next_account_number, sizeof(long int), 1, file);

    // Save accounts
    size_t account_count = 0;
    struct Account* current = accounts_head;
    while (current) {
        account_count++;
        current = current->next;
    }
    fwrite(&account_count, sizeof(size_t), 1, file);

    current = accounts_head;
    while (current) {
        struct {
            long int account_number;
            char customer_name[MAX_NAME_LENGTH];
            double balance;
        } record;

        record.account_number = current->account_number;
        strncpy(record.customer_name, current->customer_name, MAX_NAME_LENGTH);
        record.customer_name[MAX_NAME_LENGTH - 1] = '\0';
        record.balance = current->balance;

        fwrite(&record, sizeof(record), 1, file);
        current = current->next;
    }

    // Save transaction stack
    fwrite(&transaction_stack.top, sizeof(int), 1, file);
    if (transaction_stack.top >= 0) {
        fwrite(transaction_stack.transactions,
               sizeof(struct Transaction),
               transaction_stack.top + 1,
               file);
    }

    fclose(file);
}

// Load data from disk
void load_data_from_disk() {
    FILE* file = fopen(DATA_FILE, "rb");
    if (!file) {
        return; // No existing data file, start fresh
    }
    
    // Free existing accounts if any
    free_accounts();
    transaction_stack.top = -1;

    // Validate header
    char magic[6] = {0};
    if (fread(magic, sizeof(char), strlen(DATA_MAGIC), file) != strlen(DATA_MAGIC) ||
        strncmp(magic, DATA_MAGIC, strlen(DATA_MAGIC)) != 0) {
        fclose(file);
        return;
    }

    if (fread(&next_account_number, sizeof(long int), 1, file) != 1) {
        fclose(file);
        next_account_number = 1000;
        return;
    }

    size_t account_count = 0;
    if (fread(&account_count, sizeof(size_t), 1, file) != 1) {
        fclose(file);
        return;
    }

    if (account_count > 0) {
        struct {
            long int account_number;
            char customer_name[MAX_NAME_LENGTH];
            double balance;
        } *records = malloc(account_count * sizeof(*records));

        if (!records) {
            fclose(file);
            return;
        }

        if (fread(records, sizeof(*records), account_count, file) != account_count) {
            free(records);
            fclose(file);
            return;
        }

        for (size_t i = account_count; i > 0; --i) {
            struct Account* new_account = malloc(sizeof(struct Account));
            if (!new_account) {
                free(records);
                fclose(file);
                return;
            }

            new_account->account_number = records[i - 1].account_number;
            strncpy(new_account->customer_name, records[i - 1].customer_name, MAX_NAME_LENGTH);
            new_account->customer_name[MAX_NAME_LENGTH - 1] = '\0';
            new_account->balance = records[i - 1].balance;
            new_account->next = accounts_head;
            accounts_head = new_account;
        }

        free(records);
    }

    if (fread(&transaction_stack.top, sizeof(int), 1, file) != 1) {
        transaction_stack.top = -1;
        fclose(file);
        return;
    }

    if (transaction_stack.top >= 0) {
        size_t items = (size_t)transaction_stack.top + 1;
        if (fread(transaction_stack.transactions,
                  sizeof(struct Transaction),
                  items,
                  file) != items) {
            transaction_stack.top = -1;
        }
    }

    fclose(file);
}

// Free all allocated memory
void free_accounts() {
    struct Account* current = accounts_head;
    while (current) {
        struct Account* next = current->next;
        free(current);
        current = next;
    }
    accounts_head = NULL;
}

void free_transaction_stack() {
    free(transaction_stack.transactions);
}

// Send JSON response to client
void send_json_response(const char* json) {
    printf("Content-Type: application/json\r\n");
    printf("Content-Length: %zu\r\n", strlen(json));
    printf("\r\n");
    printf("%s", json);
}

// Send error response
void send_error(const char* message) {
    struct json_object *root = json_object_new_object();
    json_object_object_add(root, "status", json_object_new_string("error"));
    json_object_object_add(root, "message", json_object_new_string(message));
    
    const char *json = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
    send_json_response(json);
    
    json_object_put(root);
}

// Handle CGI request
void handle_cgi_request() {
    // Load data at the start
    load_data_from_disk();
    
    // Read POST data
    char* content_length_str = getenv("CONTENT_LENGTH");
    if (!content_length_str) {
        send_error("No content length provided");
        return;
    }
    
    int content_length = atoi(content_length_str);
    if (content_length <= 0) {
        send_error("Invalid content length");
        return;
    }
    
    char* post_data = (char*)malloc(content_length + 1);
    if (!post_data) {
        send_error("Memory allocation failed");
        return;
    }
    
    if (fread(post_data, 1, content_length, stdin) != (size_t)content_length) {
        free(post_data);
        send_error("Failed to read POST data");
        return;
    }
    post_data[content_length] = '\0';
    
    // Parse JSON request
    struct json_tokener *tok = json_tokener_new();
    struct json_object *root = json_tokener_parse_ex(tok, post_data, content_length);
    free(post_data);
    
    if (!root) {
        send_error("Invalid JSON request");
        json_tokener_free(tok);
        return;
    }
    
    // Get action from JSON
    struct json_object *action_obj;
    const char *action;
    
    if (!json_object_object_get_ex(root, "action", &action_obj) || 
        !json_object_is_type(action_obj, json_type_string)) {
        json_object_put(root);
        json_tokener_free(tok);
        send_error("No action specified");
        return;
    }
    
    action = json_object_get_string(action_obj);
    struct json_object *response = NULL;
    
    // Route to appropriate handler
    if (strcmp(action, "create_account") == 0) {
        struct json_object *name_obj, *balance_obj;
        const char *name;
        double initial_balance;
        
        if (!json_object_object_get_ex(root, "name", &name_obj) ||
            !json_object_object_get_ex(root, "initial_balance", &balance_obj)) {
            send_error("Missing required parameters");
            json_object_put(root);
            json_tokener_free(tok);
            return;
        }
        
        name = json_object_get_string(name_obj);
        initial_balance = json_object_get_double(balance_obj);
        
        if (!name || initial_balance < 0) {
            send_error("Invalid parameters for create_account");
            json_object_put(root);
            json_tokener_free(tok);
            return;
        }
        
        struct Account* account = create_account(name, initial_balance);
        if (account) {
            response = json_object_new_object();
            json_object_object_add(response, "status", json_object_new_string("success"));
            json_object_object_add(response, "account_number", json_object_new_int64(account->account_number));
            save_data_to_disk();
        } else {
            send_error("Failed to create account");
            json_object_put(root);
            json_tokener_free(tok);
            return;
        }
    }
    else if (strcmp(action, "deposit") == 0 || strcmp(action, "withdraw") == 0) {
        struct json_object *account_obj, *amount_obj;
        long int account_number;
        double amount;
        
        if (!json_object_object_get_ex(root, "account_number", &account_obj) ||
            !json_object_object_get_ex(root, "amount", &amount_obj)) {
            send_error("Missing required parameters");
            json_object_put(root);
            json_tokener_free(tok);
            return;
        }
        
        account_number = json_object_get_int64(account_obj);
        amount = json_object_get_double(amount_obj);
        
        if (account_number <= 0 || amount <= 0) {
            send_error("Invalid parameters for deposit/withdraw");
            json_object_put(root);
            json_tokener_free(tok);
            return;
        }
        
        if (strcmp(action, "withdraw") == 0) {
            amount = -amount; // Negative for withdrawal
        }
        
        if (update_balance(account_number, amount)) {
            push_transaction(account_number, 
                           amount > 0 ? "Deposit" : "Withdrawal", 
                           amount > 0 ? amount : -amount);
            
            struct Account* account = find_account(account_number);
            response = json_object_new_object();
            json_object_object_add(response, "status", json_object_new_string("success"));
            json_object_object_add(response, "new_balance", json_object_new_double(account->balance));
            save_data_to_disk();
        } else {
            send_error(amount < 0 ? "Insufficient funds or invalid account" : "Invalid account");
            json_object_put(root);
            json_tokener_free(tok);
            return;
        }
    }
    else if (strcmp(action, "get_balance") == 0) {
        struct json_object *account_obj;
        long int account_number;
        
        if (!json_object_object_get_ex(root, "account_number", &account_obj)) {
            send_error("Missing account number");
            json_object_put(root);
            json_tokener_free(tok);
            return;
        }
        
        account_number = json_object_get_int64(account_obj);
        struct Account* account = find_account(account_number);
        
        if (account) {
            response = json_object_new_object();
            json_object_object_add(response, "status", json_object_new_string("success"));
            json_object_object_add(response, "balance", json_object_new_double(account->balance));
        } else {
            send_error("Account not found");
            json_object_put(root);
            json_tokener_free(tok);
            return;
        }
    }
    else if (strcmp(action, "get_accounts") == 0) {
        response = json_object_new_object();
        json_object_object_add(response, "status", json_object_new_string("success"));

        struct json_object *accounts_array = json_object_new_array();
        struct Account* current = accounts_head;
        while (current) {
            struct json_object *acct_json = json_object_new_object();
            json_object_object_add(acct_json, "account_number", json_object_new_int64(current->account_number));
            json_object_object_add(acct_json, "customer_name", json_object_new_string(current->customer_name));
            json_object_object_add(acct_json, "balance", json_object_new_double(current->balance));
            json_object_array_add(accounts_array, acct_json);
            current = current->next;
        }

        json_object_object_add(response, "accounts", accounts_array);
    }
    else if (strcmp(action, "get_transactions") == 0) {
        response = json_object_new_object();
        json_object_object_add(response, "status", json_object_new_string("success"));
        
        struct json_object *transactions_array = json_object_new_array();
        for (int i = 0; i <= transaction_stack.top; i++) {
            struct Transaction* t = &transaction_stack.transactions[i];
            
            struct json_object *t_json = json_object_new_object();
            json_object_object_add(t_json, "account_number", json_object_new_int64(t->account_number));
            json_object_object_add(t_json, "type", json_object_new_string(t->type));
            json_object_object_add(t_json, "amount", json_object_new_double(t->amount));
            json_object_object_add(t_json, "timestamp", json_object_new_string(t->timestamp));
            
            json_object_array_add(transactions_array, t_json);
        }
        json_object_object_add(response, "transactions", transactions_array);
    }
    else {
        send_error("Unknown action");
        json_object_put(root);
        json_tokener_free(tok);
        return;
    }
    
    // Send response
    const char *json_response = json_object_to_json_string_ext(response, JSON_C_TO_STRING_PRETTY);
    send_json_response(json_response);
    
    // Cleanup
    json_object_put(response);
    json_object_put(root);
    json_tokener_free(tok);
}

int main() {
    // Initialize transaction stack
    init_transaction_stack();
    
    // Handle the CGI request
    handle_cgi_request();
    
    // Cleanup
    free_accounts();
    free_transaction_stack();
    
    return 0;
}
