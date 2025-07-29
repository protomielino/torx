#include <stdio.h>
#include <time.h>
#include "learning/ann_policy.h"

enum disr
{
    stop = 0,
    east = 1,
    north = 2,
    west = 3,
    south = 4
};

int main()
{
    // Definire il numero di stati e azioni
    int n_states = 5;  // Ad esempio, 5 stati
    int n_actions = 4; // Ad esempio, 4 azioni

    // Creare un'istanza della politica discreta
    int n_hidden = 0;
    real alpha = 0.1;
    real gamma = 0.8;
    real lambda = 0.8;
    bool eligibility = false;
    bool softmax = false;
    real randomness = 0.1;
    real init_eval = 0.0;
    bool separate_actions = false;
    ANN_Policy *policy = ANN_Policy_ctor (n_states, n_actions, n_hidden, alpha, gamma, lambda, eligibility, softmax, randomness, init_eval, separate_actions);

    real current_state = 0;
    float distance = 0;
    real reward = 0;

    // Simulazione di un ciclo di apprendimento
    for (int episode = 0; episode < 100; ++episode) {
//        current_state = rand() % n_states; // Stato attuale casuale
        current_state = distance;
//        int action = DiscretePolicy_SelectAction(policy, current_state, 0.0, -1); // Seleziona un'azione

        // Simulazione della ricompensa (può essere basata su logica di gioco o ambiente)
        // Aggiornare la politica con la ricompensa ricevuta
        int forced_a = -1;
        int action = ANN_Policy_SelectAction(policy, &current_state, reward, forced_a);

        float pos_x = 0;
        float pos_y = 0;
        switch (action) {
        case stop:
            break;
        case east:
            pos_x ++;
            break;
        case north:
            pos_y ++;
            break;
        case west:
            pos_x --;
            break;
        case south:
            pos_y --;
            break;
        default:
            break;
        }

        distance = sqrt(pos_x*pos_x + pos_y*pos_y);

        reward = 1 / distance;

//        reward = (action == 0) ? 1.0 : -1.0; // Ricompensa positiva per l'azione 0, negativa per le altre

        // Stampa dell'azione selezionata e della ricompensa
        printf("Episodio: %d, Stato: %f, Azione: %d, Ricompensa: %f\n", episode, current_state, action, reward);
    }

    // Liberare la memoria
    ANN_Policy_dtor(policy);

    return 0;
}
