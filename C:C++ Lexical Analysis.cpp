#include <stdio.h>
#include <string.h>
#include <curses.h>

#define MAXIMUM_STR 256
#define MAXIMUM_KEYWORDS 62
#define MAXIMUM_OPERATORS 12
#define MAXIMUM_SEPARATORS 15

//DFA状态定义
#define STATE_START 1  //起始
#define STATE_IDorIDENTIFIER 2  //保留字标识符
#define STATE_NUMBER 3  //数字
#define STATE_NOTE 4  //注释
#define STATE_CONSTANT 5  //常量
#define STATE_DONE  6  //完成

#define TYPE_KEYWORD 1    //保留字
#define TYPE_IDENTIFIER 2 //标识符
#define TYPE_NUMBER 3     //数字
#define TYPE_NOTE 4       //注释
#define TYPE_CONSTANT 5   //常量
#define TYPE_OPERATOR 6   //运算符
#define TYPE_SEPARATOR 7  //分隔符
#define TYPE_ERROR 8      //错误
#define TYPE_UNKNOWN 9    //未知
#define TYPE_ENDFILE 10   //文件结束

char *Operators[MAXIMUM_OPERATORS] = {"+","-","*","/","%","=","==","!=","<","<=",">",">="};

char *Separators[MAXIMUM_SEPARATORS] ={",",";",".","\'","\"","(",")","[","]","{","}","//","/*","*/","#"};

char *Keywords[MAXIMUM_KEYWORDS] = {"include","define","auto","bool","break","case","catch","char","class",

                                "const","const_cast","continue","default","delete","do","double",

                                "dynamic_cast","else","enum","explicit","extern","false","float","for",

                                "friend","goto","if","inline","int","long","mutable","namespace","new",

                                "operator","private","protected","public","register","reinterpret_cast",

                                "return","short","signed","sizeof","static","static_cast","struct",

                                "switch","template","this","throw","true","try","typedef","typeid",

                                "typename","union","unsigned","using","virtual","void","volatile","while"};


bool Is_Operators(char c)//判断是否为运算符
{
    int i;
    for (i = 0; i < MAXIMUM_OPERATORS; ++i) {
        if(Operators[i][0]==c)
            return 1;
    }
    return 0;
}

bool Is_Separators(char c)
{
    int i;
    for (i = 0; i < MAXIMUM_SEPARATORS; ++i) {
        if(Separators[i][0]==c)
            return 1;
    }
    return 0;
}

bool Is_Keywords(char *str)
{
    int i;
    for (i = 0; i <MAXIMUM_KEYWORDS ; ++i)
    {
        if(strcmp(Keywords[i],str)==0)
            return 1;
    }
    return 0;
}

void Output_OneWord(FILE *outfile,int type,char *str)
{
    switch(type)
    {
        case TYPE_KEYWORD:    fprintf(outfile,"( KEYWORD , ");break;
        case TYPE_IDENTIFIER: fprintf(outfile,"( IDENTIFIER , ");break;
        case TYPE_NUMBER:     fprintf(outfile,"( NUMBER , ");break;
        case TYPE_NOTE:       fprintf(outfile,"( NOTE , ");break;
        case TYPE_CONSTANT:   fprintf(outfile,"( CONSTANT , ");break;
        case TYPE_OPERATOR:   fprintf(outfile,"( OPERATOR , ");break;
        case TYPE_SEPARATOR:  fprintf(outfile,"( SEPARATOR , ");break;
        case TYPE_ERROR:      fprintf(outfile,"( ERROR , ");break;
        case TYPE_UNKNOWN:    fprintf(outfile,"( UNKNOWN , ");break;
        default:break;
    }
    fprintf(outfile,"%s )\n",str);
}

