-- Complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_libphonenumber" to load this file. \quit

--
-- Phone number type
--

CREATE TYPE phone_number;

CREATE FUNCTION phone_number_in(cstring) RETURNS phone_number
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_in';

CREATE FUNCTION phone_number_out(phone_number) RETURNS cstring
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_out';

-- TODO: send/recv
--CREATE FUNCTION phone_number_recv(internal) RETURNS phone_number
--    LANGUAGE c IMMUTABLE STRICT
--    AS 'pg_libphonenumber', 'phone_number_recv';
--
--CREATE FUNCTION phone_number_send(phone_number) RETURNS bytea
--    LANGUAGE c IMMUTABLE STRICT
--    AS 'pg_libphonenumber', 'phone_number_send';

-- TODO: make TOAST-able.
CREATE TYPE phone_number (
    INTERNALLENGTH = VARIABLE,
    INPUT = phone_number_in,
    OUTPUT = phone_number_out,
--    RECEIVE = phone_number_recv,
--    SEND = phone_number_send,
    ALIGNMENT = int4,
    STORAGE = plain
);

-- Constructors

CREATE FUNCTION parse_phone_number(text, text) RETURNS phone_number
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'parse_phone_number';

-- Casts

CREATE CAST (phone_number AS text)
    WITH INOUT;

-- Operators and indexing

