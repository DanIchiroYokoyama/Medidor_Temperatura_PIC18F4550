# Projeto 2: Aferidor de Temperatura de Forno Industrial

**Alunos:**
- Daniel Ichiro P. Y. Pereira (Nº USP: 15635969)
- Aljan Almeida Dias (Nº USP: 15509165)

## Introdução
Este projeto consiste no desenvolvimento de um firmware para o microcontrolador PIC18F4550 para aferição da temperatura e tempo de um forno industrial. A aplicação utiliza interrupções externas para iniciar ciclos de longa (60s) e curta (10s) duração por meio de dois temporizadores (Timer0 e Timer1). A leitura da temperatura é simulada por um potenciômetro emulando um sensor LM35 via conversor Analógico-Digital (ADC) com tensão externa de referência (Vref) fixada em 1V. Além disso, conta com um LED simulando a ativação da resistência, ligando em temperaturas superiores a 50 °C.

## Discussão dos Resultados
Os timers foram configurados em 1 segundo e 250 milissegundos corretamente baseados no *clock* e *prescaler*. Foi necessário ajustar os bits do registrador ADCON1 para 0x3B de modo a utilizar referências externas nos pinos RA2 e RA3 sem sofrer com o conflito da biblioteca padrão do MikroC. A temperatura simulada refletiu precisamente a faixa de 0 a 100 °C exibida no Display LCD no formato "XX.X °C", dispensando variáveis do tipo *float* para preservar a memória. A resistência térmica (simulada pelo LED) ativou perfeitamente nos limiares exigidos pelo projeto (acima de 50.0 °C).

## Imagens do Projeto

### Compilação Bem-Sucedida
*(Neste trecho, troque o nome do arquivo da imagem pelo exato arquivo que você subiu)*
![Compilação no MikroC](nome_da_sua_imagem_de_compilacao.png)

### Execução da Simulação
![Execução no SimulIDE](nome_da_sua_imagem_de_simulacao.png)
