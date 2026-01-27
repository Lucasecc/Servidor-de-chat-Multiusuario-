========================================
    Servidor de Chat Multiusuário (TCP)
========================================

Este projeto implementa um sistema de chat cliente-servidor em C++, projetado para alta concorrência e robustez. O servidor é capaz de gerenciar múltiplas conexões de clientes simultaneamente, utilizando threads para isolar cada sessão. Toda a atividade do sistema é registrada de forma segura através de uma biblioteca de logging assíncrona e thread-safe (`libtslog`).


1. Funcionalidades Principais
------------------------------
* Servidor Multi-Thread: Utiliza `std::thread` para lidar com cada cliente em uma thread dedicada, permitindo que múltiplos clientes interajam simultaneamente sem bloquear o servidor.
* Broadcast de Mensagens: Mensagens enviadas por um cliente são retransmitidas para todos os outros clientes conectados à sala de chat.
* Logging Concorrente: Todas as ações (conexões, desconexões, mensagens, erros) são registradas em arquivos de log através da biblioteca `libtslog`, que opera de forma assíncrona para não impactar o desempenho da aplicação.
* Desligamento Gracioso: O servidor pode ser finalizado de forma segura com `Ctrl+C`, garantindo que todas as threads sejam finalizadas e os recursos (sockets, arquivos) sejam liberados corretamente.
* Cliente Interativo: Um cliente de linha de comando simples para conectar, enviar e receber mensagens em tempo real.
* Ferramenta de Teste de Carga: Uma aplicação dedicada para simular um grande número de clientes, permitindo testar a performance e a estabilidade do servidor sob estresse.

2. Estrutura do Projeto
------------------------

├── examples/
│   ├── client.cpp
│   ├── test_clients.cpp
│   └── test_logging.cpp
├── include/
│   └── libtslog.hpp
├── logs/
│   ├── server.log
│   └── ...
├── src/
│   ├── libtslog.cpp
│   └── server.cpp
└── Makefile

3. Pré-requisitos e Compilação
-------------------------------
Para compilar e executar este projeto, você precisará de:
* Um compilador C++ com suporte para C++11 ou superior (ex: g++).
* A biblioteca `pthreads` (geralmente incluída em sistemas Linux).
* Um sistema de build como `make` ou `cmake`.

**Exemplo de Compilação Manual (usando make):**

* Com o `Makefile` na raiz do projeto, execute o seguinte comando no terminal para compilar todos os programas:
  make

* Para limpar o projeto e remover todos os arquivos compilados:
  make clean

4. Como Executar
-----------------
Certifique-se de que os programas foram compilados e estão no diretório raiz do projeto.

**1. Iniciar o Servidor:**
Abra um terminal e execute o servidor, especificando a porta em que ele deve escutar.

Uso: ./server <porta>
Exemplo: ./server 8080

O servidor começará a escutar por novas conexões e criará o arquivo `server.log`.

**2. Conectar com o Cliente Interativo:**
Abra um novo terminal para cada cliente que desejar conectar.

Uso: ./client <ip_do_servidor> <porta>
Exemplo (conectando localmente): ./client 127.0.0.1 8080

Após conectar, digite suas mensagens e pressione Enter para enviá-las. As mensagens de outros usuários aparecerão no seu terminal. Para sair, digite `/quit` ou pressione `Ctrl+C`.

**3. Executar o Teste de Carga:**
Para simular múltiplos clientes e testar o desempenho do servidor, use a aplicação `test_clients`.

Uso: ./test_clients <ip_do_servidor> <porta> <num_clientes> <msgs_por_cliente>
Exemplo: ./test_clients 127.0.0.1 8080 20 10

Este comando irá simular 20 clientes, cada um enviando 10 mensagens para o servidor. Os logs detalhados da simulação serão salvos em `logs/test_clients.log`.

5. Detalhes da Arquitetura
---------------------------

* **Modelo de Concorrência:** O servidor utiliza o modelo **Thread-per-Connection**. Para cada cliente que se conecta, uma nova thread `std::thread` é instanciada para gerenciar todo o ciclo de vida daquela conexão. Isso garante que múltiplos clientes sejam tratados em paralelo sem que um cliente bloqueie o outro.

* **Sincronização:** A comunicação entre as threads e o acesso a dados compartilhados (como a lista de clientes conectados) são protegidos usando primitivas de sincronização do C++, como `std::mutex` e `std::lock_guard`, para evitar condições de corrida.

* **Logger Assíncrono:** A biblioteca `libtslog` implementa o padrão de design **Produtor-Consumidor**. As threads da aplicação (produtoras) simplesmente adicionam mensagens de log a uma fila concorrente. Uma única thread de background (consumidora) é responsável por retirar as mensagens da fila e escrevê-las no arquivo, desacoplando a operação de I/O (lenta) da lógica principal do programa.
