//
// daytime_client.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <getopt.h>
#include "logger.hpp"
#include "stream_socket_service.hpp"

#include "libconfig.h++"

using namespace libconfig;

typedef boost::asio::basic_stream_socket<boost::asio::ip::tcp,
        services::stream_socket_service<boost::asio::ip::tcp> > debug_stream_socket;

char read_buffer[1024];

void read_handler(const boost::system::error_code& e, std::size_t bytes_transferred, debug_stream_socket* s)
{
    if (!e)
    {
        std::cout.write(read_buffer, bytes_transferred);

        s->async_read_some(boost::asio::buffer(read_buffer),
                boost::bind(read_handler, boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred, s));
    }
}

void connect_handler(const boost::system::error_code& e, debug_stream_socket* s, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
    if (!e)
    {
        s->async_read_some(boost::asio::buffer(read_buffer),
                boost::bind(read_handler, boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred, s));
    }
    else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
        s->close();
        boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
        s->async_connect(endpoint,
                boost::bind(connect_handler,
                    boost::asio::placeholders::error, s, ++endpoint_iterator));
    }
    else
    {
        std::cerr << e.message() << std::endl;
    }
}

static void usage(char *progName)
{
    fprintf(stdout, "Usage: %s --config configFile\n", basename(progName));
}

int main(int argc, char* argv[])
{
    int c;
    char *confFilename=NULL;

    while(true)
    {
        int option_index=0;
        static struct option long_options[] = {
            {"config", required_argument, 0, 'c'}
        };
        c = getopt_long(argc, argv, "c:hH?", long_options, &option_index);

        if (c==-1)
            break;

        switch(c)
        {
            case 'c':
                confFilename=strdup(optarg);
                break;
            case 'h':
            case 'H':
            case '?':
                usage(argv[0]);
                exit(0); 
        }
    }

    if(confFilename==NULL)
    {
        usage(argv[0]);
        exit(0); 
    }

    Config cfg;
    try
    {
        cfg.readFile(confFilename);

        boost::asio::io_service io_service;

        // Set the name of the file that all logger instances will use.
        services::logger logger(io_service, "");
        if(false==cfg.exists("log.file"))
        {
            std::cout << "Config file missing key \"log.file\"" << std::endl;
            free(confFilename);
            usage(argv[0]);
            exit(1);
        }
        else
        {
            bool append=true;
            cfg.lookupValue(static_cast<const char*>("log.append"), append);

            logger.use_file(cfg.lookup("log.file"), append);
        }

        if(false==cfg.exists("host"))
        {
            std::cout << "Config file missing \"host\" key" << std::endl;
            free(confFilename);
            exit(1);
        }

        // Resolve the address corresponding to the given host.
        boost::asio::ip::tcp::resolver resolver(io_service);
        // boost::asio::ip::tcp::resolver::query query(cfg.lookup("host"), "nerod");
        boost::asio::ip::tcp::resolver::query query(cfg.lookup("host"), "daytime");
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        boost::asio::ip::tcp::endpoint endpoint = *iterator;

        // Start an asynchronous connect.
        debug_stream_socket socket(io_service);
        socket.async_connect(endpoint,
                boost::bind(connect_handler,
                    boost::asio::placeholders::error, &socket, ++iterator));

        // Run the io_service until all operations have finished.
        io_service.run();
    }
    catch (ParseException &e)
    {
        std::cerr << "Error parsing config file at line " << e.getLine() << ":" << e.getError() << std::endl;
    }
    catch (SettingNotFoundException &e)
    {
        std::cerr << "Missing setting in configuration file. " << std::endl; // << e.what() << std::endl;
    }
    catch (ConfigException &e)
    {
        std::cerr << "Error parsing config file (" << e.what() << "). Did you leave a semicolon off somewhere in the conf file?" << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    if(confFilename)
        free(confFilename);

    return 0;
}
