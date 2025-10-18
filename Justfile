build:
  #!/bin/sh
  mkdir -p build
  cd build
  cmake ..
  make

buildrun:
  just build
  just run

run:
  ./build/email_client

clean:
  rm -rf build

