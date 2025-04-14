# Exercicio 4
# Problema 4
## Da problemática
1) Imagine que você tem um microcontrolador que se comunica com um sistema genérico, que pode consistir em várias outras placas, via UART. Como você garante que cada mensagem seja enviada corretamente e que seu conteúdo esteja correto?

2) Agora imagine que você recebe uma mensagem e uma interrupção é acionada sempre que uma nova informação é recebida. Dentro de cada mensagem, você tem um comando diferente e, para cada comando, você tem diferentes payloads, recebendo valores positivos e negativos.

3) Implemente as funções de interrupção e parser de forma genérica usando a linguagem C e compartilhe esse código em um repositório público no GitHub. 

OBS : Métodos de baixo nível podem ser abstraídos. Se preferir, use frameworks como Arduino, ESP32 ou mesmo outras plataformas de mercado nessa abstração de código de baixo nível.
## Resposta 1
Podemos utilizar o CRC - _Cyclic Redundancy Check_ para fazer a verificação dos dados enviados. Existem alguns tipos de CRC, o mais apropriado para microcontroladores e comunicação serial é o CRC-8.
## Resposta 2 - Interrupção
Para cada interrupção, cada microcontrolador possui um registrador e uma interrupção para quando recebe uma informação, para o caso em questão do Arduino IDE, ele tem abstraído o SerialEvent() no qual quando receber dados na serial, ele para tudo que está fazendo e vai para a rotina SerialEvent().
## Resposta 2 - Solução genérica para diferentes dispositivos
Para a resolução eu proponho uma solução genérica para fazer o envio e recebimento de dados com payloads configuráveis:

[SOF][TAMANHO][COMANDO][DADOS][CRC][EOF]

Sendo : 
* SOF : Start of Frame (início do quadro)
* TAMANHO : Tamanho em quantidade em bytes
* COMANDO : Comando de referência para saber qual dado está enviando
* DADOS : Informações referentes ao comando
* CRC : CRC-8 que fará a verificação da qualidade do pacote
* EOF : End of Frame (Final do quadro)

Para cada conjunto de SOF e EOF pode ser relacionado a um novo dispositivo na rede UART, entregando uma comunicação única para dispositivos e identificar comandos e valores diferentes para cada dispositivo.

## Resposta 2 - Solução genérica para dados positivos e negativos

Para os tipos de dados, podemos utilizar a utilização de dados de 1 bit, 8 bits(1 byte), 16 bits(2 bytes), 32 bits(4 bytes) e 64 bits(8 bytes).

## Tabela de Tipos de Variáveis em C++ (Dados Positivos e Negativos)

| Tipo           | Nome Descritivo         | Tamanho (Bytes) | Tamanho (Bits) | Faixa de Valores                            |
|----------------|--------------------------|------------------|----------------|---------------------------------------------|
| `bool`         | Booleano                 | 1                | 8              | `true` ou `false`                            |
| `char`         | Caractere com sinal      | 1                | 8              | -128 a 127                                   |
| `unsigned char`| Caractere sem sinal      | 1                | 8              | 0 a 255                                      |
| `int8_t`       | Inteiro 8 bits (signed)  | 1                | 8              | -128 a 127                                   |
| `uint8_t`      | Inteiro 8 bits (unsigned)| 1                | 8              | 0 a 255                                      |
| `short`        | Inteiro curto (signed)   | 2                | 16             | -32.768 a 32.767                             |
| `unsigned short`| Inteiro curto (unsigned)| 2                | 16             | 0 a 65.535                                   |
| `int16_t`      | Inteiro 16 bits (signed) | 2                | 16             | -32.768 a 32.767                             |
| `uint16_t`     | Inteiro 16 bits (unsigned)| 2               | 16             | 0 a 65.535                                   |
| `int`          | Inteiro padrão (signed)  | 4                | 32             | -2.147.483.648 a 2.147.483.647               |
| `unsigned int` | Inteiro sem sinal        | 4                | 32             | 0 a 4.294.967.295                            |
| `int32_t`      | Inteiro 32 bits (signed) | 4                | 32             | -2.147.483.648 a 2.147.483.647               |
| `uint32_t`     | Inteiro 32 bits (unsigned)| 4               | 32             | 0 a 4.294.967.295                            |
| `long`         | Longo (signed)           | 4 ou 8           | 32 ou 64       | Depende da arquitetura (32 ou 64 bits)      |
| `unsigned long`| Longo sem sinal          | 4 ou 8           | 32 ou 64       | Depende da arquitetura                      |
| `int64_t`      | Inteiro 64 bits (signed) | 8                | 64             | ±9.223.372.036.854.775.808                   |
| `uint64_t`     | Inteiro 64 bits (unsigned)| 8               | 64             | 0 a 18.446.744.073.709.551.615              |
| `float`        | Ponto flutuante simples  | 4                | 32             | ±1.2×10⁻³⁸ a ±3.4×10³⁸ (com precisão ~7 dígitos) |
| `double`       | Ponto flutuante duplo    | 8                | 64             | ±2.3×10⁻³⁰⁸ a ±1.7×10³⁰⁸ (com precisão ~15 dígitos) |

