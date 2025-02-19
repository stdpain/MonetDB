/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 1997 - July 2008 CWI, August 2008 - 2022 MonetDB B.V.
 */

#ifdef _MSC_VER
#include <WTypes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**** Define the ODBC Version this ODBC driver complies with ****/
/* also see ODBCGlobal.h */
#define ODBCVER 0x0352		/* Important: this must be defined before include of sqlext.h */

#include <sql.h>
#include <sqlext.h>

static void
prerr(SQLSMALLINT tpe, SQLHANDLE hnd, const char *func, const char *pref)
{
	SQLCHAR state[6];
	SQLINTEGER errnr;
	SQLCHAR msg[256];
	SQLSMALLINT msglen;

	switch (SQLGetDiagRec(tpe, hnd, 1, state, &errnr, msg, sizeof(msg), &msglen)) {
	case SQL_SUCCESS_WITH_INFO:
		if (msglen >= (signed int) sizeof(msg))
			fprintf(stderr, "(message truncated)\n");
		/* fall through */
	case SQL_SUCCESS:
		fprintf(stderr, "%s: %s: SQLstate %s, Errnr %d, Message %s\n", func, pref, (char*)state, (int)errnr, (char*)msg);
		break;
	case SQL_INVALID_HANDLE:
		fprintf(stderr, "%s: %s, invalid handle passed to error function\n", func, pref);
		break;
	case SQL_ERROR:
		fprintf(stderr, "%s: %s, unexpected error from SQLGetDiagRec\n", func, pref);
		break;
	case SQL_NO_DATA:
		break;
	default:
		fprintf(stderr, "%s: %s, weird return value from SQLGetDiagRec\n", func, pref);
		break;
	}
}

static bool
check(SQLRETURN ret, SQLSMALLINT tpe, SQLHANDLE hnd, const char *func)
{
	switch (ret) {
	case SQL_SUCCESS:
		return true;
	case SQL_SUCCESS_WITH_INFO:
		prerr(tpe, hnd, func, "Info");
		return true;
	case SQL_ERROR:
		prerr(tpe, hnd, func, "Error");
		break;
	case SQL_INVALID_HANDLE:
		fprintf(stderr, "%s: Error: invalid handle\n", func);
		exit(1);
	default:
		fprintf(stderr, "%s: Unexpected return value\n", func);
		break;
	}
	return false;
}

