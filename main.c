#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*****************************************************************************
 *
******************************************************************************/

// 添加函数流程
// 1、在符号id中添加该函数名称作为ID					ID可通过另一个项目生成
// 2、在InitSymbolFromFuntional中添加函数
// 3、在next获取到变量token时对该函数进行比较			case 中的内容可通过另一个项目生成
// 4、在语法分析中，添加遇到该函数的处理流程		

typedef enum {
	None, Number, String, Value, Functional, Keyword, Cmp,
}SymbolsType;


typedef enum {
	// NONE = 0,用来标识错误
	IF = 1, ELSE, ELIF, VAL, LLB, RLB, LBB, RBB, LT, GT, LE, GE, COMMA, SEMICOLON,AND,OR,LAND,LOR,
	FUNC1 = 32, FUNC2, FUNC3
}SymbolsId;

typedef struct Symbol {
	SymbolsType type;
	char* begin, *end;
	int id;	 
	int val; 
}Symbol;



// SymbolId 处的定义用全大写，相应的hash用大小写
// HashEnum 应当使用另一个项目生成，防止输入错误以及hash冲突
enum HashEnum {
	If = 624,
	Val = 3998,
	Elif = 59624,
	Else = 59723,
	Func1 = 615711,
	Func2 = 615712,
	Func3 = 615713,
};

/*****************************************************************************
 * 作用：  计算hash，hash用于在Next词法分析中快速判断当前tokn为哪个关键字或者函数
 * 参数：  给定一个NUL结尾的字符串
 * 返回值：根据参数得到的hash
******************************************************************************/
int StringToHash(char* str) {
	int hash_ret = 0;
	while (*str) {
		hash_ret = hash_ret * 10 + *str-'0';
		str++;
	}
	return hash_ret;
}


/*****************************************************************************
 * 用于计算hash，区别在于本函数参数使用两个char *作范围，无需NUL结尾
******************************************************************************/
int RangeStringToHash(char* begin, char* end) {
	int ret_hash = 0;
	while (begin != end) {
		ret_hash = ret_hash * 10 + *begin - '0';
		begin++;
	}
	return ret_hash;
}



/*****************************************************************************
 * TODO,测试函数,func1,func2,func3,测试不同的函数和参数
******************************************************************************/
void func1(int arg1, int arg2) {
	printf("func1函数被调用，参数1为%d，参数2为%d\n",arg1,arg2);
}
void func2(int arg) {
	printf("func2函数被调用，参数为%d\n",arg);
}
void func3(char* str) {
	printf("func3被调用，参数是%s\n", str);
}

/*****************************************************************************
 * TODO,测试函数,用于快速输出字符出得到的hash值
******************************************************************************/
void PrintHash(char* str) {
	printf("%d\n", StringToHash(str));
}

/*****************************************************************************
 * TODO,测试函数,用来快速打印token
******************************************************************************/
void Print(char* begin, char* end) {
	char buf[255] = "";
	strncpy(buf, begin, end - begin);
	printf("%s \n",buf);
}

/*****************************************************************************
 * TODO,测试函数,用来从文本读取代码文本,返回的char *由malloc获取，使用完毕应当调用free释放
******************************************************************************/
char* ReadFileToBuf(const char *path) {

	FILE* file = fopen(path, "r");
	
	if (file == NULL)
		return NULL;

	fseek(file, 0, SEEK_END);
	long len = ftell(file);
	if (len == -1L) {
		return NULL;
	}
	char* buf = malloc(len + 1);
	if (buf == NULL)
		return NULL;
	rewind(file);
	len = fread(buf, 1, len, file);
	buf[len] = '\0';

	fclose(file);
	return buf;
}

/*****************************************************************************
 * 作用：将获取的使用两个指针表示的字符串token转为以NUL结尾的字符串
 * 参数：begin表示需要转换的字符串的开头，end表示需要转换的字符串的结尾
 * 返回值：一个malloc出来的char *，内容为begin和end之前的字符，最后一个字节为NUL （该指针为malloc的指针，使用后需要释放）
******************************************************************************/
char *RangePCharToPChar(char *begin,char *end) {
	char* ret = malloc(end - begin + 1);
	if (ret == NULL)
		return NULL;
	strncpy(ret, begin, end - begin);
	ret[end - begin] = '\0';
	return ret;
}

