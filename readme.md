

Online Banking System

A complete C-based banking management system with secure user authentication, transaction processing, and account management features.

📋 Project Overview

This is a console-based banking application developed in C that provides essential banking operations including account creation, funds transfer, transaction history, and secure user authentication.

✨ Features

🔐 Security Features

· Secure Password Hashing with salt generation
· Enhanced Password Validation (minimum 8 characters with letters and numbers)
· Account Locking/Closing capabilities
· Secure Session Management

💰 Banking Operations

· Account Registration with initial deposit
· Deposit and Withdrawal operations
· Fund Transfers between accounts with rollback protection
· Balance Inquiry and account details
· Transaction History viewing
· Account Statements generation

🏦 Account Management

· Account Creation with unique account numbers
· Password Change functionality
· Account Closure with balance verification
· Monthly Interest application (admin feature)

🛠️ Technical Specifications

Programming Language

· C Language (ANSI C compatible)

Data Storage

· File-based database using binary files
· Two main databases:
  · accounts.db - Stores account information
  · transactions.db - Stores transaction records

Security

· Custom hash function for password security
· Salt generation for enhanced security
· Input validation and buffer overflow protection

📁 File Structure

```
banking_system/
├── banking_system.c      # Main source code
├── accounts.db           # Account database (auto-generated)
├── transactions.db       # Transaction database (auto-generated)
├── statement_XXXXX.txt   # Generated account statements
└── README.md            # This file
```

🔧 Installation & Compilation

Prerequisites

· C Compiler (GCC, Clang, or any ANSI C compatible compiler)
· Standard C libraries

Compilation Instructions

On Linux/Unix/macOS:

```bash
gcc -o banking_system banking_system.c
```

On Windows (MinGW):

```bash
gcc -o banking_system.exe banking_system.c
```

On Android (Termux):

```bash
pkg install clang
clang -o banking_system banking_system.c
```

Using Online Compilers:

· OnlineGDB: https://www.onlinegdb.com/
· Replit: https://replit.com/
· CodeChef IDE: https://www.codechef.com/ide

🚀 How to Use

Starting the Application

```bash
./banking_system
```

Main Menu Options

1. Register New Account - Create a new bank account
2. Login - Access existing account
3. Apply Monthly Interest - Admin function to apply interest
4. Exit - Close the application

User Dashboard Features (After Login)

1. Deposit Funds - Add money to account
2. Withdraw Funds - Remove money from account
3. Transfer Funds - Send money to other accounts
4. Change Password - Update account password
5. View Account Details - Display account information
6. View Transaction History - Show all transactions
7. Generate Account Statement - Create statement file
8. Close Account - Permanently close account
9. Logout - End current session

📝 Account Requirements

Account Number

· Must be 5-10 digits long
· Must be unique

Password Requirements

· Minimum 8 characters
· Must contain both letters and numbers
· Case sensitive

Initial Deposit

· Must be a positive amount
· No maximum limit (within system constraints)

💳 Transaction Limits

· Maximum Transaction Amount: K1,000,000.00
· Currency: Zambian Kwacha (K)
· Interest Rate: 1.5% monthly (applied via admin function)

🔒 Security Notes

· Passwords are hashed and salted before storage
· Session management prevents unauthorized access
· Input validation prevents buffer overflow attacks
· Transfer operations include rollback protection
· Generic error messages prevent account enumeration

🐛 Troubleshooting

Common Issues

1. Compilation Errors
   · Ensure you have a C compiler installed
   · Check for syntax errors in the code
2. File Permission Issues
   · Ensure write permissions in the directory
   · Check if databases can be created
3. Input Problems
   · Follow the exact input format requested
   · Ensure passwords meet complexity requirements

Database Files

· If databases become corrupted, delete accounts.db and transactions.db to reset
· Account statements are saved as statement_XXXXX.txt files

📊 Sample Usage Flow

1. Start Application → Main Menu
2. Register Account → Enter details → Account created
3. Login → Enter credentials → Access dashboard
4. Perform Transactions → Deposit/Withdraw/Transfer
5. View History → Check transaction records
6. Generate Statement → Create PDF-like statement
7. Logout → Return to main menu

🎯 Student Information

· Student ID: 2025554164
· Currency: Zambian Kwacha (K)
· Project Type: Banking System Implementation

⚠️ Important Notes

· This is an educational project
· Not recommended for production banking systems
· Uses simplified hashing for demonstration
· Always backup important data
· Keep account credentials secure

🔄 Future Enhancements

Potential improvements for this system:

· GUI implementation
· Network connectivity
· Enhanced encryption
· Database optimization
· Multi-currency support
· Audit logging
· Report generation

