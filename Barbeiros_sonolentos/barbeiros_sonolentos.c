
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>

#define N_CLIENTES 10
#define N_CADEIRAS 5

// Zona Crítica
sem_t cadeiras_usadas;
sem_t cadeira_do_barbeiro;
sem_t cabelo_cortado;
sem_t cliente_na_cadeira;
sem_t barbeiro_dormiu;

void *barbeiro()
{

    while (1)
    {
        sleep(0.5); // espera um pouco antes de ir dormir

        if (sem_trywait(&cadeira_do_barbeiro) == 0)
        { // não tem cliente na cadeira
            printf("Barbeiro dormiu na cadeira \n");
            sem_wait(&barbeiro_dormiu);
        }else{ // não tem cliente na cadeira
            sem_wait(&cliente_na_cadeira);
            printf("\tBarbeiro cortou cabelo de um Cliente \n");
            sem_post(&cabelo_cortado); // sinaliza que terminou de cortar o cabelo
        }
    }
    return NULL;
}

void *cliente(void *v)
{

    int id = *(int *)v;
    sleep(id % 3); // faz ele esperar um pouco antes de tentar entrar

    if (sem_trywait(&cadeiras_usadas) == 0){ // verifica se tem lugar na sala de espera
        printf("Cliente %d entrou na barbearia.\n", id);
        if (sem_trywait(&barbeiro_dormiu) == 0)
        { // verifica se o barbeiro esta dormindo
            sem_post(&barbeiro_dormiu);
        }else{
            sem_post(&barbeiro_dormiu);     // acorda o barbeiro
            sem_post(&cadeira_do_barbeiro); // barbeiro sai da cadeira
            printf("Cliente %d Acordou o barbeiro\n", id);
        }

        sem_wait(&cadeira_do_barbeiro); // clinte bloqueia a cadeira do barbeiro
        printf("\tCliente %d sentou na cadeira do barbeiro.\n", id);
        sem_post(&cadeiras_usadas);     // libera uma vaga na recepção
        sem_post(&cliente_na_cadeira);  // libera a cadeira da barbearia
        sem_wait(&cabelo_cortado);      // espera o barbeiro corta o cabelo
        sem_post(&cadeira_do_barbeiro); // libera a cadeira do barbeiro
        printf("Cliente %d deixou a barbearia.\n", id);
    }else{// cliente vai embora por não haver cadeiras vazias na sala de espera
        printf("Cliente %d não entrou na barbearia.\n", id);
        pthread_exit(NULL);
    }
    return NULL;
}

int main(){
    pthread_t thr_clientes[N_CLIENTES], thr_barbeiro;
    int i, id[N_CLIENTES];

    // inicia os semaforos
    sem_init(&cadeiras_usadas, 0, 5);
    sem_init(&cadeira_do_barbeiro, 0, 1);
    sem_init(&cliente_na_cadeira, 0, 0);
    sem_init(&cabelo_cortado, 0, 0);
    sem_init(&barbeiro_dormiu, 0, 0);
    sem_post(&barbeiro_dormiu);

    // Cria as threads dos clientes
    for (i = 0; i < N_CLIENTES; i++){
        id[i] = i;
        pthread_create(&thr_clientes[i], NULL, cliente, (void *)&id[i]); // id dos clientes
    }

    pthread_create(&thr_barbeiro, NULL, barbeiro, NULL);

    for (i = 0; i < N_CLIENTES; i++)
        pthread_join(thr_clientes[i], NULL);

    pthread_join(thr_barbeiro, NULL);
    return 0;
}
