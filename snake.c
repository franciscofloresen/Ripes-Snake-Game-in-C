#include "ripes_system.h"
#include <stdio.h>

// Definiciones de tamaño y colores
#define LED_MATRIX_WIDTH  35 //Define el ancho de la matriz principal de LEDs como 35 unidades
#define LED_MATRIX_HEIGHT 25 //Define la altura de la matriz principal de LEDs como 25 unidades
#define LED_MATRIX_WIDTH_2  7 //Define el ancho de una segunda matriz de LEDs como 7 unidades
#define LED_MATRIX_HEIGHT_2 5 //Define la altura de la segunda matriz de LEDs como 5 unidades
#define SNAKE_COLOR 0xFF0000 //Establece el color de la serpiente en la matriz de LEDs utilizando el código hexadecimal para rojo
#define APPLE_COLOR 0x90EE90 //Define el color de las manzanas en la matriz de LEDs usando un tono de verde claro (hexadecimal)
#define PURPLE_APPLE_COLOR 0x800080 //Establece el color de una manzana especial (morada) en la matriz de LEDs con el código hexadecimal para púrpura
#define SCORE_COLOR 0xFFFFFF //Define el color para mostrar el puntaje en blanco utilizando su código hexadecimal.
#define MAX_SNAKE_LENGTH 100 //Establece la longitud máxima que puede alcanzar la serpiente a 100 segmentos
#define SNAKE_INIT_X (LED_MATRIX_WIDTH / 2) //Define la posición inicial X de la serpiente, que es el centro del ancho de la matriz de LEDs
#define SNAKE_INIT_Y (LED_MATRIX_HEIGHT / 2) // Define la posición inicial Y de la serpiente, que es el centro de la altura de la matriz de LEDs
#define DELAY 2500 //Establece un retardo de 2500 unidades para el control de la velocidad de actualización o pausa en el juego

// Declaraciones de registros de hardware
volatile unsigned int *d_pad_up = (volatile unsigned int *)D_PAD_0_UP; //Declara un puntero a un entero sin signo volátil para el botón de dirección arriba del D-pad. Aquí se asume que D_PAD_0_UP es una dirección de memoria mapeada a este botón
volatile unsigned int *d_pad_down = (volatile unsigned int *)D_PAD_0_DOWN; //Similar al anterior pero para la dirección abajo
volatile unsigned int *d_pad_left = (volatile unsigned int *)D_PAD_0_LEFT; //Similar al anterior pero para la dirección izquierda
volatile unsigned int *d_pad_right = (volatile unsigned int *)D_PAD_0_RIGHT; //Similar al anterior pero para la dirección derecha
volatile unsigned int *led_matrix = (volatile unsigned int *)LED_MATRIX_0_BASE; //Un puntero a la base de la matriz de LEDs principal. LED_MATRIX_0_BASE sería la dirección de memoria inicial de esta matriz
volatile unsigned int *led_matrix_2 = (volatile unsigned int *)LED_MATRIX_1_BASE; //Un puntero a la base de una segunda matriz de LEDs. LED_MATRIX_1_BASE indica la dirección de memoria inicial de esta matriz secundaria

// Variable de conteo
int contador = 0; // Define una variable entera llamada contador e inicializa su valor en 0 y se utiliza para llevar la cuenta del puntaje del jugador. Cada vez que la serpiente come una manzana, el puntaje aumenta en uno, y este puntaje se muestra en la pantalla secundaria utilizando la función displayNumber (contador) en la función extendSnake (). 

// Direcciones de movimiento de la serpiente
typedef enum { //El uso del tipo enumerado (enum) en este caso proporciona una forma conveniente de representar las diferentes direcciones de movimiento posibles para la serpiente
   UP, DOWN, LEFT, RIGHT //Al definir un tipo enumerado, se asigna automáticamente un valor entero a cada miembro de la enumeración. En este caso, los miembros de la enumeración Direction (UP, DOWN, LEFT, RIGHT) tienen valores implícitos asignados por el compilador, comenzando desde 0 y aumentando en 1 para cada miembro adicional. 
} Direction; //En el código, estas variables definidas en la enumeración Direction se utilizan para indicar la dirección en la que se mueve la serpiente. 

// Estructura de posición para la manzana
typedef struct {
   int x; //Representa la coordenada x
   int y; //Representa la coordenada y
} Apple;

