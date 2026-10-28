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

## License

This project is licensed under the GNU GPL version 3 (or later). See the
[COPYING](./COPYING) file for more details.
