#include <exodus/exodus.h>

subroutine getinput(in prompt, io data)
{
        var text=prompt ^ " (" ^ data ^ ")";
        if (len(text)<30)
                text=oconv(text,"L#30");
        print(text," ? ");
        var result=input();
        if (result ne "")
                data=result;
}

function input_connection_config()
{

        var host="127.0.0.1";
        var port="5432";
        var user="postgres";

        do {

                //var pass="";
                var pass="";
                var yesno="Y";

                getinput("Postgres host",host);
                getinput("Postgres port",port);
                getinput("Postgres admin user",user);
                getinput("Postgres admin password",pass);

                var connection;
                connection = "host="^host;
                connection^=" port="^port;
                connection^=" user="^user;

                connection^=" dbname=template1";

                printl("\n",connection,"\n");
                connection^=" password="^pass;

                print(oconv("Attempting to connect ... ","L#33"));
                if (connect(connection)) {
                        printl("done!");
                        break;
                }

                getinput("Cannot connect. Try Again? ",yesno);
                if (yesno.ucase().substr(1,1) ne "Y")
                        return false;//stop("Cancelled.");

        } while (true);

        return true;

}

function runsomesql(in sql)
{

        var errmsg;
        if (not sql.sqlexec(errmsg)) {
                //printl("\nSQL " ^ errmsg ^ sql);
                printl("\nSQL " ^ errmsg);
                return false;
        }

        printl("done!");
        return true;

}

function add_pgexodus_postgres_plugin()
{

        var sql=
                "CREATE OR REPLACE FUNCTION exodus_call(bytea, bytea, bytea, bytea, bytea, int4, int4) RETURNS bytea AS 'pgexodus', 'exodus_call' LANGUAGE C IMMUTABLE;\n"
                "CREATE OR REPLACE FUNCTION exodus_extract_bytea(bytea, int4, int4, int4) RETURNS bytea AS 'pgexodus', 'exodus_extract_bytea' LANGUAGE C IMMUTABLE;\n"
                "CREATE OR REPLACE FUNCTION exodus_extract_text(bytea, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_text' LANGUAGE C IMMUTABLE;\n"
                "CREATE OR REPLACE FUNCTION exodus_extract_sort(bytea, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_sort' LANGUAGE C IMMUTABLE;\n"
//-- Remaining functions are STRICT therefore never get called with NULLS -- also return NULL if passed zero length strings
                "CREATE OR REPLACE FUNCTION exodus_extract_text2(bytea, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_text2' LANGUAGE C IMMUTABLE STRICT;\n"
                "CREATE OR REPLACE FUNCTION exodus_extract_date(bytea, int4, int4, int4) RETURNS date AS 'pgexodus', 'exodus_extract_date' LANGUAGE C IMMUTABLE STRICT;\n"
                "CREATE OR REPLACE FUNCTION exodus_extract_time(bytea, int4, int4, int4) RETURNS time AS 'pgexodus', 'exodus_extract_time' LANGUAGE C IMMUTABLE STRICT;\n"
                "CREATE OR REPLACE FUNCTION exodus_extract_datetime(bytea, int4, int4, int4) RETURNS timestamp AS 'pgexodus', 'exodus_extract_datetime' LANGUAGE C IMMUTABLE STRICT;"
                ;
        return runsomesql(sql);
}

function create_dbuser(in dbusername, in dbuserpass)
{

        var sql=
                "CREATE ROLE $dbusername$ LOGIN"
                " PASSWORD '$dbuserpass$'"
                " CREATEDB"
//"  CREATEROLE"
                ";" ;

        swapper(sql,"$dbusername$",dbusername);
        swapper(sql,"$dbuserpass$",dbuserpass);

        return runsomesql(sql);
}

function create_db(in dbname, in dbusername)
{

        var sql=
                "CREATE DATABASE $dbname$"
                " WITH ENCODING='UTF8'"
                " OWNER=$dbusername$"
                ";" ;

        swapper(sql,"$dbname$",dbname);
        swapper(sql,"$dbusername$",dbusername);

        return runsomesql(sql);
}

program()
{

        printl("-- Exodus Postgres Configuration ---");

        if (not input_connection_config())
                stop("Stopping. Cannot continue without a working connection.");

        print(oconv("Add pgexodus postgres plugin ... ","L#33"));
        if (not add_pgexodus_postgres_plugin())
                stop("Stopping. Not enough privileges");


        printl(oconv("Detaching from template1 database ... ","L#33"));
        if (not connect("dbname=postgres")) {
                stop("Stopping. Cannot connect to postgres database");
        }

//for now exodus a default database to play in with a non-secret password
        var dbusername="exodus";
        var dbname="exodus";
        var dbuserpass="somesillysecret";

        print(("Creating user "^ dbusername^ " ... ").oconv("L#33"));
        if (not create_dbuser(dbusername,dbuserpass))
                //stop();
                printl("Error: Could not create user ",dbusername);

        print(("Creating database "^ dbname^ " ... ").oconv("L#33"));
        if (not create_db(dbname,dbusername))
                //stop();
                printl("Error: Could not create database ",dbname);

}