// Estructura de posición para la serpiente
typedef struct {
   int x; //Representa la coordenada x de la cabeza de la serpiente
   int y; //Representa la coordenada y de la cabeza de la serpiente
   Direction dir; //Indica la dirección actual de movimiento de la serpiente. La dirección se representa utilizando el tipo enumerado Direction, que define las direcciones posibles (arriba, abajo, izquierda, derecha).
} Position;

// Arreglo para la serpiente
Position snake[MAX_SNAKE_LENGTH];//Se declara un arreglo llamado snake que puede contener un número máximo de elementos igual a MAX_SNAKE_LENGTH. Cada elemento del arreglo es de tipo Position, es decir, cada elemento representa una posición en el juego donde puede estar la serpiente.
int snakeLength = 1; //Se declara una variable llamada snakeLength e inicializada con el valor 1. Esta variable se utiliza para almacenar la longitud actual de la serpiente en el juego, es decir, la cantidad de segmentos que conforman la serpiente en un momento dado. Inicialmente, la longitud de la serpiente se establece en 1, lo que significa que solo tiene un segmento (la cabeza).
Apple apple, purpleApple; //Se declaran dos variables de tipo Apple llamadas apple y purpleApple. Estas variables se utilizan para representar la posición de la manzana normal y la manzana morada en el juego, respectivamente.
unsigned int randomSeed = 0; //Se declara una variable llamada randomSeed de tipo unsigned int e inicializada con el valor 0. Esta variable se utiliza para almacenar la semilla que se utiliza en la generación de números aleatorios en el juego. Es de tipo unsigned porque no queremos numeros negativos y sirve para la aleatoriedad de las manzanas
int game_over = 0; //Se declara una variable llamada game_over e inicializada con el valor 0. Esta variable se utiliza para determinar si el juego ha terminado o no. Si el valor es diferente de 0 se termina

// Determinan las dimensiones de la representación de números en la pantalla secundaria
#define NUM_WIDTH 3
#define NUM_HEIGHT 5

// Representa los números en el marcador
unsigned int numbers[10][NUM_HEIGHT] = { // El arreglo numbers que mencionas es una representación de los dígitos del 0 al 9 en una matriz de LEDs. Esta matriz de LEDs tiene un tamaño definido por la constante NUM_HEIGHT, que en este caso es 5, representando la altura de cada número en líneas de LEDs.
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b001, 0b001, 0b001}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

// Función para reiniciar los LEDs
void resetLEDs() { // Define la función resetLEDs que no toma argumentos y no devuelve un valor (void)
   for (int y = 0; y < LED_MATRIX_HEIGHT; y++) { // Este bucle for recorre cada fila de la matriz de LEDs. La variable 'y' representa el índice de la fila, y el bucle continúa mientras 'y' sea menor que LED_MATRIX_HEIGHT (el número total de filas en la matriz)
       for (int x = 0; x < LED_MATRIX_WIDTH; x++) { // Un bucle for anidado dentro del bucle de 'y' que recorre cada columna de la matriz de LEDs. La variable x representa el índice de la columna, y el bucle continúa mientras x sea menor que LED_MATRIX_WIDTH (el número total de columnas en la matriz).
           led_matrix[y * LED_MATRIX_WIDTH + x] = 0x0; // Esta línea apaga el LED en la posición actual (determinada por 'y' y 'x'). La matriz de LEDs se trata como un arreglo unidimensional, por lo que para acceder a la posición (y, x), se calcula el índice como 'y' * LED_MATRIX_WIDTH + x. Se asigna 0x0 (cero hexadecimal, que representa el color negro o LED apagado) al LED en esta posición.
       }
   }
}

// Función para reiniciar los LEDs de la pantalla secundaria
void resetLEDs_2() { // Hace lo mismo que la anterior pero para la segunda matriz que es la del puntaje
   for (int y = 0; y < LED_MATRIX_HEIGHT_2; y++) {
       for (int x = 0; x < LED_MATRIX_WIDTH_2; x++) {
           led_matrix_2[y * LED_MATRIX_WIDTH_2 + x] = 0x0;
       }
   }
}

