/**----------------------------------------------------------------------------
 * branch_log_data.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:3:21 15:52 created
**---------------------------------------------------------------------------*/
#pragma once

#define LBR_TABLE		"lbr"
#define MODULE_TABLE	"module"

#define CREATE_LBR_TABLE \
	"create table lbr ( "\
	"	idx               INTEGER PRIMARY KEY AUTOINCREMENT, "\
	"   tid               INTEGER NOT NULL, "\
	"	first_chance      CHAR NOT NULL DEFAULT 'y',  "\
	"	exception_code    INTEGER NOT NULL,  "\
	"	exception_flag    INTEGER NOT NULL, "\
	"	exception_addr    TEXT NOT NULL, "\
	"	number_parameters INTEGER NOT NULL, "\
	"	exception_info_1  INTEGER, "\
	"	exception_info_2  INTEGER, "\
	"	exception_info_3  INTEGER, "\
	"	exception_info_4  INTEGER, "\
	"	buffer			  TEXT "\
	"	) "

#define INSERT_LBR \
	"INSERT INTO lbr  "\
	"( "\
	"    tid, "\
	"    first_chance, "\
	"    exception_code, "\
	"    exception_flag, "\
	"    exception_addr, "\
	"    number_parameters, "\
	"    exception_info_1, "\
	"    exception_info_2, "\
	"    exception_info_3, "\
	"    exception_info_4, "\
	"    buffer "\
	")  "\
	"VALUES "\
	"( "\
	"    %u, "\
	"    '%c',  "\
	"    %u,  "\
	"    %u, "\
	"    '%I64u', "\
	"    %u, "\
	"    %u, "\
	"    %u, "\
	"    %u, "\
	"    %u, "\
	"    '%s' "\
	") "

#define CREATE_MODULE_TABLE \
	"create table module ( "\
	"  idx			INTEGER PRIMARY KEY AUTOINCREMENT, "\
	"  path			TEXT NOT NULL, "\
	"  base_addr    TEXT NOT NULL, "\
	"  size         INTEGER NOT NULL "\
	"	)"

#define INSERT_MODULE \
	"INSERT INTO module (path, base_addr, size) VALUES ('%s', '%I64u', '%u') "

#define SELECT_MODULE \
	"SELECT path, base_addr, size FROM module ORDER BY idx"

