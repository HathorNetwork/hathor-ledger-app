# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Support for authority outputs
- Support for mint/melt/delegate/destroy operations

## Changed

- Signal bits and version parsed separetely, version is now uint8_t instead of uint16_t.
- In sign tx command we check that change output index is valid (inside the output array) and not repeated.

## Fixed

- Auto confirmation of unreceived change outputs would fail tx signing
- Some parsing errors would not clean the global context, it could fail the next command.

## [1.1.1] - 2024-03-12

### Added

- CodeQL security check in CI

### Security

- Code improvements to prevent null pointer exception and buffer overflow.
- Update deprecated OS calls
- Clean dead code

## [1.1.0] - 2022-02-4

### Added

- Sign token command
- Reset all token signatures command
- Verify token signature command
- Send token command

### Changed

- Sign TX command now supports transactions with custom tokens

## [1.0.1] - 2021-09-22

### Added

- Support for Nano X

## [1.0.0] - 2021-08-02

### Added

- Inital commit with the brand new Hathor application
- Sign TX command
- Get XPUB command
- Confirm address command
- HTR + Version command



