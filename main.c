#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*****************************************************************************
 * BNF ���£�
 * 
 *	<if statement>		:= 'if' '(' <check_statement> ')' <statement>+ {<elif_statement>} {<else_statement>} 'fi'
 *	<check_statement>	:= <left_val> <op> <right_val> { <LOR> <left_val> <op> <right_val> }
 *	<left_val>			:= number | 'VAL'
 *	<right_val>			:= number | 'VAL'
 *	<op>				:= '&&' | '||' | '=='
 *	<elif_statement>	:= 'elif' '(' <check_statement> ')' <statement>+ {<elif_statement>} {<else_statement>}
 *	<else_statement>	:= 'else'	<statement>+
 *	<statement>			:= functional([arg] {',' arg}) ';'
 *
******************************************************************************/

// ��Ӻ�������
// 1���ڷ���id����Ӹú���������ΪID					ID��ͨ����һ����Ŀ����
// 2����InitSymbolFromFuntional����Ӻ���
// 3����next��ȡ������tokenʱ�Ըú������бȽ�			case �е����ݿ�ͨ����һ����Ŀ����

typedef enum {
	None, Number, String, Value, Functional, Keyword, Cmp,
}SymbolsType;


typedef enum {
	// NONE = 0,������ʶ����
	IF = 1, ELSE, ELIF, FI, VAL, LLB, RLB, LT, GT, LE, GE, ASSIGNMENT, EQUAL, COMMA, SEMICOLON, AND, OR, LAND, LOR,
	FUNC1 = 32, FUNC2, FUNC3
}SymbolsId;

typedef struct Symbol {
	SymbolsType type;
	char* begin, *end;
	int id;	 
	int val; 
}Symbol;

// SymbolId ���Ķ�����ȫ��д����Ӧ��hash�ô�Сд��д
// HashEnum Ӧ��ʹ����һ����Ŀ���ɣ���ֹ��������Լ�hash��ͻ
enum HashEnum {
	Fi = 597,
	If = 624,
	Val = 3998,
	Elif = 59624,
	Else = 59723,
	Func1 = 615711,
	Func2 = 615712,
	Func3 = 615713,
};

/*****************************************************************************
 * ���ã�  ����hash��hash������Next�ʷ������п����жϵ�ǰtoknΪ�ĸ��ؼ��ֻ��ߺ���
 * ������  ����һ��NUL��β���ַ���
 * ����ֵ�����ݲ����õ���hash
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
 * ���ڼ���hash���������ڱ���������ʹ������char *����Χ������NUL��β
******************************************************************************/
int RangeStringToHash(char* begin, char* end) {
	int ret_hash = 0;
	while (begin != end) {
		ret_hash = ret_hash * 10 + *begin - '0';
		begin++;
	}
	return ret_hash;
}

extern int code_line;
/*****************************************************************************
 * ���Ժ���,func1,func2,func3,���Ժ������ú�ƥ�䲻ͬ���͵Ĳ���
******************************************************************************/
void func1(int arg1, int arg2) {
	printf("func1�����ڵ�%d�б����ã�����1Ϊ%d������2Ϊ%d\n",code_line,arg1,arg2);
}
void func2(int arg) {
	printf("func2�����ڵ�%d�б����ã�����Ϊ%d\n",code_line,arg);
}
void func3(char* str) {
	printf("func3�ڵ�%d�б����ã�������%s\n",code_line, str);
}

/*****************************************************************************
 * ���Ժ���,�������ٴ�ӡtoken
******************************************************************************/
void Print(char* begin, char* end) {
	char buf[255] = "";
	strncpy(buf, begin, end - begin);
	printf("%s \n",buf);
}

/*****************************************************************************
 * ���Ժ���,�������ı���ȡ�����ı�,���ص�char *��malloc��ȡ��ʹ�����Ӧ������free�ͷ�
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
 * ���ã�����ȡ��ʹ������ָ���ʾ���ַ���tokenתΪ��NUL��β���ַ���
 * ������begin��ʾ��Ҫת�����ַ����Ŀ�ͷ��end��ʾ��Ҫת�����ַ����Ľ�β
 * ����ֵ��һ��malloc������char *������Ϊbegin��end֮ǰ���ַ������һ���ֽ�ΪNUL ����ָ��Ϊmalloc��ָ�룬ʹ�ú���Ҫ�ͷţ�
******************************************************************************/
char *RangePCharToPChar(char *begin,char *end) {
	char* ret = malloc(end - begin + 1);
	if (ret == NULL)
		return NULL;
	strncpy(ret, begin, end - begin);
	ret[end - begin] = '\0';
	return ret;
}

