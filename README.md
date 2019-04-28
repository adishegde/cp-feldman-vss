# Foundations of Cryptography Project - 2019
Implementing a Verifiable Secret Sharing Scheme (VSS Scheme).

# Libraries Used
- **ntl** - For Finite Field Arithmetic
- **openssl** - For SHA256 Hash Function

# Installation
`ntl` and `openssl` have to be installed on the system manually.

- `openssl` can be installed on most UNIX systems through the native package
manager by installing the `openssl` package. Additionally on Mac systems run
the command `ln -s /usr/local/opt/openssl/include/openssl /usr/local/include`.
- Instructions for installing `ntl` can be found on [NTL_INSTALL](https://www.shoup.net/ntl/).

# Usage
The following make targets are available (which can be built by running
`make <target_name>`):

- **test**: This tests the core working of the VSS scheme.
