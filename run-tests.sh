#!/bin/sh
PGDATA=/var/lib/postgresql/data
# TODO: don't hard-code version here.
pg_ctl=/usr/lib/postgresql/9.6/bin/pg_ctl
gosu postgres $pg_ctl "--pgdata=$PGDATA" start -w
gosu postgres make installcheck
gosu postgres $pg_ctl "--pgdata=$PGDATA" stop

