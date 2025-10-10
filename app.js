// API endpoint - update this to your CGI script URL
const API_URL = '/cgi-bin/bank.cgi';

// DOM Elements
const createAccountForm = document.getElementById('createAccountForm');
const transactionForm = document.getElementById('transactionForm');
const balanceForm = document.getElementById('balanceForm');
const refreshAccountsBtn = document.getElementById('refreshAccounts');
const refreshTransactionsBtn = document.getElementById('refreshTransactions');
const accountsTableBody = document.getElementById('accountsTableBody');
const transactionsTableBody = document.getElementById('transactionsTableBody');
const balanceResult = document.getElementById('balanceResult');
const statusMessage = document.getElementById('statusMessage');

// Initialize the application
document.addEventListener('DOMContentLoaded', () => {
    // Load initial data
    loadAccounts();
    loadTransactions();
    
    // Set up event listeners
    setupEventListeners();
});

// Set up all event listeners
function setupEventListeners() {
    // Create account form submission
    if (createAccountForm) {
        createAccountForm.addEventListener('submit', handleCreateAccount);
    }
    
    // Transaction form submission
    if (transactionForm) {
        transactionForm.addEventListener('submit', handleTransaction);
    }
    
    // Check balance form submission
    if (balanceForm) {
        balanceForm.addEventListener('submit', handleCheckBalance);
    }
    
    // Refresh buttons
    if (refreshAccountsBtn) {
        refreshAccountsBtn.addEventListener('click', loadAccounts);
    }
    
    if (refreshTransactionsBtn) {
        refreshTransactionsBtn.addEventListener('click', loadTransactions);
    }
}

// Show status message
function showStatus(message, type = 'info') {
    statusMessage.innerHTML = `
        <div class="alert alert-${type} alert-dismissible fade show" role="alert">
            ${message}
            <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
        </div>
    `;
    
    // Auto-hide after 5 seconds
    setTimeout(() => {
        const alert = statusMessage.querySelector('.alert');
        if (alert) {
            alert.classList.remove('show');
            setTimeout(() => {
                statusMessage.innerHTML = '';
            }, 150);
        }
    }, 5000);
}

// Handle API errors
function handleApiError(error) {
    console.error('API Error:', error);
    showStatus('An error occurred. Please try again later.', 'danger');
}

// Make API request
async function makeApiRequest(data) {
    try {
        const response = await fetch(API_URL, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data)
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        return await response.json();
    } catch (error) {
        handleApiError(error);
        throw error;
    }
}

// Handle create account form submission
async function handleCreateAccount(event) {
    event.preventDefault();
    
    const name = document.getElementById('customerName').value.trim();
    const initialBalance = parseFloat(document.getElementById('initialBalance').value);
    
    try {
        const data = await makeApiRequest({
            action: 'create_account',
            name: name,
            initial_balance: initialBalance
        });
        
        if (data.status === 'success') {
            showStatus(`Account created successfully! Account #${data.account_number}`, 'success');
            createAccountForm.reset();
            loadAccounts();
        } else {
            showStatus('Failed to create account. Please try again.', 'danger');
        }
    } catch (error) {
        // Error handled in makeApiRequest
    }
}

// Handle transaction form submission
async function handleTransaction(event) {
    event.preventDefault();
    
    const accountNumber = parseInt(document.getElementById('accountNumber').value);
    const transactionType = document.getElementById('transactionType').value;
    const amount = parseFloat(document.getElementById('amount').value);
    
    if (isNaN(accountNumber) || isNaN(amount) || amount <= 0) {
        showStatus('Please enter valid values', 'warning');
        return;
    }
    
    try {
        const data = await makeApiRequest({
            action: transactionType,
            account_number: accountNumber,
            amount: amount
        });
        
        if (data.status === 'success') {
            const action = transactionType === 'deposit' ? 'deposited to' : 'withdrawn from';
            showStatus(`Successfully ${action} account #${accountNumber}. New balance: ₹${data.new_balance.toFixed(2)}`, 'success');
            transactionForm.reset();
            loadAccounts();
            loadTransactions();
            
            // Update balance display if this is the current account
            const currentAccount = document.getElementById('checkAccountNumber');
            if (currentAccount && parseInt(currentAccount.value) === accountNumber) {
                updateBalanceDisplay(accountNumber, data.new_balance);
            }
        } else {
            showStatus('Transaction failed. Please check the account number and try again.', 'danger');
        }
    } catch (error) {
        // Error handled in makeApiRequest
    }
}

