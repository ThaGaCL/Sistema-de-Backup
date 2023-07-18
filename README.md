# Sistema-de-Backup
## Universidade Federal do Paraná
### CI1058 – Redes de Computadores 1

Criar um sistema de backup e recuperação de arquivos em um servidor

#### Especificações:
- Utilizar C ou C++
- Utilizar RAWSOCKETS
- Em dupla
  
#### Implementar:
- Protocolo [Kermit](https://en.wikipedia.org/wiki/Kermit_(protocol)) (Implementado)
- Backup de um ou mais arquivos (Implementado)
- Recuperação de um ou mais arquivo (Implementado)
- Comando para modificar o diretorio de backup no servidor (Implementado)
- Comandos cd e ls locais (Implementado)
- Verificação do backup de um arquivo (Implementado)

#### Mensagem:
Marcador de Inicio (8 bits) | Tam (6 bits) | Seq (6 bits) | Tipo (4 bits) | Dados | Paridade Vertical (8 bits)
