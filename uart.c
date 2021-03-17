
//Primeiramente, incluo as bibliotecas necesárias.
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include<avr/interrupt.h>
#define MAX_BUFFER 5

//Variáveis que auxiliarão a implementaçaõ do Buffer.
volatile char buffer[MAX_BUFFER];
volatile char add_buf = 0;
volatile char del_buf = 0;
volatile char ocup_buf = 0;

//Variáveis para o programa em geral:
unsigned char caractere;
unsigned char caractere_valido;
//Variáveis para a Interrupção
volatile int i =0;
volatile unsigned char sinalizacao=0;

// Strings
char c0[]="Comando: Apagar todos os LEDs\n";
char c1[]="Comando: Varredura com um LED aceso\n";
char c2[]="Comando: Varredura com um LED apagado\n" ;
char c3[]="Comando: Acender todos os LEDs\n";
char nn[]="Comando incorreto\n";
char v[]="Vazio!\n"  ;

//Ponteiros (Aqui, deixo tanto os das GPIOS quanto das Interrupções)

unsigned char *p_ubrr0h; // Ponteiro que será utilizado para definir o Baud Rate.
unsigned char *p_ubrr0l; // Ponteiro que será utilizado também para definir o Baud Rate.
unsigned char *p_ucsr0a; // Ponteiro que será utilizado para verificar se os dados foram transferidos, configurar a velocidade de transmissão dos dados e também a função de multiprocessamento.
unsigned char *p_ucsr0b; // Ponteiro que será utilziado para definir as interrupções; configurações de transmissão e recepção de dados; numero de bits.
unsigned char *p_ucsr0c; // Ponteiro para definir o modo de operaçaõ da USART; paridade;bits de parada.
unsigned char *p_udr0; // Ponteiro para o registrador de dados

//Ponteiros para a porta b do Atmega328P
unsigned char *p_ddrc;
unsigned char *p_portc;
/* Agora que ja tenho as variáveis e os ponteiros devidamente identificados,
vou começar a definir os parâmetros iniciais que o meu programa deve ter
por meio da função Setup.*/

void setup(){/*Aqui, logo no inicio eu começo desabilitando as interrupções do microcontrolador para evitar que durante o
setup dos ponteiros, não ocorra nenhum erro*/

cli();
  p_ubrr0h = (unsigned char *) 0xC5;
  p_ubrr0l = (unsigned char *) 0xC4;
  p_ucsr0a = (unsigned char *) 0xC0;
  p_ucsr0b = (unsigned char *) 0xC1;
  p_ucsr0c = (unsigned char *) 0xC2;
  p_udr0 = (unsigned char *) 0xC6;
  p_portc= (unsigned char *) 0x28;
  p_ddrc= (unsigned char* ) 0x27;

  // Enderecei cada ponteiro em seu respectivo registrador.

*p_ubrr0h |= 0x00;
*p_ubrr0l &= 0x00;
*p_ubrr0l |= 0x33; /*As configurações supracitadas em
  p_ubrr0h e p_ubrr0l são para definir um Baud Rate de 19.2kBps (51 nos regs)*/
*p_ucsr0a &= 0xFC; // Desabilito aqui a taxa de transmissão dobrada e o multiprocessamento.
*p_ucsr0b &= 0x03;
*p_ucsr0b |= 0x98; /* Aqui, habilito a rececpção completa e deixo o modo Registrador de Dados Vazio em baixa por enquanto,
visto que será ativado posteriormente*/
*p_ucsr0c &=0x00;
*p_ucsr0c |= 0x06; // Configuração para utilizar a USART em modo assincrono, 2 bits de parada, sem paridade.
*p_ddrc |= 0x38;
*p_portc &= 0xC7; // Definindo as saídas GPIO e iniciando os leds em 0.
sei();
}

//Funções que ja vieram no arquivo para o Buffer.
void adicionar_buffer(char c){
    /* Se o buffer nao esta cheio */
    if(ocup_buf < MAX_BUFFER){
        /* Adiciona valor no buffer */
      	buffer[add_buf] = c;
      	/* Incrementa o numero de posicoes utilizadas no buffer */
      	ocup_buf++;
        /* Incrementa condicionalmente o controle de posicao para adicionar.
           Se esta na ultima posicao, retorna pra primeira.
           Caso contrario, vai pra posicao seguinte. */
      	if(add_buf == (MAX_BUFFER-1)) add_buf=0;
      	else                          add_buf++;
    }
}
/* Funcao para remover valores do buffer */
char remover_buffer(){
    /* variavel auxiliar para capturar o caractere do buffer */
    char c;
    /* Se o buffer nao esta vazio */
    if (ocup_buf > 0){
    	/* Pega o caractere do buffer */
        c = buffer[del_buf];
      	/* Decrementa o numero de posicoes utilizadas no buffer */
      	ocup_buf--;
        /* Incrementa condicionalmente o controle de posicao para remover.
           Se esta na ultima posicao, retorna pra primeira.
           Caso contrario, vai pra posicao seguinte. */
      	if(del_buf == (MAX_BUFFER-1)) del_buf=0;
      	else                    	  del_buf++;
    }

    return c;
}


/* Aqui, eu defino uma função que tem como parâmetro um vetor de caracteres
para enviar com menos linhas de codigo o conteudo de um vetor de caracteres daqueles supracitados
para a USART (na instância de transmissão)*/

