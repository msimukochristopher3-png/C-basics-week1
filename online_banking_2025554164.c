#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>

#define MAX_NAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 20
#define MAX_DESCRIPTION_LENGTH 100
#define ACCOUNTS_DB "accounts.db"
#define TRANSACTIONS_DB "transactions.db"
#define MAX_TRANSACTION_AMOUNT 1000000.0

// Simple hash function for demonstration (not cryptographically secure)
void simple_sha256(const char* input, char* output) {
    uint32_t hash = 5381;
    int c;
    const char* ptr = input;
    
    while ((c = *ptr++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    // Convert to hex string (simplified for demo)
    for(int i = 0; i < 8; i++) {
        sprintf(output + (i * 8), "%08x", hash + i * 12345);
    }
    output[64] = 0;
}

// Structure to hold account information
typedef struct {
    char fullName[MAX_NAME_LENGTH];
    int accountNumber;
    char passwordHash[65];
    char salt[17];
    long balance;
    int isActive;
    time_t dateCreated;
} BankAccount;

// Structure to hold transaction information
typedef struct {
    int transactionId;
    int accountNumber;
    char type[20];
    long amount;
    long balanceAfter;
    char timestamp[20];
    char description[MAX_DESCRIPTION_LENGTH];
} Transaction;

// Global variables for current session
BankAccount currentUser;
int isLoggedIn = 0;

// Database function prototypes
int initializeDatabase();
int createAccount(const BankAccount *account);
int findAccountByNumber(int accountNumber, BankAccount *result);
int updateAccountBalance(int accountNumber, long newBalanceCents);
int updateAccountPassword(int accountNumber, const char *newPassword);
int recordTransaction(const Transaction *transaction);
int getAccountTransactions(int accountNumber, Transaction **transactions, int *count);
void freeTransactions(Transaction *transactions);
int validateEnhancedPassword(const char *password);
void clearInputBuffer();
void printHeader(const char *title);
void getCurrentTimestamp(char* buffer);
void generateSalt(char* salt, int length);
void hashPassword(const char* plain, const char* salt, char* hashed);
void safeInputString(char* buffer, int size, const char* prompt);
int getIntegerInput(const char* prompt);
float getFloatInput(const char* prompt);
int transferFundsWithRollback(int fromAccount, int toAccount, long amountCents);
int closeAccount(int accountNumber);
void applyMonthlyInterest();
void generateAccountStatement();

// Currency conversion helpers
float centsToFloat(long cents);
long floatToCents(float amount);

// Business logic function prototypes
void mainMenu();
void userMenu();
void registerAccount();
int login();
void depositFunds();
void withdrawFunds();
void transferFunds();
void changePassword();
void displayAccountDetails();
void viewTransactionHistory();
void closeCurrentAccount();

int main() {
    srand(time(NULL));
    
    if (!initializeDatabase()) {
        printf("❌ Failed to initialize database system!\n");
        return 1;
    }
    
    printf("============================================\n");
    printf("      WELCOME TO CM BANK\n");
    printf("        Student: 2025554164\n");
    printf("          Currency: Zambian Kwacha\n");
    printf("============================================\n");
    
    mainMenu();
    return 0;
}

// Currency conversion functions
float centsToFloat(long cents) {
    return (float)cents / 100.0f;
}

long floatToCents(float amount) {
    return (long)(amount * 100 + 0.5);
}

int initializeDatabase() {
    FILE *file;
    
    file = fopen(ACCOUNTS_DB, "ab");
    if (file == NULL) return 0;
    fclose(file);
    
    file = fopen(TRANSACTIONS_DB, "ab");
    if (file == NULL) return 0;
    fclose(file);
    
    return 1;
}

// Security Functions (Android compatible)
void generateSalt(char* salt, int length) {
    const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    for (int i = 0; i < length; i++) {
        salt[i] = chars[rand() % (sizeof(chars) - 1)];
    }
    salt[length] = '\0';
}

void hashPassword(const char* plain, const char* salt, char* hashed) {
    char salted[256];
    snprintf(salted, sizeof(salted), "%s%s", plain, salt);
    simple_sha256(salted, hashed);
}

// Input Handling Functions
void safeInputString(char* buffer, int size, const char* prompt) {
    printf("%s", prompt);
    memset(buffer, 0, size);
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
    } else {
        buffer[0] = 0;
        clearInputBuffer();
    }
}

int getIntegerInput(const char* prompt) {
    char buffer[100];
    int value;
    
    while (1) {
        safeInputString(buffer, sizeof(buffer), prompt);
        if (sscanf(buffer, "%d", &value) == 1) {
            return value;
        }
        printf("Invalid input! Please enter a valid number.\n");
    }
}

float getFloatInput(const char* prompt) {
    char buffer[100];
    float value;
    
    while (1) {
        safeInputString(buffer, sizeof(buffer), prompt);
        if (sscanf(buffer, "%f", &value) == 1) {
            return value;
        }
        printf("Invalid input! Please enter a valid number.\n");
    }
}

int validateEnhancedPassword(const char *password) {
    int len = strlen(password);
    if (len < 8) return 0;
    
    int hasDigit = 0, hasAlpha = 0;
    for (int i = 0; i < len; i++) {
        if (isdigit(password[i])) hasDigit = 1;
        if (isalpha(password[i])) hasAlpha = 1;
    }
    return hasDigit && hasAlpha;
}

// Database Functions
int createAccount(const BankAccount *account) {
    FILE *file = fopen(ACCOUNTS_DB, "ab");
    if (file == NULL) return 0;
    
    int result = fwrite(account, sizeof(BankAccount), 1, file);
    fclose(file);
    return result == 1;
}

int findAccountByNumber(int accountNumber, BankAccount *result) {
    FILE *file = fopen(ACCOUNTS_DB, "rb");
    if (file == NULL) return 0;
    
    BankAccount account;
    int found = 0;
    
    while (fread(&account, sizeof(BankAccount), 1, file)) {
        if (account.accountNumber == accountNumber) {
            *result = account;
            found = 1;
            break;
        }
    }
    
    fclose(file);
    return found;
}

int updateAccountBalance(int accountNumber, long newBalanceCents) {
    FILE *file = fopen(ACCOUNTS_DB, "rb+");
    if (file == NULL) return 0;
    
    BankAccount account;
    long position;
    int found = 0;
    
    while (fread(&account, sizeof(BankAccount), 1, file)) {
        position = ftell(file) - sizeof(BankAccount);
        
        if (account.accountNumber == accountNumber) {
            account.balance = newBalanceCents;
            fseek(file, position, SEEK_SET);
            int writeResult = fwrite(&account, sizeof(BankAccount), 1, file);
            found = (writeResult == 1);
            break;
        }
    }
    
    fclose(file);
    return found;
}

int updateAccountPassword(int accountNumber, const char *newPassword) {
    FILE *file = fopen(ACCOUNTS_DB, "rb+");
    if (file == NULL) return 0;
    
    BankAccount account;
    long position;
    int found = 0;
    
    while (fread(&account, sizeof(BankAccount), 1, file)) {
        position = ftell(file) - sizeof(BankAccount);
        
        if (account.accountNumber == accountNumber) {
            char newSalt[17];
            char newHash[65];
            generateSalt(newSalt, 16);
            hashPassword(newPassword, newSalt, newHash);
            
            strcpy(account.passwordHash, newHash);
            strcpy(account.salt, newSalt);
            fseek(file, position, SEEK_SET);
            fwrite(&account, sizeof(BankAccount), 1, file);
            found = 1;
            break;
        }
    }
    
    fclose(file);
    return found;
}

int recordTransaction(const Transaction *transaction) {
    FILE *file = fopen(TRANSACTIONS_DB, "ab");
    if (file == NULL) return 0;
    
    int result = fwrite(transaction, sizeof(Transaction), 1, file);
    fclose(file);
    return result == 1;
}

int getAccountTransactions(int accountNumber, Transaction **transactions, int *count) {
    FILE *file = fopen(TRANSACTIONS_DB, "rb");
    if (file == NULL) return 0;
    
    Transaction transaction;
    int capacity = 10;
    int size = 0;
    
    *transactions = malloc(capacity * sizeof(Transaction));
    if (*transactions == NULL) {
        fclose(file);
        return 0;
    }
    
    while (fread(&transaction, sizeof(Transaction), 1, file)) {
        if (transaction.accountNumber == accountNumber) {
            if (size >= capacity) {
                capacity *= 2;
                Transaction *temp = realloc(*transactions, capacity * sizeof(Transaction));
                if (temp == NULL) {
                    free(*transactions);
                    *transactions = NULL;
                    fclose(file);
                    return 0;
                }
                *transactions = temp;
            }
            (*transactions)[size++] = transaction;
        }
    }
    
    fclose(file);
    *count = size;
    return 1;
}

void freeTransactions(Transaction *transactions) {
    free(transactions);
}

// Enhanced Transfer Function with Rollback
int transferFundsWithRollback(int fromAccount, int toAccount, long amountCents) {
    FILE *file = fopen(ACCOUNTS_DB, "rb+");
    if (file == NULL) return 0;
    
    BankAccount fromAcc, toAcc;
    long fromPos = -1, toPos = -1;
    int fromFound = 0, toFound = 0;
    
    BankAccount acc;
    long position;
    
    while (fread(&acc, sizeof(BankAccount), 1, file)) {
        position = ftell(file) - sizeof(BankAccount);
        
        if (acc.accountNumber == fromAccount) {
            fromAcc = acc;
            fromPos = position;
            fromFound = 1;
        } else if (acc.accountNumber == toAccount) {
            toAcc = acc;
            toPos = position;
            toFound = 1;
        }
        if (fromFound && toFound) break;
    }
    
    if (!fromFound || !toFound) {
        fclose(file);
        return 0;
    }
    
    if (fromAcc.balance < amountCents) {
        fclose(file);
        return 0;
    }
    
    fromAcc.balance -= amountCents;
    toAcc.balance += amountCents;
    
    int success = 1;
    
    fseek(file, fromPos, SEEK_SET);
    if (fwrite(&fromAcc, sizeof(BankAccount), 1, file) != 1) {
        success = 0;
    }
    
    fseek(file, toPos, SEEK_SET);
    if (fwrite(&toAcc, sizeof(BankAccount), 1, file) != 1) {
        fromAcc.balance += amountCents;
        fseek(file, fromPos, SEEK_SET);
        fwrite(&fromAcc, sizeof(BankAccount), 1, file);
        success = 0;
    }
    
    fclose(file);
    return success;
}

// Account Management Functions
int closeAccount(int accountNumber) {
    FILE *file = fopen(ACCOUNTS_DB, "rb+");
    if (file == NULL) return 0;
    
    BankAccount account;
    long position;
    int found = 0;
    
    while (fread(&account, sizeof(BankAccount), 1, file)) {
        position = ftell(file) - sizeof(BankAccount);
        
        if (account.accountNumber == accountNumber) {
            account.isActive = 0;
            fseek(file, position, SEEK_SET);
            fwrite(&account, sizeof(BankAccount), 1, file);
            found = 1;
            break;
        }
    }
    
    fclose(file);
    return found;
}

void applyMonthlyInterest() {
    float interestRate = 0.015;
    
    FILE *file = fopen(ACCOUNTS_DB, "rb+");
    if (file == NULL) return;
    
    BankAccount account;
    long position;
    
    while (fread(&account, sizeof(BankAccount), 1, file)) {
        position = ftell(file) - sizeof(BankAccount);
        
        if (account.isActive && account.balance > 0) {
            float interest = centsToFloat(account.balance) * interestRate;
            long interestCents = floatToCents(interest);
            account.balance += interestCents;
            
            fseek(file, position, SEEK_SET);
            fwrite(&account, sizeof(BankAccount), 1, file);
            
            Transaction transaction;
            transaction.transactionId = 0;
            transaction.accountNumber = account.accountNumber;
            strcpy(transaction.type, "INTEREST");
            transaction.amount = interestCents;
            transaction.balanceAfter = account.balance;
            getCurrentTimestamp(transaction.timestamp);
            strcpy(transaction.description, "Monthly interest credit");
            recordTransaction(&transaction);
        }
    }
    
    fclose(file);
}

void generateAccountStatement() {
    char filename[100];
    snprintf(filename, sizeof(filename), "statement_%d.txt", currentUser.accountNumber);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("❌ Failed to create statement file!\n");
        return;
    }
    
    fprintf(file, "============================================\n");
    fprintf(file, "           BANK ACCOUNT STATEMENT\n");
    fprintf(file, "============================================\n");
    fprintf(file, "Account Holder: %s\n", currentUser.fullName);
    fprintf(file, "Account Number: %d\n", currentUser.accountNumber);
    fprintf(file, "Statement Date: ");
    
    char timestamp[20];
    getCurrentTimestamp(timestamp);
    fprintf(file, "%s\n", timestamp);
    fprintf(file, "Current Balance: K%.2f\n", centsToFloat(currentUser.balance));
    fprintf(file, "============================================\n");
    
    Transaction *transactions = NULL;
    int count = 0;
    
    if (getAccountTransactions(currentUser.accountNumber, &transactions, &count)) {
        fprintf(file, "Transaction History:\n");
        fprintf(file, "Date       | Type            | Amount    | Balance\n");
        fprintf(file, "-----------+-----------------+-----------+-----------\n");
        
        for (int i = 0; i < count; i++) {
            fprintf(file, "%s | %-15s | K%8.2f | K%8.2f\n",
                   transactions[i].timestamp,
                   transactions[i].type,
                   centsToFloat(transactions[i].amount),
                   centsToFloat(transactions[i].balanceAfter));
        }
        
        freeTransactions(transactions);
    }
    
    fprintf(file, "============================================\n");
    fclose(file);
    
    printf("✅ Account statement generated: %s\n", filename);
}

