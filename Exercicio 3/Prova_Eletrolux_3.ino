#include <Arduino.h>

// Constantes
const long SERIAL_BAUD_RATE = 9600;

/**
 * @brief Estrutura que contém estatísticas de um array.
 */
struct Estatisticas {
  float media;
  int max;
  int min;
  int* pares;
  int quantidadePares;

  Estatisticas()
    : media(0), max(0), min(0), pares(nullptr), quantidadePares(0) {}
};

/**
 * @brief Verifica se um número é par.
 * @details Esta função encapsula a lógica de paridade.
 * 
 * @param numero Número inteiro a ser verificado.
 * @return true Se o número for par.
 * @return false Caso contrário.
 */
bool isPar(int numero) {
  return numero % 2 == 0;
}

/**
 * @brief Calcula estatísticas de um array de inteiros.
 * 
 * @details A função executa duas passagens:
 * 1. Cálculo da média, máximo, mínimo e contagem dos números pares.
 * 2. Alocação e preenchimento de array com os números pares.
 * 
 * @param arr Array de entrada.
 * @param n Tamanho do array.
 * @return Estatisticas Estrutura contendo os dados calculados.
 */
Estatisticas calcularEstatisticas(const int arr[], int n) {
  Estatisticas estat;
  if (n <= 0) {
    return estat;  // Fail fast
  }

  long soma = 0;
  estat.max = arr[0];
  estat.min = arr[0];

  for (int i = 0; i < n; ++i) {
    int valor = arr[i];
    soma += valor;

    if (valor > estat.max) estat.max = valor;
    if (valor < estat.min) estat.min = valor;
    if (isPar(valor)) estat.quantidadePares++;
  }

  estat.media = static_cast<float>(soma) / n;

  estat.pares = new int[estat.quantidadePares];
  int indice = 0;
  for (int i = 0; i < n; ++i) {
    if (isPar(arr[i])) {
      estat.pares[indice++] = arr[i];
    }
  }

  return estat;
}

/**
 * @brief Exibe estatísticas no monitor serial.
 * 
 * @param estat Objeto com os dados estatísticos.
 */
void exibirEstatisticas(const Estatisticas& estat) {
  Serial.printf("Média: %.2f\n", estat.media);
  Serial.printf("Valor máximo: %d\n", estat.max);
  Serial.printf("Valor mínimo: %d\n", estat.min);
  Serial.printf("Números pares (%d): ", estat.quantidadePares);
  for (int i = 0; i < estat.quantidadePares; ++i) {
    Serial.printf("%d ", estat.pares[i]);
  }
  Serial.println();
}

/**
 * @brief Inicializa o Arduino e executa a lógica principal.
 */
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial);  // Aguarda conexão serial

  const int numeros[] = { 5, 8, 13, 24, 3, 42, 7, 16 };
  const int tamanho = sizeof(numeros) / sizeof(numeros[0]);

  Estatisticas estat = calcularEstatisticas(numeros, tamanho);
  exibirEstatisticas(estat);

  delete[] estat.pares;
}

/**
 * @brief Loop principal do Arduino.
 * 
 * @details Permanece vazio, pois a lógica principal é executada apenas uma vez no setup.
 */
void loop() {
  // Nada a ser feito aqui.
}
