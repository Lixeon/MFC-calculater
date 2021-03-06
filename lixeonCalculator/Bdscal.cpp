// Bdscal.cpp : 实现文件
//
#include "stdafx.h"

#include <stack>

#include "lixeonCalculator.h"
#include "Bdscal.h"
#include "math.h"
#include <cstring>


const double inf = 1e11;//极大随机值
const double eps = 1e-6; //eps 调整精度
const int MAXFUN = 3;//目前提供三个科学计算
 

//如果运算中出现很小的负数可能会出问题
//因为符号的hash也映射成负数的
//运算符的hash映射有可能和操作数重合
//根据情况改定义const double inf和const double eps以便于极大极小数据运算

#define HASHA (-inf+1)
#define HASHS (-inf+2)
#define HASHM (-inf+3)
#define HASHD (-inf+4)
#define HASHL (-inf+5)
#define ERRORX (-inf+6)
 

using namespace std;
 
static char MathFun[][4]={"sin","cos","tan"};
 

// Bdscal 对话框


IMPLEMENT_DYNAMIC(Bdscal, CDialog)

Bdscal::Bdscal(CWnd* pParent /*=NULL*/)
	: CDialog(Bdscal::IDD, pParent)
	, str(_T("\n请输入数学表达式,在最后用 = 结束\r\n支持sin,cos,tan三角函数混合运算以及多重括号判别\r\n若有错误则会提示,需返回重新开始\r\n例如:输入 1+5/10-sin(3.1415926/2)= \r\n输出为0.5\r\n请将本内容清空后输入表达式"))
{//\n回车 \r换行  这里需要\r\n来表示回车换行显示内容
}

Bdscal::~Bdscal()
{
}

void Bdscal::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, str);
}



BEGIN_MESSAGE_MAP(Bdscal, CDialog)
	ON_BN_CLICKED(IDOK, &Bdscal::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &Bdscal::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EDIT1, &Bdscal::OnEnChangeEdit1)
END_MESSAGE_MAP()




// Bdscal 消息处理程序
void Bdscal::OnBnClickedOk()
{
	CString temp;

	GetDlgItem(IDC_EDIT1)->GetWindowText(str);  //获得控件变量

	temp = str;

    memcpy(Exp,str,str.GetLength());  //拷贝到Exp字符数组

    Cac(); //进行运算以及表达式错误判断
	
	str.Format(_T("%g"),Ans);   //格式化输出

	str = str + "\r\n" +  temp ;  //显示第二行

	UpdateData(false);  //显示

}

bool Bdscal::Cac(){

    DelandLower(Exp);   //删杂

    int len=str.GetLength();   //获取表达式长度

    if(!Check(Exp,len)) return 0;   //判断表达式是否正确

    Ans=GetV(Exp,0,len);  //计算结果
    if(Equal(Ans,ERRORX))

        return 0;

    return 1;
}


//定义哈希
double Bdscal::hash(char c){
    switch(c){
        case '+':return HASHA;
        case '-':return HASHS;
        case '*':return HASHM;
        case '/':return HASHD;
        default :return HASHL;
    }
}
 
int Bdscal::Prio(double x){

    if(x<-inf+3-eps)  //代表加法和减法

        return 1;

    if(x<-inf+5-eps) //乘法和除法

        return 2;

    return 3;
}
 

//把表达式中空格和一些无效字符删除　字符全部变成小写　便于后面比较
void Bdscal::DelandLower(char *str){

    int i,j;

	//遍历
    for(i=j=0;*(str+i);i++){

        if(*(str+i)==' ' || *(str+i)=='\t')

            continue;

        if(*(str+i)>='A' && *(str+i)<='Z')

            *(str+i)+='a'-'A';
		
		*(str+j)=*(str+i);

        j++;

    }

    *(str+j)=0;
}


//查错 查错分 是否有非法字符，是否括号匹配 ，是否缺少运算符等等
bool Bdscal::Check(char *str,int & len){


	//长度不足1以及1的
    if(len<(1<<1)){
		//这里原本想用UpdateData 然后发现后面执行函数会覆盖掉现在的显示结果
        MessageBox("表达式长度异常");
		return 0;
    }

	//表达式结尾不是 =  或者倒数第二个字符 是 运算符
    if(str[len-1]!='=' || Operat(str[len-2])){

        MessageBox("表达式结尾错误");

        return 0;
    }

    str[--len] = 0;  //最后一位初始化

	//判断表达式主体是否有误
    if(!CheckCh(str,0) || !CrectB(str) || !CheckError(str,len))
        return 0;
    return 1;
}

