# Sistema de Telemetria para Formula Student

Este projeto teve como objetivo desenvolver um sistema de telemetria para um veículo da competição Formula Student, permitindo a aquisição, transmissão, armazenamento e visualização em tempo real de dados provenientes dos sensores do veículo.

## 🚀 Funcionalidades
- Aquisição de dados via CAN Bus
- Transmissão sem fios (ESP-NOW)
- Interface de filtragem de dados
- Integração com SavvyCAN
- Gravação local em SPIFFS
- PCB personalizada para o ESP32RET

## 🔧 Tecnologias Usadas
- ESP32 / Arduino IDE
- ESP-NOW
- SavvyCAN
- CAN Bus
- SPIFFS / JSON
- KiCad (PCB)

## 📁 Estrutura do Projeto
/Telemetria-FS
│
├── README.md <- Documento principal com descrição do projeto
├── LICENSE <- Licença MIT
├── docs/ <- Documentação técnica (Relatório_Final.pdf, esquemas, datasheets)
│ └── Relatorio_Final.pdf
│
├── code/ <- Código-fonte para os diferentes módulos
│ ├── ESP32RET/ <- Código para o módulo receptor (ESP32RET)
│ ├── CAN_Sniffer/ <- Código para leitura dos dados do CAN
│ └── filter_tool/ <- Ferramenta para filtragem e exportação de dados
│
├── pcb/ <- Ficheiros do projeto de PCB (KiCad, Gerbers, esquemas)
│ ├── esquematico.kicad_sch
│ ├── layout.kicad_pcb
│ └── renders/
│ └── top_view.png
│
├── test_data/ <- Dados recolhidos durante os testes de campo
│ └── run_200m_test.csv
│
├── images/ <- Diagramas e capturas de ecrã usados no README
│ ├── arquitetura_sistema.png
│ ├── espnow_fluxograma.png
│ └── interface_filtragem.png
│
└── .gitignore <- Lista de ficheiros a ignorar pelo Git
