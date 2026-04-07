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
#define eof 4

// vals (ops)
#define ADD 0
#define SUB 1
#define MUL 2
#define DIV 3

typedef struct tkn{
  int type;
  int val;
	struct tkn *next;
}token;

int lstSize;
token *head;

int prevN;
int cnt=0;

FILE* f;

int term();
int expr();
int expr_prime();

int count=1;
void flag(){
	printf("flag: %d\n",count++);
}

int term(token **cur){
  if(((*cur)->type!=eof)&&(*cur)->type==PR&&(*cur)->val==0){
		*cur=(*cur)->next;
		prevN=0;
		cnt++;
		if(expr(cur))return 1;
		return 0;
	}else if(((*cur)->type!=eof)&&(*cur)->type==N){
		*cur=(*cur)->next;
		prevN=1;
    return 0;
  }
  return 1;
}

int expr_prime(token **cur){
  if((*cur)->type==eof)return 0;
  if(((*cur)->type!=eof)&&((*cur)->type==OP)){
		*cur=(*cur)->next;
		prevN=0;
    if(((*cur)->type!=eof)&&(term(cur)==0))return expr_prime(cur);
  }
  if(((*cur)->type!=eof)&&(*cur)->type==PR&&(*cur)->val==1&&prevN){
		*cur=(*cur)->next;
		prevN=0;
		cnt--;
    return 0;
  }
  return 1;
}

int expr(token **cur){
  if((*cur)->type==eof&&cnt==0)return 0;
  if(term(cur))return 1;
  return expr_prime(cur);
}

int parse(){
	token *sen=head;
  printf("parsing...\n");
  if(expr(&sen)==0&&sen->type==eof){return 0;}
  return 1;
}

void lex(token **head,int *retSize){
	token *cur=NULL;
  (*retSize)=0;
  char c=getc(stdin);
  printf("lexing...\n");
  while(c!='\n' && c!=EOF){
		token *tmp=malloc(sizeof(token));
    if(isdigit(c)){
      ungetc(c,stdin);
      char buf[10]={};
      for(int i=0;isdigit(c=getc(stdin));i++)buf[i]=c;
      ungetc(c,stdin);
      int dat=atoi(buf);
      tmp->type=N;tmp->val=dat;
      //printf("num: %d\n",tmp->val);
    }else if(c>=40 && c<=47){
      if(c=='('){
      	tmp->type=PR;tmp->val=0;
      }else if(c==')'){
      	tmp->type=PR;tmp->val=1;
      }else if(c=='+'){
      	tmp->type=OP;tmp->val=ADD;
			}else if(c=='-'){
      	tmp->type=OP;tmp->val=SUB;
      }else if(c=='*'){
      	tmp->type=OP;tmp->val=MUL;
      }else if(c=='/'){
      	tmp->type=OP;tmp->val=DIV;
      }
      //printf("sym: %c\n",c);
    }
		if(cur){
			cur->next=tmp;
			cur=cur->next;
		}else{
			*head=tmp;
			cur=tmp;
		}
		(*retSize)++;
    c=getc(stdin);
  }
	token *end=malloc(sizeof(token));
	end->type=eof;
	if(cur)cur->next=end;
	else *head=end;
	(*retSize)++;
}

int main(){
  lex(&head,&lstSize);
  printf("TOKENS=%d\n",lstSize);
	
	printf("HEAD=0x%x\n",head);
	token *tmp=head;
	int i=1;
	while(tmp){
    printf("%d: %d %d (CUR=0x%x, NEXT=0x%x)\n",i,tmp->type,tmp->val,tmp,tmp->next);
		i++;
		tmp=tmp->next;
  }

  if(parse()){
    printf("ERR\n");
		exit(1);
  }else{
    printf("valid syntax\n");
		assemble();
  }

	while(head){
		tmp=head->next;
		free(head);
		head=tmp;
  }
	return 0;
}