//是否为数字
bool Bdscal::Is_Num(char c){

    return c >= 48 && c <= 57;
}

bool Bdscal::Operat(char c){
    switch(c){
        case '+':
        case '-':
        case '*':
        case '/':return 1;
        default :return 0;
    }
}

bool Bdscal::CheckCh(const char *str,int pos/*=0*/)
{
    int i,j,k; //i扫描到字符串第i个字符，j控制行，k控制列

	CString tmp; //消息盒子临时变量
 
	//表达式从0位置开始判断
    for( i = pos;*(str+i);i++){

		
        if(Is_Num(*(str+i))
            || Operat(*(str+i)) || *(str+i)=='.'
            || *(str+i)=='(' || *(str+i)==')')
            continue;

           for(j=0;j<MAXFUN;j++)
		   {
               for(k=0;k<MAXFUN;k++)
			   {
                   if(*(str+i+k) != *(*(MathFun+j)+k)) //递归调用MathFun 检查是否匹配数学函数
                          break;
               }
               if(k>=3)//目前就三个科学运算 且皆为三个字符
                    break;
           }
		   //不是三个科学计算任何一个
           if(j>=3)
		   {
                tmp.Format("%d位置出现非法字符",i);
		    	MessageBox(tmp);
                return 0;
           }
           else
		   {
                if(*(str+i+3)!='('){
			   	tmp.Format("%d位置缺少左括号",i+3);
			    MessageBox(tmp);
                return 0;
            }
            return CheckCh(str,i+3); //递归继续进行判断
        }
    }
    return 1;
}
 


//利用栈判断括号是否匹配
//顺便记录每对括号匹配的位置，因为后面要用，有递归实现求值功能
bool Bdscal::CrectB(const char *str)
{

    stack<int> s; //声明int型元素栈

 	CString tmp;//消息盒子临时变量

	//遍历表达式字符数组
    for(int i=0;*(str+i);i++)
	{
        if(*(str+i) != '('  && *(str+i)!=')')
            continue;
        
		//找到左括号压入栈
		if(*(str+i)=='(')
		{
            s.push(i);
	    }
        else
		
          if(s.empty()) //是否为空
		  {
		    	tmp.Format("%d位置出现多余右括号",i);
			    MessageBox(tmp);
                return 0;
          }
          else
		  {
            NextB[s.top()] = i; //返回一个在栈顶元素的应用 记录每对括号匹配的位置
            s.pop(); //弹出栈顶元素
          }
	}
    if( !s.empty() )
	{
		tmp.Format("%d位置出现多余左括号",s.top());
		MessageBox(tmp);
		return 0;
    }
    return 1;
}
 

//判断表达式主体正确性
bool Bdscal::CheckError(const char *str,int len)
{

	CString tmp;

	for(int i=0;i<len;i++){
		if(*(str+i)=='('){
			if(i<len-1 && Operat(str[i+1]) && str[i+1]!='-'){
				tmp.Format("%d位置缺少运算符",i+1);
				MessageBox(tmp);
				return 0;
			}
			if(i>0 && (Is_Num(str[i-1]) || str[i-1]==')')){
				tmp.Format("%d位置缺少运算符",i);
				MessageBox(tmp);
			}
		}
		else
			if(*(str+i)==')'){
				if(i>0 && (Operat(str[i-1]) || str[i-1]=='(')){
					if(Operat(str[i-1]))
					{
						tmp.Format("%d位置缺少运算符",i);
						MessageBox(tmp);
					}
					else
					{
						tmp.Format("%d位置缺少数字",i);
						MessageBox(tmp);
						return 0;}
					}
					if(i<len-1 && Is_Num(str[i+1])){
						tmp.Format("%d位置缺少运算符",i+1);
						MessageBox(tmp);
						return 0;
					}
				}
				else
					if(i>0 && Operat(*(str+i)) && Operat(str[i-1])){
						tmp.Format("%d位置缺少数字",i);
						MessageBox(tmp);
						return 0;
        }
    }
    return 1;
}
 
