```
#define _CRT_SECURE_NO_WARNINGS  // 去除 vs 函数不安全警告
#ifndef _HFMENCODING_H_
#define _HFMENCODING_H_
#include "Huffman.hpp"
#include <string>
#define  MAXBUFSIZE 1024

struct FileInfo  // 字符信息
{ 
	unsigned char _ch; // 当前字符
	long long _count; // 当前字符出现的次数
	std::string _charCode; // character	encoding 字符编码
	FileInfo()
		:_count(0)
	{}

	FileInfo operator+(const FileInfo& fileInfo)const // 注意参数  千万不要多写括号
	{
		FileInfo temp(fileInfo);
		temp._count+=_count;
		return temp;
	}
	bool operator!=(const FileInfo& fileInfo)const
	{
		return _count != fileInfo._count;
	}
	bool operator<(const FileInfo& fileInfo)const
	{
		return _count<fileInfo._count;
	}

};

template <class T>
class HfmEnconding
{
public:
	HfmEnconding()
	{
		for (int i = 0; i < 256; ++i)
		{
			_CodeArr[i]._ch = i;
			_CodeArr[i]._count = 0;
		}
	}

	// 压缩过程进度条
	void PrintComProBar(double allsize,double writesize)
	{
		char arr[102] = {'\0'};
		const char stat[] ={'-', '\\', '|', '/'};

		int scle = (writesize/allsize)*100;
		int count = 0;
		
		for (;count < scle;count++)
		{
			arr[count] = '#';
		}
		printf("[%-102s][%d]%%%c\r", arr, scle,stat[scle%4]);
		fflush(stdout);

	}

	// 压缩
	void CompressFile(const std::string & fileName)
	{
		FILE* openTxt = fopen(fileName.c_str(), "rb"); 
		if (NULL == openTxt)
		{
			cout << "打开文件失败！" << endl;
			return;
		}
		/*统计相同字符出现的次数*/
		unsigned char readBuf[MAXBUFSIZE] = {0};
		while (true)
		{
			// 记录问题 一定要记得读取的字符个数
			size_t _readSize = fread(readBuf, 1, MAXBUFSIZE, openTxt);
			if (_readSize == 0)
			{
				break;
			}
			else if( _readSize < 0)
			{
				perror("fread");
			}

			for(size_t idx = 0; idx < _readSize; ++idx )
			{
				//_CodeArr[ readBuf[idx] ]._ch = readBuf[idx];
				_CodeArr[ readBuf[idx] ]._count++;
			}
			
		}

		/* 构造哈弗曼树并获取编码*/
		Huffman<FileInfo> hfm(_CodeArr, sizeof(_CodeArr)/sizeof(_CodeArr[0]) , FileInfo());  // 第三个参数的问题，无名对象
		
		_GenerateHuffmanCode(hfm.GetRoot());

		
		/*开始压缩*/


		string writeInfo = GetFilePostfix(fileName); // 取得原来文件的后缀 用来写入压缩文件中 用来解压
		writeInfo += '\n';// 保存.txt 以及字符信息个字符个数，行数


		/* 如：
		.txt
		count
		a:x
		b:x
		c:x
		d:x
		....
		*/
		long long countLine = 0;  // 写入信息的统计行数
		string tempSaveStr = "";
		for (int i = 0; i < sizeof(_CodeArr)/sizeof(_CodeArr[0]); ++i)
		{
			if (_CodeArr[i]._count != 0)
			{
				countLine++;
				tempSaveStr += _CodeArr[i]._ch;
				tempSaveStr += ':';
				 char chcount[32];  // 存储每一个字符出现的次数
				itoa(_CodeArr[i]._count, chcount, 10);
				tempSaveStr += chcount;
				tempSaveStr += '\n';
			}
		}

		char strLineCount[32] = {0}; // 临时存放字符信息的行数
		itoa(countLine, strLineCount, 10);
		writeInfo += strLineCount;
		writeInfo += '\n';
		writeInfo += tempSaveStr;

		string comFileName = GetFileName(fileName);
		comFileName += ".hzp";
		// 创建压缩文件filename.hzp
		FILE* openHzp = fopen(comFileName.c_str(), "wb");

		// 写入头部信息
		fwrite(writeInfo.c_str(), 1, writeInfo.length(), openHzp);



		unsigned char writeBuf[MAXBUFSIZE] = {0};  // 读缓冲
		fseek(openTxt, 0, SEEK_SET); // 重新指向文件开始
		int pos = 0;  // 已经写入bit位个数
		int szie = 0;
		unsigned char value = 0; // 保存比特位信息

		long long filesize = hfm.GetRoot()->_weight._count;
		long long allsize = filesize;
		long long writesize = 0;
		while (true)
		{

			size_t _readSize = fread(readBuf, 1, MAXBUFSIZE, openTxt);
			if ( 0 == _readSize)
			{
				break;
			}
			else if( _readSize < 0)
			{
				perror("fread");
			}
			long long writeSize = 0; // 写入的字符个数

			while(writeSize < _readSize)
			{
				string curCharCode = _CodeArr[ readBuf[writeSize] ]._charCode; // 取得字符编码
				// 开始处理writebuf读取到的字符
				for (size_t index = 0; index < curCharCode.size(); ++index)
				{
					value = value<<1;
					if ('1' == curCharCode[index] )
					{
						value |= 1;
					}

					if (8 == ++pos)
					{
						writeBuf[szie++] = value;
						if (MAXBUFSIZE == szie)
						{
							fwrite(writeBuf, 1, MAXBUFSIZE, openHzp);
							szie = 0;
						}
						pos = 0;
						value = 0;
					}

				}
				writeSize++;
			}
			writesize += _readSize;
			PrintComProBar(allsize, writesize);
		}
		if (pos < 8) /// 这里的位置
		{
			value <<= (8-pos);
			writeBuf[szie++] = value;
			fwrite(writeBuf, 1, szie, openHzp);
		}
		fclose(openTxt);  // 关闭打开的原文件
		fclose(openHzp);  // 关闭压缩后的文件

		printf("\ncom ok\n");
	}
	// 解压
	void UnCompressFile(const std::string compressFilePath)
	{
		// 1. 获取编码按信息  验证.hzp
		FILE* openHzp = fopen(compressFilePath.c_str(), "rb");
		assert(openHzp);
		string openFilePostFix = GetFilePostfix(compressFilePath);
		string strPostfix = _ReadLine(openHzp);  // 获得原文件后缀名

		if (openFilePostFix != ".hzp")
		{
			cout << "解压失败,不是该软件压缩文件！" << endl;
			fclose(openHzp);
			return;
		}

		string newFileName = GetFileName(compressFilePath) + "_test" + strPostfix;
		FILE* openNewFile = fopen(newFileName.c_str(), "wb");
		if (openNewFile == NULL)
		{
			cout << "open newfile error\n";
			fclose(openHzp);
			return;
		}

		long long lineCount = atoi(_ReadLine(openHzp).c_str()); // 获取压缩文件中行数
		long long  indexLine = 0;
		while (indexLine < lineCount)
		{
			string strPerLine = _ReadLine(openHzp);
			if (strPerLine == "")
			{
				strPerLine = _ReadLine(openHzp);
				_CodeArr[ '\n' ]._count = atoi(strPerLine.substr(1).c_str());;

			}
			else
			{
				// 将压缩文件中存储的每个字符出现的次数存放到结构体重
				_CodeArr[ (unsigned char)strPerLine[0]]._count = atoi(strPerLine.substr(2).c_str());
			}
			
			++indexLine;
		}
		/* 还原树*/
		Huffman<FileInfo> hfm(_CodeArr, sizeof(_CodeArr)/sizeof(_CodeArr[0]), FileInfo());
		HuffmanNode<FileInfo>* pRoot = hfm.GetRoot();
		_GenerateHuffmanCode(pRoot); // 生成哈弗曼编码

		//解压
		unsigned char readBuf[MAXBUFSIZE] = {0};  
		unsigned char writeBuf[MAXBUFSIZE] = {0};
		HuffmanNode<FileInfo>* pCur = pRoot;
		int pos = 8;
		int writeSize = 0;
		long long filesize = pRoot->_weight._count; // 文件长度
		while (true)
		{
			size_t _readSize = fread(readBuf, 1,sizeof(readBuf), openHzp);
			if (_readSize == 0)
			{
				break;
			}
			
			int idx = 0;
			while (idx<_readSize)
			{
				--pos;
				if (readBuf[idx] & (1<<pos))
				{
					pCur = pCur->_pRight;
				}
				else
				{
					pCur = pCur->_pLeft;
				}
				if(pCur->_pLeft == NULL	&& pCur->_pRight == NULL)
				{
					
					writeBuf[writeSize++] = pCur->_weight._ch;
					if (MAXBUFSIZE == writeSize)
					{
						fwrite(writeBuf, 1, MAXBUFSIZE, openNewFile);
						writeSize = 0;
					}
					
					if (0 == --filesize)
					{
						fwrite(writeBuf, 1, writeSize, openNewFile);
						break;
					}
					pCur = pRoot;
				}
				if(0 == pos)
				{
					pos = 8;
					idx++;
				}
			}
			
		}
		if (openNewFile)
		{
			fclose(openNewFile);
		}
		if (openHzp)
		{
			fclose(openHzp);
		}
	}

private:
	// 获取编码
	void _GenerateHuffmanCode(HuffmanNode<FileInfo>* pRoot)
	{
		HuffmanNode<FileInfo>* pCur = pRoot;
		if (pCur)
		{
			_GenerateHuffmanCode(pCur->_pLeft);
			_GenerateHuffmanCode(pCur->_pRight);

			if (pCur->_pLeft == NULL && pCur->_pRight == NULL)
			{
				HuffmanNode<FileInfo>* pParent = pCur->_pParent;
				std::string strCode;
				while (pParent)
				{
					if (pParent->_pLeft == pCur)
					{
						strCode += '0';
					}
					if(pParent->_pRight == pCur)
					{
						strCode += '1';
					}
					pCur = pParent;
					pParent = pCur->_pParent;
				}
				reverse(strCode.begin(), strCode.end());
				_CodeArr[pRoot->_weight._ch]._charCode = strCode;
			}

			
		}
	}
	
	// 获取文件
	string GetFileName(const string& filepath)
	{
		string ret;
		ret = filepath.substr(0, filepath.find_last_of('.'));
		return ret;
	}

	// 获取后缀
	string GetFilePostfix(const string& filepath)
	{
		string ret;
		ret = filepath.substr(filepath.find_last_of('.'));
		return ret;
	}

	// 获取一行
	string _ReadLine(FILE* fp)
	{
		string strLine= "";
		if (feof(fp))
		{
			return strLine;
		}
		unsigned char c = fgetc(fp);
		while ('\n' != c)
		{
			strLine += c;
			if (feof(fp))
			{
				return strLine;
			}
			c = fgetc(fp);
		}
		return strLine;
	}
private:
	FileInfo _CodeArr[256];  // 存储序列中编码信息
};

#endif
```
