import javax.swing.*;
import javax.swing.border.TitledBorder;
import javax.swing.border.LineBorder;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.io.*;
import java.util.List;
import java.util.ArrayList;
import com.fazecast.jSerialComm.SerialPort;

public class InterfaceSerial extends JFrame {

    private JTextArea inputArea;
    private JTextArea serialMonitor;
    private SerialPort portaSelecionada;
    private JMenu portaMenu;
    private boolean filtroLigado = false;
    private Thread leituraThread;
    private boolean leituraAtiva = false;

    public InterfaceSerial() {
        setTitle("Interface Serial");
        setIconImage(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/IFS.png")));
        setSize(600, 400);
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        // MENU
        JMenuBar menuBar = new JMenuBar();
        JMenu menuPrincipal = new JMenu("Porta COM");
        portaMenu = new JMenu("Selecionar Porta");
        atualizarMenuPortas();
        menuPrincipal.add(portaMenu);
        menuBar.add(menuPrincipal);
        setJMenuBar(menuBar);

        // Áreas de texto
        JPanel topPanel = new JPanel(new GridLayout(2, 1, 10, 10));
        inputArea = new JTextArea();
        serialMonitor = new JTextArea();
        serialMonitor.setEditable(false);

        JScrollPane inputScroll = new JScrollPane(inputArea);
        JScrollPane monitorScroll = new JScrollPane(serialMonitor);

        LineBorder thickBorder = new LineBorder(Color.GRAY, 3);
        inputScroll.setBorder(BorderFactory.createTitledBorder(thickBorder, "INPUT", TitledBorder.LEFT, TitledBorder.TOP));
        monitorScroll.setBorder(BorderFactory.createTitledBorder(thickBorder, "SERIAL MONITOR", TitledBorder.LEFT, TitledBorder.TOP));

        topPanel.add(inputScroll);
        topPanel.add(monitorScroll);

        // Botões
        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 20, 10));
        JButton filtroBtn = new JButton("Filter ON/OFF");
        JButton clearBtn = new JButton("Filter CLEAR");
        JButton sendBtn = new JButton("SEND ID");
        JButton sendFilter = new JButton("SEND Filter");
        JButton desconectarBtn = new JButton("Desconectar");

        filtroBtn.addActionListener((ActionEvent e) -> {
            if (!portaAberta()) return;
            try (OutputStream out = portaSelecionada.getOutputStream()) {
                String comando = filtroLigado ? "F1FD00" : "F1FD01";
                filtroLigado = !filtroLigado;
                enviarComandoBinario(out, comando);
                appendToSerialMonitor(filtroLigado ? "\nFiltro LIGADO" : "\nFiltro DESLIGADO");
            } catch (IOException ex) {
                appendToSerialMonitor("\nErro ao enviar: " + ex.getMessage());
            }
        });

        clearBtn.addActionListener((ActionEvent e) -> {
            if (!portaAberta()) return;
            try (OutputStream out = portaSelecionada.getOutputStream()) {
                enviarComandoBinario(out, "F1FD10");
                appendToSerialMonitor("\nFiltro LIMPO");
            } catch (IOException ex) {
                appendToSerialMonitor("\nErro ao enviar: " + ex.getMessage());
            }
        });

        sendBtn.addActionListener((ActionEvent e) -> {
            String input = inputArea.getText().trim();
            if (!input.isEmpty()) {
                processarEnvioID(input);
            }
        });

        sendFilter.addActionListener((ActionEvent e) -> new Thread(this::processarEnvioFiltro).start());

        desconectarBtn.addActionListener((ActionEvent e) -> desconectarPorta());

        buttonPanel.add(filtroBtn);
        buttonPanel.add(clearBtn);
        buttonPanel.add(sendBtn);
        buttonPanel.add(sendFilter);
        buttonPanel.add(desconectarBtn);

