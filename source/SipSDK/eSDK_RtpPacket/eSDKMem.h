#pragma once

#ifdef WIN32 
extern int g_memstat;
#endif


// ����һ���ڴ������⸲�Ǻ궨�壬ʹ��Ĭ��ID����Ϊpclint�����ò�����ճ���������
#define HW_NEW(var, Type) \
	do {\
	(var) = NULL;\
	try {\
	(var) = new Type;\
	if(NULL != (var)){++g_memstat;}\
}\
	catch (...) {\
	(var) = NULL;\
}\
	} while (0)

#define HW_DELETE(var) \
	do {\
	if(NULL != var) \
{\
	delete (var);\
	(var) = NULL;\
	--g_memstat;\
}\
	} while (0)

#define HW_NEW_A(var, type, size)\
	do {\
	try {\
	(var) = new type[size];\
	if(NULL != (var)){++g_memstat;}\
}\
	catch (...) {\
	(var) = NULL;\
}\
	} while (0)

#define HW_DELETE_A(var)\
	do {\
	if(NULL != var) \
{  \
	delete [] (var);    \
	(var) = NULL;       \
	--g_memstat; \
}  \
	} while (0)

