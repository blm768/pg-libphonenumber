# pg-libphonenumber
A (partially implemented!) PostgreSQL extension that provides access to [Google's libphonenumber](https://github.com/googlei18n/libphonenumber)

## Project status

This extension is in an <strong>ALPHA</strong> state. Do not use it in production environments.

## Synopsis


    CREATE EXTENSION pg_libphonenumber;
    SELECT parse_phone_number('03 7010 1234', 'AU');
    SELECT parse_phone_number('2819010011', 'US');
    
    CREATE TABLE foo ( ph phone_number );
  
    -- DO NOT RELY ON THIS
    -- may not always this work way with implicit cast.
    SELECT '2819010011'::phone_number; -- assumes US   

## Installation

### Debian/Ubuntu

First you need install `libphonenumber-dev` and the corresponding `postgresql-server-dev` package.

    sudo apt-get install build-essential
    sudo apt-get install postgresql-server-dev-9.5
    sudo apt-get install libphonenumber-dev

Then clone this repostiroy

    cd /tmp
    git clone https://github.com/blm768/pg-libphonenumber
    cd pg-libphonenumber
    make
    sudo make install
