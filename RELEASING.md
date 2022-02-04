# Releasing

1. Update Makefile `APPVERSION`
1. Update the version test on `tests/test_version_cmd.py`
1. Update `CHANGELOG.md` with the new features
1. After merging on master make a release on github and tag it with the version number.
1. Create a PR to update [Ledger's fork](https://github.com/LedgerHQ/app-hathor/) of the app.
    - Create a PR with this [link](https://github.com/LedgerHQ/app-hathor/compare/master...HathorNetwork:master)
