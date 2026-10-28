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

## License

This project is licensed under the GNU GPL version 3 (or later). See the
[COPYING](./COPYING) file for more details.
