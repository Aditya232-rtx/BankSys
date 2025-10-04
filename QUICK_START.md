# Quick Start Guide

Follow this checklist to get the banking system running on macOS with Apache CGI.

## 1. Install Prerequisites

```bash
# Install compiler and json-c
brew install gcc json-c
```

## 2. Clone and Build

```bash
git clone https://github.com/Aditya232-rtx/BankSys.git
cd BankSys
make clean && make
```

## 3. Deploy Backend (CGI)

```bash
sudo cp bank.cgi /Library/WebServer/CGI-Executables/
sudo chmod +x /Library/WebServer/CGI-Executables/bank.cgi
```

## 4. Deploy Frontend Assets

```bash
sudo mkdir -p /Library/WebServer/Documents/bank
sudo cp index.html styles.css app.js /Library/WebServer/Documents/bank/
```

## 5. Prepare Data Directory

```bash
sudo mkdir -p /Library/WebServer/Documents/bank/data
sudo chown -R _www:_www /Library/WebServer/Documents/bank/data
sudo chmod -R 755 /Library/WebServer/Documents/bank/data
```

## 6. Enable CGI in Apache (macOS default)

```bash
sudo sed -i '' 's/#LoadModule cgi_module/LoadModule cgi_module/' /etc/apache2/httpd.conf
```

## 7. Restart Apache

```bash
sudo apachectl restart
```

## 8. Verify

- Open `http://localhost/bank/`
- Create an account (note success banner for `Account #`)
- Make a deposit/withdrawal; Tables update automatically

## 9. Troubleshooting

```bash
# Watch server logs for CGI errors
sudo tail -f /var/log/apache2/error_log
```

If you update `bank.c` or `app.js`, repeat steps 2–7 to rebuild and redeploy.