// Business Logic Functions
void mainMenu() {
    int choice;
    
    do {
        printHeader("MAIN MENU");
        printf("1. Register New Account\n");
        printf("2. Login\n");
        printf("3. Apply Monthly Interest (Admin)\n");
        printf("4. Exit\n");
        printf("============================================\n");
        
        choice = getIntegerInput("Enter your choice (1-4): ");
        
        switch(choice) {
            case 1:
                registerAccount();
                break;
            case 2:
                if (login()) {
                    userMenu();
                }
                break;
            case 3:
                applyMonthlyInterest();
                printf("✅ Monthly interest applied to all active accounts!\n");
                break;
            case 4:
                printf("Thank you for using Online Banking System!\n");
                printf("Goodbye! 👋\n");
                break;
            default:
                printf("Invalid choice! Please select 1-4.\n");
        }
    } while (choice != 4);
}

void userMenu() {
    int choice;
    
    do {
        printHeader("USER DASHBOARD");
        printf("Welcome, %s!\n", currentUser.fullName);
        printf("============================================\n");
        printf("1. Deposit Funds\n");
        printf("2. Withdraw Funds\n");
        printf("3. Transfer Funds\n");
        printf("4. Change Password\n");
        printf("5. View Account Details\n");
        printf("6. View Transaction History\n");
        printf("7. Generate Account Statement\n");
        printf("8. Close Account\n");
        printf("9. Logout\n");
        printf("============================================\n");
        
        choice = getIntegerInput("Enter your choice (1-9): ");
        
        switch(choice) {
            case 1:
                depositFunds();
                break;
            case 2:
                withdrawFunds();
                break;
            case 3:
                transferFunds();
                break;
            case 4:
                changePassword();
                break;
            case 5:
                displayAccountDetails();
                break;
            case 6:
                viewTransactionHistory();
                break;
            case 7:
                generateAccountStatement();
                break;
            case 8:
                closeCurrentAccount();
                break;
            case 9:
                isLoggedIn = 0;
                printf("Logged out successfully!\n");
                break;
            default:
                printf("Invalid choice! Please select 1-9.\n");
        }
    } while (choice != 9 && isLoggedIn);
}