const struct {
	SQLUSMALLINT info;
	enum {
		STRING,
		INTEGER,
		SHORTINT,
	} type;
	char *name;
	union {
		char *str;
		SQLUSMALLINT s;
		SQLUINTEGER i;
	};
} sqlinfo[] = {
	{
		.info = SQL_ACCESSIBLE_PROCEDURES,
		.name = "SQL_ACCESSIBLE_PROCEDURES",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_ACCESSIBLE_TABLES,
		.name = "SQL_ACCESSIBLE_TABLES",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_AGGREGATE_FUNCTIONS,
		.name = "SQL_AGGREGATE_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_AF_ALL | SQL_AF_AVG | SQL_AF_COUNT | SQL_AF_DISTINCT | SQL_AF_MAX | SQL_AF_MIN | SQL_AF_SUM,
	},
	{
		.info = SQL_ALTER_DOMAIN,
		.name = "SQL_ALTER_DOMAIN",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_ALTER_TABLE,
		.name = "SQL_ALTER_TABLE",
		.type = INTEGER,
		.i = SQL_AT_ADD_COLUMN | SQL_AT_ADD_COLUMN_DEFAULT | SQL_AT_ADD_COLUMN_SINGLE | SQL_AT_ADD_CONSTRAINT | SQL_AT_ADD_TABLE_CONSTRAINT | SQL_AT_CONSTRAINT_NAME_DEFINITION | SQL_AT_DROP_COLUMN | SQL_AT_DROP_COLUMN_CASCADE | SQL_AT_DROP_COLUMN_DEFAULT | SQL_AT_DROP_COLUMN_RESTRICT | SQL_AT_DROP_TABLE_CONSTRAINT_CASCADE | SQL_AT_DROP_TABLE_CONSTRAINT_RESTRICT | SQL_AT_SET_COLUMN_DEFAULT,
	},
#ifdef SQL_ASYNC_DBC_FUNCTIONS
	{
		.info = SQL_ASYNC_DBC_FUNCTIONS,
		.name = "SQL_ASYNC_DBC_FUNCTIONS",
		.type = INTEGER,
		.i = 0,
	},
#endif
	{
		.info = SQL_ASYNC_MODE,
		.name = "SQL_ASYNC_MODE",
		.type = INTEGER,
		.i = SQL_AM_NONE,
	},
#ifdef SQL_ASYNC_NOTIFICATION
	{
		.info = SQL_ASYNC_NOTIFICATION,
		.name = "SQL_ASYNC_NOTIFICATION",
		.type = INTEGER,
		.i = 0,
	},
#endif
	{
		.info = SQL_BATCH_ROW_COUNT,
		.name = "SQL_BATCH_ROW_COUNT",
		.type = INTEGER,
		.i = SQL_BRC_EXPLICIT,
	},
	{
		.info = SQL_BATCH_SUPPORT,
		.name = "SQL_BATCH_SUPPORT",
		.type = INTEGER,
		.i = SQL_BS_SELECT_EXPLICIT | SQL_BS_ROW_COUNT_EXPLICIT,
	},
	{
		.info = SQL_BOOKMARK_PERSISTENCE,
		.name = "SQL_BOOKMARK_PERSISTENCE",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_CATALOG_LOCATION,
		.name = "SQL_CATALOG_LOCATION",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_CATALOG_NAME,
		.name = "SQL_CATALOG_NAME",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_CATALOG_NAME_SEPARATOR,
		.name = "SQL_CATALOG_NAME_SEPARATOR",
		.type = STRING,
		.str = "",
	},
	{
		.info = SQL_CATALOG_TERM,
		.name = "SQL_CATALOG_TERM",
		.type = STRING,
		.str = "",
	},
	{
		.info = SQL_CATALOG_USAGE,
		.name = "SQL_CATALOG_USAGE",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_COLLATION_SEQ,
		.name = "SQL_COLLATION_SEQ",
		.type = STRING,
		.str = "UTF-8",
	},
	{
		.info = SQL_COLUMN_ALIAS,
		.name = "SQL_COLUMN_ALIAS",
		.type = STRING,
		.str = "Y",
	},
	{
		.info = SQL_CONCAT_NULL_BEHAVIOR,
		.name = "SQL_CONCAT_NULL_BEHAVIOR",
		.type = SHORTINT,
		.s = SQL_CB_NULL,
	},
	{
		.info = SQL_CONVERT_BIGINT,
		.name = "SQL_CONVERT_BIGINT",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_BIT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_BINARY,
		.name = "SQL_CONVERT_BINARY",
		.type = INTEGER,
		.i = SQL_CVT_BINARY | SQL_CVT_CHAR | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR | SQL_CVT_VARBINARY | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_BIT,
		.name = "SQL_CONVERT_BIT",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_BIT | SQL_CVT_CHAR | SQL_CVT_INTEGER | SQL_CVT_LONGVARCHAR | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_CHAR,
		.name = "SQL_CONVERT_CHAR",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_BINARY | SQL_CVT_BIT | SQL_CVT_GUID | SQL_CVT_CHAR | SQL_CVT_DATE | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TIME | SQL_CVT_TIMESTAMP | SQL_CVT_TINYINT | SQL_CVT_VARBINARY | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_DATE,
		.name = "SQL_CONVERT_DATE",
		.type = INTEGER,
		.i = SQL_CVT_CHAR | SQL_CVT_DATE | SQL_CVT_LONGVARCHAR | SQL_CVT_TIMESTAMP | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_DECIMAL,
		.name = "SQL_CONVERT_DECIMAL",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_DOUBLE,
		.name = "SQL_CONVERT_DOUBLE",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_FLOAT,
		.name = "SQL_CONVERT_FLOAT",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_FUNCTIONS,
		.name = "SQL_CONVERT_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_FN_CVT_CAST | SQL_FN_CVT_CONVERT,
	},
	{
		.info = SQL_CONVERT_INTEGER,
		.name = "SQL_CONVERT_INTEGER",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_BIT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_INTERVAL_DAY_TIME,
		.name = "SQL_CONVERT_INTERVAL_DAY_TIME",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_CHAR | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARCHAR | SQL_CVT_SMALLINT | SQL_CVT_TIME | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_INTERVAL_YEAR_MONTH,
		.name = "SQL_CONVERT_INTERVAL_YEAR_MONTH",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_CHAR | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_LONGVARCHAR | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_LONGVARBINARY,
		.name = "SQL_CONVERT_LONGVARBINARY",
		.type = INTEGER,
		.i = SQL_CVT_BINARY | SQL_CVT_CHAR | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR | SQL_CVT_VARBINARY | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_LONGVARCHAR,
		.name = "SQL_CONVERT_LONGVARCHAR",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_BINARY | SQL_CVT_BIT | SQL_CVT_GUID | SQL_CVT_CHAR | SQL_CVT_DATE | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TIME | SQL_CVT_TIMESTAMP | SQL_CVT_TINYINT | SQL_CVT_VARBINARY | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_NUMERIC,
		.name = "SQL_CONVERT_NUMERIC",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_REAL,
		.name = "SQL_CONVERT_REAL",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_SMALLINT,
		.name = "SQL_CONVERT_SMALLINT",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_BIT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_TIME,
		.name = "SQL_CONVERT_TIME",
		.type = INTEGER,
		.i = SQL_CVT_CHAR | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARCHAR | SQL_CVT_TIME | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_TIMESTAMP,
		.name = "SQL_CONVERT_TIMESTAMP",
		.type = INTEGER,
		.i = SQL_CVT_CHAR | SQL_CVT_DATE | SQL_CVT_LONGVARCHAR | SQL_CVT_TIME | SQL_CVT_TIMESTAMP | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_TINYINT,
		.name = "SQL_CONVERT_TINYINT",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_BIT | SQL_CVT_CHAR | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TINYINT | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_VARBINARY,
		.name = "SQL_CONVERT_VARBINARY",
		.type = INTEGER,
		.i = SQL_CVT_BINARY | SQL_CVT_CHAR | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR | SQL_CVT_VARBINARY | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CONVERT_VARCHAR,
		.name = "SQL_CONVERT_VARCHAR",
		.type = INTEGER,
		.i = SQL_CVT_BIGINT | SQL_CVT_BINARY | SQL_CVT_BIT | SQL_CVT_GUID | SQL_CVT_CHAR | SQL_CVT_DATE | SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT | SQL_CVT_INTEGER | SQL_CVT_INTERVAL_YEAR_MONTH | SQL_CVT_INTERVAL_DAY_TIME | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC | SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TIME | SQL_CVT_TIMESTAMP | SQL_CVT_TINYINT | SQL_CVT_VARBINARY | SQL_CVT_VARCHAR,
	},
	{
		.info = SQL_CORRELATION_NAME,
		.name = "SQL_CORRELATION_NAME",
		.type = SHORTINT,
		.s = SQL_CN_ANY,
	},
	{
		.info = SQL_CREATE_ASSERTION,
		.name = "SQL_CREATE_ASSERTION",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_CREATE_CHARACTER_SET,
		.name = "SQL_CREATE_CHARACTER_SET",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_CREATE_COLLATION,
		.name = "SQL_CREATE_COLLATION",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_CREATE_DOMAIN,
		.name = "SQL_CREATE_DOMAIN",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_CREATE_SCHEMA,
		.name = "SQL_CREATE_SCHEMA",
		.type = INTEGER,
		.i = SQL_CS_CREATE_SCHEMA | SQL_CS_AUTHORIZATION,
	},
	{
		.info = SQL_CREATE_TABLE,
		.name = "SQL_CREATE_TABLE",
		.type = INTEGER,
		.i = SQL_CT_CREATE_TABLE | SQL_CT_TABLE_CONSTRAINT | SQL_CT_CONSTRAINT_NAME_DEFINITION | SQL_CT_COMMIT_PRESERVE | SQL_CT_COMMIT_DELETE | SQL_CT_GLOBAL_TEMPORARY | SQL_CT_LOCAL_TEMPORARY | SQL_CT_COLUMN_CONSTRAINT | SQL_CT_COLUMN_DEFAULT,
	},
	{
		.info = SQL_CREATE_TRANSLATION,
		.name = "SQL_CREATE_TRANSLATION",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_CREATE_VIEW,
		.name = "SQL_CREATE_VIEW",
		.type = INTEGER,
		.i = SQL_CV_CREATE_VIEW | SQL_CV_CHECK_OPTION,
	},
	{
		.info = SQL_CURSOR_COMMIT_BEHAVIOR,
		.name = "SQL_CURSOR_COMMIT_BEHAVIOR",
		.type = SHORTINT,
		.s = SQL_CB_DELETE,
	},
	{
		.info = SQL_CURSOR_ROLLBACK_BEHAVIOR,
		.name = "SQL_CURSOR_ROLLBACK_BEHAVIOR",
		.type = SHORTINT,
		.s = SQL_CB_DELETE,
	},
	{
		.info = SQL_CURSOR_SENSITIVITY,
		.name = "SQL_CURSOR_SENSITIVITY",
		.type = INTEGER,
		.i = SQL_INSENSITIVE,
	},
	{
		.info = SQL_DATA_SOURCE_NAME,
		.name = "SQL_DATA_SOURCE_NAME",
		.type = STRING,
		.str = "MonetDB-Test",
	},
	{
		.info = SQL_DATA_SOURCE_READ_ONLY,
		.name = "SQL_DATA_SOURCE_READ_ONLY",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_DATABASE_NAME,
		.name = "SQL_DATABASE_NAME",
		.type = STRING,
		.str = "mTests_sql_odbc_samples",
	},
	{
		.info = SQL_DATETIME_LITERALS,
		.name = "SQL_DATETIME_LITERALS",
		.type = INTEGER,
		.i = SQL_DL_SQL92_DATE | SQL_DL_SQL92_TIME | SQL_DL_SQL92_TIMESTAMP | SQL_DL_SQL92_INTERVAL_YEAR | SQL_DL_SQL92_INTERVAL_MONTH | SQL_DL_SQL92_INTERVAL_DAY | SQL_DL_SQL92_INTERVAL_HOUR | SQL_DL_SQL92_INTERVAL_MINUTE | SQL_DL_SQL92_INTERVAL_SECOND | SQL_DL_SQL92_INTERVAL_YEAR_TO_MONTH | SQL_DL_SQL92_INTERVAL_DAY_TO_HOUR | SQL_DL_SQL92_INTERVAL_DAY_TO_MINUTE | SQL_DL_SQL92_INTERVAL_DAY_TO_SECOND | SQL_DL_SQL92_INTERVAL_HOUR_TO_MINUTE | SQL_DL_SQL92_INTERVAL_HOUR_TO_SECOND | SQL_DL_SQL92_INTERVAL_MINUTE_TO_SECOND,
	},
	{
		.info = SQL_DBMS_NAME,
		.name = "SQL_DBMS_NAME",
		.type = STRING,
		.str = "MonetDB",
	},
	{
		.info = SQL_DBMS_VER,
		.name = "SQL_DBMS_VER",
		.type = STRING,
		.str = NULL,
	},
	{
		.info = SQL_DDL_INDEX,
		.name = "SQL_DDL_INDEX",
		.type = INTEGER,
		.i = 3,
	},
	{
		.info = SQL_DEFAULT_TXN_ISOLATION,
		.name = "SQL_DEFAULT_TXN_ISOLATION",
		.type = INTEGER,
		.i = 8,
	},
	{
		.info = SQL_DESCRIBE_PARAMETER,
		.name = "SQL_DESCRIBE_PARAMETER",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_DM_VER,
		.name = "SQL_DM_VER",
		.type = STRING,
		.str = "03.52.0002.0003",
	},
#ifdef SQL_DRIVER_AWARE_POOLING_SUPPORTED
	{
		.info = SQL_DRIVER_AWARE_POOLING_SUPPORTED,
		.name = "SQL_DRIVER_AWARE_POOLING_SUPPORTED",
		.type = INTEGER,
		.i = 0,
	},
#endif
	{
		.info = SQL_DRIVER_NAME,
		.name = "SQL_DRIVER_NAME",
		.type = STRING,
		.str = "MonetDBODBClib",
	},
	{
		.info = SQL_DRIVER_ODBC_VER,
		.name = "SQL_DRIVER_ODBC_VER",
		.type = STRING,
		.str = "03.52",
	},
	{
		.info = SQL_DRIVER_VER,
		.name = "SQL_DRIVER_VER",
		.type = STRING,
		.str = NULL,
	},
	{
		.info = SQL_DROP_ASSERTION,
		.name = "SQL_DROP_ASSERTION",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_DROP_CHARACTER_SET,
		.name = "SQL_DROP_CHARACTER_SET",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_DROP_COLLATION,
		.name = "SQL_DROP_COLLATION",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_DROP_DOMAIN,
		.name = "SQL_DROP_DOMAIN",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_DROP_SCHEMA,
		.name = "SQL_DROP_SCHEMA",
		.type = INTEGER,
		.i = SQL_DS_DROP_SCHEMA | SQL_DS_CASCADE | SQL_DS_RESTRICT,
	},
	{
		.info = SQL_DROP_TABLE,
		.name = "SQL_DROP_TABLE",
		.type = INTEGER,
		.i = SQL_DT_DROP_TABLE | SQL_DT_CASCADE | SQL_DT_RESTRICT,
	},
	{
		.info = SQL_DROP_TRANSLATION,
		.name = "SQL_DROP_TRANSLATION",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_DROP_VIEW,
		.name = "SQL_DROP_VIEW",
		.type = INTEGER,
		.i = SQL_DV_DROP_VIEW | SQL_DV_CASCADE | SQL_DV_RESTRICT,
	},
	{
		.info = SQL_DYNAMIC_CURSOR_ATTRIBUTES1,
		.name = "SQL_DYNAMIC_CURSOR_ATTRIBUTES1",
		.type = INTEGER,
		.i = SQL_CA1_NEXT | SQL_CA1_ABSOLUTE | SQL_CA1_RELATIVE,
	},
	{
		.info = SQL_DYNAMIC_CURSOR_ATTRIBUTES2,
		.name = "SQL_DYNAMIC_CURSOR_ATTRIBUTES2",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_EXPRESSIONS_IN_ORDERBY,
		.name = "SQL_EXPRESSIONS_IN_ORDERBY",
		.type = STRING,
		.str = "Y",
	},
	{
		.info = SQL_FILE_USAGE,
		.name = "SQL_FILE_USAGE",
		.type = SHORTINT,
		.s = SQL_FILE_NOT_SUPPORTED,
	},
	{
		.info = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1,
		.name = "SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1",
		.type = INTEGER,
		.i = SQL_CA1_NEXT,
	},
	{
		.info = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2,
		.name = "SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_GETDATA_EXTENSIONS,
		.name = "SQL_GETDATA_EXTENSIONS",
		.type = INTEGER,
		.i = SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER | SQL_GD_BLOCK | SQL_GD_BOUND,
	},
	{
		.info = SQL_GROUP_BY,
		.name = "SQL_GROUP_BY",
		.type = SHORTINT,
		.s = SQL_GB_NO_RELATION,
	},
	{
		.info = SQL_IDENTIFIER_CASE,
		.name = "SQL_IDENTIFIER_CASE",
		.type = SHORTINT,
		.s = SQL_IC_LOWER,
	},
	{
		.info = SQL_IDENTIFIER_QUOTE_CHAR,
		.name = "SQL_IDENTIFIER_QUOTE_CHAR",
		.type = STRING,
		.str = "\"",
	},
	{
		.info = SQL_INDEX_KEYWORDS,
		.name = "SQL_INDEX_KEYWORDS",
		.type = INTEGER,
		.i = SQL_IK_NONE,
	},
	{
		.info = SQL_INFO_SCHEMA_VIEWS,
		.name = "SQL_INFO_SCHEMA_VIEWS",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_INSERT_STATEMENT,
		.name = "SQL_INSERT_STATEMENT",
		.type = INTEGER,
		.i = SQL_IS_INSERT_LITERALS | SQL_IS_INSERT_SEARCHED | SQL_IS_SELECT_INTO,
	},
	{
		.info = SQL_INTEGRITY,
		.name = "SQL_INTEGRITY",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_KEYSET_CURSOR_ATTRIBUTES1,
		.name = "SQL_KEYSET_CURSOR_ATTRIBUTES1",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_KEYSET_CURSOR_ATTRIBUTES2,
		.name = "SQL_KEYSET_CURSOR_ATTRIBUTES2",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_KEYWORDS,
		.name = "SQL_KEYWORDS",
		.type = STRING,
		.str = "ADD,ADMIN,AFTER,AGGREGATE,ALL,ALTER,ALWAYS,ANALYZE,AND,"
			"ANY,ASC,ASYMMETRIC,AT,ATOMIC,AUTHORIZATION,"
			"AUTO_INCREMENT,BEFORE,BEGIN,BEST,BETWEEN,BIG,BIGINT,"
			"BIGSERIAL,BINARY,BLOB,BY,CACHE,CALL,CASCADE,CASE,CAST,"
			"CENTURY,CHAIN,CHAR,CHARACTER,CHECK,CLIENT,CLOB,"
			"COALESCE,COLUMN,COMMENT,COMMIT,COMMITTED,CONSTRAINT,"
			"CONTINUE,CONVERT,COPY,CORRESPONDING,CREATE,CROSS,CUBE,"
			"CURRENT,CURRENT_DATE,CURRENT_ROLE,CURRENT_SCHEMA,"
			"CURRENT_TIME,CURRENT_TIMESTAMP,CURRENT_TIMEZONE,"
			"CURRENT_USER,CYCLE,DATA,DATE,DAY,DEALLOCATE,DEBUG,DEC,"
			"DECADE,DECIMAL,DECLARE,DEFAULT,DELETE,DELIMITERS,DESC,"
			"DIAGNOSTICS,DISTINCT,DO,DOUBLE,DOW,DOY,DROP,EACH,"
			"EFFORT,ELSE,ELSEIF,ENCRYPTED,END,ENDIAN,EPOCH,ESCAPE,"
			"EVERY,EXCEPT,EXCLUDE,EXEC,EXECUTE,EXISTS,EXPLAIN,"
			"EXTERNAL,EXTRACT,FALSE,FIRST,FLOAT,FOLLOWING,FOR,"
			"FOREIGN,FROM,FULL,FUNCTION,FWF,GENERATED,GLOBAL,GRANT,"
			"GROUP,GROUPING,GROUPS,HAVING,HOUR,HUGEINT,IDENTITY,IF,"
			"ILIKE,IMPRINTS,IN,INCREMENT,INDEX,INNER,INSERT,INT,"
			"INTEGER,INTERSECT,INTERVAL,INTO,IS,ISOLATION,JOIN,KEY,"
			"LANGUAGE,LARGE,LAST,LATERAL,LEFT,LEVEL,LIKE,LIMIT,"
			"LITTLE,LOADER,LOCAL,LOCALTIME,LOCALTIMESTAMP,MATCH,"
			"MATCHED,MAXVALUE,MEDIUMINT,MERGE,MINUTE,MINVALUE,"
			"MONTH,NAME,NATIVE,NATURAL,NEW,NEXT,NO,NOT,NOW,NULL,"
			"NULLIF,NULLS,NUMERIC,OBJECT,OF,OFFSET,OLD,ON,ONLY,"
			"OPTION,OPTIONS,OR,ORDER,ORDERED,OTHERS,OUTER,OVER,"
			"PARTIAL,PARTITION,PASSWORD,PATH,PLAN,POSITION,"
			"PRECEDING,PRECISION,PREP,PREPARE,PRESERVE,PRIMARY,"
			"PRIVILEGES,PROCEDURE,PUBLIC,QUARTER,RANGE,READ,REAL,"
			"RECORDS,REFERENCES,REFERENCING,RELEASE,REMOTE,RENAME,"
			"REPEATABLE,REPLACE,REPLICA,RESTART,RESTRICT,RETURN,"
			"RETURNS,REVOKE,RIGHT,ROLE,ROLLBACK,ROLLUP,ROW,ROWS,"
			"SAMPLE,SAVEPOINT,SCHEMA,SECOND,SEED,SELECT,SEQUENCE,"
			"SERIAL,SERIALIZABLE,SERVER,SESSION,SESSION_USER,SET,"
			"SETS,SIMPLE,SIZE,SMALLINT,SOME,SPLIT_PART,START,"
			"STATEMENT,STDIN,STDOUT,STORAGE,STRING,SUBSTRING,"
			"SYMMETRIC,TABLE,TEMP,TEMPORARY,TEXT,THEN,TIES,TIME,"
			"TIMESTAMP,TINYINT,TO,TRACE,TRANSACTION,TRIGGER,TRUE,"
			"TRUNCATE,TYPE,UNBOUNDED,UNCOMMITTED,UNENCRYPTED,UNION,"
			"UNIQUE,UPDATE,USER,USING,VALUE,VALUES,VARCHAR,VARYING,"
			"VIEW,WEEK,WHEN,WHERE,WHILE,WINDOW,WITH,WORK,WRITE,"
			"XMLAGG,XMLATTRIBUTES,XMLCOMMENT,XMLCONCAT,XMLDOCUMENT,"
			"XMLELEMENT,XMLFOREST,XMLNAMESPACES,XMLPARSE,XMLPI,"
			"XMLQUERY,XMLSCHEMA,XMLTEXT,XMLVALIDATE,YEAR,ZONE",
	},
	{
		.info = SQL_LIKE_ESCAPE_CLAUSE,
		.name = "SQL_LIKE_ESCAPE_CLAUSE",
		.type = STRING,
		.str = "Y",
	},
	{
		.info = SQL_MAX_ASYNC_CONCURRENT_STATEMENTS,
		.name = "SQL_MAX_ASYNC_CONCURRENT_STATEMENTS",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_MAX_BINARY_LITERAL_LEN,
		.name = "SQL_MAX_BINARY_LITERAL_LEN",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_MAX_CATALOG_NAME_LEN,
		.name = "SQL_MAX_CATALOG_NAME_LEN",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_MAX_CHAR_LITERAL_LEN,
		.name = "SQL_MAX_CHAR_LITERAL_LEN",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_MAX_COLUMN_NAME_LEN,
		.name = "SQL_MAX_COLUMN_NAME_LEN",
		.type = SHORTINT,
		.s = 1024,
	},
	{
		.info = SQL_MAX_COLUMNS_IN_GROUP_BY,
		.name = "SQL_MAX_COLUMNS_IN_GROUP_BY",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_MAX_COLUMNS_IN_INDEX,
		.name = "SQL_MAX_COLUMNS_IN_INDEX",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_MAX_COLUMNS_IN_ORDER_BY,
		.name = "SQL_MAX_COLUMNS_IN_ORDER_BY",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_MAX_COLUMNS_IN_SELECT,
		.name = "SQL_MAX_COLUMNS_IN_SELECT",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_MAX_COLUMNS_IN_TABLE,
		.name = "SQL_MAX_COLUMNS_IN_TABLE",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_MAX_CURSOR_NAME_LEN,
		.name = "SQL_MAX_CURSOR_NAME_LEN",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_MAX_DRIVER_CONNECTIONS,
		.name = "SQL_MAX_DRIVER_CONNECTIONS",
		.type = SHORTINT,
		.s = 64,
	},
	{
		.info = SQL_MAX_IDENTIFIER_LEN,
		.name = "SQL_MAX_IDENTIFIER_LEN",
		.type = SHORTINT,
		.s = 1024,
	},
	{
		.info = SQL_MAX_INDEX_SIZE,
		.name = "SQL_MAX_INDEX_SIZE",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_MAX_PROCEDURE_NAME_LEN,
		.name = "SQL_MAX_PROCEDURE_NAME_LEN",
		.type = SHORTINT,
		.s = 256,
	},
	{
		.info = SQL_MAX_ROW_SIZE,
		.name = "SQL_MAX_ROW_SIZE",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_MAX_ROW_SIZE_INCLUDES_LONG,
		.name = "SQL_MAX_ROW_SIZE_INCLUDES_LONG",
		.type = STRING,
		.str = "Y",
	},
	{
		.info = SQL_MAX_SCHEMA_NAME_LEN,
		.name = "SQL_MAX_SCHEMA_NAME_LEN",
		.type = SHORTINT,
		.s = 1024,
	},
	{
		.info = SQL_MAX_STATEMENT_LEN,
		.name = "SQL_MAX_STATEMENT_LEN",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_MAX_TABLE_NAME_LEN,
		.name = "SQL_MAX_TABLE_NAME_LEN",
		.type = SHORTINT,
		.s = 1024,
	},
	{
		.info = SQL_MAX_TABLES_IN_SELECT,
		.name = "SQL_MAX_TABLES_IN_SELECT",
		.type = SHORTINT,
		.s = 0,
	},
	{
		.info = SQL_MAX_USER_NAME_LEN,
		.name = "SQL_MAX_USER_NAME_LEN",
		.type = SHORTINT,
		.s = 1024,
	},
	{
		.info = SQL_MULTIPLE_ACTIVE_TXN,
		.name = "SQL_MULTIPLE_ACTIVE_TXN",
		.type = STRING,
		.str = "Y",
	},
	{
		.info = SQL_MULT_RESULT_SETS,
		.name = "SQL_MULT_RESULT_SETS",
		.type = STRING,
		.str = "Y",
	},
	{
		.info = SQL_NEED_LONG_DATA_LEN,
		.name = "SQL_NEED_LONG_DATA_LEN",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_NON_NULLABLE_COLUMNS,
		.name = "SQL_NON_NULLABLE_COLUMNS",
		.type = SHORTINT,
		.s = SQL_NNC_NON_NULL,
	},
	{
		.info = SQL_NULL_COLLATION,
		.name = "SQL_NULL_COLLATION",
		.type = SHORTINT,
		.s = SQL_NC_LOW,
	},
	{
		.info = SQL_NUMERIC_FUNCTIONS,
		.name = "SQL_NUMERIC_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_FN_NUM_ABS | SQL_FN_NUM_ACOS | SQL_FN_NUM_ASIN | SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2 | SQL_FN_NUM_CEILING | SQL_FN_NUM_COS | SQL_FN_NUM_COT | SQL_FN_NUM_DEGREES | SQL_FN_NUM_EXP | SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_LOG10 | SQL_FN_NUM_MOD | SQL_FN_NUM_PI | SQL_FN_NUM_POWER | SQL_FN_NUM_RADIANS | SQL_FN_NUM_RAND | SQL_FN_NUM_ROUND | SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT | SQL_FN_NUM_TAN | SQL_FN_NUM_TRUNCATE,
	},
	{
		.info = SQL_ODBC_INTERFACE_CONFORMANCE,
		.name = "SQL_ODBC_INTERFACE_CONFORMANCE",
		.type = INTEGER,
		.i = SQL_OIC_CORE,
	},
	{
		.info = SQL_ODBC_VER,
		.name = "SQL_ODBC_VER",
		.type = STRING,
		.str = "03.52",
	},
	{
		.info = SQL_OJ_CAPABILITIES,
		.name = "SQL_OJ_CAPABILITIES",
		.type = INTEGER,
		.i = SQL_OJ_LEFT | SQL_OJ_RIGHT | SQL_OJ_FULL | SQL_OJ_NESTED | SQL_OJ_NOT_ORDERED | SQL_OJ_INNER | SQL_OJ_ALL_COMPARISON_OPS,
	},
	{
		.info = SQL_ORDER_BY_COLUMNS_IN_SELECT,
		.name = "SQL_ORDER_BY_COLUMNS_IN_SELECT",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_PARAM_ARRAY_ROW_COUNTS,
		.name = "SQL_PARAM_ARRAY_ROW_COUNTS",
		.type = INTEGER,
		.i = SQL_PARC_BATCH,
	},
	{
		.info = SQL_POS_OPERATIONS,
		.name = "SQL_POS_OPERATIONS",
		.type = INTEGER,
		.i = SQL_POS_POSITION,
	},
	{
		.info = SQL_PROCEDURES,
		.name = "SQL_PROCEDURES",
		.type = STRING,
		.str = "Y",
	},
	{
		.info = SQL_PROCEDURE_TERM,
		.name = "SQL_PROCEDURE_TERM",
		.type = STRING,
		.str = "procedure",
	},
	{
		.info = SQL_QUOTED_IDENTIFIER_CASE,
		.name = "SQL_QUOTED_IDENTIFIER_CASE",
		.type = SHORTINT,
		.s = SQL_IC_SENSITIVE,
	},
	{
		.info = SQL_ROW_UPDATES,
		.name = "SQL_ROW_UPDATES",
		.type = STRING,
		.str = "N",
	},
	{
		.info = SQL_SCHEMA_TERM,
		.name = "SQL_SCHEMA_TERM",
		.type = STRING,
		.str = "schema",
	},
	{
		.info = SQL_SCHEMA_USAGE,
		.name = "SQL_SCHEMA_USAGE",
		.type = INTEGER,
		.i = SQL_SU_DML_STATEMENTS | SQL_SU_PROCEDURE_INVOCATION | SQL_SU_TABLE_DEFINITION | SQL_SU_INDEX_DEFINITION | SQL_SU_PRIVILEGE_DEFINITION,
	},
	{
		.info = SQL_SCROLL_OPTIONS,
		.name = "SQL_SCROLL_OPTIONS",
		.type = INTEGER,
		.i = SQL_SO_STATIC,
	},
	{
		.info = SQL_SEARCH_PATTERN_ESCAPE,
		.name = "SQL_SEARCH_PATTERN_ESCAPE",
		.type = STRING,
		.str = "\\",
	},
	{
		.info = SQL_SERVER_NAME,
		.name = "SQL_SERVER_NAME",
		.type = STRING,
		.str = "MonetDB",
	},
	{
		.info = SQL_SPECIAL_CHARACTERS,
		.name = "SQL_SPECIAL_CHARACTERS",
		.type = STRING,
		.str = "!$&'()*+,-./:;<=>?@[]^`{|}~",
	},
	{
		.info = SQL_SQL_CONFORMANCE,
		.name = "SQL_SQL_CONFORMANCE",
		.type = INTEGER,
		.i = SQL_SC_SQL92_FULL,
	},
	{
		.info = SQL_SQL92_DATETIME_FUNCTIONS,
		.name = "SQL_SQL92_DATETIME_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_SDF_CURRENT_DATE | SQL_SDF_CURRENT_TIME | SQL_SDF_CURRENT_TIMESTAMP,
	},
	{
		.info = SQL_SQL92_FOREIGN_KEY_DELETE_RULE,
		.name = "SQL_SQL92_FOREIGN_KEY_DELETE_RULE",
		.type = INTEGER,
		.i = SQL_SFKD_CASCADE | SQL_SFKD_NO_ACTION | SQL_SFKD_SET_DEFAULT | SQL_SFKD_SET_NULL,
	},
	{
		.info = SQL_SQL92_FOREIGN_KEY_UPDATE_RULE,
		.name = "SQL_SQL92_FOREIGN_KEY_UPDATE_RULE",
		.type = INTEGER,
		.i = SQL_SFKU_CASCADE | SQL_SFKU_NO_ACTION | SQL_SFKU_SET_DEFAULT | SQL_SFKU_SET_NULL,
	},
	{
		.info = SQL_SQL92_GRANT,
		.name = "SQL_SQL92_GRANT",
		.type = INTEGER,
		.i = SQL_SG_DELETE_TABLE | SQL_SG_INSERT_COLUMN | SQL_SG_INSERT_TABLE | SQL_SG_REFERENCES_TABLE | SQL_SG_REFERENCES_COLUMN | SQL_SG_SELECT_TABLE | SQL_SG_UPDATE_COLUMN | SQL_SG_UPDATE_TABLE | SQL_SG_WITH_GRANT_OPTION,
	},
	{
		.info = SQL_SQL92_NUMERIC_VALUE_FUNCTIONS,
		.name = "SQL_SQL92_NUMERIC_VALUE_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_SNVF_CHAR_LENGTH | SQL_SNVF_CHARACTER_LENGTH | SQL_SNVF_EXTRACT | SQL_SNVF_OCTET_LENGTH | SQL_SNVF_POSITION,
	},
	{
		.info = SQL_SQL92_PREDICATES,
		.name = "SQL_SQL92_PREDICATES",
		.type = INTEGER,
		.i = SQL_SP_BETWEEN | SQL_SP_COMPARISON | SQL_SP_EXISTS | SQL_SP_IN | SQL_SP_ISNOTNULL | SQL_SP_ISNULL | SQL_SP_LIKE | SQL_SP_MATCH_FULL | SQL_SP_MATCH_PARTIAL | SQL_SP_MATCH_UNIQUE_FULL | SQL_SP_MATCH_UNIQUE_PARTIAL | SQL_SP_OVERLAPS | SQL_SP_QUANTIFIED_COMPARISON | SQL_SP_UNIQUE,
	},
	{
		.info = SQL_SQL92_RELATIONAL_JOIN_OPERATORS,
		.name = "SQL_SQL92_RELATIONAL_JOIN_OPERATORS",
		.type = INTEGER,
		.i = SQL_SRJO_CORRESPONDING_CLAUSE | SQL_SRJO_CROSS_JOIN | SQL_SRJO_EXCEPT_JOIN | SQL_SRJO_FULL_OUTER_JOIN | SQL_SRJO_INNER_JOIN | SQL_SRJO_INTERSECT_JOIN | SQL_SRJO_LEFT_OUTER_JOIN | SQL_SRJO_NATURAL_JOIN | SQL_SRJO_RIGHT_OUTER_JOIN | SQL_SRJO_UNION_JOIN,
	},
	{
		.info = SQL_SQL92_REVOKE,
		.name = "SQL_SQL92_REVOKE",
		.type = INTEGER,
		.i = SQL_SR_DELETE_TABLE | SQL_SR_GRANT_OPTION_FOR | SQL_SR_INSERT_COLUMN | SQL_SR_INSERT_TABLE | SQL_SR_REFERENCES_COLUMN | SQL_SR_REFERENCES_TABLE | SQL_SR_SELECT_TABLE | SQL_SR_UPDATE_COLUMN | SQL_SR_UPDATE_TABLE,
	},
	{
		.info = SQL_SQL92_ROW_VALUE_CONSTRUCTOR,
		.name = "SQL_SQL92_ROW_VALUE_CONSTRUCTOR",
		.type = INTEGER,
		.i = SQL_SRVC_VALUE_EXPRESSION | SQL_SRVC_NULL | SQL_SRVC_DEFAULT | SQL_SRVC_ROW_SUBQUERY,
	},
	{
		.info = SQL_SQL92_STRING_FUNCTIONS,
		.name = "SQL_SQL92_STRING_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_SSF_CONVERT | SQL_SSF_LOWER | SQL_SSF_UPPER | SQL_SSF_SUBSTRING | SQL_SSF_TRIM_BOTH | SQL_SSF_TRIM_LEADING | SQL_SSF_TRIM_TRAILING,
	},
	{
		.info = SQL_SQL92_VALUE_EXPRESSIONS,
		.name = "SQL_SQL92_VALUE_EXPRESSIONS",
		.type = INTEGER,
		.i = SQL_SVE_CASE | SQL_SVE_CAST | SQL_SVE_COALESCE | SQL_SVE_NULLIF,
	},
	{
		.info = SQL_STANDARD_CLI_CONFORMANCE,
		.name = "SQL_STANDARD_CLI_CONFORMANCE",
		.type = INTEGER,
		.i = SQL_SCC_XOPEN_CLI_VERSION1 | SQL_SCC_ISO92_CLI,
	},
	{
		.info = SQL_STATIC_CURSOR_ATTRIBUTES1,
		.name = "SQL_STATIC_CURSOR_ATTRIBUTES1",
		.type = INTEGER,
		.i = SQL_CA1_NEXT | SQL_CA1_ABSOLUTE | SQL_CA1_RELATIVE,
	},
	{
		.info = SQL_STATIC_CURSOR_ATTRIBUTES2,
		.name = "SQL_STATIC_CURSOR_ATTRIBUTES2",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_STRING_FUNCTIONS,
		.name = "SQL_STRING_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_FN_STR_ASCII | SQL_FN_STR_CHAR | SQL_FN_STR_CHARACTER_LENGTH | SQL_FN_STR_CHAR_LENGTH | SQL_FN_STR_CONCAT | SQL_FN_STR_DIFFERENCE | SQL_FN_STR_INSERT | SQL_FN_STR_LCASE | SQL_FN_STR_LEFT | SQL_FN_STR_LENGTH | SQL_FN_STR_LOCATE | SQL_FN_STR_LOCATE_2 | SQL_FN_STR_LTRIM | SQL_FN_STR_OCTET_LENGTH | SQL_FN_STR_POSITION | SQL_FN_STR_REPEAT | SQL_FN_STR_REPLACE | SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM | SQL_FN_STR_SOUNDEX | SQL_FN_STR_SPACE | SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE,
	},
	{
		.info = SQL_SUBQUERIES,
		.name = "SQL_SUBQUERIES",
		.type = INTEGER,
		.i = SQL_SQ_CORRELATED_SUBQUERIES | SQL_SQ_COMPARISON | SQL_SQ_EXISTS | SQL_SQ_IN | SQL_SQ_QUANTIFIED,
	},
	{
		.info = SQL_SYSTEM_FUNCTIONS,
		.name = "SQL_SYSTEM_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_FN_SYS_DBNAME | SQL_FN_SYS_IFNULL | SQL_FN_SYS_USERNAME,
	},
	{
		.info = SQL_TABLE_TERM,
		.name = "SQL_TABLE_TERM",
		.type = STRING,
		.str = "table",
	},
	{
		.info = SQL_TIMEDATE_ADD_INTERVALS,
		.name = "SQL_TIMEDATE_ADD_INTERVALS",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_TIMEDATE_DIFF_INTERVALS,
		.name = "SQL_TIMEDATE_DIFF_INTERVALS",
		.type = INTEGER,
		.i = 0,
	},
	{
		.info = SQL_TIMEDATE_FUNCTIONS,
		.name = "SQL_TIMEDATE_FUNCTIONS",
		.type = INTEGER,
		.i = SQL_FN_TD_CURRENT_DATE | SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_CURRENT_TIMESTAMP | SQL_FN_TD_CURDATE | SQL_FN_TD_CURTIME | SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK | SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_EXTRACT | SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE | SQL_FN_TD_MONTH | SQL_FN_TD_NOW | SQL_FN_TD_QUARTER | SQL_FN_TD_SECOND | SQL_FN_TD_WEEK | SQL_FN_TD_YEAR,
	},
	{
		.info = SQL_TXN_CAPABLE,
		.name = "SQL_TXN_CAPABLE",
		.type = SHORTINT,
		.s = SQL_TC_ALL,
	},
	{
		.info = SQL_TXN_ISOLATION_OPTION,
		.name = "SQL_TXN_ISOLATION_OPTION",
		.type = INTEGER,
		.i = SQL_TXN_SERIALIZABLE,
	},
	{
		.info = SQL_UNION,
		.name = "SQL_UNION",
		.type = INTEGER,
		.i = SQL_U_UNION | SQL_U_UNION_ALL,
	},
	{
		.info = SQL_USER_NAME,
		.name = "SQL_USER_NAME",
		.type = STRING,
		.str = "monetdb",
	},
	{
		.info = SQL_XOPEN_CLI_YEAR,
		.name = "SQL_XOPEN_CLI_YEAR",
		.type = STRING,
		.str = "1995",
	},
};

