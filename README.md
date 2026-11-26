# Terminal Email Client

An email client for terminal users. Currently, it supports sending emails via any IMAP/SMTP server and retrieving received emails from the server.

## Usage

Run the `email_client` and enter your required information to login to your
email account (login, password and IMAP/SMTP server addresses).

_If updating from v1.0 to v2.0, you may need to remove the configuration file at
`~/.config/email_client/config.json`_.

## Installation

### Arch

Email-Client
[is avaliable on the Arch User Repository](https://aur.archlinux.org/packages/email-client).

### Manual Build

```sh
cmake -S . -B build
cmake --build build --parallel "$(nproc)"
./build/email_client
```

The first build downloads and compiles the dependencies, which takes a few minutes.

If you have [`just`](https://just.systems) installed, you can also use the
`build`, `run`, `buildrun` and `clean` recipes from the provided `Justfile`;
`just buildrun` builds and starts the client in one command.

## Development

### Local test mail server

`dev/mail-server.sh` runs a local [GreenMail](https://greenmail-mail-test.github.io/greenmail/)
server in Docker (or Podman), so the client can be tested without a real email
account. It requires `docker` or `podman`, `openssl` and `curl`.

```sh
dev/mail-server.sh run    # start the server if needed and run ./build/email_client against it
dev/mail-server.sh send   # send a test mail from bob to alice
dev/mail-server.sh down   # stop the server
```

Run `dev/mail-server.sh` without arguments to see all commands. With `just`:
`just dev-run`, `just mail-send`, `just mail-up`, `just mail-down`.

| Account | Password | SMTP | IMAP |
|---|---|---|---|
| `alice@localhost` | `alice` | `smtps://localhost:3465` | `imaps://localhost:3993` |
| `bob@localhost` | `bob` | `smtps://localhost:3465` | `imaps://localhost:3993` |

- `run` uses a separate client config and cache in `dev/.state/`, with both
  accounts already added. Your real config is not touched.
- The server's certificate is issued by a dev CA generated in `dev/.certs/`.
  `run` passes it to the client via `SSL_CERT_FILE`, so nothing is added to the
  system trust store. Running `./build/email_client` directly uses only the
  system CAs and rejects the dev server.
- GreenMail keeps mail in memory only, so stopping the server clears all
  mailboxes.

### Tests

Unit tests use [GoogleTest](https://google.github.io/googletest/) (with GMock).
They are off by default, so a normal build does not download GoogleTest.

```sh
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --parallel "$(nproc)"
ctest --test-dir build/tests --output-on-failure
```

Or `just test`. Extra arguments go to `ctest`, e.g. `just test -R HtmlParser`
runs only the tests whose name matches `HtmlParser`.

All sources except `src/main.cpp` are built into the `email_client_core`
library, which both the client and the tests link against. CI (GitHub
Actions) builds the project and runs the tests on every pull request and on
every push to `main`.

Conventions (see `tests/backend/html_parser_test.cpp` for an example):

- Test files mirror `src/`: tests for `src/backend/html_parser.cpp` go in
  `tests/backend/html_parser_test.cpp`. Add new files to `tests/CMakeLists.txt`.
- Suite name: the tested class or file + `Test`, e.g. `HtmlParserTest`.
- Test name: what the code should do, e.g. `DecodeQuotedPrintableDecodesHexEscapes`.
- Use `EXPECT_*` by default. Use `ASSERT_*` only when the rest of the test
  makes no sense after a failure, e.g. before using an element of a vector
  whose size was just checked.
- Unit tests must not use the network or the user's config.

### Releases and notifications

Every push to `main` (i.e. every merged pull request) runs
`.github/workflows/aur-release.yml`. After the CI passes, it compares the
version with the [AUR package](https://aur.archlinux.org/packages/email-client)
and releases only if it is newer:

1. builds and installs the package from `aur/PKGBUILD` in an Arch container,
2. tags the commit with the version, e.g. `2.1.0`,
3. pushes the updated `PKGBUILD` and `.SRCINFO` to the AUR.

The version is `project(VERSION)` in `CMakeLists.txt` plus `pkgrel` in
`aur/PKGBUILD`. To release:

- new code: bump `VERSION` and set `pkgrel=1`,
- a packaging-only fix of the released version: bump `pkgrel` only (the
  existing tag is packaged again, not the new commit).

A merge without a version bump runs the CI and publishes nothing.

The team's Discord channel gets a message for every PR merged into `main`
(`.github/workflows/discord-pr-merged.yml`) and one for every release: success
when the new version is on the AUR, an error when the CI or the release failed.

Repository secrets (Settings → Secrets and variables → Actions):

| Secret | Value |
|---|---|
| `AUR_SSH_PRIVATE_KEY` | Private SSH key whose public key is on the AUR account maintaining the package |
| `DISCORD_WEBHOOK_URL` | Discord webhook URL of the channel; without it the messages are skipped |

## License

This project is licensed under the GNU GPL version 3 (or later). See the
[COPYING](./COPYING) file for more details.