void registerAccount() {
    printHeader("ACCOUNT REGISTRATION");
    
    BankAccount newAccount;
    char password[MAX_PASSWORD_LENGTH];
    char confirmPassword[MAX_PASSWORD_LENGTH];
    
    clearInputBuffer();
    safeInputString(newAccount.fullName, MAX_NAME_LENGTH, "Enter your full name: ");
    
    if (strlen(newAccount.fullName) < 2) {
        printf("Name must be at least 2 characters long.\n");
        return;
    }
    
    newAccount.accountNumber = getIntegerInput("Create your account number (must be 5-10 digits): ");
    
    if (newAccount.accountNumber < 10000 || newAccount.accountNumber > 9999999999) {
        printf("❌ Account number must be 5 to 10 digits!\n");
        return;
    }
    
    BankAccount existingAccount;
    if (findAccountByNumber(newAccount.accountNumber, &existingAccount)) {
        printf("❌ Account number already exists! Please choose a different number.\n");
        return;
    }
    
    safeInputString(password, MAX_PASSWORD_LENGTH, "Enter password (min 8 chars, mix of letters and numbers): ");
    
    if (!validateEnhancedPassword(password)) {
        printf("❌ Password must be at least 8 characters with both letters and numbers.\n");
        return;
    }
    
    safeInputString(confirmPassword, MAX_PASSWORD_LENGTH, "Confirm your password: ");
    
    if (strcmp(password, confirmPassword) != 0) {
        printf("❌ Passwords do not match! Registration failed.\n");
        return;
    }
    
    generateSalt(newAccount.salt, 16);
    hashPassword(password, newAccount.salt, newAccount.passwordHash);
    
    float initialDeposit = getFloatInput("Enter initial deposit amount (K): ");
    if (initialDeposit < 0) {
        printf("Invalid deposit amount!\n");
        return;
    }
    
    newAccount.balance = floatToCents(initialDeposit);
    newAccount.isActive = 1;
    newAccount.dateCreated = time(NULL);
    
    if (!createAccount(&newAccount)) {
        printf("❌ Failed to create account! Please try again.\n");
        return;
    }
    
    Transaction transaction;
    transaction.transactionId = 0;
    transaction.accountNumber = newAccount.accountNumber;
    strcpy(transaction.type, "ACCOUNT_CREATION");
    transaction.amount = newAccount.balance;
    transaction.balanceAfter = newAccount.balance;
    getCurrentTimestamp(transaction.timestamp);
    strcpy(transaction.description, "Account created with initial deposit");
    
    if (!recordTransaction(&transaction)) {
        printf("⚠️  Account created but failed to record transaction.\n");
    }
    
    printf("\n🎉 CONGRATULATIONS! 🎉\n");
    printf("============================================\n");
    printf("✅ ACCOUNT SUCCESSFULLY REGISTERED!\n");
    printf("============================================\n");
    printf("Account Holder: %s\n", newAccount.fullName);
    printf("Account Number: %d\n", newAccount.accountNumber);
    printf("Initial Balance: K%.2f\n", centsToFloat(newAccount.balance));
    printf("============================================\n");
    printf("Thank you for choosing Online Banking System!\n");
    printf("Your account is now active and ready to use.\n");
    printf("You can login to access all banking services.\n");
    printf("============================================\n\n");
}