// 全局变量定义
// 符号表里存放的实际只有 关键字if elif else 和 函数 （不支持变量定义所以不存在变量
Symbol symbols[128], cur_symbol;
char* str;

/*****************************************************************************
 * 作用：词法分析
 * 返回值：根据当前处理的token返回相应的symbol，该symbol中保存该token的type、id、val等信息
******************************************************************************/
Symbol Next(void) {
	Symbol token;
	token.begin = token.end = NULL;
	token.type = None;
	while (*str) {
		if (*str <= '9' && *str >= '0') {
			int val = 0;
			token.begin = str;
			while (*str <= '9' && *str >= '0') {
				val = val * 10 + *str - '0';
				str++;
			}
			token.val = val;
			token.end = str;
			token.type = Number;
			return token;
		}
		else if (*str <= 'z' && *str >= 'a'
			|| *str <= 'Z' && *str >= 'A') {
			token.begin = str;
			while (*str <= 'z' && *str >= 'a'
				|| *str <= 'Z' && *str >= 'A'
				|| *str <= '9' && *str >= '0') {
				str++;
			}
			token.end = str;


			// 变量存在三种情况
			// 1、关键字 if elif else VAL
			// 2、函数名
			switch (RangeStringToHash(token.begin,token.end)) {
			// 关键字
			case If:
				token.type = Keyword;
				token.id = IF;
				return token;
			case Elif:
				token.type = Keyword;
				token.id = IF;	// elif 被认为是 if
				return token;
			case Else:
				token.type = Keyword;
				token.id = ELSE;
				return token;
			case Val:
				token.type = Keyword;
				token.id = VAL;
				return token;

			// 函数
			case Func1:
				token.type = Functional;
				token.id = FUNC1;
				return token;
			case Func2:
				token.type = Functional;
				token.id = FUNC2;
				return token;
			case Func3:
				token.type = Functional;
				token.id = FUNC3;
				return token;
			default:
				printf("解析到变量，但不支持变量定义");
				token.type = Value;
				return token;
			}
			//if (strncmp(token.begin, symbols[IF].begin, 2) == 0) {
			//	token.type = Keyword;
			//	token.id = IF;
			//	return token;
			//}
			//if (strncmp(token.begin, symbols[ELIF].begin, 4) == 0) {
			//	token.type = Keyword;
			//	token.id = IF;	// elif 被认为是 if
			//	return token;
			//}
			//if (strncmp(token.begin, symbols[ELSE].begin, 4) == 0) {
			//	token.type = Keyword;
			//	token.id = ELSE;
			//	return token;
			//}
			//if (strncmp(token.begin, symbols[VAL].begin, 3) == 0) {
			//	token.type = Keyword;
			//	token.id = VAL;
			//	return token;
			//}

			//// 和内置函数比较
			//if (strncmp(token.begin, symbols[FUNC1].begin, 5) == 0) {
			//	token.type = Functional;
			//	token.id = FUNC1;
			//	return token;
			//}
			//if (strncmp(token.begin, symbols[FUNC2].begin, 5) == 0) {
			//	token.type = Functional;
			//	token.id = FUNC2;
			//	return token;
			//}
			//if (strncmp(token.begin, symbols[FUNC3].begin, 5) == 0) {
			//	token.type = Functional;
			//	token.id = FUNC3;
			//	return token;
			//}
			//// 都不相同时认为是变量,实际上，不支持变量定义时应认为该表达式不可达
			//token.type = Value;
			//return token;

		}
		else switch (*str) {
		case '(':
			token.type = Keyword;
			token.id = LLB;
			token.begin = str;
			token.end = ++str;
			return token;
		case ')':
			token.type = Keyword;
			token.id = RLB;
			token.begin = str;
			token.end = ++str;
			return token;
		case '{':
			token.type = Keyword;
			token.id = LBB;
			token.begin = str;
			token.end = ++str;
			return token;
		case '}':
			token.type = Keyword;
			token.id = RBB;
			token.begin = str;
			token.end = ++str;
			return token;
		case '>': {
			token.begin = str;
			if (*(str+1) == '=') {
				token.id = GE;
				str++;
			}
			else {
				token.id = GT;
			}
			token.type = Cmp;
			token.end = ++str;
			return token;

		}
		case '<': {
			token.begin = str;
			if (*(str+1) == '=') {
				token.id = LE;
				str++;
			}
			else {
				token.id = LT;
			}
			token.type = Cmp;
			token.end = ++str;
			return token;
		}

		case ',':
			token.type = Keyword;
			token.id = COMMA;
			token.begin = str;
			token.end = ++str;
			return token;
		case ';':
			token.type = Keyword;
			token.id = SEMICOLON;
			token.begin = str;
			token.end = ++str;
			return token;
		case '"':
			// begin 指向第一个",end指向第二个"的后一个元素
			token.type = String;
			token.begin = ++str;
			while (*str != '"') {
				str++;
			}
			token.end = str++;
			return token;

		case ' ':
		case '\t':
		case '\r':
		case '\n':
			str++;
			return token;

		case '&': {
			token.begin = str;
			token.id = AND;
			if (*(str+1) == '&') {
				token.id = LAND;
				str++;
			}
			token.type = Keyword;
			token.end = ++str;
			return token;
		}
		case '|': {
			token.begin = str;
			token.id = OR;
			if (*(str+1) == '|') {
				token.id = LOR;
				str++;
			}
			token.type = Keyword;
			token.end = ++str;
			return token;
		}
		default:
			printf("next不支持该token\n");
			return token;
		}

	}
	return token;
}


