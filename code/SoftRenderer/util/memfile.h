/**
  @file IMemFile.h
  
  @brief 内存文件封装，用于文件读取

  @author yikaiming

  更改日志 history
  ver:1.0
   
 */

#ifndef IMemFile_h__
#define IMemFile_h__

	struct SrMemFile
	{
		SrMemFile():m_open(false),
			m_data(NULL),
			m_size(0),
			m_binary(false),
			m_unicode(false)
		{

		}
		~SrMemFile()
		{
			Close();
		}

		SrMemFile(const SrMemFile& other);

		SrMemFile& operator= (const SrMemFile& rhs);

		virtual void Open( const char* diskPath );

		virtual void Close()
		{
			if(m_data)
			{
				delete[] m_data;
				m_data = NULL;
			}
		}

		virtual bool IsOpen() const
		{
			return m_open;
		}

		virtual int Size() const
		{
			if(m_unicode)
			{
				return m_size - 2;
			}
			return m_size;
		}

		virtual const char* Data() const
		{
			if(m_unicode)
			{
				return m_data + 2;
			}
			return m_data;
		}

		char* m_data;
		int m_size;
		bool m_open;
		bool m_binary;
		bool m_unicode;
		int m_encoding;
	};

#endif  //_IMEMFILE_H