int login() {
    printHeader("ACCOUNT LOGIN");
    
    int accountNumber = getIntegerInput("Enter your account number: ");
    char password[MAX_PASSWORD_LENGTH];
    safeInputString(password, MAX_PASSWORD_LENGTH, "Enter your password: ");
    
    BankAccount account;
    if (findAccountByNumber(accountNumber, &account)) {
        if (!account.isActive) {
            printf("❌ Login failed! Invalid account number or password.\n");
            return 0;
        }
        
        char testHash[65];
        hashPassword(password, account.salt, testHash);
        
        if (strcmp(account.passwordHash, testHash) == 0) {
            currentUser = account;
            isLoggedIn = 1;
            printf("✅ Login successful! Welcome back, %s!\n", account.fullName);
            return 1;
        }
    }
    
    printf("❌ Login failed! Invalid account number or password.\n");
    return 0;
}

void depositFunds() {
    printHeader("DEPOSIT FUNDS");
    
    float amount = getFloatInput("Enter amount to deposit (K): ");
    
    if (amount <= 0) {
        printf("❌ Invalid amount! Please enter a positive number.\n");
        return;
    }
    
    if (amount > MAX_TRANSACTION_AMOUNT) {
        printf("❌ Deposit amount exceeds maximum transaction limit of K%.2f!\n", MAX_TRANSACTION_AMOUNT);
        return;
    }
    
    long amountCents = floatToCents(amount);
    long newBalanceCents = currentUser.balance + amountCents;
    
    if (!updateAccountBalance(currentUser.accountNumber, newBalanceCents)) {
        printf("❌ Failed to process deposit! Please try again.\n");
        return;
    }
    
    Transaction transaction;
    transaction.transactionId = 0;
    transaction.accountNumber = currentUser.accountNumber;
    strcpy(transaction.type, "DEPOSIT");
    transaction.amount = amountCents;
    transaction.balanceAfter = newBalanceCents;
    getCurrentTimestamp(transaction.timestamp);
    strcpy(transaction.description, "Cash deposit");
    
    if (!recordTransaction(&transaction)) {
        printf("⚠️  Deposit processed but failed to record transaction.\n");
    }
    
    currentUser.balance = newBalanceCents;
    
    printf("✅ Deposit successful!\n");
    printf("New balance: K%.2f\n", centsToFloat(currentUser.balance));
}