_OBS: Gerado por chatGPT e conferido por [apostila da Unicamp](https://www.ic.unicamp.br/~ducatte/mc102/aula03.pdf)_

Para a nossa aplicação e para efeito do exemplo, iremos utilizar o int8_t e uint8_t, para demonstrar a mudança de sinal em apenas um byte e um uint32_t e int32_t para demonstrar a mudança de sinal com 4 bytes.

## Resposta 3 - Implementação do código
Das implementações propostas, segue a lista:
* SerialEvent() como interrupção genérica de recepção de dados via UART
* SerialParserPacket para recepção dos dados
* Verificação por meio de CRC-8
* Implementação de um SOF e EOF para cada dispositivo personalizado (Overdelivery)
* 

### Serial Event
<details>
  <summary>Ver código SerialEvent</summary>

```cpp
/**
 * @brief Evento automático chamado quando há dados na Serial
 */
void serialEvent() {
  while (Serial.available()) {
    byte c = Serial.read();
    receiver->processByte(c);
  }
}
```
</details>

### SerialParserPacket
<details>
  <summary>Ver código SerialParserPacket</summary>
  
```cpp
/**
 * @class SerialPacketParser
 * @brief Classe responsável por processar pacotes recebidos
 */
class SerialPacketParser {
private:
  ICRC* crcCalc;
  FrameConfig framing;

  enum State { WAIT_HEADER, READ_LENGTH, READ_PAYLOAD, READ_CRC, READ_FOOTER };
  State state = WAIT_HEADER;

  byte buffer[MAX_BUFFER_SIZE];
  byte index = 0;
  byte length = 0;
  byte receivedCRC = 0;

  /**
   * @brief Reinicia a máquina de estados
   */
  void reset() {
    index = 0;
    state = WAIT_HEADER;
  }

public:
  /**
   * @brief Construtor principal
   * @param crc Algoritmo de CRC utilizado
   * @param config Configuração de framing
   */
  SerialPacketParser(ICRC* crc, FrameConfig config)
      : crcCalc(crc), framing(config) {}

  /**
   * @brief Processa byte recebido, interpretando protocolo serial
   * @param byteRecebido Byte individual recebido
   * @return true se pacote válido e completo, false caso contrário
   */
  bool processByte(byte byteRecebido) {
    switch (state) {
      case WAIT_HEADER:
        if (byteRecebido == framing.sof) {
          index = 0;
          state = READ_LENGTH;
        }
        break;

      case READ_LENGTH:
        length = byteRecebido;
        if (length > MAX_BUFFER_SIZE - 2) {
          Serial.println("[ERRO] Tamanho inválido.");
          reset();
          return false;
        }
        state = READ_PAYLOAD;
        break;

      case READ_PAYLOAD:
        buffer[index++] = byteRecebido;
        if (index >= length) state = READ_CRC;
        break;

      case READ_CRC:
        receivedCRC = byteRecebido;
        state = READ_FOOTER;
        break;

      case READ_FOOTER:
        if (byteRecebido != framing.eof) {
          Serial.println("[ERRO] EOF inválido.");
          reset();
          return false;
        }

        byte crcCalculated = crcCalc->compute(buffer, length);
        Serial.printf("[DEBUG] CRC recebido: 0x%02X | calculado: 0x%02X\n",
                      receivedCRC, crcCalculated);

        if (crcCalculated != receivedCRC) {
          Serial.println("[ERRO] CRC inválido.");
          reset();
          return false;
        }

        Serial.println("[OK] CRC válido.");
        onPacketReceived(buffer[0], &buffer[1], length - 1);
        reset();
        return true;
    }
    return false;
  }
  /**
   * @brief Manipula o pacote válido recebido
   * @param cmd Comando do pacote
   * @param data Dados recebidos
   * @param dataLen Tamanho dos dados
   */
  virtual void onPacketReceived(byte cmd, const byte* data, byte dataLen) {
    Serial.printf("[✔] CMD=0x%02X | Dados:", cmd);
    for (byte i = 0; i < dataLen; i++) Serial.printf(" 0x%02X", data[i]);
    Serial.println();
  }
};

```
</details>



