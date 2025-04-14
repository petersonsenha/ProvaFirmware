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
 