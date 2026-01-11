package com.swagaria.util;

import com.swagaria.network.Server;
import com.swagaria.data.terrain.TerrainConfig;

import javax.swing.*;
import javax.swing.text.DefaultCaret;
import java.awt.*;
import java.io.OutputStream;
import java.io.PrintStream;

public class ServerGUI extends JFrame
{
    private JTextArea consoleArea;
    private final JTextField seedField;
    private final JSpinner playerLimitSpinner;
    private final JButton startButton;
    private final JButton stopButton;
    private final JLabel statusLabel;

    private Server serverInstance;

    public ServerGUI()
    {
        setTitle("Swagaria Server Manager");
        setSize(800, 600);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);

        JTabbedPane tabbedPane = new JTabbedPane();

        //server tab
        JPanel serverPanel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(10, 10, 10, 10);
        gbc.fill = GridBagConstraints.HORIZONTAL;

        //status
        gbc.gridx = 0; gbc.gridy = 0;
        serverPanel.add(new JLabel("Status:"), gbc);
        statusLabel = new JLabel("Offline");
        statusLabel.setForeground(Color.RED);
        statusLabel.setFont(new Font("SansSerif", Font.BOLD, 14));
        gbc.gridx = 1;
        serverPanel.add(statusLabel, gbc);

        //seed input
        gbc.gridx = 0; gbc.gridy = 1;
        serverPanel.add(new JLabel("World Seed:"), gbc);
        seedField = new JTextField(String.valueOf(System.currentTimeMillis()));
        gbc.gridx = 1;
        serverPanel.add(seedField, gbc);

        //player limit
        gbc.gridx = 0; gbc.gridy = 2;
        serverPanel.add(new JLabel("Player Limit:"), gbc);
        playerLimitSpinner = new JSpinner(new SpinnerNumberModel(10, 1, 100, 1));
        gbc.gridx = 1;
        serverPanel.add(playerLimitSpinner, gbc);

        //buttons
        JPanel controlPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 20, 10));
        startButton = new JButton("Start Server");
        stopButton = new JButton("Stop Server");
        stopButton.setEnabled(false);
        controlPanel.add(startButton);
        controlPanel.add(stopButton);

        gbc.gridx = 0; gbc.gridy = 3; gbc.gridwidth = 2;
        serverPanel.add(controlPanel, gbc);

        //console tab
        JPanel consolePanel = new JPanel(new BorderLayout());

        //clear console button
        JPanel consoleHeader = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        JButton clearButton = new JButton("Clear Console");
        clearButton.setFocusable(false);
        clearButton.addActionListener(e -> consoleArea.setText(""));
        consoleHeader.add(clearButton);

        consoleArea = new JTextArea();
        consoleArea.setEditable(false);
        consoleArea.setBackground(new Color(30, 30, 30));
        consoleArea.setForeground(new Color(0, 200, 0));
        consoleArea.setFont(new Font("Monospaced", Font.PLAIN, 12));

        DefaultCaret caret = (DefaultCaret) consoleArea.getCaret();
        caret.setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);

        JScrollPane scrollPane = new JScrollPane(consoleArea);
        scrollPane.setBorder(BorderFactory.createTitledBorder("Live Logs"));

        consolePanel.add(consoleHeader, BorderLayout.NORTH);
        consolePanel.add(scrollPane, BorderLayout.CENTER);

        //redirect system.out to here
        redirectSystemStreams();

        tabbedPane.addTab("Server Control", serverPanel);
        tabbedPane.addTab("Console Output", consolePanel);

        add(tabbedPane);

        //button logic
        startButton.addActionListener(e -> startServer());
        stopButton.addActionListener(e -> stopServer());
    }

    private void startServer()
    {
        try
        {
            long seed;
            try
            {
                seed = Long.parseLong(seedField.getText());
            }
            catch (NumberFormatException e)
            {
                seed = seedField.getText().hashCode();
            }

            int playerLimit = (int) playerLimitSpinner.getValue();

            TerrainConfig.setSeed(seed);

            serverInstance = new Server(25565, playerLimit);
            Thread serverThread = new Thread(() ->
            {
                try
                {
                    serverInstance.start();
                } catch (Exception ex)
                {
                    System.err.println("[GUI] Server stopped: " + ex.getMessage());
                    SwingUtilities.invokeLater(this::onServerStoppedUI);
                }
            });

            serverThread.start();
            onServerStartedUI();
        }
        catch (Exception ex)
        {
            JOptionPane.showMessageDialog(this, "Failed to start: " + ex.getMessage());
        }
    }

    private void stopServer()
    {
        if (serverInstance != null)
        {
            serverInstance.stop();
            onServerStoppedUI();
        }
    }

    private void onServerStartedUI()
    {
        startButton.setEnabled(false);
        stopButton.setEnabled(true);
        seedField.setEnabled(false);
        playerLimitSpinner.setEnabled(false);
        statusLabel.setText("Online");
        statusLabel.setForeground(new Color(0, 150, 0));
    }

    private void onServerStoppedUI()
    {
        startButton.setEnabled(true);
        stopButton.setEnabled(false);
        seedField.setEnabled(true);
        playerLimitSpinner.setEnabled(true);
        statusLabel.setText("Offline");
        statusLabel.setForeground(Color.RED);
    }

    private void redirectSystemStreams()
    {
        OutputStream out = new OutputStream()
        {
            @Override
            public void write(int b)
            {
                updateConsole(String.valueOf((char) b));
            }

            @Override
            public void write(byte[] b, int off, int len) {
                updateConsole(new String(b, off, len));
            }
        };
        System.setOut(new PrintStream(out, true));
        System.setErr(new PrintStream(out, true));
    }

    private void updateConsole(String text) {
        SwingUtilities.invokeLater(() -> consoleArea.append(text));
    }

    public static void main(String[] args)
    {
        SwingUtilities.invokeLater(() -> {
            new ServerGUI().setVisible(true);
        });
    }
}