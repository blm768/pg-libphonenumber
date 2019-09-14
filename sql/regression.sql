CREATE EXTENSION IF NOT EXISTS pg_libphonenumber;

-- TODO: test in/out functions.

-- Test phone number parsing
select parse_phone_number('555-555-5555', 'US');
select parse_phone_number('005-555-5555', 'US');
select parse_phone_number('555-555-5555555555', 'US');
-- Produces an error from libphonenumber
select parse_phone_number('555-555-55555555555', 'US');

-- Test operators
select parse_phone_number('555-555-5555', 'US') = parse_phone_number('555-555-5555', 'US');
select parse_phone_number('555-555-5555', 'US') <> parse_phone_number('555-055-5555', 'US');
select parse_phone_number('555-555-5555', 'US') = parse_phone_number('555-555-555', 'US');

select parse_phone_number('555-545-5555', 'US') < parse_phone_number('555-555-5555', 'US');
select parse_phone_number('555-565-5555', 'US') > parse_phone_number('555-555-5555', 'US');
select parse_phone_number('555-555-555', 'US') < parse_phone_number('555-555-5555', 'US');
select parse_phone_number('555-555-555', 'US') > parse_phone_number('555-555-5555', 'US');

-- TODO: test country codes.

-- Test packed phone number parsing
select parse_packed_phone_number('555-555-5555', 'US');
select parse_packed_phone_number('005-555-5555', 'US');
-- These two should produce errors because the number is too long.
-- Produces an error in pg-libphonenumber's code
select parse_packed_phone_number('555-555-5555555555', 'US');
-- Produces an error from libphonenumber
select parse_packed_phone_number('555-555-55555555555', 'US');

-- Do we get correct country codes?
-- TODO: expand.
select phone_number_country_code(parse_phone_number('+1-555-555-5555', 'USA'));
select phone_number_country_code(parse_packed_phone_number('+1-555-555-5555', 'USA'));