/*****************************************************************************
 * 作用：获取下一个类型非空(即，不为空白符、制表符、换行符等)的token
 * 返回值：获取到的token信息
******************************************************************************/
Symbol Match(void) {
	Symbol symbol;
	do {
		symbol = Next();
	} while (symbol.type == None && *str != '\0');

	if (*str == '\0') {
		symbol.id = symbol.type = symbol.val = 0;
		symbol.begin = symbol.end = NULL;
		cur_symbol = symbol;
		return symbol;
	}

	cur_symbol = symbol;
	return symbol;
}


/*****************************************************************************
 * 作用：获取下一个token，并判断是否为指定Id
 * 参数：SymbolId类型，为想要获取的token的id
 * 返回值：获取成功时返回获取到的token信息，否则表示获取到意料之外的符号，返回一个token，id和type均置为None
******************************************************************************/
Symbol MatchById(SymbolsId symbol_id) {

	Symbol symbol;
	do {
		symbol = Next();
	} while (symbol.type == None && *str != '\0');

	if (*str == '\0') {
		symbol.id = symbol.type = symbol.val = 0;
		symbol.begin = symbol.end = NULL;
		cur_symbol = symbol;
		return symbol;
	}

	if (symbol.id != symbol_id) {
		printf("错误的预期符号 by id\n");
		symbol.id = symbol.type = None;
	}
	cur_symbol = symbol;
	return symbol;
}


/*****************************************************************************
 * 作用：获取下一个token，并判断是否为指定类型
 * 参数：SymbolType类型，为想要获取的token的type
 * 返回值：获取成功时返回获取到的token信息，否则表示获取到意料之外的符号，返回一个token，id和type均置为None
******************************************************************************/
Symbol MatchByType(SymbolsType symbol_type) {

	Symbol symbol;
	do {
		symbol = Next();
	} while (symbol.type == None && *str != '\0');

	if (*str == '\0') {
		symbol.id = symbol.type = symbol.val = 0;
		symbol.begin = symbol.end = NULL;
		cur_symbol = symbol;
		return symbol;
	}
	if (symbol.type != symbol_type) {
		printf("错误的预期符号 by type\n");
		symbol.id = symbol.type = None;
	}
	cur_symbol = symbol;
	return symbol;
}


/*****************************************************************************
 * 作用：获取下一个Id为指定Id的token,中间获取到的Id不为指定id的token都将被舍弃
 * 参数：SymbolId类型，为想要获取的token的id
 * 返回值：获取成功时返回获取到的token信息，否则表示遇到代码文本结尾空字符，返回一个token，该token所在内存被写0 TODO,测试赋值和memset速度
******************************************************************************/
Symbol MatchUntilById(SymbolsId symbol_id) {
	Symbol symbol;
	do {
		symbol = Next();
	} while (symbol.id != symbol_id && *str != '\0');
	if (*str == '\0') {
		symbol.id = symbol.type = symbol.val = 0;
		symbol.begin = symbol.end = NULL;
		cur_symbol = symbol;
		return symbol;
	}
	cur_symbol = symbol;

	return symbol;
}


