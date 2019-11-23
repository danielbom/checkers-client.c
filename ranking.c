#include <string.h>


/* Inicializa o ranking com strings vazias */

/* Inicia o ranking*/
void init_ranking(struct result * ranking) {

  reset_ranking(ranking);
}
    
/* Adiciona um resultado no  ranking */
void add_ranking(struct result * ranking, 
    struct result * latest_game) {

    int i;
    for (i = RANKINGSIZE; i >= 0; i--)
    {
    	strcpy(ranking[i+1].won, ranking[i].won);
    	strcpy(ranking[i+1].lost, ranking[i].lost);
    
    strcpy(ranking[0].won, latest_game->won);
    strcpy(ranking[0].lost, latest_game->lost);

    ranking++;
    }
    }

/* Limpa o ranking */
void reset_ranking(struct result * ranking) {

    int i;
    for(i = 0; i <= RANKINGSIZE; i++)
    {
    	strcpy(ranking->won, "");
    	strcpy(ranking->lost, "");
    	
    	ranking++;
    }
}

/* Exibe os 10 melhores jogadores */
void show_ranking(struct result * ranking) {

    int i;
    printf("Draughts~Checkers Ranking.\n");
    printf("===========================================================\n");
    printf("| Winner                    | Loser                       |\n");
    printf("| >>>>>>>>>> | >>>>>>>>> |\n");
    
    for(i = 0; i < RANKINGSIZE; i++)
    {
    	printf("| %-25s | %-27s |\n",ranking[i].won, ranking[i].lost);
    }
}

