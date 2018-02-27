/*
 * Written by Kimi kan, 2016-12-23,  with c++14, Windows & Linux
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <os/inttypes.h>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <tuple>

namespace tools 
{
	typedef unsigned short uint16;

	template<class T>
	class Words : public std::vector<std::shared_ptr<T> > 
        { 
		static bool Less1(const std::shared_ptr<T>& t1
			, const std::shared_ptr<T>& t2) 
                {
			return t1->_gbkValue < t2->_gbkValue;
		}
		static bool Less2(const std::shared_ptr<T>& t1, uint16 gbkValue) 
		{
			return t1->_gbkValue < gbkValue;
		}

	public:
		void Sort() 
		{
			std::sort(this->begin(), this->end(), Less1);
		}

		std::shared_ptr<T> FindItemByGbkValue(uint16 value) 
		{
			auto iter = std::lower_bound(this->begin(), this->end(), value, Less2);
			if (iter != this->end()) 
			{
				if ((*iter)->_gbkValue == value) 
				{
					return *iter;
				}
			}

			return nullptr;
		}
	};

	/* Word with different pronunciations */
	class Word 
	{
	public:
		uint16 _gbkValue;
		std::vector<char> _pys;
	};


	Words<Word> __raredUsedWords;
	Words<Word> __polyPronuWords;


	/* Normal GBK word */
	bool CheckGBKWordTable(uint16 tmp, char& result) 
	{
		if (tmp >= 45217 && tmp <= 45252) result = ('A');
		else if (tmp >= 45253 && tmp <= 45760) result = ('B');
		else if (tmp >= 45761 && tmp <= 46317) result = ('C');
		else if (tmp >= 46318 && tmp <= 46825) result = ('D');
		else if (tmp >= 46826 && tmp <= 47009) result = ('E');
		else if (tmp >= 47010 && tmp <= 47296) result = ('F');
		else if (tmp >= 47297 && tmp <= 47613) result = ('G');
		else if (tmp >= 47614 && tmp <= 48118) result = ('H');
		else if (tmp >= 48119 && tmp <= 49061) result = ('J');
		else if (tmp >= 49062 && tmp <= 49323) result = ('K');
		else if (tmp >= 49324 && tmp <= 49895) result = ('L');
		else if (tmp >= 49896 && tmp <= 50370) result = ('M');
		else if (tmp >= 50371 && tmp <= 50613) result = ('N');
		else if (tmp >= 50614 && tmp <= 50621) result = ('O');
		else if (tmp >= 50622 && tmp <= 50905) result = ('P');
		else if (tmp >= 50906 && tmp <= 51386) result = ('Q');
		else if (tmp >= 51387 && tmp <= 51445) result = ('R');
		else if (tmp >= 51446 && tmp <= 52217) result = ('S');
		else if (tmp >= 52218 && tmp <= 52697) result = ('T');
		else if (tmp >= 52698 && tmp <= 52979) result = ('W');
		else if (tmp >= 52980 && tmp <= 53688) result = ('X');
		else if (tmp >= 53689 && tmp <= 54480) result = ('Y');
		else if (tmp >= 54481 && tmp <= 55289) result = ('Z'); // 0xd7f9
		else return false;
		return true;
	}


	const std::vector<char>& GetPys(unsigned char i1, unsigned char i2, bool& isPoly) 
	{
		unsigned int tmp = i1 * 256 + i2;
		isPoly = false;
		//polyphonic
		auto ptrToWord = __polyPronuWords.FindItemByGbkValue(tmp);
		if (ptrToWord != nullptr) 
		{
			isPoly = true;
			return ptrToWord->_pys;
		}
		static std::vector<char> tmpVec;
		tmpVec.clear();
		// GBK rule
		if (i2 >= 0xa1) 
		{ //GB2312 second byte >= 0xa1
			if (i1 == 0xa2) 
			{ //处理 ⒆ 类型的数字符号
				if ((i2 >= 0xc5) && (i2 <= 0xcd)) // ⑴--->⑼
					tmpVec.push_back((i2 - 0xc5) + '1');
				else if ((i2 >= 0xce) && (i2 <= 0xd8)) 
				{ // ⑽-->⒇
					int iTemp = i2 - 0xce + 10;
					char szPY[3];
                    sprintf(szPY, "%d", iTemp);
					//itoa(iTemp, szPY, 10);
					tmpVec.push_back(szPY[0]);
					tmpVec.push_back(szPY[1]);
				}
			}
			else if (i1 == 0xa3) 
			{ //Full symbol

				if ((i2 >= 0xb0) && (i2 <= 0xb9))
					tmpVec.push_back((i2 - 0xb0) + '0');
				else if ((i2 >= 0xc1) && (i2 <= 0xda))
					tmpVec.push_back((i2 - 0xc1) + 'A');
				else if ((i2 >= 0xe1) && (i2 <= 0xfa))
					tmpVec.push_back((i2 - 0xe1) + 'A');
			}
			else if (i1 >= 0xb0) 
			{ // GBK first byte >= 0xb0
				char result;
				if (CheckGBKWordTable(tmp, result)) 
				{
					tmpVec.push_back(result);
				}
			}
		}

		// uncommon word handle
		if (tmpVec.size() <= 0) 
		{ 
			auto ptrToWord = __raredUsedWords.FindItemByGbkValue(tmp);
			if (ptrToWord != nullptr) 
			{
				return ptrToWord->_pys;
			}

		}

		return tmpVec;
	}

	/* List result for ... */
	void ListResult(std::vector< std::tuple<bool, std::vector<char> > >& vecs
		, std::vector<std::string>& result, int start, char* tmp, int index) 
	{
		if (start == vecs.size()) 
		{
			result.push_back(tmp);
			return;
		}

		auto& s = vecs[start];
		auto ispoly = std::get<0>(s);
		auto& s2 = std::get<1>(s);
		if (!ispoly) 
		{
			memcpy(tmp + index, &s2[0], s2.size());
			ListResult(vecs, result, start + 1, tmp, index + s2.size());
		}
		else 
		{
			for (unsigned int j = 0; j < s2.size(); ++j) 
			{
				tmp[index] = s2[j];
				ListResult(vecs, result, start + 1, tmp, index + 1);
			} //Password
		}
	}

	bool GetPyString(std::vector<char>& in, std::vector<std::string>& result) 
	{

		std::vector< std::tuple<bool, std::vector<char> > > vecs;

		for (unsigned int i = 0; i < in.size(); i++) 
		{
			if ((unsigned char)(in[i]) >= 0x80) 
			{
				bool isPoly = false;
				auto pyVec = GetPys(in[i], in[i + 1], isPoly);
				i++;
				if(!pyVec.empty())
					vecs.push_back(std::make_tuple(isPoly, pyVec));
			}
			else 
			{
				if ((in[i] != ' ') && (in[i] != '-')) 
				{ //del' '
					std::vector<char> tmpVec;
					tmpVec.push_back(in[i]);
					vecs.push_back(std::make_tuple(false, tmpVec));
				}
			}
		}

		if (vecs.size() > 0) 
		{
			char tmp[20] = { '\0' };
			ListResult(vecs, result, 0, tmp, 0);
			for (unsigned int i = 0; i < result.size(); ++i ) 
			{
				auto v = result[i];
				if (v.length() > 1 && v[0] == '*') 
				{
					result.push_back(v.c_str() + 1);
				}
			}
		}
		return true;
	}

	void InitPY() 
	{
		FILE* hFile = fopen("uncommon.txt", "rt"); // uncommon used words
		char szBuff[512];
		char* lpBuff = NULL;
		if (hFile) 
		{
			lpBuff = fgets(szBuff, 512, hFile);
			while (lpBuff) 
			{
				int i1 = (unsigned char)lpBuff[0];
				int i2 = (unsigned char)lpBuff[1];
				auto word = std::make_shared<Word>();
				word->_pys.push_back(lpBuff[3]);
				word->_gbkValue = i1 * 256 + i2;
				__raredUsedWords.push_back(word);
				lpBuff = fgets(szBuff, 512, hFile);
			}
			fclose(hFile);
		}
		__raredUsedWords.Sort();

		hFile = fopen("polyphonic.txt", "rt"); // polyphonic
		if (hFile) 
		{
			lpBuff = fgets(szBuff, 512, hFile);
			while (lpBuff) 
			{
				int i1 = (unsigned char)lpBuff[0];
				int i2 = (unsigned char)lpBuff[1];
				auto word = std::make_shared<Word>();
				word->_gbkValue = i1 * 256 + i2;
				int iStrLen = strlen(lpBuff);
				if (lpBuff[iStrLen - 1] == 0x0a)
					lpBuff[iStrLen - 1] = '\0';
				int iPYs = strlen(lpBuff) - 2;
				iPYs /= 2;
				for (int i = 0; i < iPYs; i++) 
				{
					word->_pys.push_back(lpBuff[3 + i * 2]);
				}
				__polyPronuWords.push_back(word);
				lpBuff = fgets(szBuff, 512, hFile);
			}
			fclose(hFile);
		}
		__polyPronuWords.Sort();
	}
}

