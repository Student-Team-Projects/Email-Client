# Terminal Email Client

An email client for terminal users.

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

#### 1. Create a build directory for the C++ project

`mkdir build && cd build`

#### 2. Run CMake to configure the project

`cmake ..`

#### 3. Build the C++ project

`cmake --build .`

#### 4. Run the C++ email client application

`./email_client`

If you have [`just`](https://just.systems) installed, you can also use the
`build`, `run`, `buildrun` and `clean` recipes from the provided `Justfile`.

## License

This project is licensed under the GNU GPL version 3 (or later). See the
[COPYING](./COPYING) file for more details.
