[discord.logo]: images/logos/discord.png (Discord)
[email.logo]: images/logos/email.png (Discord)
[telegram.logo]: images/logos/telegram.png (Telegram)

[email.link]: mailto:contact@hathor.network
[discord.link]: https://discord.gg/aWukZZX
[telegram.link]: https://t.me/HathorOfficial

# Tutorial Wallet Desktop with Ledger

## Overview

We will teach how to connect a Ledger device device with Hathor wallet and how to operate the main features of the wallet with the Ledger device.

### Table of contents

- [Introduction](#introduction)
- [How to connect the wallet with your Ledger](#connect)
- [View transaction history](#view-transaction-history)
- [View transaction info](#view-transaction-info)
- [Receive HTR tokens](#receive-htr-tokens)
- [Send HTR tokens](#send-htr-tokens)

## Introduction

The Hathor app allows users to send and receive tokens on Hathor network, this includes HTR and any custom token or NFT on the mainnet.

### Why should you use Ledger

When sending tokens a wallet needs access to the private key to sign the transaction.
Usually the device where the wallet is installed (computers, smartphones, etc.) is made to be multipurpose and not for security.

The Ledger device is made with security as the main concern, no other software or person can access the device without physical access and knowledge of your pin code.
The private key is never loaded on your computer and all signatures can only be made with user interaction.

### Requirements

- A Ledger Nano S or Nano X with Hathor's app installed.
- The Hathor Desktop Wallet installed on your PC. (Mobile wallet is still not supported)

[Installation instructions](https://medium.com/hathor-network/hathor-is-available-on-ledger-wallet-31cb1613b060) with Ledger Live.

## Connect

First the device should be connected to the PC and then we should open the wallet (on the PC) and the Hathor app (on the Ledger device).

![Choose wallet screen](images/01-choose-type.png)

Once you get to the screen above open your Hathor app and select `Hardware wallet`.

![Second step](images/02-wait-find-device.png)

When the wallet finds the device and checks that the app is ready it will enter the second step.

The wallet will ask permission to access the wallet public key.

To confirm access, go to the `Approve` screen on the device and press with both buttons.

Once confirmed the wallet will load your addresses and transaction history.

You are now ready to send and receive HTR tokens.

## View transaction history

The `Wallet` main screen has a list of previous transactions with basic info on the transaction and a link to view transaction info.

![Wallet screen](images/03-wallet-screen.png)

Information on transaction:

- Date and time of the transaction
- Id of the transaction (with link to transaction info page)
- Type (Effect on wallet balance: send, receive or no effect)
- Value (of the effect on wallet balance)

This list paginates and will go on to the first transaction found on the wallet.

## View transaction info

![transaction info screen](images/06-tx-info-screen.png)

By clicking on the transaction id on the `View transaction History` a page with all information on the transaction opens.

Here you can check any information pertaining to the transaction.

## Receive HTR tokens

![Show address](images/04-show-full-address.png)

Your wallet is [BIP44](https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki) compliant, so it is composed of many addresses.
If you want to receive tokens you need an address:

- On the `Wallet` main screen upper right corner there is a box with the current address
- Click on "Show full address" to show the complete address, check your Ledger device
- On the Ledger device a prompt to verify the address will appear. You should verify that the address on the screen matches the address on the device.
- After checking the address you must click with both buttons on the `Approve` screen (on the Ledger device)

Any errors on the address will make you lose your tokens.
Only trust addresses that match the Ledger generated address.

The address can be used by anyone to send tokens to your wallet.
Obs: The address shown on the modal can change once it receives a transaction, but it still is part of your wallet and any tokens sent to the address will still be sent to your wallet.

## Send HTR tokens

Go to the "Send Tokens" screen and here you can enter which outputs your transaction is going to have.

![Send token screen](images/05-send-tokens.png)

Output info required:

- Destination address
- Value

You can add up to 255 outputs for any transaction.
Remember: The sum of the outputs values must be equal or less than your available balance.

The wallet can automatically select the inputs for the transaction.
Optionally, you can select them yourself, in which case you need to provide for each input:

- Transaction id
- Output index (which output from transaction id will be spent)

The transaction will not be sent if the inputs total value do not match the outputs total value.
If the inputs total value exceeds the outputs total value the wallet will generate a change output with the remainder of the tokens.
(Ledger will verify that the output belongs to your wallet if it exists, so you can focus on confirming the outputs you intend to send)

Once all inputs and outputs are setup, the wallet will send this information to the Ledger device and it will ask for your confirmation.

First you will need to confirm each output. Remember to check the address and values.
If all outputs are confirmed, the device will ask your permission to sign the transaction.
If confirmed, you will need to wait for the signing process to be done.
(You can also exit while the signing is ongoing, making the transaction fail)

The wallet will get the signature information and send the transaction, after which you will be directed to the `Wallet` screen.

Observations:
- Only HTR (Hathor native token) is supported with the Ledger app.
- Time lock is disabled for transactions with Ledger.

## Contact Info

- [![Email][email.logo] Email][email.link]
- [![Discord][discord.logo] Discord][discord.link]
- [![Telegram][telegram.logo] Telegram][telegram.link]