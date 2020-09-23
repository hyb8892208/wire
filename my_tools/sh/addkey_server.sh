#!/usr/bin/expect -f
## addkey.sh ipaddress 
set ipaddress  [lrange $argv 0 0]
set password  [lrange $argv 1 1]
set organization  [lrange $argv 2 2]

spawn /usr/sbin/ast_tls_cert -C $ipaddress -O "$organization" -d /etc/asterisk/tls
set timeout 30
expect "*/ca.key:*" 
send "$password\r"

expect "*/ca.key:*" 
send "$password\r"
expect "*/ca.key:*" 
send "$password\r"
expect "*/ca.key:*" 
send "$password\r"
expect eof
exit
