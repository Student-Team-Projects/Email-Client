# Terminal Email Client
An email client for terminal users. \
Logging only into gmail accounts using [application passwords](https://support.google.com/mail/answer/185833?hl=en) is supported for now. 

# Arch
Email-Client [is now on AUR](https://aur.archlinux.org/packages/email-client)!

# Manual Build

#### 0. Initialize submodules
If this is your first clone of the repository (or after pulling new submodules):

`git submodule update --init --recursive`

#### 1. Create a build directory for the C++ project  
`mkdir build && cd build`

#### 2. Run CMake to configure the project  
`cmake ..`

#### 3. Build the C++ project  
`cmake --build .`

#### 4. Run the C++ email client application  
./email_client


If you have [`just`](https://just.systems) installed, you can also use the
`build`, `run`, `buildrun` and `clean` recipes from the provided `Justfile`.

# System Requirements
Make sure the following tools are installed on your system:

`sudo apt update`

`sudo apt install build-essential cmake git libncursesw5-dev libgpm-dev`