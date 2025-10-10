#!/bin/bash

# Stop on any error
set -e

echo "🚀 Starting deployment of BankSys..."

# Navigate to project directory
cd "$(dirname "$0")"

# Build the project
echo "🔨 Building the project..."
make clean
make

# Create necessary directories
echo "📂 Setting up directories..."
sudo mkdir -p /Library/WebServer/CGI-Executables
sudo mkdir -p /Library/WebServer/Documents/bank
sudo mkdir -p /Library/WebServer/Documents/bank/data

# Deploy files
echo "📦 Deploying files..."
sudo cp bank.cgi /Library/WebServer/CGI-Executables/
sudo cp index.html styles.css app.js /Library/WebServer/Documents/bank/

# Set permissions
echo "🔒 Setting permissions..."
sudo chmod +x /Library/WebServer/CGI-Executables/bank.cgi
sudo chown -R _www:_www /Library/WebServer/Documents/bank/data
sudo chmod -R 755 /Library/WebServer/Documents/bank/data

# Start/Restart Apache
echo "🔄 Restarting Apache..."
if pgrep -x "httpd" > /dev/null; then
    sudo apachectl restart
else
    sudo apachectl start
fi

echo "✅ Deployment complete!"
echo "🌐 Access the application at: http://localhost/bank/"
echo "📝 Logs can be viewed with: sudo tail -f /var/log/apache2/error_log"