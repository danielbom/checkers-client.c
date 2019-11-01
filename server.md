
# Projeto de Sistemas Distribuídos

## Lobby de Chat

### Servidor Generico de Mensagens
  - Responsável por armazenar as conexões dos clientes.
  - Responsável por tratar erros de conexões e disponibilizar meios simples de tratá-los.
  - Deve ser semelhante a um sistema de lobby.

### Cliente Generico de Mensagens
  - Deve ser capaz de criar salas do servidor de mensagens e também se conectar a elas.

  - Criar uma sala
  - Conectar a uma sala
  - Enviar mensagem

## Game

### Cliente do Jogo : Cliente Generico de Mensagens
  - Controla o andamento do jogo (Permitir o envio de mensagens)
  - Salas de tamanho fixo igual a 2

  - Criar um jogo (Criar)
  - Logar em um jogo (Connetar)

### **(Maybe)** Servidor do Jogo : Servidor Generico de Mensagens
  - Controle de pontuação dos jogadores
  - Ranking