#include <iostream>

using namespace tools;
/*
 * Test application, you can keep it logic via the .txt configuration
 */
int main(int argc, char **argv) 
{
	double df = -1;
	InitPY();

	std::vector<std::string> result;
	std::string strs[]{ "DR长园集","长园集团","钽" ,"乐沭亳","万 科A" ,"14锦昉债","重亳沭厦"
	,"长园集团","PR亳州债" ,"工重银行","三鑫医疗" ,"长1集团","*ST新亿","*亳沭厦","测试枮◆" };
	
	{
		std::string str = "*亳州债";
		std::vector<std::string> result;
		std::vector<char> src;
		for (unsigned int j = 0; j < str.length(); ++j) 
		{
			src.push_back(str[j]);
		}
		GetPyString(src, result);

		std::cout << str << "" << "" << std::endl;
		for (unsigned int j = 0; j < result.size(); ++j) 
		{
			std::cout << result[j].c_str() << std::endl;
		}

		std::cout << std::endl;
	}

	for (int i = 0; i < 15; ++i) 
	{
		std::vector<std::string> result;
		std::vector<char> src;
		for (unsigned int j = 0; j < strs[i].length(); ++j) 
		{
			src.push_back(strs[i][j]);
		}
		GetPyString(src, result);

		std::cout << strs[i] << "" << "" << std::endl;
		for (unsigned int j = 0; j < result.size(); ++j) 
		{
			std::cout << result[j].c_str() << std::endl;
		}

		std::cout << std::endl;
	}
	return 0;
}
