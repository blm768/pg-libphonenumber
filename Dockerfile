# TODO: make version variable?
FROM postgres:9.6

RUN apt-get update && apt-get install -y \
    build-essential \
    postgresql-server-dev-9.6 \
    libphonenumber6 \
    sudo

CMD ["/data/run-tests.sh"]

COPY ./ /data/
RUN chown -R postgres:postgres /data/

# Patch the entrypoint script so it always initializes the DB.
RUN patch /usr/local/bin/docker-entrypoint.sh < /data/docker-entrypoint.patch

WORKDIR /data
RUN make && make install
