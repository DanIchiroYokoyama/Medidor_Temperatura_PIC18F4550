// Daniel Ichiro P. Y. Pereira 15635969
// Aljan Almeida Dias 15509165
// Conexões do LCD (PORTB -> PORTD para liberar RB0 e RB1)
sbit LCD_RS at RD4_bit;
sbit LCD_EN at RD5_bit;
sbit LCD_D4 at RD0_bit;
sbit LCD_D5 at RD1_bit;
sbit LCD_D6 at RD2_bit;
sbit LCD_D7 at RD3_bit;
sbit LCD_RS_Direction at TRISD4_bit;
sbit LCD_EN_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD0_bit;
sbit LCD_D5_Direction at TRISD1_bit;
sbit LCD_D6_Direction at TRISD2_bit;
sbit LCD_D7_Direction at TRISD3_bit;
// Conexão do LED (Representando a resistência do forno)
sbit LED_Forno at RE0_bit;
sbit LED_Forno_Direction at TRISE0_bit;
// Variáveis globais
volatile short tempo_ciclo = 0;
volatile unsigned short modo_ativo = 0; // 0: parado, 1: 60s (TMR0), 2: 10s (TMR1)
volatile unsigned short cont_250ms = 0;
volatile unsigned short flag_lcd = 0;
// Variáveis para leitura de temperatura (sem usar float)
unsigned int adc_val = 0;
unsigned int temp_int = 0; // Armazena a temperatura multiplicada por 10 (ex: 25.4°C = 254)
char txt_lcd[9]; // String aumentada para caber a formatação da temperatura (ex: " 100.0°C")
// Funções de Inicialização
void ini_timers(){
     // Configuração do Timer0 (Base de tempo de 1 segundo para o modo 60s)
     T0CON = 0x84;       // 16 Bits, Clk Interno, Prescaler 1:32
     TMR0H = 0x0B;       // Carga para estourar em 1s com cristal de 8MHz
     TMR0L = 0xDC;
     INTCON.TMR0IE = 1;  // Habilita interrupção do Timer0
     T0CON.TMR0ON = 0;   // Inicia desligado
     // Configuração do Timer1 (Base de tempo de 250ms para o modo 10s)
     T1CON = 0x31;       // 16 Bits, Clk Interno, Prescaler 1:8 (Bit 5-4 = 11)
     TMR1H = 0x0B;       // Carga para estourar em 250ms com cristal de 8MHz
     TMR1L = 0xDC;
     PIR1.TMR1IF = 0;    // Limpa a flag do Timer1
     PIE1.TMR1IE = 1;    // Habilita interrupção do Timer1
     T1CON.TMR1ON = 0;   // Inicia desligado
}
void ini_interr(){
     // Configuração dos pinos RB0 e RB1 como entrada
     TRISB.RB0 = 1;
     TRISB.RB1 = 1;
     // Configuração da interrupção na borda de subida
     INTCON2.INTEDG0 = 1;
     INTCON2.INTEDG1 = 1;
     // Limpa as flags das interrupções externas
     INTCON.INT0IF = 0;
     INTCON3.INT1IF = 0;
     // Habilita as interrupções externas INT0 e INT1
     INTCON.INT0IE = 1;
     INTCON3.INT1IE = 1;
     // Habilita as interrupções globais e de periféricos
     INTCON.PEIE = 1;
     INTCON.GIE = 1;
}
// Rotina de Serviço de Interrupção (ISR)
void interrupt(){
    // Botão 1 - INT0 (Inicia Ciclo Longo de 60s)
    if (INTCON.INT0IF){
       if (modo_ativo == 0){
          tempo_ciclo = 60;
          modo_ativo = 1;
          // Recarrega e inicia o Timer0
          TMR0H = 0x0B;
          TMR0L = 0xDC;
          T0CON.TMR0ON = 1;
          flag_lcd = 1;
       }
       INTCON.INT0IF = 0;
    }
    // Botão 2 - INT1 (Inicia Ciclo Curto de 10s)
    if (INTCON3.INT1IF){
       if (modo_ativo == 0){
          tempo_ciclo = 10;
          modo_ativo = 2;
          cont_250ms = 0;
          // Recarrega e inicia o Timer1
          TMR1H = 0x0B;
          TMR1L = 0xDC;
          T1CON.TMR1ON = 1;
          flag_lcd = 1;
       }
       INTCON3.INT1IF = 0;
    }
    // Interrupção do Timer0 (Estoura a cada 1 segundo no modo_ativo == 1)
    if (INTCON.TMR0IF){
        TMR0H = 0x0B; // Recarga do Timer0
        TMR0L = 0xDC;
        if (modo_ativo == 1) {
            tempo_ciclo--;
            if (tempo_ciclo <= 0) {
                tempo_ciclo = 0;
                modo_ativo = 0;   // Finaliza a contagem
                T0CON.TMR0ON = 0; // Desliga o Timer0
            }
            flag_lcd = 1; // Solicita atualização do LCD
        }
        INTCON.TMR0IF = 0;
    }
    // Interrupção do Timer1 (Estoura a cada 250ms no modo_ativo == 2)
    if (PIR1.TMR1IF){
        TMR1H = 0x0B; // Recarga do Timer1
        TMR1L = 0xDC;
        if (modo_ativo == 2) {
            cont_250ms++;
            if (cont_250ms >= 4) { // 4 * 250ms = 1 segundo
                cont_250ms = 0;
                tempo_ciclo--;
                if (tempo_ciclo <= 0) {
                    tempo_ciclo = 0;
                    modo_ativo = 0;   // Finaliza a contagem
                    T1CON.TMR1ON = 0; // Desliga o Timer1
                }
            }
            flag_lcd = 1; // Atualiza o LCD de forma regressiva contínua
        }
        PIR1.TMR1IF = 0;
    }
}
// Função para formatar e exibir valores inteiros como se fossem decimais (Sem Float)
void exibir_temperatura(unsigned int valor) {
    unsigned short centenas, dezenas, unidades, decimais;
    // Decompõe o número (Ex: 1000 -> centenas=1, dezenas=0, unidades=0, decimais=0)
    centenas = valor / 1000;
    dezenas  = (valor % 1000) / 100;
    unidades = (valor % 100) / 10;
    decimais = valor % 10;
    // Converte os algarismos para caracteres ASCII e joga no LCD
    if (centenas > 0) {
        txt_lcd[0] = centenas + '0';
    } else {
        txt_lcd[0] = ' ';
    }
    txt_lcd[1] = dezenas + '0';
    txt_lcd[2] = unidades + '0';
    txt_lcd[3] = '.';
    txt_lcd[4] = decimais + '0';
    txt_lcd[5] = 223; // ASCII correspondente ao símbolo de grau '°' no display LCD
    txt_lcd[6] = 'C';
    txt_lcd[7] = '\0'; // Terminador de string
    Lcd_Out(2, 7, txt_lcd); // Exibe na linha 2 a partir da coluna 7
}
void main(){
    // Configura o pino do LED como saída
    LED_Forno_Direction = 0;
    LED_Forno = 0; // Inicia desligado
    // Inicializações de Periféricos
    ini_timers();
    ini_interr();
    // Inicialização do LCD
    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);
    // Inicialização do ADC
    ADC_Init();
    // Correção do Bug descrita no PDF (página 7):
    // O valor 0x3B (0011 1011) configura os bits VCFG1 e VCFG0 em 1 para permitir
    // utilizar as tensões de referência externas conectadas aos pinos A2 e A3.
    ADCON1 = 0x3B;
    // Mensagem base fixa no display
    Lcd_Out(1, 1, "Tempo: 00s");
    Lcd_Out(2, 1, "Temp: ");
    while(1){
        // Tratamento da amostragem de temperatura e atualização do LCD
        if (flag_lcd == 1) {
            // 1. Atualiza o Tempo no LCD
            ByteToStr(tempo_ciclo, txt_lcd);
            // O comando ByteToStr gera espaços em branco à esquerda
            Lcd_Out(1, 8, txt_lcd + 1); // Exibe o tempo formatado na linha 1
            Lcd_Out(1, 11, "s ");
            // 2. Realiza a leitura analógica apenas se o processo estiver ativo
            if (modo_ativo != 0) {
                // Utilizando ADC_Get_Sample conforme recomendado no PDF para Vref externa
                adc_val = ADC_Get_Sample(0); // Lê o canal AN0 (RA0)
                // Cálculo da temperatura sem usar FLOAT:
                // Com Vref de 1.0V e ADC de 10 bits (1023 passos).
                // O LM35 fornece 10mV/°C. Multiplicamos por 10 para preservar uma casa decimal:
                // Cast para unsigned long impede overflow da variável de 16-bits antes da divisão.
                temp_int = (unsigned int)(((unsigned long)adc_val * 1000) / 1023);
                // Exibe a temperatura formatada como XX.X °C
                exibir_temperatura(temp_int);
                // 3. Controle da Resistência (LED)
                // Requisito: "Adicionar um LED que acende para temperaturas maiores que 50 °C"
                if (temp_int > 500) {       // Maior que 50.0 °C -> Liga
                    LED_Forno = 1;
                } else {                    // Caso contrário -> Desliga
                    LED_Forno = 0;
                }
            } else {
                // Se a contagem acabou, zera os indicadores visuais padrão ou mantém congelado
                Lcd_Out(1, 8, " 00");
                LED_Forno = 0; // Desliga o forno ao fim do ciclo
            }
            flag_lcd = 0; // Limpa a flag para aguardar o próximo evento de tempo
        }
    }
}