// ȫ�ֱ�������
// ���ű����ŵ�ʵ��ֻ�� �ؼ���if elif else �� ���� ����֧�ֱ����������Բ����ڱ���
Symbol symbols[128], cur_symbol;
char* str;
int code_line;

/*****************************************************************************
 * ���ã��ʷ�����
 * ����ֵ�����ݵ�ǰ�����token������Ӧ��symbol����symbol�б����token��type��id��val����Ϣ
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


			// ���������������
			// 1���ؼ��� if elif else VAL
			// 2��������
			switch (RangeStringToHash(token.begin,token.end)) {
			// �ؼ���
			case If:
				token.type = Keyword;
				token.id = IF;
				return token;
			case Elif:
				token.type = Keyword;
				token.id = IF;	// elif ����Ϊ�� if
				return token;
			case Else:
				token.type = Keyword;
				token.id = ELSE;
				return token;
			case Fi:
				token.type = Keyword;
				token.id = FI;
				return token;
			case Val:
				token.type = Keyword;
				token.id = VAL;
				return token;

			// ����
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
				printf("����������������֧�ֱ�������");
				token.type = Value;
				return token;
			}
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
		case '=': {
			token.begin = str;
			if (*(str + 1) == '=') {
				token.id = EQUAL;
				str++;
			}
			else {
				token.id = ASSIGNMENT;
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
			// begin ָ���һ��",endָ��ڶ���"�ĺ�һ��Ԫ��
			token.type = String;
			token.begin = ++str;
			while (*str != '"') {
				str++;
			}
			token.end = str++;
			return token;

		case '\n':
			code_line++;
		case ' ':
		case '\t':
		case '\r':
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
			printf("next��֧�ָ�token\n");
			return token;
		}
	}
	return token;
}


/*****************************************************************************
 * ���ã���ȡ��һ�����ͷǿ�(������Ϊ�հ׷����Ʊ�������з���)��token
 * ����ֵ����ȡ����token��Ϣ
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
 * ���ã���ȡ��һ��token�����ж��Ƿ�Ϊָ��Id
 * ������SymbolId���ͣ�Ϊ��Ҫ��ȡ��token��id
 * ����ֵ����ȡ�ɹ�ʱ���ػ�ȡ����token��Ϣ�������ʾ��ȡ������֮��ķ��ţ�����һ��token��id��type����ΪNone
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
		printf("�����Ԥ�ڷ��� by id\n");
		symbol.id = symbol.type = None;
	}
	cur_symbol = symbol;
	return symbol;
}


/*****************************************************************************
 * ���ã���ȡ��һ��token�����ж��Ƿ�Ϊָ������
 * ������SymbolType���ͣ�Ϊ��Ҫ��ȡ��token��type
 * ����ֵ����ȡ�ɹ�ʱ���ػ�ȡ����token��Ϣ�������ʾ��ȡ������֮��ķ��ţ�����һ��token��id��type����ΪNone
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
		printf("�����Ԥ�ڷ��� by type\n");
		symbol.id = symbol.type = None;
	}
	cur_symbol = symbol;
	return symbol;
}


/*****************************************************************************
 * ���ã���ȡ��һ��IdΪָ��Id��token,�м��ȡ����Id��Ϊָ��id��token����������
 * ������SymbolId���ͣ�Ϊ��Ҫ��ȡ��token��id
 * ����ֵ����ȡ�ɹ�ʱ���ػ�ȡ����token��Ϣ�������ʾ���������ı���β���ַ�������һ��token����token�����ڴ汻д0 TODO,���Ը�ֵ��memset�ٶ�
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
 * ���ã���ȡ��һ��TypeΪָ��Type��token,�м��ȡ����Type��Ϊָ��type��token����������
 * ������SymbolType���ͣ�Ϊ��Ҫ��ȡ��token������
 * ����ֵ����ȡ�ɹ�ʱ���ػ�ȡ����token��Ϣ�������ʾ���������ı���β���ַ�������һ��token����token�����ڴ汻д0 TODO,���Ը�ֵ��memset�ٶ�
******************************************************************************/
Symbol MatchUntilByType(SymbolsType symbol_type) {
	Symbol symbol;
	do {
		symbol = Next();
	} while (symbol.type != symbol_type && *str != '\0');
	if (*str == '\0') {
		symbol.id = symbol.type = symbol.val = 0;
		symbol.begin = symbol.end = NULL;
	}
	cur_symbol = symbol;
	return symbol;
}


