CREATE EXTENSION IF NOT EXISTS pg_libphonenumber;

--Test phone number parsing
select parse_phone_number('555-555-5555', 'US');
--These two should produce errors because the number is too long.
select parse_phone_number('555-555-5555555555', 'US');
select parse_phone_number('555-555-55555555555', 'US');

