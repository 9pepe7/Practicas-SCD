#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

const int n_fumadores = 3; // Numero de fumadores y de ingredientes distintos
Semaphore mostr_vacio = 1,
/* Semáforo que controla a la hebra del estanquero. Valdrá 1 cuando el mostrador esté vacío, y ésta pueda poner en él un ingrediente nuevo, y cero cuando ya haya un ingrediente en el mostrador. Su valor inicial, por tanto, será 1.*/
          ingr_disp[n_fumadores] = {0,0,0};
/* Array de semáforos, análogos al array de hebras de fumadores, que se activarán cuando un fumador i tenga su ingrediente en el mostrador preparado para ser recogido. Su valor inicial por tanto será cero, ya que de primeras no habrá ningún ingrediente preparado. */




//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

int genera_ingr(){
  chrono::milliseconds duracion(aleatorio<20,200>());
  int i=aleatorio<0,n_fumadores-1>();
  this_thread::sleep_for(duracion);
  return i;
}
void funcion_hebra_estanquero(){
  while (true){
    sem_wait(mostr_vacio);
    int i=genera_ingr();
    cout << "Se ha puesto en el mostrador el ingrediente " << i << endl;
    sem_signal(ingr_disp[i]);
  }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void funcion_hebra_fumador(const int num_fumador){
   while(true){
     sem_wait(ingr_disp[num_fumador]);
     sem_signal(mostr_vacio);
     fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   thread hebra_estanquero(funcion_hebra_estanquero),
          hebras_fumadores [n_fumadores];
   for(int i=0; i<n_fumadores; ++i){
     hebras_fumadores[i]=thread(funcion_hebra_fumador,i);
   }
   hebra_estanquero.join();
   for(int i=0; i<n_fumadores; ++i){
     hebras_fumadores[i].join();
   }
}
