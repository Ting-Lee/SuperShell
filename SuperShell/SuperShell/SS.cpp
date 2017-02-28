#include "SS.h"

#include <iostream>


SS::SS(void)
{
}


SS::~SS(void)
{
	//delete map HMOUDLE
}

void SS::call()
{
	try
	{
	}
	catch (...)
	{

	}
}

bool SS::LoadLib(string libName)
{
	HMODULE hm = LoadLibraryA(libName.c_str());
	if (hm==INVALID_HANDLE_VALUE)return false;

	m_str_hm[libName]=hm;//insert
}

void SS::getInput()
{
	while(true)
	{
		std::getline(std::cin,m_input);
		ayaInput();
	}	

}

/*Input is a need execute function or load library? */
bool SS::tryBeLib(string str)
{



	string::size_type ret = str.find("LoadLibrary");
	if (ret==0)
	{
		/*
		LoadLibrary("LibName");
		We need LibName.
		*/
		std::string::size_type index = str.find("\"");
		if (index==string::npos)return;
		str.erase(0,index+1);

		index = str.find("\"");
		if (index==string::npos)return;
		str.erase(index);

		if(str.empty())return;

		bool result =LoadLib(str);
		if (!result)
		{
			Error("Load Library Fault:%s",str);
		}
		return true;

	}
	else
	{
		return false;
	}
}

FARPROC SS::getFuncAddr(string functionName)
{
	std::string::size_type LKindex = functionName.find("(");
	if (LKindex==string::npos)return;


	functionName.erase(LKindex);

	map<string,HMODULE>::iterator it;

	it = m_str_hm.begin();

	FARPROC funT=nullptr;

	while(it != m_str_hm.end())
	{
		funT = GetProcAddress(it->second,functionName.c_str());
		if (funT!=nullptr)
		{
			break;
		}
		it++;         
	}
	return funT;
}

void SS::getArgcs(string argList)
{
	std::string::size_type LKindex = argList.find("(");
	if (LKindex==string::npos)
	{
		Error("Don't find (");
		return;
	}


	argList.erase(0,LKindex);// 5,'a',"b")

	char* pArg =(char*) argList.c_str();



	for(;;++pArg)
	{
		char* tmp = pArg;

		if (IsEnd(pArg))
		{
			break;
		}
		IsSpace(pArg);//Need Be First
		IsNumber(pArg);
		IsNeg(pArg);



		if(*pArg=='}')
		{
			break;
		}
		if (*pArg==',')
		{
			++pArg;
		}

		if (tmp==pArg)//No man decide it
		{
			Error("What's mean:0x%x\n",*pArg);
		}



	}
	return;
}



bool SS::tryBeFunc(string functionName)
{
	std::string str = functionName;
	std::string str_2 = functionName;

	FARPROC funT = getFuncAddr(str_2);

	string argList = str;

	getArgcs(argList);

	executeCode(funT);

	return true;
}

void SS::executeCode(FARPROC funT)
{
	/*
	string			"abc"	Point
	char			'a'		Immediate number
	unsigned int	324		Immediate number
	int				-2222	Immediate number	hex,10, 8?
	float			
	double


	*/

	//Excute Code

	{
		//zero argv

		typedef  int (__cdecl *int_FUN_VOID)(void);

		int_FUN_VOID funA = (int_FUN_VOID)funT;
		if (funA==nullptr)return;

		try
		{
			funA();//temp to do,we need use SS::call()
		}
		catch (...)
		{
			return;
		}
	}
}

void SS::ayaInput()
{
	string str = m_input;
	str.erase(0, str.find_first_not_of(" \t")); // Remove header spaces
	str.erase(str.find_last_not_of(" \t") + 1); // Remove trailling spaces
	if(true == str.empty())return;

	string strTryBeLib=str;

	if (tryBeLib(strTryBeLib))
	{
		return;
	}


	string functionName=str;

	tryBeFunc(functionName);


	return;

}

bool SS::IsEnd(char* p)
{
	if (*p=='\0')
	{
		return true;
	}
	else
	{
		return false;
	}

}

void SS::IsSpace(char* p)
{
	if (*p==0x20)
	{
		++p;
	}	
}

