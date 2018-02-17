#!/bin/sh

docker build -t pg_libphonenumber_test -f docker/Dockerfile .
docker run -it --mount type=tmpfs,destination=/var/lib/postgresql/data pg_libphonenumber_test
