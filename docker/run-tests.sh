#!/bin/sh

# Used in the Docker image to run tests

PGDATA=/var/lib/postgresql/data
pg_ctl=/usr/lib/postgresql/${PG_MAJOR}/bin/pg_ctl
"$pg_ctl" "--pgdata=${PGDATA}" start -w
make installcheck
"$pg_ctl" "--pgdata=${PGDATA}" stop
if [ -f regression.diffs ]; then
    cat regression.diffs
    cat regression.out
fi
