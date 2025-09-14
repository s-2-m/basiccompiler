/*
 * RUDIMENTARY JIT COMPILER FOR MATHEMATICAL EXPRESSIONS
 *
 * TAKES POSITIVE NUMBERS OR SYMBOLS: 
   '(', ')', '+', '-', '*', '/'
 *
 * DOES NOT FOLLOW PEDMAS
 *
 * NUMBERS MUST BE POSITIVE
 *
 * */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// types (identifiers not yet implemented)
#define N   0
#define ID  1
#define PR  2
#define OP  3

// vals (ops)
#define ADD 0
#define SUB 1
#define MUL 2
#define DIV 3

typedef struct{
  int type;
  int val;
}token;

token **lst;
size_t max=256;
int ind;
FILE* f;

int term();
int expr();
int expr_prime();


int count=1;
void flag(){
	printf("flag: %d\n",count++);
}

void assemble(){
  printf("generating assembly...\n");
  f=fopen("output.asm","w+");
  fprintf(f,"%s","\t.text\n");
  fprintf(f,"%s","\t.globl _start\n");
  fprintf(f,"%s","_start:\n");
  fprintf(f,"%s","\tmov $0, %rdi\n");
  token **stack=malloc(max*sizeof(token*));
	int first=1,p=0,i=0,k=0,sym,num;
	token *t;

	void domath(){
		if(first){
			first=0;
			k--;
			fprintf(f,"\tmov $%d, %rdi\n",stack[k]->val);
		}
		if(k!=0){
      //debug val and sym
      int sym,num;
      if(p==0){
        k--;
        sym=stack[k]->val;
        k--;
        num=stack[k]->val;
        //printf("p %d %d\n",sym,num);
      }else{
        k--;
        num=stack[k]->val;
        k--;
        sym=stack[k]->val;
        //printf("!p %d %d\n",sym,num);
      }
			if(sym==0){
				fprintf(f,"\tadd $%d, %rdi\n",num);
			}else if(sym==1){
				fprintf(f,"\tsub $%d, %rdi\n",num);
			}else if(sym==2){
				fprintf(f,"\tmov $%d, %rax\n",num);
				fprintf(f,"%s","\tmul %rdi\n");
				fprintf(f,"%s","\tmov %rax, %rdi\n");
			}else if(sym==3){
				fprintf(f,"%s","\tmov %rdi, %rax\n");
				fprintf(f,"\tmov $%d, %rdi\n",num);
				fprintf(f,"%s","\txor %rdx, %rdx\n");
				fprintf(f,"%s","\tdiv %rdi\n");
				fprintf(f,"%s","\tmov %rax, %rdi\n");
			}
		}
	}

	for(int i=0;lst[i]!=NULL;i++){
		token *t=lst[i];
		if(t->type==N||t->type==OP){
			//printf("number val: %d, k: %d\n",t->val,k);
			stack[k++]=t;
		}
		if(t->type==PR&&t->val==1){
      //if(k>0){printf("%d\n",stack[k-1]->val);}
			domath();
      p=1;
		}
	}
  //printf("k : %d, f %d, p %d\n",k,first,p);
	while(k!=0){
		domath();
	}
  fprintf(f,"%s","\tmov $60, %rax\n");
  fprintf(f,"%s","syscall\n");
  fclose(f);
}

int term(){
  if((lst[ind]->type==PR)&&(lst[ind]->val==0)){
    ind++;
    return expr();
  }else if(lst[ind]->type==N){
    ind++;
    return 0;
  }
  return 1;
}

int expr_prime(){
  if(lst[ind]->type==OP){
    ind++;
    if(term()==0){return expr_prime();}
  }
  if((lst[ind]->type==PR)&&(lst[ind]->val==1)){
    ind++;
    return 0;
  }else if(lst[ind]==NULL){return 0;}
  return 1;
}

int expr(){
  if(lst[ind]==NULL){return 0;}
  if(term()){return 1;}
  return expr_prime();
}

int parse(){
  ind=0;
  printf("parsing...\n");
  if(expr()==0&&lst[ind]==NULL){return 0;}
  return 1;
}

void lex(token **lst,int *retSize){
  token tmp;
  char c=getc(stdin);
  (*retSize)=0;

  printf("lexing...\n");

  while(c!='\n'&&(*retSize)<255){
    if(isdigit(c)){
      ungetc(c,stdin);
      char buf[19]={};
      for(int i=0;isdigit(c=getc(stdin));i++){
        buf[i]=c;
      }
      ungetc(c,stdin);
      int dat=atoi(buf);
      tmp=(token){N,dat};

      //printf("num: %d \n",tmp.val);

    }else if(c>=40 && c<=47){
      if(c=='('){
        tmp=(token){PR,0};
      }else if(c==')'){
        tmp=(token){PR,1};
      }else if(c=='+'){
        tmp=(token){OP,ADD};
      }else if(c=='-'){
        tmp=(token){OP,SUB};
      }else if(c=='*'){
        tmp=(token){OP,MUL};
      }else if(c=='/'){
        tmp=(token){OP,DIV};
      }

      //printf("sym: %c \n",c);

    }
    if(c!=','){
      lst[*retSize]=malloc(sizeof(token));
      lst[*retSize]->type=tmp.type;
      lst[*retSize]->val=tmp.val;
      (*retSize)++;
    }
    c=getc(stdin);
  }
  lst[*retSize]=NULL;
}

int main(){
  lst=malloc(max*sizeof(token*));
  int lstSize;
  lex(lst,&lstSize);

  //printf("tokens: %d\n",lstSize);
  for(int i=0;lst[i]!=NULL;i++){
    token *tkn=lst[i];
    //printf("%d: %d %d \n",i,tkn->type,tkn->val);
  }
  /*if(parse()){
    printf("ERR\n");
		exit(1);
  }else{
    printf("valid syntax\n");
		assemble();
  }*/
	assemble();

	char *const arg1[]={"touch","test.o",NULL};
	char *const arg2[]={"touch","test",NULL};
	char *const arg3[]={"as","-o","test.o","output.asm",NULL};
	char *const arg4[]={"ld","-o","test","test.o",NULL};

	int retcode;
	if(fork()==0){
		execve("/usr/bin/touch",arg1,NULL);
	}
  wait(0);
	
	if(fork()==0){
	  execve("/usr/bin/touch",arg2,NULL);
	}
  wait(0);
	
  printf("assembling...\n");
	if(fork()==0){
		execve("/usr/bin/as",arg3,NULL);
	}
  wait(0);

  printf("linking...\n");
	if(fork()==0){
		execve("/usr/bin/ld",arg4,NULL);
	}
  wait(0);

  printf("executing...\n");
	if(fork()==0){
		execv("test",NULL);
	}else{
    int s;
    wait(&s);
    printf("result is : %d\n",s>>8);
  }
  return 0;
}