void withdrawFunds() {
    printHeader("WITHDRAW FUNDS");
    
    printf("Current balance: K%.2f\n", centsToFloat(currentUser.balance));
    float amount = getFloatInput("Enter amount to withdraw (K): ");
    
    if (amount <= 0) {
        printf("❌ Invalid amount! Please enter a positive number.\n");
        return;
    }
    
    if (amount > MAX_TRANSACTION_AMOUNT) {
        printf("❌ Withdrawal amount exceeds maximum transaction limit of K%.2f!\n", MAX_TRANSACTION_AMOUNT);
        return;
    }
    
    long amountCents = floatToCents(amount);
    if (amountCents > currentUser.balance) {
        printf("❌ Insufficient funds! Your balance is K%.2f\n", centsToFloat(currentUser.balance));
        return;
    }
    
    long newBalanceCents = currentUser.balance - amountCents;
    
    if (!updateAccountBalance(currentUser.accountNumber, newBalanceCents)) {
        printf("❌ Failed to process withdrawal! Please try again.\n");
        return;
    }
    
    Transaction transaction;
    transaction.transactionId = 0;
    transaction.accountNumber = currentUser.accountNumber;
    strcpy(transaction.type, "WITHDRAWAL");
    transaction.amount = amountCents;
    transaction.balanceAfter = newBalanceCents;
    getCurrentTimestamp(transaction.timestamp);
    strcpy(transaction.description, "Cash withdrawal");
    
    if (!recordTransaction(&transaction)) {
        printf("⚠️  Withdrawal processed but failed to record transaction.\n");
    }
    
    currentUser.balance = newBalanceCents;
    
    printf("✅ Withdrawal successful!\n");
    printf("New balance: K%.2f\n", centsToFloat(currentUser.balance));
}