// Inicialización de la serpiente
void setupSnake() { // La función setupSnake () está diseñada para inicializar las condiciones de inicio para la serpiente en el juego.
   snake[0].x = SNAKE_INIT_X; // Establece la coordenada X de la cabeza de la serpiente (primer segmento, snake[0]) en la posición inicial definida por SNAKE_INIT_X, que es la mitad del ancho de la matriz de LEDs principal.
   snake[0].y = SNAKE_INIT_Y; // Establece la coordenada Y de la cabeza de la serpiente en la posición inicial SNAKE_INIT_Y, que es la mitad de la altura de la matriz de LEDs principal.
   snake[0].dir = RIGHT; // Asigna la dirección inicial de la serpiente a RIGHT, haciendo que la serpiente empiece moviéndose hacia la derecha. RIGHT es uno de los valores del tipo enumerado Direction.
   resetLEDs(); // Llama a la función resetLEDs () para apagar todos los LEDs en la matriz principal, limpiando cualquier estado previo antes de comenzar el juego.
   resetLEDs_2(); // Llama a la función resetLEDs_2 () para hacer lo mismo en la segunda matriz de LEDs, que es la utilizada para mostrar el puntaje.
}

// Renderizado de la serpiente en la matriz de LEDs
void renderSnake() { // Esta función se encarga de mostrar visualmente la serpiente en la matriz de LEDs
   for (int i = 0; i < snakeLength; i++) { // Este bucle for recorre cada segmento de la serpiente desde el inicio (i = 0) hasta el final del arreglo de la serpiente (i < snakeLength). snakeLength representa el número de segmentos que la serpiente tiene actualmente.
       for (int dx = 0; dx < 2; dx++) { // Un bucle anidado que itera dos veces, comenzando en dx = 0 hasta dx < 2. Este bucle se utiliza para expandir cada segmento de la serpiente a lo ancho de dos píxeles para hacerla más visible y gruesa en la matriz
           for (int dy = 0; dy < 2; dy++) { // Otro bucle anidado dentro del anterior que también itera dos veces, de dy = 0 a dy < 2. Similar a dx, este bucle expande cada segmento de la serpiente a lo alto
               int nx = snake[i].x + dx; // Calcula la coordenada X del LED en la matriz (nx) al sumar el desplazamiento dx a la posición X del segmento actual de la serpiente (snake[i].x). Es para alargarla en x
               int ny = snake[i].y + dy; // Calcula la coordenada Y del LED en la matriz (ny) de manera similar, sumando el desplazamiento dy a la posición Y del segmento actual de la serpiente (snake[i].y). Es para alargarla en y
               if(nx >= 0 && nx < LED_MATRIX_WIDTH && ny >= 0 && ny < LED_MATRIX_HEIGHT) { // Verifica que las coordenadas calculadas (nx, ny) estén dentro de los límites de la matriz de LEDs. Esto evita intentar dibujar fuera del rango de la matriz, lo cual podría causar errores.
                   led_matrix[ny * LED_MATRIX_WIDTH + nx] = SNAKE_COLOR; // Si las coordenadas están dentro de los límites válidos, se asigna el color de la serpiente (SNAKE_COLOR) al LED correspondiente en la matriz. La expresión ny * LED_MATRIX_WIDTH + nx convierte las coordenadas bidimensionales en un índice unidimensional para acceder a la matriz de LEDs tratada como un arreglo lineal
               }
           }
       }
   }
}

// Creación de una nueva manzana en una posición aleatoria
void createApple() { // Esta función se encarga de generar una nueva posición para la manzana en la matriz de LEDs.
   do {
       apple.x = rand() % (LED_MATRIX_WIDTH - 2) + 1; // Genera una posición aleatoria para la coordenada X de la manzana. rand () genera un número aleatorio. % (LED_MATRIX_WIDTH - 2) asegura que el número aleatorio esté en el rango de 0 a (LED_MATRIX_WIDTH - 3). Al sumar 1, ajustamos el rango a 1 a (LED_MATRIX_WIDTH - 2), evitando los bordes de la matriz de LEDs. 
       apple.y = rand() % (LED_MATRIX_HEIGHT - 2) + 1; // Lo mismo de la línea anterior pero para la coordenada Y
   } while (led_matrix[apple.y * LED_MATRIX_WIDTH + apple.x] == SNAKE_COLOR); // La condición del bucle verifica si la posición generada está ocupada por la serpiente. Si es verdadero, significa que la posición está ocupada por la serpiente, y el bucle debe repetirse para encontrar una nueva posición.
}

