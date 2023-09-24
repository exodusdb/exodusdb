-- SQL script to setup exodus user login and database for general operations

--	*** NOTE *** 'template1' database MUST be configured first,
--	otherwise exodus database will be missing its configuration.

--	user 'exodus' - granted login with password and createdb, createrole permissions

--	database 'exodus' - created as a copy of template1

--	Superuser rights are probably required to alter the exodus role/user.

	\set ON_ERROR_STOP on
	\set VERBOSITY verbose

-- user 'exodus'

	-- The role was created without login permission when creating the extension
	-- since its objects are assigned to exodus.
	--
	-- Allow login with password and creation of users and databases
	-- but not superuser
	ALTER ROLE exodus
		LOGIN
		PASSWORD 'somesillysecret'
		CREATEDB
		CREATEROLE
	;

-- database 'EXODUS'

	-- CREATE DATABASE EXODUS OWNER exodus;
	-- Hack using dblink to avoid error if database already exists
	CREATE EXTENSION IF NOT EXISTS dblink;
	DO $$
		BEGIN
			PERFORM dblink_exec('', 'CREATE DATABASE EXODUS OWNER exodus');
			EXCEPTION WHEN duplicate_database THEN RAISE NOTICE '%, skipping', SQLERRM USING ERRCODE = SQLSTATE;
		END
	$$;