void transferFunds() {
    printHeader("FUND TRANSFER");
    
    int targetAccountNumber = getIntegerInput("Enter recipient's account number: ");
    float amount;
    
    if (targetAccountNumber == currentUser.accountNumber) {
        printf("❌ Cannot transfer to your own account!\n");
        return;
    }
    
    BankAccount targetAccount;
    if (!findAccountByNumber(targetAccountNumber, &targetAccount)) {
        printf("❌ Recipient account not found!\n");
        return;
    }
    
    if (!targetAccount.isActive) {
        printf("❌ Recipient account is closed!\n");
        return;
    }
    
    printf("Recipient: %s\n", targetAccount.fullName);
    printf("Current balance: K%.2f\n", centsToFloat(currentUser.balance));
    amount = getFloatInput("Enter amount to transfer (K): ");
    
    if (amount <= 0) {
        printf("❌ Invalid amount! Please enter a positive number.\n");
        return;
    }
    
    if (amount > MAX_TRANSACTION_AMOUNT) {
        printf("❌ Transfer amount exceeds maximum transaction limit of K%.2f!\n", MAX_TRANSACTION_AMOUNT);
        return;
    }
    
    long amountCents = floatToCents(amount);
    if (amountCents > currentUser.balance) {
        printf("❌ Insufficient funds! Your balance is K%.2f\n", centsToFloat(currentUser.balance));
        return;
    }
    
    if (!transferFundsWithRollback(currentUser.accountNumber, targetAccountNumber, amountCents)) {
        printf("❌ Failed to process transfer! Please try again.\n");
        return;
    }
    
    Transaction senderTransaction, receiverTransaction;
    
    senderTransaction.transactionId = 0;
    senderTransaction.accountNumber = currentUser.accountNumber;
    strcpy(senderTransaction.type, "TRANSFER_SENT");
    senderTransaction.amount = amountCents;
    senderTransaction.balanceAfter = currentUser.balance - amountCents;
    getCurrentTimestamp(senderTransaction.timestamp);
    snprintf(senderTransaction.description, MAX_DESCRIPTION_LENGTH, 
             "Transfer to account %d (%s)", targetAccountNumber, targetAccount.fullName);
    
    receiverTransaction.transactionId = 0;
    receiverTransaction.accountNumber = targetAccountNumber;
    strcpy(receiverTransaction.type, "TRANSFER_RECEIVED");
    receiverTransaction.amount = amountCents;
    receiverTransaction.balanceAfter = targetAccount.balance + amountCents;
    getCurrentTimestamp(receiverTransaction.timestamp);
    snprintf(receiverTransaction.description, MAX_DESCRIPTION_LENGTH, 
             "Transfer from account %d (%s)", currentUser.accountNumber, currentUser.fullName);
    
    if (!recordTransaction(&senderTransaction) || !recordTransaction(&receiverTransaction)) {
        printf("⚠️  Transfer processed but failed to record some transactions.\n");
    }
    
    currentUser.balance -= amountCents;
    
    printf("✅ Transfer successful!\n");
    printf("Transferred: K%.2f to %s\n", amount, targetAccount.fullName);
    printf("Your new balance: K%.2f\n", centsToFloat(currentUser.balance));
}

