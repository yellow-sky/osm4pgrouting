/***************************************************************************
*   Copyright (C) 2008 by Daniel Wendt   								   *
*   gentoo.murray@gmail.com   											   *
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

#include "stdafx.h"
#include "Configuration.h"
#include "ConfigurationParserCallback.h"
#include "OSMDocumentParser.h"
#include "Way.h"
#include "Node.h"
#include "Relation.h"
#include "Export2DB.h"
#include "WaySplitter.h"

using namespace osm;
using namespace xml;
using namespace std;

void _error()
{
    cout << "following params are required: " << endl;
    cout << "-file <file>  -- name of your osm xml file" << endl;
    cout << "-conf <conf>  -- name of your configuration xml file" << endl;
    cout << "-dbname <dbname> -- name of your database" << endl;
    cout << "-user <user> -- name of the user, which have write access to the database" << endl;
    cout << "optional:" << endl;
    cout << "-host <host>  -- host of your postgresql database (default: 127.0.0.1)" << endl;
    cout << "-port <port> -- port of your database (default: 5432)" << endl;
    cout << "-passwd <passwd> --  password for database access" << endl;
    cout << "-prefixtables <prefix> --  add at the beginning of table names" << endl;
    cout << "-clean -- drop previously created tables" << endl;
    cout << "-skipnodes -- don't import the nodes table" << endl;
}

int main(int argc, char* argv[])
{
    std::string file;
    std::string cFile;
    std::string host="127.0.0.1";
    std::string user;
    std::string port="5432";
    std::string dbname;
    std::string passwd;
    std::string prefixtables="";
    bool skipnodes = false;
    bool clean = false;
    if(argc >=7 && argc <=19)
    {
        int i=1;
        while( i<argc)
        {
            if(strcmp(argv[i],"-file")==0)
            {
                i++;
                file = argv[i];
            }

            else if(strcmp(argv[i],"-conf")==0)
            {
                i++;
                cFile = argv[i];
            }

            else if(strcmp(argv[i],"-host")==0)
            {
                i++;
                host = argv[i];
            }
            else if(strcmp(argv[i],"-dbname")==0)
            {
                i++;
                dbname = argv[i];
            }
            else if(strcmp(argv[i],"-user")==0)
            {
                i++;
                user = argv[i];
            }
            else if(strcmp(argv[i],"-port")==0)
            {
                i++;
                port = argv[i];
            }
            else if(strcmp(argv[i],"-passwd")==0)
            {
                i++;
                passwd = argv[i];
            }
            else if(strcmp(argv[i],"-prefixtables")==0)
            {
                i++;
                prefixtables = argv[i];
            }
            else if(strcmp(argv[i],"-clean")==0)
            {
                clean = true;
            }
            else if(strcmp(argv[i],"-skipnodes")==0)
            {
                skipnodes = true;
            }
            else
            {
                cout << "unknown parameter: " << argv[i] << endl;
                _error();
                return 1;
            }

            i++;
        }

    }
    else
    {
        _error();
        return 1;
    }

    if(file.empty() || cFile.empty() || dbname.empty() || user.empty())
    {
        _error();
        return 1;
    }

    Export2DB exporter(host, user, dbname, port, passwd, prefixtables);
    if(exporter.connect()==1)
        return 1;

    XMLParser parser;

    cout << "Trying to load config file " << cFile.c_str() << endl;

    Configuration* config = new Configuration();
    ConfigurationParserCallback cCallback( *config );

    cout << "Trying to parse config" << endl;

    int ret = parser.Parse(cCallback, cFile.c_str());
    if (ret!=0) {
      cerr << "Failed to parse config file " << cFile.c_str() << endl;
      return 1;
    }
    
    //############# Prepare DB
    if( clean )
    {
      cout << "Dropping tables..." << endl;
      exporter.dropTables();
    }
    
    cout << "Creating tables..." << endl;
    exporter.createTables();
    
    cout << "Adding tag types and classes to database..." << endl;
    exporter.exportTypesWithClasses(config->m_Types);
    
    
    //############# Load Data    
    cout << "Trying to load data" << endl;

    OSMDocumentParser osmParser(*config, &exporter);

    cout << "Trying to parse data" << endl;

    ret = parser.Parse( osmParser, file.c_str() );
    if( ret!=0 ) {
        if( ret == 1 )
            cerr << "Failed to open data file" << endl;
        cerr << "Failed to parse data file " << file.c_str() << endl;
        return 1;
    }
    
     //############# Export2DB
    osmParser.SaveAllBuffers();
    
    //############# Split Ways
    cout << "Split ways" << endl;
    WaySplitter splitter(host, user, dbname, port, passwd, prefixtables);
    if(splitter.connect()==1)
        return 1;
    splitter.splitWays();


    //############# CreateTopology
    cout << "Creating topology..." << endl;
    exporter.createTopology();
    


    cout << "#########################" << endl;

    //cout << "size of streets: " << document->m_Ways.size() <<	endl;
    //cout << "size of splitted ways : " << document->m_SplittedWays.size() <<	endl;

    cout << "finished" << endl;

    //string n;
    //getline( cin, n );
    return 0;
}