// Renderizado de la manzana en la matriz de LEDs
void renderApple() { // Es responsable de visualizar la manzana en la matriz de LEDs. La manzana se representa como un cuadrado de 2x2 LEDs para que sea más visible en la matriz.
   for (int dx = 0; dx < 2; dx++) { // Este bucle for itera dos veces, con la variable dx tomando los valores 0 y 1. Se utiliza para expandir la manzana horizontalmente en la matriz de LEDs, efectivamente haciendo que cada manzana ocupe dos columnas.
       for (int dy = 0; dy < 2; dy++) { // Similar al bucle para dx, este bucle for itera dos veces para los valores 0 y 1 de dy. Se utiliza para expandir la manzana verticalmente en la matriz de LEDs, haciendo que cada manzana ocupe dos filas.
           int nx = apple.x + dx; // Aquí, nx es calculado como la posición en X de la manzana (apple.x) más el desplazamiento dx. Esto determina la columna de la matriz de LEDs en la que debe encenderse un LED específico para formar parte de la representación visual de la manzana.
           int ny = apple.y + dy; // De manera similar, ny es la posición en Y de la manzana (apple.y) más el desplazamiento dy. Esto determina la fila de la matriz de LEDs en la que debe encenderse un LED específico.
           led_matrix[ny * LED_MATRIX_WIDTH + nx] = APPLE_COLOR; //Esta línea asigna el color de la manzana (APPLE_COLOR, un tono de verde claro definido previamente) al LED en la posición calculada. La expresión ny * LED_MATRIX_WIDTH + nx convierte las coordenadas bidimensionales (nx, ny) en un índice unidimensional en el arreglo led_matrix, que representa la matriz de LEDs.
       }
   }
}

// Creación de una manzana morada en una posición aleatoria
void createPurpleApple() { // Esta función se encarga de generar una nueva posición para la manzana morada en la matriz de LEDs.
   do {
       purpleApple.x = rand() % (LED_MATRIX_WIDTH - 2) + 1; // Genera una coordenada X aleatoria para la manzana morada, evitando los bordes de la matriz.
       purpleApple.y = rand() % (LED_MATRIX_HEIGHT - 2) + 1; // Genera una coordenada Y aleatoria para la manzana morada, también evitando los bordes.
   } while (led_matrix[purpleApple.y * LED_MATRIX_WIDTH + purpleApple.x] == SNAKE_COLOR || // Revisa si la posición generada está ocupada por la serpiente.
            (purpleApple.x == apple.x && purpleApple.y == apple.y)); // Revisa si la posición generada es la misma que la de la manzana regular.
            // El bucle while asegura que la posición generada para la manzana morada en el juego no esté ocupada por la serpiente y no coincida con la posición de la manzana regular. Esto se hace para evitar superposiciones y conflictos en los objetivos del juego, asegurando que cada manzana tenga su propio espacio único en la matriz de LEDs.
}

// Renderizado de la manzana morada en la matriz de LEDs
void renderPurpleApple() { // Se encarga de visualizar la manzana morada en la matriz de LED
   for (int dx = 0; dx < 2; dx++) { // Bucle para expandir la manzana morada a lo ancho (2 columnas).
       for (int dy = 0; dy < 2; dy++) { // Bucle para expandir la manzana morada a lo alto (2 filas).
           int nx = purpleApple.x + dx; // Calcula la coordenada X del LED en la matriz.
           int ny = purpleApple.y + dy; // Calcula la coordenada Y del LED en la matriz.
           led_matrix[ny * LED_MATRIX_WIDTH + nx] = PURPLE_APPLE_COLOR; // Enciende el LED en la posición calculada con el color de la manzana morada.
       }
   }
}