//计算数值的话，考虑有括号和+-*/优先级 采用后缀表达式计算，也就是逆波兰式
//把运算符映射成很小的实数
//保证这些实数不会出现在操作数里面，具体的逆波兰式计算表达式
double Bdscal::GetV(const char *str,int st/*头*/,int ed/*尾*/){

    struct P{
        double x,flag;
        bool point;
        int sign;
        P(){Init();}
        void Init()
		{
			x=0.0;flag=1e-1;
			sign=1;point=0;//　 x数值 flag精度 sign标识  point 小数点
        }
    }Num;//struct 定义P类  默认public权限

    stack<double> S; //double型元素栈

    double *Suffix=new double[ed-st+1]; //后缀

    int sz=0;

    int i;

	CString tmp;

	//遍历
    for( i = st;i < ed;i++)
	{
        if(Is_Num(*(str+i)) || *(str+i)=='.')
  //为了进行小数点的判断
            if(*(str+i)=='.')

                if(Num.point==1)
				{
					tmp.Format("%d位置出现多余小数点",i);
					MessageBox(tmp);
					return ERRORX;
                }
                else

                    Num.point = 1;
            else

                if(Num.point==1){

                    Num.x+=Num.flag*(*(str+i)-48); //小数

                    Num.flag*=1e-1;//后一位

                }
                else
                    Num.x=Num.x*1e1+(*(str+i)-48); //进一位

        else{
            if(i>st && Is_Num(str[i-1])){

                Suffix[sz++]=Num.x*Num.sign;

                Num.Init();
            }

            if(*(str+i)=='s' || *(str+i)=='c' || *(str+i)=='t'){

                double ret=0.0;

				//计算数值，采用递归的写法
				//比如表达式 3+sin(3.14+2) 先计算位置0到length这个区间的数值
				//然后递归计算3+位置2到位置length这个区间的数值
                switch(*(str+i)){
                    case 's':ret=sin(GetV(str,i+4,NextB[i+3]));break;
                    case 'c':ret=cos(GetV(str,i+4,NextB[i+3]));break;
                    default :ret=tan(GetV(str,i+4,NextB[i+3]));
                }

                if(Equal(ret,ERRORX))

                    return ERRORX;

                Num.x=ret;

                Suffix[sz++]=Num.x*Num.sign;

                Num.Init(); //归零

                i=NextB[i+3];
            }

            else

            if(*(str+i)==')'){
                while(!S.empty() && !Equal(HASHL,S.top())){
                    Suffix[sz++]=S.top();
                    S.pop();
                }
                S.pop();
            }
            else{
                char c=*(str+i);
                if(*(str+i)=='-'){
                    Num.sign=-Num.sign;
                    if(i>st && str[i-1]!='(')
                        c='+';
                    else
                        continue;
                }
                while(!S.empty() && !Equal(S.top(),HASHL) && Prio(S.top())>=Prio(hash(c))){
                    Suffix[sz++]=S.top();
                    S.pop();
                }
                S.push(hash(c));
            }
        }
    }
    if(Is_Num(str[ed-1]))
        Suffix[sz++]=Num.x*Num.sign;
    while(!S.empty()){
        Suffix[sz++]=S.top();
        S.pop();
    }
    double a,b,cur;
    for(i=0;i<sz;i++){
        cur=Suffix[i];
        if(cur>-inf+10){
            S.push(cur);
        }
        else{
            b=S.top();
            S.pop();
            a=S.top();
            S.pop();
            if(Equal(HASHA,cur))
                S.push(a+b);
            else
            if(Equal(HASHS,cur))
                S.push(a-b);
            else
            if(Equal(HASHM,cur))
                S.push(a*b);
            else
            {
                if(Equal(b,0.0))
                {
                    MessageBox("错误:除数出现0!");
                    return ERRORX;
                }
                S.push(a/b);
            }
        }
    }
    delete []Suffix;
    return S.top();
}

bool Bdscal::Equal(double a,double b){
    if(fabs(a-b)<eps)
        return 1;
    return 0;
}

void Bdscal::OnBnClickedCancel()
{
	OnCancel();
}
void Bdscal::OnEnChangeEdit1()
{
}