CREATE FUNCTION phone_number_equal(phone_number, phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_equal';

CREATE OPERATOR = (
    leftarg = phone_number,
    rightarg = phone_number,
    procedure = phone_number_equal,
    commutator = =,
    negator = <>,
    restrict = eqsel,
    join = eqjoinsel,
    hashes = true,
    merges = true
);

CREATE FUNCTION phone_number_not_equal(phone_number, phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_not_equal';

CREATE OPERATOR <> (
    leftarg = phone_number,
    rightarg = phone_number,
    procedure = phone_number_not_equal,
    commutator = <>,
    negator = =,
    restrict = neqsel,
    join = neqjoinsel
);

CREATE FUNCTION phone_number_less(phone_number, phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_less';

CREATE OPERATOR < (
    leftarg = phone_number,
    rightarg = phone_number,
    procedure = phone_number_less,
    commutator = >,
    negator = >=,
    restrict = scalarltsel,
    join = scalarltjoinsel
);

CREATE FUNCTION phone_number_less_or_equal(phone_number, phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_less_or_equal';

CREATE OPERATOR <= (
    leftarg = phone_number,
    rightarg = phone_number,
    procedure = phone_number_less_or_equal,
    commutator = >=,
    negator = >,
    restrict = scalarltsel,
    join = scalarltjoinsel
);

CREATE FUNCTION phone_number_greater(phone_number, phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_greater';

CREATE OPERATOR > (
    leftarg = phone_number,
    rightarg = phone_number,
    procedure = phone_number_greater,
    commutator = >,
    negator = <=,
    restrict = scalargtsel,
    join = scalargtjoinsel
);

CREATE FUNCTION phone_number_greater_or_equal(phone_number, phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_greater_or_equal';

CREATE OPERATOR >= (
    leftarg = phone_number,
    rightarg = phone_number,
    procedure = phone_number_greater_or_equal,
    commutator = >=,
    negator = <,
    restrict = scalargtsel,
    join = scalargtjoinsel
);

CREATE FUNCTION phone_number_cmp(phone_number, phone_number) RETURNS integer
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'phone_number_cmp';

CREATE OPERATOR CLASS phone_number_ops
    DEFAULT FOR TYPE phone_number USING btree AS
        OPERATOR 1 <,
        OPERATOR 2 <=,
        OPERATOR 3 =,
        OPERATOR 4 >=,
        OPERATOR 5 >,
        FUNCTION 1 phone_number_cmp(phone_number, phone_number);

--
-- Packed number type
--

CREATE TYPE packed_phone_number;

CREATE FUNCTION packed_phone_number_in(cstring) RETURNS packed_phone_number
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_in';

CREATE FUNCTION packed_phone_number_out(packed_phone_number) RETURNS cstring
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_out';

CREATE FUNCTION packed_phone_number_recv(internal) RETURNS packed_phone_number
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_recv';

CREATE FUNCTION packed_phone_number_send(packed_phone_number) RETURNS bytea
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_send';

CREATE TYPE packed_phone_number (
    INTERNALLENGTH = 8,
    INPUT = packed_phone_number_in,
    OUTPUT = packed_phone_number_out,
    RECEIVE = packed_phone_number_recv,
    SEND = packed_phone_number_send,
    ALIGNMENT = double,
    STORAGE = plain
);

-- Constructors

CREATE FUNCTION parse_packed_phone_number(text, text) RETURNS packed_phone_number
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'parse_packed_phone_number';

-- Casts

CREATE CAST (packed_phone_number AS text)
    WITH INOUT;

-- Operators and indexing

CREATE FUNCTION packed_phone_number_equal(packed_phone_number, packed_phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_equal';

CREATE OPERATOR = (
    leftarg = packed_phone_number,
    rightarg = packed_phone_number,
    procedure = packed_phone_number_equal,
    commutator = =,
    negator = <>,
    restrict = eqsel,
    join = eqjoinsel,
    hashes = true,
    merges = true
);

CREATE FUNCTION packed_phone_number_not_equal(packed_phone_number, packed_phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_not_equal';

CREATE OPERATOR <> (
    leftarg = packed_phone_number,
    rightarg = packed_phone_number,
    procedure = packed_phone_number_not_equal,
    commutator = <>,
    negator = =,
    restrict = neqsel,
    join = neqjoinsel
);

CREATE FUNCTION packed_phone_number_less(packed_phone_number, packed_phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_less';

CREATE OPERATOR < (
    leftarg = packed_phone_number,
    rightarg = packed_phone_number,
    procedure = packed_phone_number_less,
    commutator = >,
    negator = >=,
    restrict = scalarltsel,
    join = scalarltjoinsel
);

CREATE FUNCTION packed_phone_number_less_or_equal(packed_phone_number, packed_phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_less_or_equal';

CREATE OPERATOR <= (
    leftarg = packed_phone_number,
    rightarg = packed_phone_number,
    procedure = packed_phone_number_less_or_equal,
    commutator = >=,
    negator = >,
    restrict = scalarltsel,
    join = scalarltjoinsel
);

CREATE FUNCTION packed_phone_number_greater(packed_phone_number, packed_phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_greater';

CREATE OPERATOR > (
    leftarg = packed_phone_number,
    rightarg = packed_phone_number,
    procedure = packed_phone_number_greater,
    commutator = >,
    negator = <=,
    restrict = scalargtsel,
    join = scalargtjoinsel
);

CREATE FUNCTION packed_phone_number_greater_or_equal(packed_phone_number, packed_phone_number) RETURNS bool
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_greater_or_equal';

CREATE OPERATOR >= (
    leftarg = packed_phone_number,
    rightarg = packed_phone_number,
    procedure = packed_phone_number_greater_or_equal,
    commutator = >=,
    negator = <,
    restrict = scalargtsel,
    join = scalargtjoinsel
);

CREATE FUNCTION packed_phone_number_cmp(packed_phone_number, packed_phone_number) RETURNS integer
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_cmp';

CREATE OPERATOR CLASS packed_phone_number_ops
    DEFAULT FOR TYPE packed_phone_number USING btree AS
        OPERATOR 1 <,
        OPERATOR 2 <=,
        OPERATOR 3 =,
        OPERATOR 4 >=,
        OPERATOR 5 >,
        FUNCTION 1 packed_phone_number_cmp(packed_phone_number, packed_phone_number);

--
-- General functions
--

CREATE FUNCTION phone_number_country_code(packed_phone_number) RETURNS integer
    LANGUAGE c IMMUTABLE STRICT
    AS 'pg_libphonenumber', 'packed_phone_number_country_code';