// Mostrar un número en la pantalla secundaria
void displayNumber(int num) { // Se encarga de mostrar los numeros en la matriz 2
    resetLEDs_2(); // Limpia la pantalla secundaria para prepararla para mostrar el número. 
    int digit1 = num / 10; // Calcula el primer dígito del número.
    int digit2 = num % 10; // Calcula el segundo dígito del número tomando el residuo.
    int offset = 0; // Inicializa un desplazamiento para posicionar los dígitos en la pantalla secundaria. Es el que permite la separación cuando tenemos 2 numeros en pantalla.

    if (num >= 10) { // Si el número es mayor o igual a 10
        for (int y = 0; y < NUM_HEIGHT; y++) { // Bucle para cada fila en la representación de dígitos.
            for (int x = 0; x < NUM_WIDTH; x++) { // Bucle para cada columna en la representación de dígitos.
                if (numbers[digit1][y] & (1 << (NUM_WIDTH - 1 - x))) // Comprueba si el bit correspondiente está activo en la representación del dígito.
                    led_matrix_2[y * LED_MATRIX_WIDTH_2 + offset + x] = SCORE_COLOR; // Enciende el LED correspondiente en la posición de la pantalla secundaria.
            }
        }
        offset += NUM_WIDTH + 1; // Ajusta el desplazamiento para el próximo dígito.
    }

    for (int y = 0; y < NUM_HEIGHT; y++) { // Bucle para cada fila en la representación del segundo dígito.
        for (int x = 0; x < NUM_WIDTH; x++) { // Bucle para cada columna en la representación del segundo dígito.
            if (numbers[digit2][y] & (1 << (NUM_WIDTH - 1 - x))) // Comprueba si el bit correspondiente está activo en la representación del dígito.
                led_matrix_2[y * LED_MATRIX_WIDTH_2 + offset + x] = SCORE_COLOR; // Enciende el LED correspondiente en la posición de la pantalla secundaria.
        }
    }
}

// Extender la serpiente
void extendSnake() { // se encarga de extender la longitud de la serpiente en el juego
    if (snakeLength + 2 <= MAX_SNAKE_LENGTH) { // Se verifica si la serpiente puede crecer sin superar la longitud máxima permitida (MAX_SNAKE_LENGTH). La serpiente se extenderá por dos segmentos si esta condición es verdadera.
        for (int j = 0; j < 2; j++) { // Este bucle for se repite dos veces, indicando que la serpiente se extenderá por dos segmentos en esta función.
            Position newHead = snake[0]; // Se crea una nueva variable newHead de tipo Position, y se inicializa con la posición de la cabeza actual de la serpiente (snake[0]).
            switch (snake[0].dir) { // Este bloque switch ajusta las coordenadas de newHead basándose en la dirección actual de la cabeza de la serpiente. Modifica la coordenada x o y para mover la cabeza en la dirección deseada.
                case UP:    newHead.y--; break;
                case DOWN:  newHead.y++; break;
                case LEFT:  newHead.x--; break;
                case RIGHT: newHead.x++; break;
            }
            for (int i = snakeLength; i > 0; i--) { // Este bucle for mueve cada segmento de la serpiente una posición hacia atrás en el arreglo, para hacer espacio para el nuevo segmento de cabeza en la primera posición.
                snake[i] = snake[i - 1]; 
            }
            snake[0] = newHead; // La nueva posición calculada para la cabeza se asigna al primer elemento del arreglo snake, efectivamente moviendo la cabeza de la serpiente a su nueva posición.
            snakeLength++; // Incrementa la variable snakeLength, que lleva la cuenta de cuántos segmentos tiene la serpiente.
        }
        contador++;
        displayNumber(contador); // Al final de cada extensión, el contador se incrementa, y el nuevo puntaje se muestra en la pantalla usando la función displayNumber.
    }
}

