# Hathor Custom Token

## Overview

The hathor custom token serialized

## Structure

### Token

| Field | Size (bytes) | Description |
| --- | :---: | --- |
| `version` | 1 | Custom token version byte |
| `uid` | 32 | UID, the transaction id that created this token |
| `symbol_len` | 1 | Length of the token symbol |
| `symbol` | `symbol_len` | Token symbol |
| `name_len` | 1 | Length of the token name |
| `name` | `name_len` | Token name |

Obs: We only allow ascii symbol and name due to a limitation on the Ledger display

The symbol and name are UTF-8 encoded but since we only allow the ascii printable each character only has 1 byte.
Since the symbol max length of characters is 5 it's max length in bytes will also be 5.
Similarly the name will have a max length of 30 bytes.

### Signature

The token signature is a `sha256` digest of the token data described above with a salt.
Since it's a `sha256` the length is fixed at 32 bytes.