/*****************************************************************************
 * 作用：获取下一个Type为指定Type的token,中间获取到的Type不为指定type的token都将被舍弃
 * 参数：SymbolType类型，为想要获取的token的类型
 * 返回值：获取成功时返回获取到的token信息，否则表示遇到代码文本结尾空字符，返回一个token，该token所在内存被写0 TODO,测试赋值和memset速度
******************************************************************************/
Symbol MatchUntilByType(SymbolsType symbol_type) {
	Symbol symbol;
	do {
		symbol = Next();
	} while (symbol.type != symbol_type && *str != '\0');
	if (*str == '\0') {
		symbol.id = symbol.type = symbol.val = 0;
		symbol.begin = symbol.end = NULL;
		cur_symbol = symbol;
	}
	return symbol;
}


/*****************************************************************************
 * 作用：获取下一个类型不为None的token，中间获取的类型为None的token都将被丢弃
******************************************************************************/
void IgnoreNoneToken() {
	Symbol symbol;
	do {
		symbol = Next();
	} while (symbol.type == None && *str != '\0');
	cur_symbol = symbol;
}


/*****************************************************************************
 * 作用：将代码文本指针移动到本行结尾的'\n'上,中间内容全部忽略
******************************************************************************/
void IgnoreOneLine() {
	while (*str != '\n' && *str != '\0') {
		str++;
	}
}

/*****************************************************************************
 * 作用：判断left_val op right_val 的结果
 * 参数1：左侧操作数
 * 参数2：比较运算符，可以是下列四种之一: <   <=   >   >=
 * 参数3：右侧操作数
 * 结果：如果left_val op right_val 的结果为真，返回1。否则返回0
******************************************************************************/
int CheckByTwoValue(int left_val,SymbolsId op, int right_val) {
	int result = 0;
	switch (op)
	{
	case LT: // <
		if (left_val < right_val) 
			result = 1;
		break;
	case LE: // <=
		if (left_val <= right_val)
			result = 1;
		break;
	case GT: // >
		if (left_val > right_val) 
			result = 1;
		break;
	case GE: // >=
		if (left_val >= right_val)
			result = 1;
		break;
	}
	return result;
}


/*****************************************************************************
 * 作用：从src中获取一个如下格式的判断语句（left_val op right_val），并交给 CheckByTwoValue 解析该语句结果
 * 结果：如果left_val op right_val 的结果为真，返回1。否则返回0
******************************************************************************/
int CheckExpression() {
		// MatchByType(Keyword);
		Symbol left_val = Match();
		Symbol op = Match();
		Symbol right_val = Match();

		// VAL op number
		if (left_val.type == Keyword) {
			return CheckByTwoValue(symbols[VAL].val, op.id, right_val.val);
		}
		// number op VAL
		else if (left_val.type == Number) {
			return CheckByTwoValue(right_val.val, op.id, symbols[VAL].val);
		}
		else {
			printf("错误的条件格式\n");
			return 0;
		}
}

/*****************************************************************************
 * 作用：不断从str中获取如下格式的判断语句（left_val op right_val）,并交给CheckExpression进行判断，
 *	     对返回的判断结果不断做相应位运算,直到获取到token ')'，判断结束，返回判断结果
 *
 * 结果：返回判断语句是否成立，成立返回1，不成立返回0
******************************************************************************/
int CheckAllExpression() {

	int ret = CheckExpression();

	while (1) {
		Match();
		switch (cur_symbol.id) {
		case LAND: {
			int rhs = CheckExpression();
			ret &= rhs;
			break;
		}
		case LOR: {
			int rhs = CheckExpression();
			ret |= rhs;
			break;
		}
		default:
			// RLB  ')'    异常的结尾会使cur_symbol.id 置为None,也会跳出while
			return ret;
			break;
		}

	}
	return ret;
}

