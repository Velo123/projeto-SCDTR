
duty=gain_test1.pwm;
lux=gain_test1.lux;

colors = lines(10);

% Regressão linear com todos os dados
p1 = polyfit(duty, lux, 1);
lux_fit1 = polyval(p1, duty);
mse1 = mean((lux - lux_fit1).^2);

% Regressão linear apenas para duty >= 0.2
idx = duty >= 0.2;
p2 = polyfit(duty(idx), lux(idx), 1);
lux_fit2 = polyval(p2, duty);
mse2 = mean((lux(idx) - polyval(p2, duty(idx))).^2);

% Gráfico
figure
plot(duty, lux, '.','Color',colors(1,:), 'MarkerSize', 12)
hold on
plot(duty, lux_fit1,'Color',colors(2,:),  'LineWidth', 2)
plot(duty(21:101), lux_fit2(21:101),'Color',colors(5,:), 'LineWidth', 2)

grid on

xlabel('Duty Cycle','FontSize',14)
ylabel('Lux','FontSize',14)

title('PWM vs LUX','FontSize',16)

legend('Dados','Ajuste Linear (todos os dados)','Ajuste Linear (duty ≥ 0.2)','FontSize',12,'Location','northwest')

set(gca,'FontSize',12)

% Texto com equações
txt = sprintf(['Todos os dados: Lux = %.3f Duty + %.3f\nErro quadrático = %.3f\n\n' ...
               'Duty ≥ 0.2: Lux = %.3f Duty + %.3f\nErro quadrático = %.3f'], ...
               p1(1), p1(2), mse1, p2(1), p2(2), mse2);

text(0.05, max(lux)*0.65, txt, 'FontSize',12,'BackgroundColor','white')