void Analyse_Lexical(FILE *infile,FILE *outfile)
{
    char peek;//字符读入存储变量
    char str[MAXIMUM_STR];//存储过程字符串
    int local;//过程位置标记数（标记读到词的第几位）
    int Line_No=1;//行号
    int State;//状态
    int Type;//类型
    int flag_Note;//注释行数标记,限制当注释符为//时，注释只能有一行
    int flag_Dot;//小数点数标记
    fprintf(outfile,"Line No.%d---------------------------------------\n",Line_No);
    while(!feof(infile))
    {
        local=0;
        State=STATE_START;
        flag_Note=0;
        flag_Dot=0;
        while(State!=STATE_DONE)
        {
            peek=fgetc(infile);
            switch(State)
            {
                case STATE_START:
                    if (peek==' ');
                    else if(peek=='\n')
                    {
                        Line_No++;
                        fprintf(outfile,"Line No.%d---------------------------------------\n",Line_No);
                    }
                    else if((peek>='a'&&peek<='z')||(peek>='A'&&peek<='Z')||peek=='_')
                    {
                        State=STATE_IDorIDENTIFIER;//开头为字母或下划线的可能为标识符或下划线，进入标识符的DFA
                        Type=TYPE_IDENTIFIER;//type先置为IDENTIFIER，若为保留字，再做修改
                        str[local]=peek;
                        local++;
                    }
                    else if(peek>='0'&&peek<='9')
                    {
                        State=STATE_NUMBER;//进入数字的DFA
                        Type=TYPE_NUMBER;
                        str[local]=peek;
                        local++;
                    }
                    else if(peek=='/')
                    {
                        str[local]=peek;
                        local++;
                        peek=fgetc(infile);
                        if(peek=='/')//为//型注释
                        {
                            flag_Note=1;
                            State=STATE_NOTE;
                            Type=TYPE_NOTE;
                            str[local]=peek;
                            local++;
                        }
                        else if(peek=='*')//为/*型注释
                        {
                            State=STATE_NOTE;
                            Type=TYPE_NOTE;
                            str[local]=peek;
                            local++;
                        }
                        else//为运算符除号
                        {
                            State=STATE_DONE;
                            Type=TYPE_OPERATOR;
                            fseek(infile,-1,SEEK_CUR);
                            local=1;
                        }

                    }
                    else if(peek=='<'||peek=='>')
                    {
                        int f1;//记录前一个符号的开口方向
                        if(peek=='<')
                        {
                            f1=0;
                        }
                        else
                        {
                            f1=1;
                        }
                        State=STATE_DONE;
                        Type=TYPE_OPERATOR;
                        str[local]=peek;
                        local++;
                        peek=fgetc(infile);
                        if(peek=='=')
                        {
                            str[local]=peek;
                            local=2;
                        }
                        else if(peek=='<'&&f1==0)
                        {
                            str[local]=peek;
                            local=2;
                        }
                        else if(peek=='>'&&f1==1)
                        {
                            str[local]=peek;
                            local=2;
                        }
                        else if(peek=='<'&&f1==1)
                        {
                            State=STATE_DONE;
                            Type=TYPE_ERROR;
                            str[local]=peek;
                            local=2;
                        }
                        else if(peek=='>'&&f1==0)
                        {
                            State=STATE_DONE;
                            Type=TYPE_ERROR;
                            str[local]=peek;
                            local=2;
                        }
                        else
                        {
                            fseek(infile,-1,SEEK_CUR);
                            local=1;
                        }
                    }
                    else if(peek=='!')
                    {
                        State=STATE_DONE;
                        str[local]=peek;
                        peek=fgetc(infile);
                        local++;
                        if(peek=='=')
                        {
                            Type=TYPE_OPERATOR;
                            str[local]=peek;
                            local=2;
                        }
                        else
                        {
                            Type=TYPE_UNKNOWN;
                            fseek(infile,-1,SEEK_CUR);
                            local=1;
                        }
                    }
                    else if(peek=='+')
                    {
                        State=STATE_DONE;
                        str[local]=peek;
                        peek=fgetc(infile);
                        local++;
                        if(peek=='+')
                        {
                            Type=TYPE_OPERATOR;
                            str[local]=peek;
                            local=2;
                        }
                        else
                        {
                            Type=TYPE_OPERATOR;
                            fseek(infile,-1,SEEK_CUR);
                            local=1;
                        }
                    }
                    else if(peek=='\''||peek=='\"')
                    {
                        State=STATE_CONSTANT;
                        Type=TYPE_CONSTANT;
                        str[local]=peek;
                        local=1;
                    }
                    else if(Is_Operators(peek))
                    {
                        State=STATE_DONE;
                        Type=TYPE_OPERATOR;
                        str[local]=peek;
                        local=1;
                    }
                    else if(Is_Separators(peek))
                    {
                        State=STATE_DONE;
                        Type=TYPE_SEPARATOR;
                        str[local]=peek;
                        local=1;
                    }
                    else if(peek==EOF)
                    {
                        State=STATE_DONE;
                        Type=TYPE_ENDFILE;
                    }
                    else
                    {
                        State=STATE_DONE;
                        Type=TYPE_UNKNOWN;
                    }

                    break;//case STATE_START

                case STATE_IDorIDENTIFIER:
                    if((peek>='a'&&peek<='z')||(peek>='A'&&peek<='Z')||(peek>='0'&&peek<='9')||peek=='_')
                    {
                        str[local]=peek;
                        local++;
                    }
                    else if(peek==' '||peek=='\n'||Is_Separators(peek)||Is_Operators(peek))
                    {
                        State=STATE_DONE;
                        fseek(infile,-1,SEEK_CUR);
                    }
                    else
                    {
                        State=STATE_DONE;
                        Type=TYPE_ERROR;
                    }

                    break;//case STATE_IDorIDENTIFIER

                case STATE_NUMBER:
                    if(peek>='0'&&peek<='9')
                    {
                        str[local]=peek;
                        local++;
                    }
                    else if(peek=='.')
                    {
                        str[local]=peek;
                        local++;
                        peek=fgetc(infile);
                        if(flag_Dot==0)
                        {
                            if(peek>='0'&&peek<='9')
                            {
                                str[local]=peek;
                                local++;
                                Type=TYPE_NUMBER;
                                flag_Dot=1;
                            }
                            else
                            {
                                fseek(infile,-1,SEEK_CUR);
                                State=STATE_DONE;
                                Type=TYPE_ERROR;
                            }
                        }
                        else
                        {
                            Type=TYPE_ERROR;
                            str[local]=peek;
                            local++;
                        }
                    }
                    else if(peek==' '||peek=='\n'||Is_Operators(peek)||Is_Separators(peek))
                    {
                        State=STATE_DONE;
                        fseek(infile,-1,SEEK_CUR);
                    }
                    else
                    {
                        str[local]=peek;
                        local++;
                        Type=TYPE_ERROR;
                    }
                    break; //case STATE_NUMBER

                case STATE_NOTE:
                    if(flag_Note==1)
                    {
                        if(peek=='\n')
                        {
                            State=STATE_DONE;
                            fseek(infile,-1,SEEK_CUR);
                        }
                        else
                        {
                            str[local]=peek;
                            local++;
                        }
                    }
                    else
                    {
                        if(peek=='\n')
                        {
                            Line_No++;
                            str[local]=peek;
                            local++;
                        }
                        else if(peek=='*')
                        {
                            str[local]=peek;
                            local++;
                            peek=fgetc(infile);
                            if(peek=='/')
                            {
                                State=STATE_DONE;
                                str[local]=peek;
                                local++;
                            }
                            else
                            {
                                fseek(infile,-1,SEEK_CUR);
                            }
                        }
                        else
                        {
                            str[local]=peek;
                            local++;
                        }
                    }
                    break;//case STATE_NOTE
                case STATE_CONSTANT:
                    if(peek=='\n')
                    {
                        Line_No++;
                        str[local]=peek;
                        local++;
                    }
                    else if(peek=='\''||peek=='\"')
                    {
                        State=STATE_DONE;
                        str[local]=peek;
                        local++;
                    }
                    else if(peek=='\\')
                    {
                        str[local]=peek;
                        local++;
                        peek=fgetc(infile);
                        str[local]=peek;
                        local++;
                    }
                    else
                    {
                        str[local]=peek;
                        local++;
                    }
                    break;//case STATE_CONSTANT

                case STATE_DONE:
                    break;//case STATE_DONE

                default:
                    break;

            }

        }
        str[local]='\0';
        if(Type==TYPE_IDENTIFIER)
        {
            if(Is_Keywords(str))
            {
                Type=TYPE_KEYWORD;
            }
        }
        Output_OneWord(outfile,Type,str);
    }
}

int main()
{
    FILE *infile,*outfile;
    infile=fopen("/Users/shangyigeng/Desktop/test.cpp","r");
    outfile=fopen("/Users/shangyigeng/Desktop/analyzeresult.txt","w");
    Analyse_Lexical(infile,outfile);
    fprintf(outfile,"-------------------------------end of file\n");
    fclose(infile);
    fclose(outfile);
    printf("SUCCESS!");
    return 0;
}