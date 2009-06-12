#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_ERROR

#ifdef DELTA_LOG
    // borrowed from dtUtil/log.h
    #define LOG_DEBUG(msg)\
        dtUtil::Log::GetInstance().LogMessage(__FUNCTION__, __LINE__, msg,dtUtil::Log::LOG_DEBUG);
    #define LOG_INFO(msg)\
        dtUtil::Log::GetInstance().LogMessage(__FUNCTION__, __LINE__, msg,dtUtil::Log::LOG_INFO);
    #define LOG_ERROR(msg)\
        dtUtil::Log::GetInstance().LogMessage(__FUNCTION__, __LINE__, msg,dtUtil::Log::LOG_ERROR);
#endif
