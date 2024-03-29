
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>

typedef struct {
    double media_chegada;
    double media_servico;
    double tempo_simulacao;
} parametros;

typedef struct {
    unsigned long int no_eventos;
    double tempo_anterior;
    double soma_areas;
} little;

void inicia_little(little * l){
    l->no_eventos = 0;
    l->tempo_anterior = 0.0;
    l->soma_areas = 0.0;
}

void le_parametros(parametros * params){
    printf("Informe o tempo medio entre clientes (s): ");
    scanf("%lF", &params->media_chegada);
    params->media_chegada = 1.0/params->media_chegada;

    printf("Informe o tempo medio de servico (s): ");
    scanf("%lF", &params->media_servico);
    params->media_servico = 1.0/params->media_servico;

    printf("Informe o tempo a ser simulado (s): ");
    scanf("%lF", &params->tempo_simulacao);
}

double uniforme() {
	double u = rand() / ((double) RAND_MAX + 1);
	//limitando entre (0,1]
	u = 1.0 - u;

	return (u);
}

double min(double d1, double d2){
    if(d1 < d2) return d1;
    return d2;
}

int main(){
    int semente = time(NULL);
    srand(semente);

    //le valores parametrizados
    parametros params;
    le_parametros(&params);

    //variaveis de controle da simulacao
    double tempo_decorrido = 0.0;
    double tempo_chegada = (-1.0/params.media_chegada) * log(uniforme());
    double tempo_saida = DBL_MAX;
    unsigned long int fila = 0;
    unsigned long int max_fila = 0;

    double tempo_coleta = 10.0;

    //variaveis de medidas de interesse
    double soma_ocupacao = 0.0;
    /**
     * Little
    */
    little e_n;
    little e_w_chegada;
    little e_w_saida;
    inicia_little(&e_n);
    inicia_little(&e_w_chegada);
    inicia_little(&e_w_saida);
    /**
     * Little -- fim
    */

    printf("segundo,erro_little\n");
    while(tempo_decorrido < params.tempo_simulacao){
        tempo_decorrido = min(min(tempo_chegada, tempo_saida), tempo_coleta);
        //printf("%lF\n", tempo_decorrido);

        if(tempo_decorrido == tempo_chegada){
            //chegada
            //a cabeca da fila eh quem esta em atendimento
            if(!fila){
                double tempo_servico =
                    (-1.0/params.media_servico)
                    * log(uniforme());
                
                tempo_saida = tempo_decorrido
                    + tempo_servico;

                soma_ocupacao += tempo_servico;
            }
            fila++;
            max_fila = fila > max_fila?
                fila:max_fila;
            
            tempo_chegada = tempo_decorrido + 
            (-1.0/params.media_chegada) * log(uniforme());

            //calculo little -- E[N]
            e_n.soma_areas += 
                (tempo_decorrido - e_n.tempo_anterior)
                * e_n.no_eventos;
            e_n.no_eventos++;
            e_n.tempo_anterior = tempo_decorrido;
            
            //calculo little -- E[W] - chegada
            e_w_chegada.soma_areas +=
                (tempo_decorrido - e_w_chegada.tempo_anterior)
                * e_w_chegada.no_eventos;
            e_w_chegada.no_eventos++;
            e_w_chegada.tempo_anterior = tempo_decorrido;
        } else if(tempo_decorrido == tempo_saida){
            //saida
            fila--;
            if(fila){
                double tempo_servico =
                    (-1.0/params.media_servico)
                    * log(uniforme());
                
                tempo_saida = tempo_decorrido
                    + tempo_servico;

                soma_ocupacao += tempo_servico;
            }else{
                tempo_saida = DBL_MAX;
            }

            //calculo little -- E[N]
            e_n.soma_areas += 
                (tempo_decorrido - e_n.tempo_anterior)
                * e_n.no_eventos;
            e_n.no_eventos--;
            e_n.tempo_anterior = tempo_decorrido;

            //calculo little -- E[W] - saida
            e_w_saida.soma_areas +=
                (tempo_decorrido - e_w_saida.tempo_anterior)
                * e_w_saida.no_eventos;
            e_w_saida.no_eventos++;
            e_w_saida.tempo_anterior = tempo_decorrido;
        } else if (tempo_decorrido == tempo_coleta) {
            e_n.soma_areas += (tempo_decorrido - e_n.tempo_anterior) * e_n.no_eventos;
            e_w_chegada.soma_areas += (tempo_decorrido - e_w_chegada.tempo_anterior) * e_w_chegada.no_eventos;
            e_w_saida.soma_areas += (tempo_decorrido - e_w_saida.tempo_anterior) * e_w_saida.no_eventos;

            e_n.tempo_anterior = tempo_decorrido;
            e_w_chegada.tempo_anterior = tempo_decorrido;
            e_w_saida.tempo_anterior = tempo_decorrido;

            double e_n_calculo = e_n.soma_areas / tempo_decorrido;
            double e_w_calculo = (e_w_chegada.soma_areas - e_w_saida.soma_areas)/e_w_chegada.no_eventos;
            double lambda = e_w_chegada.no_eventos / tempo_decorrido;

            // printf("E[N]: %lF\n", e_n_calculo);    
            // printf("E[W]: %lF\n", e_w_calculo);
            printf("%.0lF,%.20lF\n", tempo_coleta, fabs(e_n_calculo - lambda * e_w_calculo));
            tempo_coleta += 10;
        } else{
            printf("Evento invalido!\n");
            return(1);
        }
    }

    printf("\n");
    printf("Ocupacao: %lF\n", (soma_ocupacao/tempo_decorrido));
    printf("tamanho maximo da fila: %ld\n", max_fila);

    double e_n_calculo = e_n.soma_areas / tempo_decorrido;
    double e_w_calculo = (e_w_chegada.soma_areas - e_w_saida.soma_areas)/e_w_chegada.no_eventos;

    printf("E[N]: %lF\n", e_n_calculo);    
    printf("E[W]: %lF\n", e_w_calculo);

    return 0;
}