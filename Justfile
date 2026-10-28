build:
  cmake -S . -B build
  cmake --build build --parallel {{num_cpus()}}

buildrun:
  just build
  just run

run:
  ./build/email_client

clean:
  rm -rf build