// Handle check balance form submission
async function handleCheckBalance(event) {
    event.preventDefault();
    
    const accountNumber = parseInt(document.getElementById('checkAccountNumber').value);
    
    if (isNaN(accountNumber)) {
        showStatus('Please enter a valid account number', 'warning');
        return;
    }
    
    try {
        const data = await makeApiRequest({
            action: 'get_balance',
            account_number: accountNumber
        });
        
        if (data.status === 'success') {
            updateBalanceDisplay(accountNumber, data.balance);
        } else {
            balanceResult.innerHTML = '<div class="alert alert-warning">Account not found</div>';
        }
    } catch (error) {
        // Error handled in makeApiRequest
    }
}

// Update balance display
function updateBalanceDisplay(accountNumber, balance) {
    balanceResult.innerHTML = `
        <div class="text-center">
            <div class="text-muted small">Account #${accountNumber}</div>
            <div class="display-6 fw-bold text-primary">₹${balance.toFixed(2)}</div>
            <div class="text-muted small">Current Balance</div>
        </div>
    `;
}

// Load accounts into the table
async function loadAccounts() {
    try {
        const data = await makeApiRequest({
            action: 'get_accounts'
        });

        if (data.status === 'success') {
            const accounts = (data.accounts || []).map(account => ({
                account_number: account.account_number,
                customer_name: account.customer_name,
                balance: account.balance
            })).sort((a, b) => a.account_number - b.account_number);

            updateAccountsTable(accounts);
        }
    } catch (error) {
        // Error handled in makeApiRequest
    }
}

// Update accounts table with data
function updateAccountsTable(accounts) {
    if (!accountsTableBody) return;
    
    if (accounts.length === 0) {
        accountsTableBody.innerHTML = `
            <tr>
                <td colspan="3" class="text-center text-muted">No accounts found</td>
            </tr>
        `;
        return;
    }
    
    accountsTableBody.innerHTML = accounts.map(account => `
        <tr>
            <td>${account.account_number}</td>
            <td>${account.customer_name || 'N/A'}</td>
            <td class="fw-bold ${account.balance >= 0 ? 'text-success' : 'text-danger'}">
                ₹${Math.abs(account.balance).toFixed(2)} ${account.balance < 0 ? '(Overdrawn)' : ''}
            </td>
        </tr>
    `).join('');
}

// Load transactions into the table
async function loadTransactions() {
    try {
        const data = await makeApiRequest({
            action: 'get_transactions'
        });
        
        if (data.status === 'success') {
            updateTransactionsTable(data.transactions || []);
        }
    } catch (error) {
        // Error handled in makeApiRequest
    }
}

// Update transactions table with data
function updateTransactionsTable(transactions) {
    if (!transactionsTableBody) return;
    
    if (transactions.length === 0) {
        transactionsTableBody.innerHTML = `
            <tr>
                <td colspan="4" class="text-center text-muted">No transactions found</td>
            </tr>
        `;
        return;
    }
    
    // Sort transactions by timestamp (newest first)
    transactions.sort((a, b) => 
        new Date(b.timestamp) - new Date(a.timestamp)
    );
    
    transactionsTableBody.innerHTML = transactions.map(tx => `
        <tr>
            <td>${formatDate(tx.timestamp)}</td>
            <td>${tx.account_number}</td>
            <td>${tx.type}</td>
            <td class="fw-bold ${getTransactionClass(tx.type)}">
                ${formatTransactionAmount(tx.type, tx.amount)}
            </td>
        </tr>
    `).join('');
}

function getTransactionClass(type) {
    return type === 'Withdrawal' ? 'text-danger' : 'text-success';
}

function formatTransactionAmount(type, amount) {
    const prefix = type === 'Withdrawal' ? '-' : '+';
    return `${prefix}₹${amount.toFixed(2)}`;
}

// Format date for display
function formatDate(timestamp) {
    if (!timestamp) return 'N/A';
    
    const date = new Date(timestamp);
    if (isNaN(date.getTime())) return 'Invalid Date';
    
    return date.toLocaleString('en-US', {
        year: 'numeric',
        month: 'short',
        day: 'numeric',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    });
}