Symbol MatchUntilByIF() {
	Symbol symbol;
	do {
		symbol = Next();
	} while (!(symbol.id == IF || symbol.id == ELSE || symbol.id == FI)
		&& *str != '\0');
	if (*str == '0') {
		memset(&symbol,0,sizeof(Symbol));
	}
	cur_symbol = symbol;
	return symbol;
}


/*****************************************************************************
 * ���ã���ȡ��һ�����Ͳ�ΪNone��token���м��ȡ������ΪNone��token����������
******************************************************************************/
void IgnoreNoneToken() {
	Symbol symbol;
	do {
		symbol = Next();
	} while (symbol.type == None && *str != '\0');
	cur_symbol = symbol;
}


/*****************************************************************************
 * ���ã��������ı�ָ���ƶ������н�β��'\n'��,�м�����ȫ������
******************************************************************************/
void IgnoreOneLine() {
	while (*str != '\n' && *str != '\0') {
		str++;
	}
}

/*****************************************************************************
 * ���ã��ж�left_val op right_val �Ľ��
 * ����1����������
 * ����2���Ƚ����������������������֮һ: <   <=   >   >=
 * ����3���Ҳ������
 * ��������left_val op right_val �Ľ��Ϊ�棬����1�����򷵻�0
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
	case EQUAL:
		if (left_val == right_val)
			result = 1;
		break;
	}

	return result;
}


/*****************************************************************************
 * ���ã���src�л�ȡһ�����¸�ʽ���ж���䣨left_val op right_val���������� CheckByTwoValue �����������
 * ��������left_val op right_val �Ľ��Ϊ�棬����1�����򷵻�0
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
			return CheckByTwoValue(left_val.val, op.id, symbols[VAL].val);
		}
		else {
			printf("�����������ʽ\n");
			return 0;
		}
}

/*****************************************************************************
 * ���ã����ϴ�str�л�ȡ���¸�ʽ���ж���䣨left_val op right_val��,������CheckExpression�����жϣ�
 *	     �Է��ص��жϽ����������Ӧλ����,ֱ����ȡ��token ')'���жϽ����������жϽ��
 *
 * ����������ж�����Ƿ��������������1������������0
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
			// RLB  ')'    �쳣�Ľ�β��ʹcur_symbol.id ��ΪNone,Ҳ������while
			return ret;
			break;
		}

	}
	return ret;
}

typedef struct {
	int val;
	int is_string;
}Arg;
Arg args[16];
void MatchAllArg() {
	int i = 0;
	// ��һ��match��ƥ�����(����֮���ƥ��','���ߡ�����
	while (Match().id != RLB) {
		// �жϣ���������־�ֱ�Ӹ�ֵ
		Symbol arg = Match();
		if (arg.type == Number) {
			args[i].val = arg.val;
		}
		else if (arg.type == String) {
			args[i].val = RangePCharToPChar(arg.begin, arg.end);
			args[i].is_string = 1;
		}
		else {
			printf("����Ĳ���\n");
		}
		// ������ַ�����mallocһ���ڴ棬ָ���args[i].val , args[i].flag ��Ϊ 1����ʾ���ַ���
	}
}

/*****************************************************************************
 * ���ã���������֧��������䣬if�ͺ�������, ���ݵ�ǰtoken�ж�ִ���������Ĵ���
******************************************************************************/
void Statement(void) {

	if (cur_symbol.id == IF) {
		// ��ȡif ����
		MatchById(LLB);
		int result = CheckAllExpression();

		// ����������Ҫִ��if�µ����
		if (result == 1) {
			//TODO		��֧�ֵ���ָ��
			while (Match().type == Functional)
				Statement();				

			MatchUntilById(FI);

			return;
		}
		
		// ���if δ���У���ƥ�䵽��һ��elif��if����else ��fi
		// MatchToIfEnd()
		switch (MatchUntilByIF().id) {

		case IF:
			Statement();
			return;
		case ELSE: {
			while (Match().type == Functional)
				Statement();				
			return;
		}
		case FI:
			return;
		default:
			printf("������������Ʒ�\n");
			return;
		}

		return;
	}

	else if (cur_symbol.type == Functional) {
	/*
		int arg1,arg2,arg3,arg4,arg5,arg6,arg7;
		while (MatchArg())	// �������в����������η���arg1 - arg7
			;
		// val�д�ŵ��Ǻ�����ַ
		cur_symbol.val(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
	*/
		void (*func)() = symbols[cur_symbol.id].val;
		MatchAllArg();
		func(args[0], args[1], args[2], args[3], args[4]);
		MatchById(SEMICOLON);


		//switch (cur_symbol.id) {
		//case FUNC1: {
		//	MatchById(LLB);
		//	Symbol arg1 = MatchByType(Number);
		//	MatchById(COMMA);
		//	Symbol arg2 = MatchByType(Number);
		//	MatchById(RLB);
		//	MatchById(SEMICOLON);
		//	func1(arg1.val, arg2.val);
		//	break;
		//}
		//case FUNC2: {
		//	MatchById(LLB);
		//	Symbol arg1 = MatchByType(Number);
		//	MatchById(RLB);
		//	MatchById(SEMICOLON);
		//	func2(arg1.val);
		//	break;
		//}
		//case FUNC3: {
		//	MatchById(LLB);
		//	Symbol arg1 = MatchByType(String);
		//	MatchById(RLB);
		//	MatchById(SEMICOLON);
		//	char* buf = RangePCharToPChar(arg1.begin, arg1.end);
		//	func3(buf);
		//	free(buf);
		//}
		//default:
		//	break;
		//}
	}

}


