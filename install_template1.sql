-- SQL script to setup the template1 database - requires superuser

--	extension 'pgexodus'
--	collation 'exodus_natural'
--	extension 'unaccent' and function 'immutable_unaccent'
--	schema 'dict'
--	table 'lists'
--	table 'dict.voc'

	\set ON_ERROR_STOP on
	\set VERBOSITY verbose

-- database template1 - requires superuser

--	\connect template1;

-- extension 'pgexodus' - requires superuser

	-- DROP EXTENSION IF EXISTS pgexodus;
	CREATE EXTENSION IF NOT EXISTS pgexodus;

-- collation 'exodus_natural'

	-- DROP COLLATION IF EXISTS exodus_natural;
	CREATE COLLATION IF NOT EXISTS exodus_natural (provider = icu, locale = 'en@colNumeric=yes', DETERMINISTIC = false);
	ALTER COLLATION exodus_natural OWNER TO exodus;

-- extension 'unaccent' and function 'immutable_unaccent'

	-- DROP EXTENSION IF EXISTS unaccent;
	CREATE EXTENSION IF NOT EXISTS unaccent;
	CREATE OR REPLACE FUNCTION public.immutable_unaccent(text) RETURNS text AS
		$$
			SELECT public.unaccent('public.unaccent', $1)
			-- schema-qualify function and dictionary
		$$  LANGUAGE sql IMMUTABLE PARALLEL SAFE STRICT
	;
	ALTER FUNCTION public.immutable_unaccent(text) OWNER TO exodus;

-- schema 'dict'

	-- DROP SCHEMA IF EXISTS dict;
	CREATE SCHEMA IF NOT EXISTS dict AUTHORIZATION exodus;
	ALTER SCHEMA dict OWNER TO exodus;

-- table 'lists'

    CREATE TABLE IF NOT EXISTS lists (key TEXT PRIMARY KEY, data TEXT);
	ALTER TABLE lists OWNER TO exodus;

-- table 'dict.voc'

    CREATE TABLE IF NOT EXISTS dict.voc (key TEXT PRIMARY KEY, data TEXT);
	ALTER TABLE dict.voc OWNER TO exodus;