// Navegar por la serpiente
void navigateSnake() {
    if (*d_pad_up && snake[0].dir != DOWN) { // Si el botón de arriba está presionado y la serpiente no se mueve hacia abajo
        snake[0].dir = UP; // Cambia la dirección de la serpiente a hacia arriba
    } else if (*d_pad_down && snake[0].dir != UP) { // Si el botón de abajo está presionado y la serpiente no se mueve hacia arriba
        snake[0].dir = DOWN; // Cambia la dirección de la serpiente a hacia abajo
    } else if (*d_pad_left && snake[0].dir != RIGHT) { // Si el botón izquierdo está presionado y la serpiente no se mueve hacia la derecha
        snake[0].dir = LEFT; // Cambia la dirección de la serpiente a hacia la izquierda
    } else if (*d_pad_right && snake[0].dir != LEFT) { // Si el botón derecho está presionado y la serpiente no se mueve hacia la izquierda
        snake[0].dir = RIGHT; // Cambia la dirección de la serpiente a hacia la derecha
    }

    Position nextPosition = snake[0]; // Crea una copia de la posición actual de la cabeza de la serpiente
    switch (nextPosition.dir) { // Calcula la nueva posición de la cabeza de la serpiente en función de su dirección
        case UP:    nextPosition.y = (nextPosition.y - 1 + LED_MATRIX_HEIGHT) % LED_MATRIX_HEIGHT; break; // Mueve hacia arriba, ciclo en el borde superior
        case DOWN:  nextPosition.y = (nextPosition.y + 1) % LED_MATRIX_HEIGHT; break; // Mueve hacia abajo, ciclo en el borde inferior
        case LEFT:  nextPosition.x = (nextPosition.x - 1 + LED_MATRIX_WIDTH) % LED_MATRIX_WIDTH; break; // Mueve hacia la izquierda, ciclo en el borde izquierdo
        case RIGHT: nextPosition.x = (nextPosition.x + 1) % LED_MATRIX_WIDTH; break; // Mueve hacia la derecha, ciclo en el borde derecho
    }
    
    if (nextPosition.x == purpleApple.x && nextPosition.y == purpleApple.y) { // Verifica si la serpiente ha chocado con una manzana morada
        game_over = 1; // Si es así, termina el juego
        return; // Sale de la función
    }

    if (nextPosition.x == apple.x && nextPosition.y == apple.y) { // Verifica si la serpiente ha chocado con una manzana regular
        extendSnake(); // Extiende la longitud de la serpiente
        createApple(); // Crea una nueva manzana regular en una posición aleatoria
        createPurpleApple(); // Crea una nueva manzana morada en una posición aleatoria
    } else { // Mueve cada segmento de la serpiente a la posición del segmento anterior para avanzar
        for (int i = snakeLength - 1; i > 0; i--) { 
            snake[i] = snake[i - 1];
        }
        snake[0] = nextPosition; // Establece la nueva posición de la cabeza de la serpiente
    }
}

// Detectar colisión de la serpiente consigo misma
int detectCollision() { // Definición de la función que detecta colisión de la serpiente consigo misma.
    for (int i = 1; i < snakeLength; i++) { // Itera sobre todos los segmentos del cuerpo de la serpiente, empezando desde el segundo segmento
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) { // Compara la posición de la cabeza de la serpiente (snake[0]) con cada uno de los otros segmentos del cuerpo. Si las coordenadas de la cabeza coinciden con las de cualquier otro segmento, hay una colisión.
            return 1; // Devuelve 1, indicando que se ha detectado una colisión
        }
    }
    return 0;// Si se completa el bucle y no se detectan colisiones, devuelve 0.
}

// Pausa
void pause() { // Definición de la función que crea una pausa o retardo en el juego.
    for (volatile unsigned int i = 0; i < DELAY; ++i) { // Inicia un bucle que itera un número de veces definido por la constante DELAY. El cuerpo del bucle está vacío. Simplemente se utiliza el bucle para consumir tiempo y crear un retardo
    }
}

// Función principal
int main() { // Definición de la función principal del programa, que controla el flujo del juego.
    setupSnake(); // Inicializa la serpiente en el centro de la matriz de LEDs.
    createApple(); // Crea una manzana regular en una posición aleatoria.
    createPurpleApple(); // Crea una manzana morada en otra posición aleatoria.

    while (!game_over) { // Inicia un bucle que se ejecuta mientras el juego no haya terminado.
        navigateSnake(); // Actualiza la dirección de la serpiente basada en la entrada del usuario y la mueve.
        if (detectCollision()) { // Verifica si la serpiente ha colisionado consigo misma
            game_over = 1; // Si detecta una colisión, marca el juego como terminado.
            break; // Sale del bucle mientras.
        }

        resetLEDs(); // Apaga todos los LEDs para preparar la siguiente renderización.
        renderSnake(); // Dibuja la serpiente en la matriz de LEDs.
        renderApple(); // Dibuja la manzana regular.
        renderPurpleApple(); // Dibuja la manzana morada.
        pause(); // Pausa el juego para hacer que la velocidad de movimiento de la serpiente sea jugable.
    }

    return 0;// Una vez que el juego ha terminado, retorna 0 para indicar que el programa ha terminado correctamente.
}
