# 0. Provide valid configuration in config.json
Provide list of recipients in 

# 1. Create a build directory for the C++ project
mkdir build && cd build

# 2. Run CMake to configure the project
cmake ..

# 3. Build the C++ project
cmake --build .

# 4. Run the C++ email client application
./my_email_app
