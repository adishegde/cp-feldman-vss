# Foundations of Cryptography Project - 2019
Implementing a Verifiable Secret Sharing Scheme (VSS Scheme).

## Disclaimer
These programs are not safe. They were implemented as a proof of concept and
should **NOT** be used in the real world. The core API (`src/vss.cpp`) however,
follows the exact description of Feldman's scheme.

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
- **init**: This generates the prime number and the generator required for running
the protocol. The data is stored in a file (default: `gen_out.dat`).
- **dealer**: Dealer program. Given a file containing the secret, it computes the
shares and distributes it to other parties. It also behaves like a normal party
and can reconstruct the secret.
- **party**: Represents individual parties. The parties can be made malicious
through command line arguments. The parties first query their share from the
dealer and can then recompute shares by interacting with any other party.
- **proto**: This target is an alias to generate the `init`, `dealer` and `party`
binaries.
