#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_ERROR
#ifdef WATCHER_LOGGER_H
    // borrowed from logger.h
    #define LOG_DEBUG(message, ...) { try{ LOG4CXX_DEBUG(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } }
    #define LOG_INFO(message, ...) { try{ LOG4CXX_INFO(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } }
    #define LOG_ERROR(message, ...) { try{ LOG4CXX_ERROR(logger, message ## __VA_ARGS__); } catch(std::exception &e) { std::cerr << "Exception in logging: " << e.what() << std::endl; } }
#endif