// val ��ֵ�仯ʱ����, ����str��Ϊ������������
void AnalyseSliderVale(char *pstr,int val) {

	// ÿ����������ʱ����Ҫ���³�ʼ��
	symbols[VAL].val = val;
	str = pstr;
	code_line = 1;

	while (*str) {
		Symbol token = Match();

		
		switch (token.type) {
		case Number: {
			Print(token.begin, token.end);
			printf("�����ȡ�����֣�ֵ��%d\n", token.val);
			break;
		}
		case Keyword: {
			if (token.id == IF) {
				Statement();
			}
			else if (token.id == VAL) {
				printf("�����ȡ���ؼ���VAL,ֵΪ%d\n",symbols[VAL].val);
			}
			else {
				printf("�����ȡ������\n");
			}
			break;

		}
		case Value: {
			Print(token.begin, token.end);
			printf("�����ȡ������\n");
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
 * ���ã���ʼ�����ű��еĹؼ���,���÷��ű�����ӦԪ�ص�����ΪKeyword,idΪ��ӦSymbolsId,�Լ�bgin��end���ڻ�ȡhash
******************************************************************************/
void InitKeywordsFromSymbols(){
#define INIT_KEY_SYMBOLS(key,str,len)  \
	symbols[key].type = Keyword; \
	symbols[key].id = IF;\
	symbols[key].begin = &str[0];\
	symbols[key].end = symbols[key].begin + len;\

	INIT_KEY_SYMBOLS(IF,"if",2);
	INIT_KEY_SYMBOLS(IF,"fi",2);
	INIT_KEY_SYMBOLS(VAL, "VAL", 3);
	INIT_KEY_SYMBOLS(ELIF, "elif", 4);
	INIT_KEY_SYMBOLS(ELSE, "else", 4);
#undef  INIT_KEY_WORDS
}

/*****************************************************************************
 * ���ã���ʼ�����ű��еĺ��������÷��ű�����ӦԪ�ص�����ΪFunctional,idΪ��ӦSymbolsId,�Լ�begin��end���ڻ�ȡhash
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
 * ���ã���ʼ�����ű�
 *      ���� InitKeywordsFromSymbos ��ʼ�����ű��еĹؼ��֣�
 *      ���� InitFunctionalFromSymbols ��ʼ�����ű��еĺ���
******************************************************************************/
void InitSymbol() {
	InitKeywordsFromSymbols();
	InitFunctionalFromSymbols();
}

int main(int argc,char *argv[]) {

	// TODO�����ڽ���֮ǰ��Ҫ��ʼ�����ű�һ��
	InitSymbol();

	for (int i = 0; i < 6; ++i) {
		char* command = ReadFileToBuf("./command.txt");
		int val = i;
		printf("val is %d\n", val);
		AnalyseSliderVale(command,val);
		free(command);

		for (long long tmp = 0; tmp < 10000000000; ++tmp)
			;

		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

	}


	return 0;
}