void changePassword() {
    printHeader("CHANGE PASSWORD");
    
    char currentPassword[MAX_PASSWORD_LENGTH];
    char newPassword[MAX_PASSWORD_LENGTH];
    char confirmPassword[MAX_PASSWORD_LENGTH];
    
    safeInputString(currentPassword, MAX_PASSWORD_LENGTH, "Enter current password: ");
    
    char testHash[65];
    hashPassword(currentPassword, currentUser.salt, testHash);
    if (strcmp(currentUser.passwordHash, testHash) != 0) {
        printf("❌ Current password is incorrect!\n");
        return;
    }
    
    safeInputString(newPassword, MAX_PASSWORD_LENGTH, "Enter new password (min 8 chars, mix of letters and numbers): ");
    
    if (!validateEnhancedPassword(newPassword)) {
        printf("❌ Password must be at least 8 characters with both letters and numbers.\n");
        return;
    }
    
    safeInputString(confirmPassword, MAX_PASSWORD_LENGTH, "Confirm new password: ");
    
    if (strcmp(newPassword, confirmPassword) != 0) {
        printf("❌ Passwords do not match!\n");
        return;
    }
    
    if (!updateAccountPassword(currentUser.accountNumber, newPassword)) {
        printf("❌ Failed to change password! Please try again.\n");
        return;
    }
    
    Transaction transaction;
    transaction.transactionId = 0;
    transaction.accountNumber = currentUser.accountNumber;
    strcpy(transaction.type, "PASSWORD_CHANGE");
    transaction.amount = 0;
    transaction.balanceAfter = currentUser.balance;
    getCurrentTimestamp(transaction.timestamp);
    strcpy(transaction.description, "Password changed successfully");
    
    if (!recordTransaction(&transaction)) {
        printf("⚠️  Password changed but failed to record transaction.\n");
    }
    
    findAccountByNumber(currentUser.accountNumber, &currentUser);
    
    printf("✅ Password changed successfully!\n");
}

