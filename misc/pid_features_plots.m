% Carregar os dados com preservação dos nomes das colunas
opts = detectImportOptions('PID_test.csv');
opts.VariableNamingRule = 'preserve';
data = readtable('PID_test.csv', opts);

% Extração de variáveis (ajustado para nomes exatos)
t = data.time;
lux_meas = data.lux_meas;
lux_ref = data.lux_ref;
duty = data.duty_cycle;
colors=lines(5);

% Definição dos intervalos e títulos solicitados
indices = {2963:3562, 5618:6217, 10376:11276, 13771:14571};
titulos = {'Sem Set-point Weighting', 'Com Set-point Weighting', ...
           'Com Anti-Windup', 'Sem Anti-Windup'};

% --- FIGURA 1: QUATRO GRÁFICOS DE ZOOM ---
% ... (código anterior para carregar dados e definir índices/títulos)

figure('Name', 'Análise de Regimes PID', 'Color', [1 1 1]);

for i = 1:4
    subplot(2,2,i);
    idx = indices{i};
    
    t0 = t(idx(1)); % tempo inicial para este intervalo
    t_plot = (t(idx) - t0)/1000;  % eixo x começando em 0
    
    % Eixo da Esquerda
    yyaxis left
    plot(t_plot, lux_meas(idx), 'Color',colors(1,:), 'LineWidth', 1.2); hold on;
    plot(t_plot, lux_ref(idx), 'Color',colors(2,:), 'LineWidth', 1.2);
    ylabel('Lux');
    xlabel('Time (s)')
    ylim([19, 36]);

    % Eixo da Direita
    yyaxis right
    plot(t_plot, duty(idx), 'Color',colors(3,:), 'LineWidth', 1.5);
    ylabel('Duty Cycle (0-1)');
    xlabel('Time (s)')
    ylim([-0.1, 1.1]);
    
    % Adicionar título
    title(titulos{i});
    
    % Configurar cores dos eixos
    ax = gca;
    ax.YAxis(1).Color = 'k'; % eixo esquerdo
    ax.YAxis(2).Color = colors(3,:); % eixo direito
    
    legend({'Lux Medido', 'Referência', 'Duty Cycle'}, 'Location', 'best', 'FontSize', 8);
    grid on;
end