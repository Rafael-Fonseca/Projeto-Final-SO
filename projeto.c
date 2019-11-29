#define _GNU_SOURCE
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>


// 64kB stack
#define FIBER_STACK 1024*64
struct c {
	/********************************************************************************************
	** atomic_int saldo;                                                                       **
	** Tentamos executar o controle da variável que sofre o efeito da concorrência,            **
	** valendo nos das variáveis atômicas, no entanto, embora tenha surtido efeito no código,  **
	** não foi o suficiente para extinguir o problema.                                         **
	********************************************************************************************/

	int saldo;
};

typedef struct c conta;

// Inicialização de variáveis, inclusive do mutex que é a estrutura que controla a concorrência neste programa.
conta from, to;
int valor;
pthread_mutex_t lock;


// The child thread will execute one of these functions
void transferencia_from_to(void *arg){

	pthread_mutex_lock(&lock); // Ativa o bloqueio na região crítica do código.

	if (from.saldo >= valor){ // Checa se a conta que está enviando dinheiro, tem recursos suficientes.
		from.saldo -= valor;
		to.saldo += valor;
	
		printf("Transferência concluída com sucesso!\n");
		printf("Saldo de c1: %d\n", from.saldo);
		printf("Saldo de c2: %d\n", to.saldo);

	}
	else{
		printf("Saldo insuficiente da conta origem\n");
	}

	pthread_mutex_unlock(&lock); // Desativa o bloqueio na região crítica.

return;
}

/*****************************************************************
** A função abaixo, transferencia_to_from, deveria ser evitada  **
** com a criação da função transferencia, recebendo como parâ-  **
** metro as variáveis do tipo conta, from e to, no entanto      **
** devido à falta de domínio sobre a linguagem C, o grupo optou **
** por executar esse amadorismo momentâneo no código.           **
*****************************************************************/

void transferencia_to_from(void *arg){

	pthread_mutex_lock(&lock); // Ativa o bloqueio na região crítica do código.

	if (to.saldo >= valor){ // Checa se a conta que está enviando dinheiro, tem recursos suficientes.
		to.saldo -= valor;
		from.saldo += valor;
	
		printf("Transferência concluída com sucesso!\n");
		printf("Saldo de c1: %d\n", from.saldo);
		printf("Saldo de c2: %d\n", to.saldo);

	}
	else{
		printf("Saldo insuficiente da conta origem\n");
	}

	pthread_mutex_unlock(&lock); // Desativa o bloqueio na região crítica.

return;
}


int main()
{
	//Inicialização de variáveis locais.
	void* stack;
	pid_t pid;
	int counter; // Esta variável será usada para saber quantas threads foram criadas
	pthread_t pthread_id[100];
	srand(time(NULL)); //esta linha garante que a função rand será efetivamente randomica

	// Allocate the stack
	stack = malloc( FIBER_STACK );
	if ( stack == 0 )
	{
		perror("malloc: could not allocate stack");
		exit(1);
	}

	// Todas as contas começam com saldo 100
	from.saldo = 100; //Teste from.saldo = 10 é bom para observar os resultados!
	to.saldo = 100;
	printf( "Transferindo 1,00 de uma conta para outra. \n" );
	valor = 1;
	counter = 0;

	while(from.saldo > 0 && to.saldo > 0 && counter < 100){ //Garante que só ocorreram 100 transferencias por vez 
		if( rand() % 100 >= 10){
			// Chama transferencia_from_to em 90% dos casos
			pthread_create(&pthread_id[counter], NULL, (void *)transferencia_from_to, NULL);
			counter++;
		}else{ 
			// Chama transferencia_to_from nos outros 10% dos casos
			pthread_create(&pthread_id[counter], NULL, (void *)transferencia_to_from, NULL);
			counter++;
		}

	}

	for(int i = 0; i < counter; i++)
	    {
		//Espera que as threads terminem de executar suas tarefas
		pthread_join(pthread_id[i], NULL);
	    }


	// Free the stack
	free( stack );
	printf("Transferências concluídas e memória liberada.\n");

	//Destroi o mutex
	pthread_mutex_destroy(&lock);
	return 0;
}

// Para compilar no linux digitar no terminal o comando # gcc -std=c11 projeto.c -lpthread -o projeto

