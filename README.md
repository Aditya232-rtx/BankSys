# C-Based Web Banking System

A secure and efficient web-based banking system with a C backend and modern web frontend. This application implements core banking operations using linked lists for account management and a stack for transaction history.

## Features

- **Account Management**: Create and manage bank accounts
- **Transactions**: Deposit and withdraw funds with validation
- **Real-time Updates**: View account balances and transaction history
- **Data Persistence**: All data is saved to disk and persists between server restarts
- **Responsive Design**: Works on desktop and mobile devices

## Prerequisites

- GCC (GNU Compiler Collection)
- libjson-c (for JSON parsing)
- A web server with CGI support (e.g., Apache, Nginx with FastCGI)
- Modern web browser

## Installation

1. **Install Dependencies**

   On Ubuntu/Debian:
   ```bash
   sudo apt update
   sudo apt install gcc libjson-c-dev apache2
   ```

   On macOS (using Homebrew):
   ```bash
   brew install gcc json-c
   ```

2. **Configure Web Server**

   For Apache:
   ```bash
   # Enable CGI module if not already enabled
   sudo a2enmod cgi
   
   # Create CGI directory if it doesn't exist
   sudo mkdir -p /usr/lib/cgi-bin
   
   # Set permissions
   sudo chmod 755 /usr/lib/cgi-bin
   
   # Restart Apache
   sudo systemctl restart apache2
   ```

3. **Compile and Install**

   ```bash
   # Clone the repository
   git clone https://github.com/Aditya232-rtx/BankSys.git
   cd BankSys
   
   # Compile the C program
   make
   
   # Install the CGI script
   sudo make install
   ```

4. **Configure Web Server (Apache Example)**

   Edit the Apache configuration to serve the frontend from the cloned repository and handle CGI requests:
   
   ```apache
   <VirtualHost *:80>
       ServerAdmin webmaster@localhost
       DocumentRoot /Library/WebServer/Documents/bank
       
       <Directory "/Library/WebServer/Documents/bank">
           Options Indexes FollowSymLinks
           AllowOverride None
           Require all granted
       </Directory>
       
       ScriptAlias /cgi-bin/ /Library/WebServer/CGI-Executables/
       <Directory "/Library/WebServer/CGI-Executables">
           AllowOverride None
           Options +ExecCGI -MultiViews +SymLinksIfOwnerMatch
           Require all granted
           AddHandler cgi-script .cgi
       </Directory>
   </VirtualHost>
   ```

   Then restart Apache:
   ```bash
   sudo apachectl restart
   ```

## Usage

1. Open your web browser and navigate to:
   ```
   http://localhost/
   ```

2. **Create an Account**
   - Fill in the customer name and initial balance
   - Click "Create Account"
   - Note your account number for future reference

3. **Make Transactions**
   - Enter the account number
   - Select transaction type (Deposit/Withdraw)
   - Enter the amount
   - Click "Process Transaction"

4. **Check Balance**
   - Enter an account number
   - Click "Check"

5. **View Accounts and Transactions**
   - The Accounts table shows all accounts and their balances
   - The Transactions table shows the most recent transactions
   - Use the Refresh buttons to update the data

## Project Structure

- `bank.h` - Header file with data structures and function declarations
- `bank.c` - Main C program with all the business logic
- `Makefile` - Build configuration
- `index.html` - Frontend HTML
- `styles.css` - Styling for the web interface
- `app.js` - Frontend JavaScript for handling user interactions
- `bank_data.dat` - Data file (created automatically)

## API Endpoints

The C backend exposes the following API endpoints via CGI:

- `POST /cgi-bin/bank.cgi` - Handle all banking operations
  - `action=create_account` - Create a new account
  - `action=deposit` - Deposit money to an account
  - `action=withdraw` - Withdraw money from an account
  - `action=get_balance` - Get account balance
  - `action=get_transactions` - Get transaction history

## Security Considerations

- All sensitive operations require a valid account number
- Input validation is performed on both client and server sides
- Data is persisted to disk in a binary format
- For production use, consider adding:
  - User authentication
  - HTTPS encryption
  - Rate limiting
  - Input sanitization

## Troubleshooting

- **CGI Script Not Executing**
  - Ensure the script has execute permissions: `chmod +x /usr/lib/cgi-bin/bank.cgi`
  - Check Apache error logs: `tail -f /var/log/apache2/error.log`
  - Verify that the CGI module is enabled: `sudo a2enmod cgi`

- **JSON Parsing Errors**
  - Ensure libjson-c is installed
  - Check that the Content-Type header is set to application/json

- **Data Not Persisting**
  - Verify that the web server user has write permissions to the data directory
  - Check for any errors in the server logs

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with C for high performance
- Uses JSON-C for JSON parsing
- Bootstrap 5 for responsive design
- Modern JavaScript for dynamic updates
