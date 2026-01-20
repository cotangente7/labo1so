Laboratorio 1 Mybash:
Pasos para ejecutar.
Una vez que estan todos los archivos completos, en el repositorio
realizar los siguientes test.
1) make test-command.
2) make test- parsing
3) make test
una vez que pasen al 100% Realizar:
- make clean
- make
- ./mybash
Si no hay errores de compilacion, se deberia abrir una terminal
mybash>

Descripcion de archivos principales:
command.c :
es la implementacion del tad, definimos las estructuras del comando y del pipeline y las funciones bases para hacer la shell
funciones como crear un comando nuevo, destruir un comando, devovler el primer argumento etc.
Despues implementamos funciondes del pipeline para lograr una secuencia de comandos.
Nos ayudo mucho lo que hicimos en alrogitmos 2
El execute.c es basicamente lo que ejecuta los comandos hechos en command.c.
Ejecuta procesos externos usando fork e implemente los pipelines correctamente,
conectando la salida de un comando con la entrada del siguiente, haciendo asi una secuencia de comandos
el butilin.c define y ejecuta comandos internos, en este archivo se implementan los comandos cd, exit y help
y parsing.c lo que hace es recibir la entrada que da el usuario, la lee  en palabras y operadores,
y la tranforma en un scommand para que la shell lo pueda ejecutar.
Es el archivo que relaciona al usuario con la shell
mybash: Es una shell m  nima que imita el comportamiento de bash. Lee comandos que ingresa el usuario,
 los interpreta y luego los ejecuta. Soporta comandos internos como cd, exit y help, as
 como programas externos usando fork y execvp. Tambi  n maneja redirecciones de entrada y
salida y la creaci  n de pipelines entre procesos, permitiendo encadenar varios comandos.
Metodologia de trabajo:
El trabajo se dividio de la siguiente manera:
Armado de archivos commnad.c, execute.c , parsing.c , builtin.c lo realizarion Sebastian Arabia y Nahuel Acevedo.
Luego el armado e implementacion de mybash.c lo realizo Constanza Lobos

Uso de asistencia de Ia:
Hemos utilizado ChatGpt para orientarnos en las metologias del codigo, correccion de errores, sacarnos dudas particulares sobre que significa
cada modulo, tecnicas de programacion y explicacion de conceptos basicos del laboratorio.
