/**
  @file GtLogger.h
  
  @author Kaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef SrLogger_h__
#define SrLogger_h__

class SrLogger
{
public:
	SrLogger(void);
	~SrLogger(void);

	void Log(const char* line);
	void Log(const char* format, va_list args);

private:
	void FlushToFile();
	char* m_data;
	uint32 m_size;
};
extern SrLogger* g_logger;

//	 Simple logs of data with low verbosity.
inline void GtLog( const char* format, ... )
{
	if (g_logger)		
	{
		va_list args;
		va_start(args,format);
		g_logger->Log( format, args );
		va_end(args);
	}
}

#endif