void para_Usart(char msg[]) {
  int i = 0;
  while(msg[i] != '\0') /* O Caractere '\0' define, por padrão, o final de uma string ou vetor de caracteres
    logo, coloco essa condição dentro do while para garantir que saiámos do while
    assim que o vetor for inteiramente transmitido*/
  { if((*p_ucsr0a & 0x20)==0x20){// verifica-se se o registrador UDR0 está vázio (UDRE0 = 1) esta pronto para receber
      *p_udr0 = msg[i]; // Aqui, enviamos de fato a mensagem para a USART
    i++;}
  }
}

/*Nas proximas linhas de código, implementei 4 funções
(1) Todos os Leds são apagados;
(2) Faço uma varredura por todos os leds com um led acesso! Fazendo:
0-0-1 ; 0-1-0; 1-0-0 e volta. Como 001=0x08, 010=0x10 100=0x20, temos a função
abaixo escrita.
(3)Varredura por todos os leds com um led apagado! Mesma lógica usada acima, mas
com as mascaras barradas
(4) Apago todos os leds */

void apaga_led(void){ *p_portc = 0x00;} //1
void liga_led(void){*p_portc |= 0x38;}//4
void varredura_aceso(void){//2
*p_portc = 0x08;
_delay_ms(500);
  *p_portc = 0x10;
  _delay_ms(500);
  *p_portc = 0x20;
  _delay_ms(500);
   *p_portc = 0x10;
  _delay_ms (500);
  *p_portc = 0x08;}
void varredura_apagado(void){
*p_portc = 0x30;
_delay_ms(500);
  *p_portc = 0x28;
  _delay_ms(500);
  *p_portc = 0x18;
  _delay_ms(500);
   *p_portc = 0x28;
  _delay_ms (500);
  *p_portc = 0x30;
}

/* A função a seguir, faz algumas operações para quando o buffer está
vazio (logica na main) e é preciso computar a ultima sequencia de operações validas*/
    void ultimo(){
  if (caractere_valido == '0') {
  apaga_led();}
  else if (caractere_valido == '1'){
    varredura_aceso(); }
  else if (caractere_valido == '2') {
    varredura_apagado();}
  else if (caractere_valido == '3')
  {liga_led();
  }}
 /*A função á seguir, verifica e executa o comando guardado em uma variável
 que pegará os valores da remoção de um item do buffer (o item que será computado)
 */
void comandos(void){
  if(caractere=='0'){
    para_Usart(c0);
    apaga_led();}
  else if (caractere =='1'){
    para_Usart(c1);
    varredura_aceso();}
  else if (caractere=='2'){
      para_Usart(c2);
    varredura_apagado();}
  else if(caractere=='3'){
    para_Usart(c3);
    liga_led();
  }

  }
  /*Note que para cada caso, eu chamo uma das funções que alteram o valor do LED, definidas
a partir da linha 136 e mando para a usart o anuncio de que vou começar a computar uma sequencia
de operações. Dando um breve descritivo do que será realizado. */


/* Aqui, defino oque a rotina de tratamento de interrupção da interrupçaõ
ligada à recepção faz. No caso, apenas adiciona um valor da entrada serial em um dos espaços
vazios do buffer.*/

ISR(USART_RX_vect){
   adicionar_buffer(*p_udr0);
}
/*Aqui, defino o que a rotina de tratamento de interrução da interrupção associada à transmissão faz.
No caso ela envia para o monitor, sempre que não há conteúdos no buffer, (lógica dentro da main) o vetor que
fala que o buffer está vazio! Quando o vetor é totalmente enviado, ela seta um sinalizador que será
utilizado futuramente e desabilita a interrupção para não ficar enviando o mesmo pacote. */
ISR(USART_UDRE_vect){
 if (v[i] != '\0') {
        *p_udr0 = v[i];
        i++; }
    else {
      	i = 0;
        sinalizacao = 1;
        *p_ucsr0b &= ~0x20; }}



int main(void){
setup(); /*Chamo a função Setup, já que é a partir daqui que o programa começa a
compilação, logo, é importante que as coisas sejam bem iniciadas.*/
 *p_ucsr0b |= 0x20; // Ativo as interrupções de transmissão, uma vez que as desativei no Setup.


 while(1){
    _delay_ms(1); //Otimização proposta pelo PAD em um vídeo (problema no TinkerCAD).
   if (ocup_buf != 0){/*Vou explicar oque eu fiz aqui dentro desse if:
               A. (1) A condição para a entrada no IF é se o nosso buffer ele tem algum dado
                para ser removido. Ou seja, se sua ocupação é diferente de zero.Então tomo o valor de saida do buffer.
                (2) A partir dai eu avalio se o comando é valido (0>=c<=3). Ai aqui eu desmembro em dois
                possiveis casos.
                (2.1) É valido. Então eu mando acontecer a função que trata o comando e envia a mensagem
                para a USART.
                (2.2) Não é valido. Então, eu mando a mensagem de invalido para a USART.
                B. O comando que responde o IF externo é um else if que faz a ultima operação valida com
                os leds, seta a sinalização em low e volta a enviar serialmente a mensagem de vazio (nessa programação
                eu ja tinha deixado na rotina.)Por ultimo, habilito denovo a interrupção que pode ter sido desabilitada
                em algum momento do programa. */
        caractere=remover_buffer();
    if (caractere >='0' && caractere <= '3') {
      caractere_valido = caractere;
      comandos();
    }
           else {para_Usart(nn);
              	ultimo();}
            _delay_ms(500); // Aqui eu espero os 500ms para a proxima operação desse loop.
        }
        else{ if (sinalizacao == 1) {sinalizacao = 0;
                ultimo();
              	_delay_ms(500); //Também aqui, espero 500ms para a proxima operação do loop.
                *p_ucsr0b |= 0x20; }}
}
return 0;
}
