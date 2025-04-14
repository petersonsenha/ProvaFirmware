const byte DEFAULT_SOF = 0x7E;
const byte DEFAULT_EOF = 0x7F;
const byte MAX_BUFFER_SIZE = 64;

/**
 * @class FrameConfig
 * @brief Estrutura de configuração de framing
 * @details Define os bytes de início (SOF) e fim (EOF) dos pacotes
 */
struct FrameConfig {
  byte sof;
  byte eof;

  /**
   * @brief Construtor com valores padrão ou customizados
   * @param start Byte SOF
   * @param end Byte EOF
   */
  FrameConfig(byte start = DEFAULT_SOF, byte end = DEFAULT_EOF)
      : sof(start), eof(end) {}
};

/**
 * @interface ICRC
 * @brief Interface para algoritmos de CRC
 */
class ICRC {
public:
  /**
   * @brief Calcula o CRC de um bloco de dados
   * @param data Ponteiro para dados de entrada
   * @param len Comprimento do bloco de dados
   * @return byte CRC calculado
   */
  virtual byte compute(const byte* data, byte len) = 0;
  virtual ~ICRC() = default;
};

/**
 * @class CRC8ATM
 * @brief Implementação do CRC8 com polinômio ATM (0x07)
 */
class CRC8ATM : public ICRC {
public:
  /**
   * @brief Implementação do cálculo CRC8-ATM
   * @param data Dados a serem processados
   * @param len Quantidade de bytes
   * @return byte Resultado do CRC8
   */
  byte compute(const byte* data, byte len) override {
    byte crc = 0x00;
    for (byte i = 0; i < len; i++) {
      crc ^= data[i];
      for (byte j = 0; j < 8; j++) {
        crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
      }
    }
    return crc;
  }
};

/**
 * @class SerialPacket
 * @brief Classe responsável pelo envio de pacotes com framing e CRC
 */
class SerialPacket {
private:
  ICRC* crcCalc;
  FrameConfig framing;

public:
  /**
   * @brief Construtor principal
   * @param crc Ponteiro para implementação de ICRC
   * @param config Estrutura FrameConfig com SOF e EOF
   */
  SerialPacket(ICRC* crc, FrameConfig config) : crcCalc(crc), framing(config) {}

  /**
   * @brief Envia um comando com dados encapsulados
   * @param cmd Comando identificador
   * @param data Ponteiro para dados do payload
   * @param dataLen Tamanho do payload em bytes
   */
  void send(byte cmd, const byte* data, byte dataLen) {
    if (dataLen + 1 > MAX_BUFFER_SIZE) {
      Serial.println("[ERRO] Dados excedem buffer permitido.");
      return;
    }

    byte len = dataLen + 1;
    byte buffer[MAX_BUFFER_SIZE];
    buffer[0] = cmd;
    memcpy(&buffer[1], data, dataLen);

    byte crc = crcCalc->compute(buffer, len);

    Serial.printf("[ENVIO] CMD: 0x%02X | Dados:", cmd);
    for (byte i = 0; i < dataLen; i++) Serial.printf(" 0x%02X", data[i]);
    Serial.printf(" | CRC: 0x%02X\n", crc);

    Serial.write(framing.sof);
    Serial.write(len);
    Serial.write(buffer, len);
    Serial.write(crc);
    Serial.write(framing.eof);
  }
};

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

/**
 * @class MeuParser
 * @brief Implementação concreta do parser para comandos específicos
 */
class MeuParser : public SerialPacketParser {
public:
  MeuParser(ICRC* crc, FrameConfig config) : SerialPacketParser(crc, config) {}

  void onPacketReceived(byte cmd, const byte* data, byte dataLen) override {
    SerialPacketParser::onPacketReceived(cmd, data, dataLen);
    if (cmd == 0x10 && dataLen == 2 && data[0] == 0xAA && data[1] == 0xBB) {
      Serial.println("-> Comando esperado recebido!");
    } else {
      Serial.println("-> Conteúdo inesperado.");
    }
  }
};

/**
 * @brief Decodifica 1 byte para uint8_t.
 * @param data Ponteiro para byte.
 * @return Valor uint8_t (0 a 255)
 */
uint8_t decodeUInt8(const byte* data) {
  return data[0];
}

/**
 * @brief Decodifica 1 byte para int8_t.
 * @param data Ponteiro para byte.
 * @return Valor int8_t (-128 a 127)
 */
int8_t decodeInt8(const byte* data) {
  return static_cast<int8_t>(data[0]);
}

/**
 * @brief Decodifica 4 bytes (little endian) para uint32_t.
 * @param data Ponteiro para os bytes.
 * @return Valor uint32_t
 */
uint32_t decodeUInt32(const byte* data) {
  return ((uint32_t)data[0]) |
         ((uint32_t)data[1] << 8) |
         ((uint32_t)data[2] << 16) |
         ((uint32_t)data[3] << 24);
}

/**
 * @brief Decodifica 4 bytes (little endian) para int32_t.
 * @param data Ponteiro para os bytes.
 * @return Valor int32_t
 */
int32_t decodeInt32(const byte* data) {
  return (int32_t)(
         ((uint32_t)data[0]) |
         ((uint32_t)data[1] << 8) |
         ((uint32_t)data[2] << 16) |
         ((uint32_t)data[3] << 24));
}
ICRC* crc;
SerialPacket* sender;
MeuParser* receiver;

/**
 * @brief Setup principal do Arduino
 */
void setup() {
  Serial.begin(9600);
  while(!Serial){}
  
  FrameConfig dispositivoA(DEFAULT_SOF, DEFAULT_EOF);
  crc = new CRC8ATM();
  sender = new SerialPacket(crc, dispositivoA);
  receiver = new MeuParser(crc, dispositivoA);

  byte dados[] = {0xAA, 0xBB};
  sender->send(0x10, dados, sizeof(dados));
}

/**
 * @brief Loop vazio pois a leitura é feita via evento serial
 */
void loop() {}

/**
 * @brief Evento automático chamado quando há dados na Serial
 */
void serialEvent() {
  while (Serial.available()) {
    byte c = Serial.read();
    receiver->processByte(c);
  }
}
