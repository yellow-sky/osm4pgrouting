/***************************************************************************
 *   Copyright (C) 2014 by Nikulin Evgeniy   								   *
 *   nikulin.e@gmail.com   											   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef WAYSPLITTER_H
#define WAYSPLITTER_H
#include "libpq-fe.h"
#include "stdafx.h"



/**
 * This class connects to a postgresql database. For using this class,
 * you also need to install postgis and pgrouting
 */

class WaySplitter
{
public:
	/**
	 * Constructor
	 * @param host Host address of the database
	 * @param user a user, who has write access to the database
	 * @param dbname name of the database
	 *
	 */
 	WaySplitter(std::string host, std::string user, std::string dbname, std::string port, std::string password, std::string tables_prefix);
 	
 	/**
 	 * Destructor
 	 * closes the connection to the database
 	 */
 	~WaySplitter();

 	//! connects to database
 	int connect();
 	
	//split ways in DB
	void splitWays(long chunkSize = 100000);
	

private:
	PGconn *mycon;
	std::string conninf;
	std::string tables_prefix;
	
	void createCountColumn();
	void fillPointsCount();
	void createPointsIndex();
	long getWaysCount();

};

#endif