void displayAccountDetails() {
    printHeader("ACCOUNT DETAILS");
    
    printf("Account Holder: %s\n", currentUser.fullName);
    printf("Account Number: %d\n", currentUser.accountNumber);
    printf("Current Balance: K%.2f\n", centsToFloat(currentUser.balance));
    printf("Account Status: %s\n", currentUser.isActive ? "Active" : "Closed");
    
    char dateStr[20];
    struct tm *tm_info = localtime(&currentUser.dateCreated);
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", tm_info);
    printf("Date Created: %s\n", dateStr);
    
    printf("============================================\n");
}

void viewTransactionHistory() {
    printHeader("TRANSACTION HISTORY");
    
    Transaction *transactions = NULL;
    int count = 0;
    
    if (!getAccountTransactions(currentUser.accountNumber, &transactions, &count)) {
        printf("❌ Failed to load transaction history!\n");
        return;
    }
    
    printf("Account: %s (%d)\n", currentUser.fullName, currentUser.accountNumber);
    printf("Current Balance: K%.2f\n", centsToFloat(currentUser.balance));
    printf("============================================\n");
    
    if (count == 0) {
        printf("No transactions found for this account.\n");
    } else {
        printf("Date       | Type            | Amount    | Balance   | Description\n");
        printf("-----------+-----------------+-----------+-----------+----------------\n");
        
        for (int i = 0; i < count; i++) {
            printf("%s | %-15s | K%8.2f | K%8.2f | %s\n",
                   transactions[i].timestamp,
                   transactions[i].type,
                   centsToFloat(transactions[i].amount),
                   centsToFloat(transactions[i].balanceAfter),
                   transactions[i].description);
        }
    }
    
    printf("============================================\n");
    
    if (transactions != NULL) {
        freeTransactions(transactions);
    }
}

void closeCurrentAccount() {
    printHeader("CLOSE ACCOUNT");
    
    printf("⚠️  WARNING: This action is irreversible!\n");
    printf("All account data will be preserved but the account will be closed.\n");
    printf("You will not be able to login or perform transactions.\n\n");
    
    char confirmation[10];
    safeInputString(confirmation, sizeof(confirmation), "Type 'CLOSE' to confirm: ");
    
    if (strcmp(confirmation, "CLOSE") != 0) {
        printf("Account closure cancelled.\n");
        return;
    }
    
    char password[MAX_PASSWORD_LENGTH];
    safeInputString(password, MAX_PASSWORD_LENGTH, "Enter your password to confirm: ");
    
    char testHash[65];
    hashPassword(password, currentUser.salt, testHash);
    if (strcmp(currentUser.passwordHash, testHash) != 0) {
        printf("❌ Password incorrect! Account closure failed.\n");
        return;
    }
    
    if (currentUser.balance > 0) {
        printf("❌ Cannot close account with remaining balance. Please withdraw all funds first.\n");
        return;
    }
    
    if (!closeAccount(currentUser.accountNumber)) {
        printf("❌ Failed to close account! Please try again.\n");
        return;
    }
    
    Transaction transaction;
    transaction.transactionId = 0;
    transaction.accountNumber = currentUser.accountNumber;
    strcpy(transaction.type, "ACCOUNT_CLOSURE");
    transaction.amount = 0;
    transaction.balanceAfter = 0;
    getCurrentTimestamp(transaction.timestamp);
    strcpy(transaction.description, "Account closed permanently");
    recordTransaction(&transaction);
    
    printf("✅ Account closed successfully!\n");
    isLoggedIn = 0;
}

// Utility functions
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printHeader(const char *title) {
    printf("\n============================================\n");
    printf("          %s\n", title);
    printf("============================================\n");
}

void getCurrentTimestamp(char* buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 20, "%Y-%m-%d %H:%M", t);
}