        add(topPanel, BorderLayout.CENTER);
        add(buttonPanel, BorderLayout.SOUTH);
    }

    private void atualizarMenuPortas() {
        portaMenu.removeAll();
        SerialPort[] ports = SerialPort.getCommPorts();

        if (ports.length == 0) {
            JMenuItem semPortas = new JMenuItem("Nenhuma porta encontrada");
            semPortas.setEnabled(false);
            portaMenu.add(semPortas);
            return;
        }

        for (SerialPort porta : ports) {
            JMenuItem item = new JMenuItem(porta.getSystemPortName());
            item.addActionListener(e -> conectarPorta(porta));
            portaMenu.add(item);
        }
    }

    private void conectarPorta(SerialPort porta) {
        if (portaSelecionada != null && portaSelecionada.isOpen()) {
            portaSelecionada.closePort();
        }

        portaSelecionada = porta;
        portaSelecionada.setBaudRate(1_000_000);

        if (portaSelecionada.openPort()) {
            appendToSerialMonitor("\nPorta selecionada: " + porta.getSystemPortName());
            iniciarLeituraContinua();
        } else {
            appendToSerialMonitor("\nErro ao abrir a porta.");
        }
    }

    private void desconectarPorta() {
        if (portaSelecionada != null && portaSelecionada.isOpen()) {
            leituraAtiva = false;
            if (leituraThread != null) leituraThread.interrupt();
            portaSelecionada.closePort();
            appendToSerialMonitor("\nPorta desconectada.");
        }
        portaSelecionada = null;
    }

    private void iniciarLeituraContinua() {
        leituraAtiva = true;
        leituraThread = new Thread(() -> {
            try {
                InputStream in = portaSelecionada.getInputStream();
                StringBuilder buffer = new StringBuilder();

                while (leituraAtiva) {
                    while (in.available() > 0) {
                        char c = (char) in.read();
                        if (c == '\n') {
                            appendToSerialMonitor("\nRX: " + buffer.toString().trim());
                            buffer.setLength(0);
                        } else {
                            buffer.append(c);
                        }
                    }
                    Thread.sleep(100);
                }
            } catch (Exception e) {
                appendToSerialMonitor("\n[Erro leitura contínua]: " + e.getMessage());
            }
        });

        leituraThread.start();
    }

    private boolean portaAberta() {
        if (portaSelecionada == null || !portaSelecionada.isOpen()) {
            appendToSerialMonitor("\nSelecione uma porta COM primeiro no menu.");
            return false;
        }
        return true;
    }

    private void enviarTextoParaSerial(String texto) {
        if (!portaAberta()) return;

        try (OutputStream out = portaSelecionada.getOutputStream()) {
            byte[] dados = (texto + "\n").getBytes();
            out.write(dados);
            out.flush();
            appendToSerialMonitor("\nTX: " + texto);
        } catch (IOException ex) {
            appendToSerialMonitor("\nErro ao enviar: " + ex.getMessage());
        }
    }

    private void enviarComandoBinario(OutputStream out, String comando) throws IOException {
        if (comando.length() < 4 || !comando.startsWith("F1")) {
            appendToSerialMonitor("\nComando inválido: " + comando);
            return;
        }

        try {
            byte header = (byte) 0xF1;
            byte tipo = (byte) Integer.parseInt(comando.substring(2, 4), 16);

            String idHex = comando.substring(4);
            long id = Long.parseLong(idHex, 16);  // agora ID pode ter até 4 bytes
            if (id < 0 || id > 0xFFFFFFFFL) {
                appendToSerialMonitor("\nID fora do intervalo (0x00000000–0xFFFFFFFF): " + idHex);
                return;
            }

            int tamanhoID = (id <= 0xFF) ? 1 : (id <= 0xFFFF) ? 2 : (id <= 0xFFFFFF) ? 3 : 4;
            byte[] buffer = new byte[2 + tamanhoID];
            buffer[0] = header;
            buffer[1] = tipo;

            // Inserindo os bytes em ordem big-endian (MSB primeiro)
            for (int i = 0; i < tamanhoID; i++) {
                buffer[2 + i] = (byte) ((id >> (8 * (tamanhoID - 1 - i))) & 0xFF);
            }

            out.write(buffer);
            out.flush();

            // Mostrar os bytes transmitidos
            StringBuilder sb = new StringBuilder("\nTX (bytes):");
            for (byte b : buffer) {
                sb.append(String.format(" %02X", b));
            }
            appendToSerialMonitor(sb.toString());

        } catch (NumberFormatException e) {
            appendToSerialMonitor("\nErro no formato do comando: " + comando);
        }
    }




    private void dormir() {
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    private void processarEnvioID(String input) {
        if (!portaAberta()) return;

        String[] partes = input.split(",");
        if (partes.length != 2) {
            appendToSerialMonitor("\nFormato inválido. Use: 0xID,T ou 0xID,F");
            return;
        }

        String idStr = partes[0].trim();
        String tipo = partes[1].trim().toUpperCase();

        if (!idStr.startsWith("0x") && !idStr.startsWith("0X")) {
            appendToSerialMonitor("\nID deve começar com '0x'. Ex: 0x7F,T");
            return;
        }

        try {
            int id = Integer.parseInt(idStr.substring(2), 16);
            if (id < 0 || id > 0xFFFFFF) {
                appendToSerialMonitor("\nID fora do intervalo (0x000000–0xFFFFFF): " + idStr);
                return;
            }

            if (!tipo.equals("T") && !tipo.equals("F")) {
                appendToSerialMonitor("\nTipo inválido. Use T ou F");
                return;
            }

            appendToSerialMonitor(String.format("\nID a adicionar: 0x%06X (%s)", id, tipo));


            String comando = tipo.equals("T") ? "F1FE" + String.format("%06X", id)
                    : "F1FF" + String.format("%06X", id);

            try (OutputStream out = portaSelecionada.getOutputStream()) {
                enviarComandoBinario(out, comando);
            }
        } catch (NumberFormatException ex) {
            appendToSerialMonitor("\nID hexadecimal inválido: " + idStr);
        } catch (IOException ex) {
            appendToSerialMonitor("\nErro ao enviar: " + ex.getMessage());
        }
    }

    private void processarEnvioFiltro() {
        if (!portaAberta()) return;

        JFileChooser chooser = new JFileChooser();
        chooser.setDialogTitle("Selecione o ficheiro com os IDs");
        int resultado = chooser.showOpenDialog(this);

        if (resultado != JFileChooser.APPROVE_OPTION) return;

        File ficheiro = chooser.getSelectedFile();
        List<String[]> buffer = new ArrayList<>();

        try (BufferedReader leitor = new BufferedReader(new FileReader(ficheiro))) {
            String linha;
            while ((linha = leitor.readLine()) != null) {
                String[] partes = linha.split(",");
                if (partes.length == 2) {
                    buffer.add(new String[]{partes[0].trim(), partes[1].trim().toUpperCase()});
                }
            }
        } catch (IOException e) {
            appendToSerialMonitor("\nErro ao ler o ficheiro: " + e.getMessage());
            return;
        }

        appendToSerialMonitor("\nTotal de IDs encontrados: " + buffer.size());

        try (OutputStream out = portaSelecionada.getOutputStream()) {
            for (String[] partes : buffer) {
                String id = partes[0];
                String tipo = partes[1];

                String comando = switch (tipo) {
                    case "T" -> "F1FE" + String.format("%06X", Integer.parseInt(id, 16));
                    case "F" -> "F1FF" + String.format("%06X", Integer.parseInt(id, 16));
                    default -> null;
                };

                if (comando == null) continue;

                enviarComandoBinario(out, comando);
                dormir();
            }

            appendToSerialMonitor("\nEnvio finalizado.");
        } catch (IOException e) {
            appendToSerialMonitor("\nErro ao enviar: " + e.getMessage());
        }
    }

    // Método para autoscroll
    private void appendToSerialMonitor(String texto) {
        serialMonitor.append(texto);
        serialMonitor.setCaretPosition(serialMonitor.getDocument().getLength());
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new InterfaceSerial().setVisible(true));
    }
}
