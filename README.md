# **Http Proxy App:**

A http/https proxy server that can be used to test a web app and inspect the traffic.

## **System Design:**

Comming soon.

## **Features:**
	
Comming soon.

## **Requirements:**

- Openssl.
- Chromium.

## Generate self signed CARoot Certificate:

```bash
# Generate Key
openssl genrsa -out CyberWeb.key 2048

# Generate CARoot Cert
openssl req -x509 -sha256 -new -nodes -key CyberWeb.key -days 3650 -out CyberWeb.crt
```

## How to use (This will be updated soon to be more easy.)

Install OpenSSL for devlopement
```bash
sudo apt install openssl libssl-dev
```

Then
```bash
cmake -S src -B build
```

1- Open build directory then create hosts folder.
2- Generate the Root Certificate using the previous command line.
3- Copy those cert and key files to 'hosts' directory.

Now cd to 'build' directory if you are not in and type
Then:

```bash
make
```

then run 
Then
```bash
./CyberWeb
```

Now, Open chromium using the following command:
```bash
chromium --proxy-server=127.0.0.1:8080
```
go to chromium settings and import the RootCA cert. and now the proxy server is working.