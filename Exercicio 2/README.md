# Exercício 2
Imagine uma situação onde você tem uma placa eletrônica com um microcontrolador e um botão. 
Você precisa verificar se o botão está pressionado (estado ON) ou não (estado OFF) e por quanto tempo o botão é mantido pressionado. 
Se o botão for pressionado, você também precisa ter uma definição clara do estado variando de ON, PROTECTED e OFF; conforme o diagrama de estado abaixo. 
O estado protected é definido como um estágio intermediário entre a transição do estado ON para o estado OFF, de modo que cada vez que o botão é liberado, o estado do botão muda para PROTECTED por 10 segundos antes de ir para o estado OFF. 
Implemente essa lógica de forma genérica usando a linguagem C e compartilhe esse código por meio de um repositório público no GitHub. 
Métodos de baixo nível podem ser abstraídos. 
Se preferir, use frameworks como Arduino, ESP32 ou até mesmo outras plataformas de mercado nessa abstração de código de baixo nível.

## Resposta teórica
Para a situação, podemos utilizar uma pequena máquina de estados finitos que utiliza um estado condicional e intertravado, desenvolvido apenas para essa aplicação.
O microcontrolador utilizado será o ESP32S3.

Segue link no WOKWI para ver a resolução do problema : https://wokwi.com/projects/428174829227723777

<details>
<summary>Exibir Código fonte </summary>

```cpp
/**
 * @file fsm_button.ino
 * @brief FSM para controle de botão com estados ON, PROTECTED e OFF.
 * 
 * @details
 * Esta FSM gerencia um botão utilizando estados finitos. Ao pressionar, entra em ON; ao soltar, entra em PROTECTED por 10 segundos. Se o botão não for pressionado novamente, vai para OFF.
 */

#include <Arduino.h>

/// Constantes
const int PIN_BUTTON = 2;
const unsigned long PROTECTED_TIMEOUT = 10000;   ///< Tempo de proteção após liberação do botão (ms)
const unsigned long DEBOUNCE_DELAY = 50;         ///< Tempo de debounce para leitura do botão (ms)
const unsigned long DEFAULT_BAUD_RATE = 9600;    ///<details Baud rate padrão para comunicação serial

/// Enum de Estados
enum FSMState {
  OFF,
  ON,
  PROTECTED
};

/// Classe FSM encapsulada
class ButtonFSM {
private:
  FSMState state;
  unsigned long pressStartTime;
  unsigned long protectedStartTime;

  bool isButtonPressed() {
    return digitalRead(PIN_BUTTON) == LOW;
  }

  void transitionTo(FSMState newState) {
    state = newState;
    switch (state) {
      case ON:
        pressStartTime = millis();
        Serial.printf("Estado: ON (Botão pressionado)\n");
        break;
      case PROTECTED:
        protectedStartTime = millis();
        Serial.printf("Estado: PROTECTED\n");
        break;
      case OFF:
        Serial.printf("Estado: OFF\n");
        break;
    }
  }

  void initializeSerial() {
    Serial.begin(DEFAULT_BAUD_RATE);
    delay(100); // Pequeno delay para garantir inicialização da Serial
  }

public:
  /**
   * @brief Construtor da FSM
   */
  ButtonFSM() : state(OFF), pressStartTime(0), protectedStartTime(0) {}

  /**
   * @brief Inicializa a FSM e configura o pino do botão.
   */
  void begin() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    initializeSerial();
  }

  /**
   * @brief Atualiza o estado da FSM com base na leitura do botão.
   */
  void update() {
    bool buttonPressed = isButtonPressed();
    switch (state) {
      case OFF:
        if (buttonPressed) {
          transitionTo(ON);
        }
        break;

      case ON:
        if (buttonPressed) {
          Serial.printf("Botão pressionado por %lu ms\n", millis() - pressStartTime);
        } else {
          Serial.printf("Botão liberado após %lu ms\n", millis() - pressStartTime);
          transitionTo(PROTECTED);
        }
        break;

      case PROTECTED:
        if (buttonPressed) {
          Serial.printf("Botão pressionado novamente.\n");
          transitionTo(ON);
        } else if (millis() - protectedStartTime >= PROTECTED_TIMEOUT) {
          Serial.printf("Tempo de proteção expirou.\n");
          transitionTo(OFF);
        }
        break;
    }
    delay(DEBOUNCE_DELAY); // Debounce simples
  }
};

ButtonFSM fsm;

void setup() {
  fsm.begin();
}

void loop() {
  fsm.update();
}

</details>
```