# 📚 reriutils - Personal C Static Library Collection

## 🌟 Project Overview

This repository contains the source code for **`libreriutils.a`**, a personal collection of static utility functions developed in C. It provides a modular set of tools covering various aspects of system programming, including network communication, file I/O, logging, configuration file parsing, and secure SSL/TLS communication using OpenSSL.

The project utilizes **Autotools (Autoconf/Automake)** to ensure high **portability** and a **flexible build environment** across different UNIX-like systems.


## 🛠️ Build and Usage Guide

This project is built using the **Autotools** suite.



### 1. Prerequisites

You will need the following tools and libraries installed on your system:

* GCC (C Compiler)
* Make
* Autoconf
* Automake
* MySQL Client Development Libraries (Optional: Required for DB functionality)
* OpenSSL Development Libraries (Optional: Required for SSL/TLS functionality)



### 2. Setting up the Build Environment

After cloning the repository, execute the provided shell script in the root directory. This script generates the necessary Autotools configuration files (`configure`).

```
./script.sh
```


### 3. Setting up the Build Environment

Run the make command to compile the source files and create the static library.
This will generate the libreriutils.a static library file in the root directory.

```
make lib
```



### 4. Running Tests
You can verify the functionality of the library modules by running the included test suite.
The test target, defined in Makefile.am, builds and executes all sample programs located under the tests subdirectory.

```
make test
```




## ⚙️ Handling External Dependencies
The project's configure.ac script manages critical external dependencies transparently:

MySQL: The configuration checks for the presence of the MySQL header (mysql/mysql.h) and the client library (libmysqlclient). It supports defining the installation path using the --with-mysql-dir option.

OpenSSL: SSL/TLS support is enabled by passing the *openssl* option to ./configure. This automatically links the required libraries (-lssl -lcrypto).

📝 Integration Guide
To integrate libreriutils.a into your own C projects:

Include Headers: Copy the necessary header files (from the include directory) into your project's include path.

Linking: Compile your application and link against the static library, ensuring you include any external dependencies required (like OpenSSL or MySQL).