int
main(int argc, char **argv)
{
	SQLHANDLE env;
	SQLHANDLE dbc;
	char *dsn = "MonetDB";
	char *user = "monetdb";
	char *pass = "monetdb";
	SQLRETURN ret;
	char str[4096];
	SQLSMALLINT resultlen;
	SQLUSMALLINT s;
	SQLUINTEGER i;

	if (argc > 1)
		dsn = argv[1];
	if (argc > 2)
		user = argv[2];
	if (argc > 3)
		pass = argv[3];
	if (argc > 4 || *dsn == '-') {
		fprintf(stderr, "Usage: %s [datasource [user [password]]]\n", argv[0]);
		exit(1);
	}

	if (SQLAllocHandle(SQL_HANDLE_ENV, NULL, &env) != SQL_SUCCESS) {
		fprintf(stderr, "Cannot allocate ODBC environment handle\n");
		exit(1);
	}

	ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) (uintptr_t) SQL_OV_ODBC3, 0);
	check(ret, SQL_HANDLE_ENV, env, "SQLSetEnvAttr");

	ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
	check(ret, SQL_HANDLE_ENV, env, "SQLAllocHandle 1");

	ret = SQLConnect(dbc, (SQLCHAR *) dsn, SQL_NTS, (SQLCHAR *) user, SQL_NTS, (SQLCHAR *) pass, SQL_NTS);
	if (!check(ret, SQL_HANDLE_DBC, dbc, "SQLConnect"))
		exit(1);

	for (size_t n = 0; n < sizeof(sqlinfo) / sizeof(sqlinfo[0]); n++) {
		switch (sqlinfo[n].type) {
		case STRING:
			ret = SQLGetInfo(dbc, sqlinfo[n].info, str, sizeof(str), &resultlen);
			if (check(ret, SQL_HANDLE_DBC, dbc, "SQLGetInfo") &&
			    sqlinfo[n].str &&
			    strcmp(sqlinfo[n].str, str) != 0)
				fprintf(stderr, "%s: expected: %s, received: %s\n", sqlinfo[n].name, sqlinfo[n].str, str);
			break;
		case INTEGER:
			ret = SQLGetInfo(dbc, sqlinfo[n].info, &i, sizeof(i), &resultlen);
			if (check(ret, SQL_HANDLE_DBC, dbc, "SQLGetInfo") &&
			    sqlinfo[n].i != i)
				fprintf(stderr, "%s: expected: %u, received: %u\n", sqlinfo[n].name, (unsigned int) sqlinfo[n].i, (unsigned int) i);
			break;
		default:	/* case SHORTINT */
			ret = SQLGetInfo(dbc, sqlinfo[n].info, &s, sizeof(s), &resultlen);
			if (check(ret, SQL_HANDLE_DBC, dbc, "SQLGetInfo") &&
			    sqlinfo[n].s != s)
				fprintf(stderr, "%s: expected: %u, received: %u\n", sqlinfo[n].name, (unsigned int) sqlinfo[n].s, (unsigned int) s);
			break;
		}
	}

	ret = SQLDisconnect(dbc);
	check(ret, SQL_HANDLE_DBC, dbc, "SQLDisconnect");

	ret = SQLFreeHandle(SQL_HANDLE_DBC, dbc);
	check(ret, SQL_HANDLE_DBC, dbc, "SQLFreeHandle (DBC)");

	ret = SQLFreeHandle(SQL_HANDLE_ENV, env);
	check(ret, SQL_HANDLE_ENV, env, "SQLFreeHandle (ENV)");

	return 0;
}
