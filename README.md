# **CyberWeb - HTTP/HTTPS Proxy App**

**CyberWeb** is an HTTP/HTTPS proxy server aimed at security researchers for inspecting web traffic. It allows for the capture and analysis of HTTP and HTTPS requests and responses, making it useful for tasks such as vulnerability assessment and traffic monitoring. 

## **System Design**

*Coming soon.*

This section will include details about the architecture and design principles of the CyberWeb Proxy App, including how it handles traffic interception, SSL termination, and other core functionalities.

## **Features**

*Coming soon.*

Future updates will list all key features of CyberWeb, including but not limited to:

- Traffic interception and analysis.
- SSL/TLS decryption.
- Request/response modification.
- Logging and visualization of traffic.

## **Requirements**

To run CyberWeb, you'll need:

- **OpenSSL**: For handling secure HTTPS connections and generating certificates.
- **Chromium**: A browser that will be used to route traffic through the proxy for analysis.

Ensure both are installed on your system before proceeding with the setup.

## **Generate Self-Signed CA Root Certificate**

In order to intercept HTTPS traffic, you need to create a self-signed root certificate. Follow these steps:

1. **Generate the RSA Key:**
    ```bash
    openssl genrsa -out CyberWeb.key 2048
    ```

2. **Generate the CA Root Certificate:**
    ```bash
    openssl req -x509 -sha256 -new -nodes -key CyberWeb.key -days 3650 -out CyberWeb.crt
    ```

This will create a certificate valid for 10 years, which you'll use to sign the traffic between the proxy and the target application.

## **How to Use**

This guide will walk you through the steps to compile and run CyberWeb. Future updates will simplify the process, but for now, follow these steps:

### Step 1: Install Dependencies

1. Install OpenSSL and its development libraries:
    ```bash
    sudo apt install openssl libssl-dev
    ```

2. Make sure you have CMake installed for building the project. If not, install it:
    ```bash
    sudo apt install cmake
    ```

### Step 2: Build the Project

1. Navigate to the project’s source directory:
    ```bash
    cmake -S src -B build
    ```

2. Change to the `build` directory and create a `hosts` folder:
    ```bash
    cd build
    mkdir hosts
    ```

### Step 3: Set Up the Certificates

1. Generate the root certificate using the OpenSSL commands provided earlier.
2. Copy the generated `CyberWeb.key` and `CyberWeb.crt` files into the `hosts` directory inside `build`.

### Step 4: Compile the Project

1. Compile the source code:
    ```bash
    make
    ```

2. After the build is complete, you can run the CyberWeb proxy:
    ```bash
    ./CyberWeb
    ```

### Step 5: Configure Chromium

To route your web traffic through CyberWeb:

1. Launch Chromium with the following command to set up the proxy server:
    ```bash
    chromium --proxy-server=127.0.0.1:8080
    ```

2. Open Chromium’s settings, navigate to *Certificates*, and import the `CyberWeb.crt` certificate that you generated. This will ensure that HTTPS traffic is trusted and decrypted by the proxy server.

3. Once imported, the proxy server will be up and running, allowing you to inspect traffic between the browser and web applications.


---

### User Interface

Currently, CyberWeb uses a simple IPC (Inter-Process Communication) to redirect HTTPS traffic to a graphical user interface (GUI), which is developed in a separate project. You can find the GUI project [here](https://github.com/abdelfetah18/cyber-web-gui). This allows users to visualize the intercepted traffic through the GUI. A more detailed explanation of how IPC works and integrates with the proxy server will be provided soon.