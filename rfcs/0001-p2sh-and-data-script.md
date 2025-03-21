# Summary
[summary]: #summary

Add support for P2SH scripts and data scripts on the Hathor Ledger app.

# Motivation
[motivation]: #motivation

P2SH and data scripts are used in some projects that may send large amounts of tokens by the project and by users, so we require added protection for these use-cases by allowing the users to use the Ledger device with P2SH and data scripts outputs.

# Guide-level explanation
[guide-level-explanation]: #guide-level-explanation

When parsing the transaction we will need to decode each element, the current design aims to add some versatility to this decoding.
Currently we only support P2PKH (Pay-To-PubKey-Hash) outputs and to add support for P2SH (Pay-To-Script-Hash) outputs and data output we need to change how we parse, validate and show the output to the user.

## Output scripts

An output has:
- Value: 4 or 8 bytes
- Token data: 1 byte
- Script: variable number of bytes

The script describes how the output will be spent, for instance which address owns the output or in the case of the data output, that it cannot be spent.
The information on the script will be used so the user can confirm the output or deny it, meaning some data will have to be extracted from the script and processed.

---

### P2PKH

The structure is
`OP_DUP(1b) | OP_HASH160(1b) | PUBKEY_HASH_LEN(1b) | PUBKEY_HASH(20b) | OP_EQUALVERIFY(1b) | OP_CHECKSIG(1b)`

Which amounts to 25 bytes, the `PUBKEY_HASH` is the address the output is sending tokens to.

### P2SH

The structure is
`OP_HASH160(1b) | SCRIPT_HASH_LEN(1b) | SCRIPT_HASH(20b) | OP_EQUAL(1b)`

Which amounts to 23 bytes, the `SCRIPT_HASH` is the address the output is sending tokens to.

Obs: P2PKH and P2SH also have a timelock variant which is not supported on the Ledger, but adds 6 bytes to the start of the script with the structure `4(1b) | TIMESTAMP (4b) | OP_CHECKTIMESTAMP (1b)`.

### Data script

The structure of a data script can be either:
- `OP_PUSHDATA1(1b) | DATA_LEN(1b) | DATA(var)` 
	- For data lengths of $[1, 255]$ but usually used for $(75, 255]$ due to the extra opcode.
- `DATA_LEN(1b) | DATA(var)`
	- Only for data lengths of $[1, 75]$

---

## Script Parser

The parser will be changed to identify the type of script and if it is valid extract the data to the global context.
This way of doing the extraction allows the code to read the data, validate and return an error if invalid, otherwise continue safely by copying the required data.

When identifying we should:
- Check if it is a P2PKH script
- Check if it is a P2SH script
- Check if it is a Data script
- Return as Unknown script (not allowed)

## Parsed data

Once the type of script is identified we need to extract, validate then store the data for the user to confirm.
To do this we will change the output structure to allow storing the new information on the global context.

```c

// src/transaction/types.h

/**
 * Script types
 */
typedef enum {
    SCRIPT_UNKNOWN = 0,
    SCRIPT_P2PKH = 1,
    SCRIPT_P2SH = 2,
    SCRIPT_DATA = 3,
} script_type_t;

/**
 * Output script info
 */
typedef struct {
    script_type_t type;
    uint8_t hash[PUBKEY_HASH_LEN];  // P2SH or P2PKH
    uint8_t data_index;  // Index on G_context.tx_info.output_datas 
} output_script_info_t;

/**
 * Data script type alias
 */
typedef uint8_t data_script_t[MAX_DATA_SCRIPT_LEN];

/**
 * Structure for transaction output
 */
typedef struct {
    uint8_t index;
    uint64_t value;
    uint8_t token_data;
    output_script_info_t script;
} tx_output_t;

// src/types.h

/**
 * Structure for transaction information context.
 */
typedef struct {
    // ...
    token_symbol_t* tokens[TX_MAX_TOKENS];
    uint8_t tokens_len;

    data_script_t output_datas[TX_MAX_DATA_OUTPUTS];
    uint8_t output_datas_len;
} sign_tx_ctx_t;
```

To avoid provisioning more data than necessary the data script bytes will be saved on an array on the global context and the output will only have the index to its data.
This way we can have different values for the max number of outputs and max number of data outputs.

> [!IMPORTANT]
> For Ledger Nano S:
> - MAX_ALLOWED_DATA_OUTPUTS = 1
> - MAX_DATA_SCRIPT_LEN = 53 // Data will go up to 50 bytes
> 
> For other devices:
> - MAX_ALLOWED_DATA_OUTPUTS = 4
> - MAX_DATA_SCRIPT_LEN = 153 // Data will go up to 150 bytes

## UI Interaction
### P2SH addresses

This can be done by creating a method to convert script hash to base58 address, which requires the network's p2sh version byte, then display normally on the address step.

```c
/**
 * Convert script hash160 to base58 address.
 *
 * address = version byte + script_hash + first 4 bytes of sha256d(hash)
 *
 * @param[in]  script_hash
 *   Script hash160
 *   An array of at least 20 bytes (uint8_t).
 * @param[in]  script_hash_len
 *   Length of script_hash buffer
 * @param[out] out
 *   Pointer to output byte buffer for address.
 * @param[in]  outlen
 *   Length of output byte buffer.
 * @return 0 on success
 *
 */
int address_from_script_hash(const uint8_t *script_hash,
                             size_t script_hash_len,
                             uint8_t *out,
                             size_t outlen);

```

### Data script outputs

The current UI step for output confirmation has the steps:
- Output number
- Show Address
- Show HTR amount

For an user to confirm a data output we need to replace the address step with a step to confirm the data on the output (ASCII encoded).

We do not require an amount step since the amount of a data output is always 0.01 HTR. The protocol allows for other custom tokens but the Ledger app will only allow HTR.

```c
// Address output confirmation
UX_FLOW(ux_display_tx_output_flow,
        &ux_display_review_output_step,  // Output <curr>/<total>
        &ux_display_address_step,        // address
        &ux_display_amount_step,         // HTR <value>
        &ux_display_approve_step,        // accept
        &ux_display_reject_step,         // reject
        FLOW_LOOP);

// Data output confirmation
UX_FLOW(ux_display_tx_data_output_flow,
        &ux_display_review_output_step,  // Output <curr>/<total>
        &ux_display_data_script_step,    // data from data script
        &ux_display_approve_step,        // accept
        &ux_display_reject_step,         // reject
        FLOW_LOOP);
```
