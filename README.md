# Terminal Email Client
An email client for terminal users. \
Logging only into gmail accounts using [application passwords](https://support.google.com/mail/answer/185833?hl=en) is supported for now. 

# Arch
Email-Client [is now on AUR](https://aur.archlinux.org/packages/email-client)!

# Manual Build
1. Create a build directory for the C++ project  
`mkdir build && cd build`

2. Run CMake to configure the project  
`cmake ..`

3. Build the C++ project  
`cmake --build .`

4. Run the C++ email client application  
./email_client


If you have [`just`](https://just.systems) installed, you can also use the
`build`, `run`, `buildrun` and `clean` recipes from the provided `Justfile`.