/*****************************************************************************
 * 作用：语句分析，支持两种语句，if和函数调用, 根据当前token判断执行那种语句的处理
******************************************************************************/
void Statement(void) {

	if (cur_symbol.id == IF) {
		// 获取if 条件
		MatchById(LLB);
		int result = CheckAllExpression();

		// 条件成立需要执行if下的语句
		if (result == 1) {
			Match();

			//TODO		仅支持单条指令
			Statement();
			MatchUntilById(ELSE);
			IgnoreNoneToken();
			IgnoreOneLine();
			Match();
			Statement();

			return;
		}
		
		IgnoreNoneToken();
		IgnoreOneLine();
		Match();
		if (cur_symbol.id == ELSE) {
			MatchByType(Functional);
			Statement();
		}
		else {
			Statement();
		}

		return;
	}

	else if (cur_symbol.type == Functional) {
		switch (cur_symbol.id) {
		case FUNC1: {
			MatchById(LLB);
			Symbol arg1 = MatchByType(Number);
			MatchById(COMMA);
			Symbol arg2 = MatchByType(Number);
			MatchById(RLB);
			MatchById(SEMICOLON);
			func1(arg1.val, arg2.val);
			break;
		}
		case FUNC2:{
			MatchById(LLB);
			Symbol arg1 = MatchByType(Number);
			MatchById(RLB);
			MatchById(SEMICOLON);
			func2(arg1.val);
			break;
		}
		case FUNC3: {
			MatchById(LLB);
			Symbol arg1 = MatchByType(String);
			MatchById(RLB);
			MatchById(SEMICOLON);
			char* buf = RangePCharToPChar(arg1.begin, arg1.end);
			func3(buf);
			free(buf);
		}
		default:
			break;
		}
	}

}


// val 数值变化时调用, 设置str作为代码分析的入口
void AnalyseSliderVale(char *pstr,int val) {

	symbols[VAL].val = val;
	str = pstr;

	while (*str) {
		Symbol token = Match();

		
		switch (token.type) {
		case Number: {
			Print(token.begin, token.end);
			printf("意外获取到数字，值是%d\n", token.val);
			break;
		}
		case Keyword: {
			if (token.id == IF) {
				Statement();
			}
			else if (token.id == VAL) {
				printf("意外获取到关键字VAL,值为%d\n",symbols[VAL].val);
			}
			else {
				printf("意外获取到符号\n");
			}
			break;

		}
		case Value: {
			Print(token.begin, token.end);
			printf("意外获取到变量\n");
			break;
		}
		case Functional: {
			Statement();
			break;
		}

		default:
			break;
		}
	}
}

/*****************************************************************************
 * 作用：初始化符号表中的关键字,设置符号表中相应元素的类型为Keyword,id为相应SymbolsId,以及bgin和end用于获取hash
******************************************************************************/
void InitKeywordsFromSymbols(){
#define INIT_KEY_SYMBOLS(key,str,len)  \
	symbols[key].type = Keyword; \
	symbols[key].id = IF;\
	symbols[key].begin = &str[0];\
	symbols[key].end = symbols[key].begin + len;\

	INIT_KEY_SYMBOLS(IF,"if",2);
	INIT_KEY_SYMBOLS(VAL, "VAL", 3);
	INIT_KEY_SYMBOLS(ELIF, "elif", 4);
	INIT_KEY_SYMBOLS(ELSE, "else", 4);
#undef  INIT_KEY_WORDS
}

/*****************************************************************************
 * 作用：初始化符号表中的函数，设置符号表中相应元素的类型为Functional,id为相应SymbolsId,以及begin和end用于获取hash
******************************************************************************/
void InitFunctionalFromSymbols() {
#define INIT_FUNCTIONAL_SYMBOLS(key,str,len)  \
	symbols[key].type = Functional;\
	symbols[key].id = FUNC1;\
	symbols[key].begin = &#str [0];\
	symbols[key].end = symbols[key].begin + len;\
	symbols[key].val = (int)&str;

	INIT_FUNCTIONAL_SYMBOLS(FUNC1,func1,5);
	INIT_FUNCTIONAL_SYMBOLS(FUNC2,func2,5);
	INIT_FUNCTIONAL_SYMBOLS(FUNC3,func3,5);

#undef INIT_FUNCIONAL_SYMBOLS
}

/*****************************************************************************
 * 作用：初始化符号表
 *      调用 InitKeywordsFromSymbos 初始化符号表中的关键字，
 *      调用 InitFunctionalFromSymbols 初始化符号表中的函数
******************************************************************************/
void InitSymbol() {
	InitKeywordsFromSymbols();
	InitFunctionalFromSymbols();
}

int main(int argc,char *argv[]) {

	// TODO程序在解析之前需要初始化符号表一次
	InitSymbol();

	char* command = ReadFileToBuf("./command.txt");
	int val = 3;
	printf("val is %d\n", val);
	AnalyseSliderVale(command,val);
	free(command);

	return 0;
}