void SS::IsNumber(char* pArg,bool neg)
{
	if (*pArg>=0x30&&*pArg<=0x39)//number			m_argVectorList
	{

		int m_number = 0;
		for (;;++pArg)	
		{
			if(*pArg>=0x30&&*pArg<=0x39)
			{
				m_number = m_number*10+*pArg-0x30;
			}
			else if(*pArg==0x20)
			{
				for (;*pArg==0x20;++pArg){}	// del space

				if (*pArg==',')//like:		234 ,    234 )
				{
					--pArg;
					goto NUMBER_SAVE;

				}
				else if (*pArg=='}')
				{
					--pArg;//keep next is '}'
					goto NUMBER_SAVE;
				}
				else
				{
					Error("What's mean:0x%x\n",*pArg);
					return; //err like: 234 2   or      234 :
				}

			}
			else if (*pArg=='x'||*pArg=='X')//hex
			{
				++pArg;

				if (m_number!=0)
				{
					Error("What's mean:'x' left dont is 0\n",*pArg);//like    (9x9)
					return;
				}
				else//hex
				{
					for (;;++pArg)
					{
						m_number = m_number * 16 + (*pArg & 15) + (*pArg >= 'A' ? 9 : 0);
					}
					//warning: We can't find 0x7buay87 from err
				}
			}
			else
			{
				Error("What's mean:0x%x\n",*pArg);
				return;//err
			}



		}
NUMBER_SAVE:

		if (neg)
		{
			m_number*=-1;
		}
		argFormat tmp;
		tmp.style = INTEGERX;
		tmp.value = m_number;

		m_argVectorList.push_back(tmp);


		++pArg;

	}
}

void SS::IsNeg(char* p)
{
	if (*p=='-')
	{
		++p;
		IsNumber(p,true);
	}
}


void SS::IsString(char* p)
{
	if (*p!='"')
	{
		return;
	}

	++p;
	/*
	if it is not "
	\" don't is "

	other:  \Number :this version don't permit.
	\r
	\n

	"" be \0
	*/
	char* strStart = p;

#define MAX_LENGTH_STRING 1024

	//char result[MAX_LENGTH_STRING]={0};
	char* result =(char*) malloc(MAX_LENGTH_STRING);
	if (nullptr==result)
	{
		Error("No enough memory in malloc,SIZE:0x%x",MAX_LENGTH_STRING);
	}
	memset(result,MAX_LENGTH_STRING,0);

	for (size_t i=0;;++i)
	{
		if (i>=MAX_LENGTH_STRING-10)
		{
			Error("So long string!");
		}
		char tmp;

		if (HandleBackslash(p,tmp))
		{
			result[i]=tmp;
		}
		else if (*p=='"')//string end
		{
			break;
		}
		else if (*p=='\0')
		{
			Error("Terminate in unexpected!");
		}
		else
		{
			result[i]=*p;
		}

	}
	
	

	argFormat tmp;
	tmp.style = STRINGX;
	tmp.str = result;

	m_argVectorList.push_back(tmp);



	++p;
}

void SS::IsChar(char* p)
{
	if (*p!='\'')
	{
		return;
	}
	++p;

	int value=0;

	char cTmp;
	if (HandleBackslash(p,cTmp))
	{
		value=cTmp;
	}
	else if (*p=='"')//string end
	{
		Warning("two ' middle don't find anything.");
		value=0;
		goto CHAR_GO;
	}
	else if (*p=='\0')
	{
		Error("Terminate in unexpected!");
	}
	else
	{
		value=*p;
	}
	++p;

	if (*p!='\'')
	{
		Error("Don't find  ' be end");
	}
CHAR_GO:
	argFormat tmp;
	tmp.style = CHARX;
	tmp.value = value;

	m_argVectorList.push_back(tmp);

	++p;

}

bool SS::HandleBackslash(char* p,char& result)
{
	if (*p=='\\')
	{
		//TODO: More
		++p;
		switch (*p)
		{
		case '\\':
			{
				result='\\';
				break;
			}
		case 'n':
			{
				result='\n';
				break;
			}
		case 'r':
			{
				result='\r';
				break;
			}
		case '"':
			{
				result='\'';
				break;
			}
		case '\0':
			{
				Error("Terminate in unexpected!");
				break;
			}
		default:
			{
				result=*p;
				break;
			}
		}
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void SS::Error(char* fmt,...)
{
	va_list ap;
	va_start(ap,fmt);

	Handle_Exception(STAGE_COMPILE,LEVEL_ERROR,fmt,ap);
	va_end(ap);

	throw("Error");
}

void SS::Warning(char* fmt,...)
{
	va_list ap;
	va_start(ap,fmt);

	Handle_Exception(STAGE_COMPILE,LEVEL_WARNING,fmt,ap);
	va_end(ap);

	throw("Error");
}

void SS::Execute_error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Handle_Exception(STAGE_LINK,LEVEL_ERROR, fmt, ap);
	va_end(ap);
}

void SS::Handle_Exception(int stage,int level,char* fmt,va_list ap)
{
	char buf[1024];
	vsprintf_s(buf,sizeof(buf),fmt,ap);
	if (stage == STAGE_COMPILE)
	{
		if (level==LEVEL_WARNING)
		{
			printf("Compair Warning:%s\n",buf);
		}
		else
		{
			printf("Compair Error:%s\n",buf);
			throw("Error");
		}
	}
	else
	{
		printf("Execut Error:%s\n",buf);
		throw("Error");
	}
}
