// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con un único productor y un único consumidor)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   n_productores            = 4,
   n_consumidores           = 5,
   id_buffer                = n_productores,
   num_procesos_esperado    = n_productores+n_consumidores+1,
   num_items                = 20,
   tam_vector               = 10,
   etiq_prod                = 0,
   etiq_cons                = 1;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio() {
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}
// ---------------------------------------------------------------------
// producir produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir(){
   static int contador = 0 ;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++ ;
   //cout << "Productor ha producido valor " << contador << endl << flush;
   return contador ;
}
// ---------------------------------------------------------------------

void funcion_productor(const int &n_orden){
  cout << "Se inicia productor " << n_orden << endl << flush;
  const int valores_a_producir = num_items/n_productores;
  for ( int i= 0 ; i < valores_a_producir ; i++ ){
      // producir valor
      int valor_prod = valores_a_producir*n_orden + producir();
      // enviar valor
      cout << "Productor " << n_orden <<  " va a enviar valor " << valor_prod << endl << flush;
      MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_prod, MPI_COMM_WORLD );
  }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons ){
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor(const int &n_orden){
  cout << "Se inicia consumidor " << n_orden << endl << flush;
  const int valores_a_consumir = num_items/n_consumidores;
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( int i=0 ; i < valores_a_consumir; i++ ){
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_cons, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_cons, MPI_COMM_WORLD,&estado );
      cout << "Consumidor " << n_orden << " ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec );
   }
}
// ---------------------------------------------------------------------

void funcion_buffer(){
  cout << "Se inicia buffer." <<endl;
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              primera_ocupada     = 0, // índice de primera celda ocupada
              num_celdas_ocupadas = 0, // número de celdas ocupadas
              etiq_aceptable ;    // identificador de emisor aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido

   for( unsigned int i=0 ; i < num_items*2 ; i++ ){
      // 1. determinar si puede enviar solo prod., solo cons, o todos
      switch(num_celdas_ocupadas){
        case 0: etiq_aceptable = etiq_prod; break;
        case tam_vector: etiq_aceptable = etiq_cons; break;
        default: etiq_aceptable = MPI_ANY_TAG; break;
      }

      // 2. recibir un mensaje del emisor o emisores aceptables

      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado );

      // 3. procesar el mensaje recibido

      switch( estado.MPI_TAG ){ // leer emisor del mensaje en metadatos
         case etiq_prod: // si ha sido el productor: insertar en buffer
            buffer[primera_libre] = valor ;
            primera_libre = (primera_libre+1) % tam_vector ;
            num_celdas_ocupadas++ ;
            cout << "Buffer ha recibido valor " << valor << endl ;
            break;

         case etiq_cons: // si ha sido el consumidor: extraer y enviarle
            valor = buffer[primera_ocupada] ;
            primera_ocupada = (primera_ocupada+1) % tam_vector ;
            num_celdas_ocupadas-- ;
            cout << "Buffer va a enviar valor " << valor << endl ;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_cons, MPI_COMM_WORLD);
            break;
      }
   }
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual ){
     if (num_items%(n_productores*n_consumidores) == 0){
        // ejecutar la operación apropiada a 'id_propio'
        if ( id_propio < n_productores )
           funcion_productor(id_propio);
        else if ( id_propio == n_productores )
           funcion_buffer();
        else // id_propio > n_productores
           funcion_consumidor( id_propio-(n_productores+1) );
     } else {
       if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
       { cout << "el número de items es:    " << num_items << endl
              << "y no es multiplo de np y nc" << num_procesos_actual << endl
              << "(programa abortado)" << endl ;
       }
     }
   } else {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
