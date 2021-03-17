Recepção
A interrupção do tipo “recepção completa” será explorada nesta atividade para que o microcontrolador
ATMega328P receba comandos de controle através do teclado do computador. Uma vez que o
microcontrolador pode estar realizando operações que consomem tempo enquanto novos comandos são
enviados pelo usuário, vamos utilizar a estrutura de um buffer circular para armazenar temporariamente a
sequência de comandos recebidos. O processamento dos comandos, então, se dará a partir do primeiro
elemento da fila (ou seja, do comando mais antigo armazenado no buffer). Por simplicidade, vamos
trabalhar com um buffer circular relativamente pequeno, de tamanho igual a 5 (cinco). A tabela abaixo
mostra os possíveis comandos e os respectivos efeitos sobre o conjunto de LEDs.



0 - Apaga todos os LEDs por 500ms 


1 - Varredura com um LED aceso (vai e volta), repetindo
ininterruptamente a sequência: 1-0-0; 500ms; 0-1-0; 500ms; 0-0-1;
500ms; 0-1-0; 500ms. (PC3-PC4-PC5; 0, LED apagado; 1, LED aceso)



2 - Varredura com um LED apagado, repetindo ininterruptamente a
sequência: 0-1-1; 500ms; 1-0-1; 500ms; 1-1-0; 500ms; 1-0-1; 500ms;


3 - Acende todos os LEDs por 500ms 



A rotina de serviço associada à recepção deve, portanto, inserir cada novo comando na próxima posição
livre do buffer circular, caso haja alguma.
Com isso, sempre que houver comandos disponíveis no buffer circular, o programa deve processar o
comando mais antigo. Além, disso, ao executar um comando, o sistema deve enviar uma mensagem de
confirmação pela porta serial, uma única vez. Caso não haja comandos a processar, o sistema repete a
execução do último comando válido. Se o comando não for válido, ou seja, se o caractere recebido for
diferente de 0, 1, 2 ou 3, o programa deve encaminhar uma mensagem de erro e permanecer repetindo o
último comando válido. A tabela abaixo apresenta as mensagens referentes a cada comando recebido.

Comando Mensagem
0 "Comando: Apagar todos os LEDs\n"
1 "Comando: Varredura com um LED aceso\n"
2 "Comando: Varredura com um LED apagado\n"
3 "Comando: Acender todos os LEDs\n"
Qualquer outro caractere "Comando incorreto\n"

Transmissão
Conforme já descrito, cada vez que um comando for processado, uma mensagem de texto deve ser
retornada pela porta serial, segundo a tabela já apresentada. Para enviar cada mensagem, i.e., cada
sequência de caracteres, vamos novamente utilizar o mecanismo de interrupção. No caso da transmissão,

há duas opções de eventos que disparam interrupções: (1) “transmissão completa” ou (2) “buffer de
transmissão vazio”.
Além disso, sempre que não houver comandos a processar no buffer circular (ou seja, o buffer está vazio), a
seguinte mensagem "Vazio!\n" deve ser enviada a cada 500ms.

Cuidado:
- A interrupção associada ao evento “buffer de transmissão vazio” é disparada continuamente caso o
buffer esteja limpo. Sendo assim, é preciso desligar esta interrupção ao terminar de transmitir uma
sequência completa de caracteres; caso contrário, uma nova interrupção será gerada mesmo se não
quisermos transmitir outra mensagem.
Lembrete: caso as rotinas de serviço de interrupção tenham de acessar ou modificar conteúdos de variáveis
do programa principal, é importante declará-las como variáveis globais com o qualificador volatile para
assegurar o correto funcionamento.


Especificações da USART:
- Velocidade de transmissão normal (i.e., modo double-speed desativado);
- Modo de transmissão multi-processador desabilitado;
- Número de bits de dados por frame igual a 8;
- Modo assíncrono de funcionamento da USART;
- Sem bits de paridade;
- Uso de um bit de parada;
- Baud rate igual a 19.200 bps.
