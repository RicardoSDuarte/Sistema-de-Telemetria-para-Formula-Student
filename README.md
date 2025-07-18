# 🏎️ Sistema de Telemetria para Formula Student

Este projeto tem como objetivo o desenvolvimento de um sistema de telemetria sem fios para um veículo da competição **Formula Student**, permitindo a **aquisição**, **transmissão**, **armazenamento** e **visualização em tempo real** de dados provenientes dos sensores do veículo, com base em comunicação via **CAN Bus** e **ESP-NOW**.

---

## 🚀 Funcionalidades

- 📡 Aquisição de dados via CAN Bus
- 📶 Transmissão sem fios utilizando ESP-NOW
- 🧰 Interface personalizada para filtragem e exportação de dados
- 🔗 Integração com o software SavvyCAN
- 💾 Gravação local em SPIFFS no microcontrolador
- 🛠️ PCB personalizada desenvolvida para o módulo ESP32RET

---

## 🛠️ Ferramentas e Software Utilizados

- **Desenvolvimento e Programação:** Arduino IDE, IntelliJ IDEA Community Edition  
- **Microcontrolador:** XiaoESP32s3  
- **Análise de Dados e Diagnóstico CAN:** SavvyCAN  
- **Design de PCB:** Altium Designer  
- **Documentação e Versionamento:** GitHub, LaTeX 

---

## 📁 Estrutura do Projeto
.
├── projeto/                  # Código-fonte do sistema
│   ├── microcontroladores/  # Código para os microcontroladores (ESP32)
│   └── aplicacao/           # Código da aplicação de visualização e interface
│
├── docs/                    # Documentação técnica
│   ├── relatorios/          # Relatórios do projeto em LaTeX/PDF
│   ├── pcb/                 # Ficheiros do design da PCB (Altium/KiCad)
│   └── datasheets/          # Datasheets dos componentes utilizados
│
├── README.md                # Ficheiro de apresentação do projeto
└── LICENSE                  # Licença de utilização do projeto


