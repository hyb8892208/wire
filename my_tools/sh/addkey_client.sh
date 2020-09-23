#!/usr/bin/expect -f

## addkey.sh ipaddress 
set ipaddress  [lrange $argv 0 0]
set password  [lrange $argv 1 1]
set organization  [lrange $argv 2 2]
set filename  [lrange $argv 3 3]

spawn /usr/sbin/ast_tls_cert -m client -c /etc/asterisk/tls/ca.crt -k /etc/asterisk/tls/ca.key -C $ipaddress -O "$organization" -d /etc/asterisk/tls -o $filename
set timeout 30
expect "*/ca.key:*" 
send "$password\r"
expect "*/ca.key:*" 

expect eof
exit



