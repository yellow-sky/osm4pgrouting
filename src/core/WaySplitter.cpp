/***************************************************************************
 *   Copyright (C) 2014 by Nikulin Evgeniy   				   *
 *   nikulin.e@gmail.com   						   *
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
#include "WaySplitter.h"
#include "stdafx.h"

using namespace std;

WaySplitter::WaySplitter(std::string host, std::string user, std::string dbname, std::string port, std::string passwd, std::string tables_prefix)
:mycon(0)
{
    this->conninf="host="+host+" user="+user+" dbname="+ dbname +" port="+port;
    this->tables_prefix = tables_prefix;
    if(!passwd.empty())
        this->conninf+=" password="+passwd;
}

WaySplitter::~WaySplitter()
{
	PQfinish(mycon);
}

int WaySplitter::connect()
{
	cout << conninf<< endl;
	//mycon =PQconnectdb("user=postgres dbname=template1 hostaddr=127.0.0.1 port=5432");
	mycon =PQconnectdb(conninf.c_str());

	ConnStatusType type =PQstatus(mycon);
		if(type==CONNECTION_BAD)
		{
			cout << "connection failed"<< endl;
			return 1;
		}
		else
		{
			cout << "connection success"<< endl;
			return 0;
		}
}

void WaySplitter::splitWays(long chunkSize)
{
    //points count
    createPointsIndex();
    createCountColumn();
    fillPointsCount();
    
    //create indexes
    createWayNodesIndex();
    createWayTagsIndex();
    
    //splitt ways
    
}

long WaySplitter::getWaysCount()
{
    std::string ways_count("SELECT count(*) FROM " + tables_prefix + "temp_ways;");
    PGresult *result = PQexec(mycon, ways_count.c_str());
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "Count ways failed: " << PQerrorMessage(mycon) << std::endl;
        PQclear(result);
    } else {
	long count = atol(PQgetvalue(result, 0,0));
        std::cout << "ways_count:" <<  count <<  std::endl;
	return count;
    }
}

void WaySplitter::createPointsIndex()
{
    std::string create_index("CREATE INDEX " + tables_prefix + "temp_way_node_nodeid_idx ON " + tables_prefix + "temp_way_node (node_id ASC NULLS LAST);");
    PGresult *result = PQexec(mycon, create_index.c_str());
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cerr << "Index creation failed: " << PQerrorMessage(mycon) << std::endl;
        PQclear(result);
    } else {
        std::cout << "temp_way_node_nodeid_idx index created" << std::endl;
    }
}

void WaySplitter::createWayNodesIndex()
{
    std::string create_index("CREATE INDEX " + tables_prefix + "temp_way_node_wayid_idx ON " + tables_prefix + "temp_way_node (way_id ASC NULLS LAST);");
    PGresult *result = PQexec(mycon, create_index.c_str());
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cerr << "Index creation failed: " << PQerrorMessage(mycon) << std::endl;
        PQclear(result);
    } else {
        std::cout << "temp_way_node_wayid_idx index created" << std::endl;
    }
}

void WaySplitter::createWayTagsIndex()
{
    std::string create_index("CREATE INDEX " + tables_prefix + "temp_way_tag_wayid_idx ON " + tables_prefix + "temp_way_tag (way_id ASC NULLS LAST);");
    PGresult *result = PQexec(mycon, create_index.c_str());
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cerr << "Index creation failed: " << PQerrorMessage(mycon) << std::endl;
        PQclear(result);
    } else {
        std::cout << "temp_way_tag_wayid_idx index created" << std::endl;
    }
}



void WaySplitter::createCountColumn()
{
    std::string add_column("ALTER TABLE " + tables_prefix + "temp_way_node ADD COLUMN num_of_use integer;");
    PGresult *result = PQexec(mycon, add_column.c_str());
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cerr << "Add column failed: " << PQerrorMessage(mycon) << std::endl;
        PQclear(result);
    } else {
        std::cout << "Column created" << std::endl;
    }
}

void WaySplitter::fillPointsCount()
{
    std::string add_column("UPDATE " + tables_prefix + "temp_way_node t1 SET num_of_use = (SELECT count(*) from " + tables_prefix + "temp_way_node t2 where t2.node_id=t1.node_id)");
    PGresult *result = PQexec(mycon, add_column.c_str());
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cerr << "Point counts update failed: " << PQerrorMessage(mycon) << std::endl;
        PQclear(result);
    } else {
        std::cout << "Point counts update" << std::endl;
    }
}